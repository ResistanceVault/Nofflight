/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "simplebuffertest.h"
#include <ace/utils/tag.h>
#include <ace/utils/extview.h>

#ifdef AMIGA

// Flags for internal usage.
#define SIMPLEBUFFERTEST_FLAG_X_SCROLLABLE 1
#define SIMPLEBUFFERTEST_FLAG_COPLIST_RAW  2

static void simpleBufferTestSetBack(tSimpleBufferTestManager *pManager, tBitMap *pBack) {
#if defined(ACE_DEBUG)
	if(pManager->pBack && pManager->pBack->Depth != pBack->Depth) {
		logWrite("ERR: buffer bitmaps differ in BPP!\n");
		return;
	}
#endif
	pManager->pBack = pBack;
}

static void simpleBufferTestSetFront(
	tSimpleBufferTestManager *pManager, tBitMap *pFront, UBYTE isScrollX
) {
	logBlockBegin(
		"simpleBufferTestSetFront(pManager: %p, pFront: %p, isScrollX: %hhu)",
		pManager, pFront, isScrollX
	);
#if defined(ACE_DEBUG)
	if(pManager->pFront && pManager->pFront->Depth != pFront->Depth) {
		logWrite("ERR: buffer bitmaps differ in BPP!\n");
		return;
	}
#endif

	pManager->uBfrBounds.uwX = bitmapGetByteWidth(pFront) << 3;
	pManager->uBfrBounds.uwY = pFront->Rows;
	pManager->pFront = pFront;
	UWORD uwModulo = pFront->BytesPerRow - (pManager->sCommon.pVPort->uwWidth >> 3);
	UWORD uwDDfStrt;
	if(
		!isScrollX || pManager->uBfrBounds.uwX <= pManager->sCommon.pVPort->uwWidth
	) {
		uwDDfStrt = 0x0038;
		pManager->ubFlags &= ~SIMPLEBUFFERTEST_FLAG_X_SCROLLABLE;
	}
	else {
		pManager->ubFlags |= SIMPLEBUFFERTEST_FLAG_X_SCROLLABLE;
		uwDDfStrt = 0x0030;
		uwModulo -= 2;
	}
	logWrite("Modulo: %u\n", uwModulo);

	// Update (rewrite) copperlist
	// TODO this could be unified with copBlock being set with copSetMove too
	tCopList *pCopList = pManager->sCommon.pVPort->pView->pCopList;
	if(pManager->ubFlags & SIMPLEBUFFERTEST_FLAG_COPLIST_RAW) {
		// Since simpleBufferProcess only updates bitplane ptrs and shift,
		// copperlist must be shaped here.
		// WAIT is calc'd in same way as in copBlockCreate in simpleBufferCreate().
		tCopCmd *pCmdList = &pCopList->pBackBfr->pList[pManager->uwCopperOffset];
		/*logWrite(
			"Setting copperlist %p at offs %u\n",
			pCopList->pBackBfr, pManager->uwCopperOffset
		);*/
		//copSetWait(&pCmdList[0].sWait, 0xE2-7*4, pManager->sCommon.pVPort->uwOffsY + 0x2C-2);
		copSetMove(&pCmdList[0].sMove, &g_pCustom->bplcon2, 0x0024);
		copSetMove(&pCmdList[1].sMove, &g_pCustom->ddfstop, 0x00D0);    // Data fetch
		copSetMove(&pCmdList[2].sMove, &g_pCustom->ddfstrt, uwDDfStrt);
		copSetMove(&pCmdList[3].sMove, &g_pCustom->bpl1mod, uwModulo);  // Bitplane modulo
		copSetMove(&pCmdList[4].sMove, &g_pCustom->bpl2mod, uwModulo);
		copSetMove(&pCmdList[5].sMove, &g_pCustom->bplcon1, 0);         // Shift: 0
		UBYTE i;
		ULONG ulPlaneAddr;
		for (i = 0; i < pManager->sCommon.pVPort->ubBPP; ++i) {
			ulPlaneAddr = (ULONG)pManager->pFront->Planes[i];
			copSetMove(&pCmdList[6 + i*2 + 0].sMove, &g_pBplFetch[i].uwHi, ulPlaneAddr >> 16);
			copSetMove(&pCmdList[6 + i*2 + 1].sMove, &g_pBplFetch[i].uwLo, ulPlaneAddr & 0xFFFF);
		}
		// Copy to front buffer since it needs initialization there too
		CopyMem(
			&pCopList->pBackBfr->pList[pManager->uwCopperOffset],
			&pCopList->pFrontBfr->pList[pManager->uwCopperOffset],
			(6+2*pManager->sCommon.pVPort->ubBPP)*sizeof(tCopCmd)
		);
	}
	else {
		tCopBlock *pBlock = pManager->pCopBlock;
		pBlock->uwCurrCount = 0; // Rewind to beginning
		copMove(pCopList, pBlock, &g_pCustom->ddfstop, 0x00D0);     // Data fetch
		copMove(pCopList, pBlock, &g_pCustom->ddfstrt, uwDDfStrt);
		copMove(pCopList, pBlock, &g_pCustom->bpl1mod, uwModulo);   // Bitplane modulo
		copMove(pCopList, pBlock, &g_pCustom->bpl2mod, uwModulo);
		copMove(pCopList, pBlock, &g_pCustom->bplcon1, 0);          // Shift: 0
		for (UBYTE i = 0; i < pManager->sCommon.pVPort->ubBPP; ++i) {
			ULONG ulPlaneAddr = (ULONG)pManager->pFront->Planes[i];
			copMove(pCopList, pBlock, &g_pBplFetch[i].uwHi, ulPlaneAddr >> 16);
			copMove(pCopList, pBlock, &g_pBplFetch[i].uwLo, ulPlaneAddr & 0xFFFF);
		}
	}
	logBlockEnd("simplebufferTestSetFront()");
}

tSimpleBufferTestManager *SimpleBufferTestCreate(void *pTags, ...) {
	va_list vaTags;
	tCopList *pCopList;
	tSimpleBufferTestManager *pManager;
	UWORD uwBoundWidth, uwBoundHeight;
	UBYTE ubBitmapFlags;
	tBitMap *pFront = 0, *pBack = 0;
	UBYTE isCameraCreated = 0;

	logBlockBegin("simpleBufferTestCreate(pTags: %p, ...)", pTags);
	va_start(vaTags, pTags);

	// Init manager
	pManager = memAllocFastClear(sizeof(tSimpleBufferTestManager));
	pManager->sCommon.process = (tVpManagerFn)SimpleBufferTestProcess;
	pManager->sCommon.destroy = (tVpManagerFn)SimpleBufferTestDestroy;
	pManager->sCommon.ubId = VPM_SCROLL;
	logWrite("Addr: %p\n", pManager);

	tVPort *pVPort = (tVPort*)tagGet(pTags, vaTags, TAG_SIMPLEBUFFERTEST_VPORT, 0);
	if(!pVPort) {
		logWrite("ERR: No parent viewport (TAG_SIMPLEBUFFERTEST_VPORT) specified!\n");
		goto fail;
	}
	pManager->sCommon.pVPort = pVPort;
	logWrite("Parent VPort: %p\n", pVPort);

	// Buffer bitmap
	uwBoundWidth = tagGet(
		pTags, vaTags, TAG_SIMPLEBUFFERTEST_BOUND_WIDTH, pVPort->uwWidth
	);
	uwBoundHeight = tagGet(
		pTags, vaTags, TAG_SIMPLEBUFFERTEST_BOUND_HEIGHT, pVPort->uwHeight
	);
	ubBitmapFlags = tagGet(
		pTags, vaTags, TAG_SIMPLEBUFFERTEST_BITMAP_FLAGS, BMF_CLEAR
	);
	logWrite("Bounds: %ux%u\n", uwBoundWidth, uwBoundHeight);
	pFront = bitmapCreate(
		uwBoundWidth, uwBoundHeight, pVPort->ubBPP, ubBitmapFlags
	);
	if(!pFront) {
		logWrite("ERR: Can't alloc buffer bitmap!\n");
		goto fail;
	}

	UBYTE isDblBfr = tagGet(pTags, vaTags, TAG_SIMPLEBUFFERTEST_IS_DBLBUF, 0);
	if(isDblBfr) {
		pBack = bitmapCreate(
			uwBoundWidth, uwBoundHeight, pVPort->ubBPP, ubBitmapFlags
		);
		if(!pBack) {
			logWrite("ERR: Can't alloc buffer bitmap!\n");
			goto fail;
		}
	}

	// Find camera manager, create if not exists
	pManager->pCamera = (tCameraManager*)vPortGetManager(pVPort, VPM_CAMERA);
	if(!pManager->pCamera) {
		isCameraCreated = 1;
		pManager->pCamera = cameraCreate(
			pVPort, 0, 0, uwBoundWidth, uwBoundHeight, isDblBfr
		);
	}

	pCopList = pVPort->pView->pCopList;
	if(pCopList->ubMode == COPPER_MODE_BLOCK) {
		// CopBlock contains: bitplanes + shiftX
		pManager->pCopBlock = copBlockCreate(
			// WAIT is already in copBlock so 1 instruction less
			pCopList, SimpleBufferTestGetRawCopperlistInstructionCount(pVPort->ubBPP) - 1,
			// Vertically addition from DiWStrt, horizontally a bit before last fetch.
			// First to set are ddf, modulos & shift so they are changed during fetch.
			0xE2-7*4, pVPort->uwOffsY + 0x2C-1
		);
	}
	else {
		const UWORD uwInvalidCopOffs = -1;
		pManager->ubFlags |= SIMPLEBUFFERTEST_FLAG_COPLIST_RAW;
		pManager->uwCopperOffset = tagGet(
			pTags, vaTags, TAG_SIMPLEBUFFERTEST_COPLIST_OFFSET, uwInvalidCopOffs
		);
		if(pManager->uwCopperOffset == uwInvalidCopOffs) {
			logWrite(
				"ERR: Copperlist offset (TAG_SIMPLEBUFFERTEST_COPLIST_OFFSET) not specified!\n"
			);
			goto fail;
		}
		logWrite("Copperlist offset: %u\n", pManager->uwCopperOffset);
	}

	UBYTE isScrollX = tagGet(pTags, vaTags, TAG_SIMPLEBUFFERTEST_USE_X_SCROLLING, 1);
	simpleBufferTestSetFront(pManager, pFront, isScrollX);
	simpleBufferTestSetBack(pManager, pBack ? pBack : pFront);

	// Add manager to VPort
	vPortAddManager(pVPort, (tVpManager*)pManager);
	logBlockEnd("simpleBufferTestCreate()");
	va_end(vaTags);
	return pManager;

fail:
	if(pBack && pBack != pFront) {
		bitmapDestroy(pBack);
	}
	if(pFront) {
		bitmapDestroy(pFront);
	}
	if(pManager) {
		if(pManager->pCamera && isCameraCreated) {
			cameraDestroy(pManager->pCamera);
		}
		memFree(pManager, sizeof(tSimpleBufferTestManager));
	}
	logBlockEnd("simpleBufferTestCreate()");
	va_end(vaTags);
	return 0;
}

void SimpleBufferTestDestroy(tSimpleBufferTestManager *pManager) {
	logBlockBegin("simpleBufferTestDestroy()");
	if(!(pManager->ubFlags & SIMPLEBUFFERTEST_FLAG_COPLIST_RAW)) {
		copBlockDestroy(
			pManager->sCommon.pVPort->pView->pCopList, pManager->pCopBlock
		);
	}
	if(pManager->pBack != pManager->pFront) {
		bitmapDestroy(pManager->pBack);
	}
	bitmapDestroy(pManager->pFront);
	memFree(pManager, sizeof(tSimpleBufferTestManager));
	logBlockEnd("simpleBufferTestDestroy()");
}

void SimpleBufferTestProcess(tSimpleBufferTestManager *pManager) {
	UWORD uwShift;
	ULONG ulBplOffs;
	ULONG ulPlaneAddr;

	const tCameraManager *pCamera = pManager->pCamera;
	tCopList *pCopList = pManager->sCommon.pVPort->pView->pCopList;

	// Calculate X movement: bitplane shift, starting word to fetch
	if(pManager->ubFlags & SIMPLEBUFFERTEST_FLAG_X_SCROLLABLE) {
        logWrite("è scrollabile\n");
		uwShift = (16 - (pCamera->uPos.uwX & 0xF)) & 0xF;  // Bitplane shift - single
		uwShift = (uwShift << 4) | uwShift;                // Bitplane shift - PF1 | PF2
		logWrite("pCamera->uPos.uwX alessio:%u\n",pCamera->uPos.uwX);
		ulBplOffs = ((pCamera->uPos.uwX - 1) >> 4) << 1;   // Must be ULONG!
	}
	else {
        logWrite("NON è scrollabile\n");
		uwShift = 0;
		ulBplOffs = (pCamera->uPos.uwX >> 4) << 1;
	}

	// Calculate Y movement
	ulBplOffs += pManager->pBack->BytesPerRow * pCamera->uPos.uwY;
	logWrite("ulBplOffs alessio:%d %u %u\n",ulBplOffs,pManager->pBack->BytesPerRow , pCamera->uPos.uwY);

	// Copperlist - regen bitplane ptrs, update shift
	// TODO could be unified by using copSetMove in copBlock
	if(pManager->ubFlags & SIMPLEBUFFERTEST_FLAG_COPLIST_RAW) {
		tCopCmd *pCmdList = &pCopList->pBackBfr->pList[pManager->uwCopperOffset];
		copSetMove(&pCmdList[5].sMove, &g_pCustom->bplcon1, uwShift);
        		logWrite("bplcon 1 alessio:%u\n",uwShift);

                

		for(UBYTE i = 0; i < pManager->sCommon.pVPort->ubBPP; ++i) {
			ulPlaneAddr = ((ULONG)pManager->pBack->Planes[i]) + ulBplOffs;
            logWrite("bitplanes alessio:%x\n",ulPlaneAddr);
			copSetMove(&pCmdList[6 + i*2 + 0].sMove, &g_pBplFetch[i].uwHi, ulPlaneAddr >> 16);
			copSetMove(&pCmdList[6 + i*2 + 1].sMove, &g_pBplFetch[i].uwLo, ulPlaneAddr & 0xFFFF);
		}
	}
	else {
		pManager->pCopBlock->uwCurrCount = 4; // Rewind to shift cmd pos
        copMove(pCopList, pManager->pCopBlock, &g_pCustom->bplcon1, uwShift);
		for(UBYTE i = 0; i < pManager->pBack->Depth; ++i) {
			ulPlaneAddr = ((ULONG)pManager->pBack->Planes[i]) + ulBplOffs;
			copMove(pCopList, pManager->pCopBlock, &g_pBplFetch[i].uwHi, ulPlaneAddr >> 16);
			copMove(pCopList, pManager->pCopBlock, &g_pBplFetch[i].uwLo, ulPlaneAddr & 0xFFFF);
		}
	}

	// Swap buffers if needed
	if(pManager->pBack != pManager->pFront) {
		tBitMap *pTmp = pManager->pBack;
		pManager->pBack = pManager->pFront;
		pManager->pFront = pTmp;
	}
}

UBYTE simpleBufferTestIsRectVisible(
	tSimpleBufferTestManager *pManager,
	UWORD uwX, UWORD uwY, UWORD uwWidth, UWORD uwHeight
) {
	return (
		uwX >= pManager->pCamera->uPos.uwX - uwWidth &&
		uwX <= pManager->pCamera->uPos.uwX + pManager->sCommon.pVPort->uwWidth &&
		uwY >= pManager->pCamera->uPos.uwY - uwHeight &&
		uwY <= pManager->pCamera->uPos.uwY + pManager->sCommon.pVPort->uwHeight
	);
}

UBYTE SimpleBufferTestGetRawCopperlistInstructionCount(UBYTE ubBpp) {
	UBYTE ubInstructionCount = (
		1 +       // WAIT cmd
		2 +       // DDFSTOP / DDFSTART setup
		2 +       // Odd / even modulos
		1 +       // X-shift setup in bplcon
		2 * ubBpp // 2 * 16-bit MOVE for each bitplane
	);
	return ubInstructionCount;
}

#endif // AMIGA