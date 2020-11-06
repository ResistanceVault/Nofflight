//#include "../include/resistancelogo.h"

#include <ace/managers/game.h>					// For using gameClose
#include <ace/managers/system.h>				// For systemUnuse and systemUse
#include <ace/managers/viewport/simplebuffer.h> // Simple buffer
#include <ace/utils/palette.h>
#include <ace/managers/blit.h>
#include <ace/managers/key.h> // Keyboard processing
#include "../include/mivampirademocolors.h"
#include "../include/main.h"

#define copSetMoveBackAndFront(var, var2)                    \
	copSetMove(&pCmdListBack[ubCopIndex].sMove, var, var2);  \
	copSetMove(&pCmdListFront[ubCopIndex].sMove, var, var2); \
	ubCopIndex++;

static tVPort *s_pVpMain; // Viewport for playfield
static tSimpleBufferManager *s_pMainBuffer;

static tView *s_pView; // View containing all the viewports

static tVPort *s_pVpMain; // Viewport for playfield
static tSimpleBufferManager *s_pMainBuffer;
static UBYTE *pBuffer;

void mivampiraLogoGsCreate(void)
{
	pBuffer = g_pBuffer;
	//LoadRes(38400, "data/mivampirademo.raw");
	if (pBuffer == NULL)
		gameExit();

	s_pView = viewCreate(0,
						 TAG_VIEW_GLOBAL_CLUT, 1, // Same Color LookUp Table for all viewports
												  //                     TAG_VIEW_COPLIST_MODE, VIEW_COPLIST_MODE_RAW,
												  //                     TAG_VIEW_COPLIST_RAW_COUNT, ulRawSize,
						 TAG_END);				  // Must always end with TAG_END or synonym: TAG_DONE

	s_pVpMain = vPortCreate(0,
							TAG_VPORT_VIEW, s_pView,
							TAG_VPORT_BPP, 4, // 4 bits per pixel, 16 colors

							TAG_END);
	s_pMainBuffer = simpleBufferCreate(0,
									   TAG_SIMPLEBUFFER_VPORT, s_pVpMain, // Required: parent viewport
									   TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR,
									   //                                  TAG_SIMPLEBUFFER_COPLIST_OFFSET, 0,

									   TAG_END);

	//UWORD s_uwCopRawOffs = SimpleBufferTestGetRawCopperlistInstructionCount(5);

	s_pVpMain->pPalette[0] = 0x0000;
	s_pVpMain->pPalette[1] = 0x0FFF;
	s_pVpMain->pPalette[2] = 0x0DEE;
	s_pVpMain->pPalette[3] = 0x0CDD;
	s_pVpMain->pPalette[4] = 0x0BCC;
	s_pVpMain->pPalette[5] = 0x0ABB;
	s_pVpMain->pPalette[6] = 0x09AA;
	s_pVpMain->pPalette[7] = 0x0899;

	s_pVpMain->pPalette[8] = 0x0788;
	s_pVpMain->pPalette[9] = 0x0677;
	s_pVpMain->pPalette[10] = 0x0566;
	s_pVpMain->pPalette[11] = 0x0455;
	s_pVpMain->pPalette[12] = 0x0344;
	s_pVpMain->pPalette[13] = 0x0233;
	s_pVpMain->pPalette[14] = 0x0122;
	s_pVpMain->pPalette[15] = 0x0011;

	// We don't need anything from OS anymore
	//systemUnuse();

	// Load the view
	viewLoad(s_pView);
}

//char buf[51200];

void mivampiraLogoGsLoop(void)
{
	static int iFrameNo = 0;
	static BYTE bDimCounter = 0;
	static BYTE bDimCounter2 = 15;

	/*if (keyCheck(KEY_ESCAPE))
    {
        gameExit();
    }*/

	static int copy = 1;
	if (copy == 1)
	{

		copy = 2;
		pBuffer = g_pBuffer;
	}
	else if (copy == 2)
	{
		//copyToMainBpl((unsigned char*)buf, 0, 0);
		UBYTE *lol = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[0]);
		memcpy(lol, pBuffer, 9600);
		lol = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[1]);
		memcpy(lol, pBuffer + 9600, 9600);

		lol = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[2]);
		memcpy(lol, pBuffer + 9600 * 2, 9600);

		lol = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[3]);
		memcpy(lol, pBuffer + 9600 * 3, 9600);

		copy = 3;
	}

	if (bDimCounter <= 15)
	{

		/*s_pVpMain->pPalette[0] = paletteColorDim(0x0000, bDimCounter);
		s_pVpMain->pPalette[1] = paletteColorDim(0x0FFF, bDimCounter);
		s_pVpMain->pPalette[2] = paletteColorDim(0x0DEE, bDimCounter);
		s_pVpMain->pPalette[3] = paletteColorDim(0x0CDD, bDimCounter);
		s_pVpMain->pPalette[4] = paletteColorDim(0x0BCC, bDimCounter);
		s_pVpMain->pPalette[5] = paletteColorDim(0x0ABB, bDimCounter);
		s_pVpMain->pPalette[6] = paletteColorDim(0x09AA, bDimCounter);
		s_pVpMain->pPalette[7] = paletteColorDim(0x0899, bDimCounter);

		s_pVpMain->pPalette[8] = paletteColorDim(0x0788, bDimCounter);
		s_pVpMain->pPalette[9] = paletteColorDim(0x0677, bDimCounter);
		s_pVpMain->pPalette[10] = paletteColorDim(0x0566, bDimCounter);
		s_pVpMain->pPalette[11] = paletteColorDim(0x0455, bDimCounter);
		s_pVpMain->pPalette[12] = paletteColorDim(0x0344, bDimCounter);
		s_pVpMain->pPalette[13] = paletteColorDim(0x0233, bDimCounter);
		s_pVpMain->pPalette[14] = paletteColorDim(0x0122, bDimCounter);
		s_pVpMain->pPalette[15] = paletteColorDim(0x0011, bDimCounter);*/

		UBYTE ubCount = 0;
		UWORD *p_uwPalette = (UWORD *)mivampirademocolors_data;
		for (ubCount = 0; ubCount < 16; ubCount++)
		{
			UWORD newColor = paletteColorInc(*p_uwPalette, bDimCounter);

			s_pVpMain->pPalette[ubCount] = newColor;
			p_uwPalette++;
		}

		if ((iFrameNo % 10) == 0)
			bDimCounter++;
	}

	if (iFrameNo > 300)
	{
		if (bDimCounter2 >= 0)
		{

			s_pVpMain->pPalette[0] = paletteColorDim(0x0000, bDimCounter2);
			s_pVpMain->pPalette[1] = paletteColorDim(0x0FFF, bDimCounter2);
			s_pVpMain->pPalette[2] = paletteColorDim(0x0DEE, bDimCounter2);
			s_pVpMain->pPalette[3] = paletteColorDim(0x0CDD, bDimCounter2);
			s_pVpMain->pPalette[4] = paletteColorDim(0x0BCC, bDimCounter2);
			s_pVpMain->pPalette[5] = paletteColorDim(0x0ABB, bDimCounter2);
			s_pVpMain->pPalette[6] = paletteColorDim(0x09AA, bDimCounter2);
			s_pVpMain->pPalette[7] = paletteColorDim(0x0899, bDimCounter2);

			s_pVpMain->pPalette[8] = paletteColorDim(0x0788, bDimCounter2);
			s_pVpMain->pPalette[9] = paletteColorDim(0x0677, bDimCounter2);
			s_pVpMain->pPalette[10] = paletteColorDim(0x0566, bDimCounter2);
			s_pVpMain->pPalette[11] = paletteColorDim(0x0455, bDimCounter2);
			s_pVpMain->pPalette[12] = paletteColorDim(0x0344, bDimCounter2);
			s_pVpMain->pPalette[13] = paletteColorDim(0x0233, bDimCounter2);
			s_pVpMain->pPalette[14] = paletteColorDim(0x0122, bDimCounter2);
			s_pVpMain->pPalette[15] = paletteColorDim(0x0011, bDimCounter2);

			if ((iFrameNo % 10) == 0)
				bDimCounter2--;
		}

		//gameExit();return ;
	}
	if (bDimCounter2 < 0)
	{
		myChangeState(5);
		/*ULONG radiallinespositions_size = 192000 / 4;
		LoadRes(radiallinespositions_size, "data/radiallinesallpositions.bin");*/
		return;
	}
	if (copy >= 3)
		iFrameNo++;
	viewUpdateCLUT(s_pView);
	vPortWaitForEnd(s_pVpMain);
}

void mivampiraLogoGsDestroy(void)
{
	//unLoadRes();

	// This will also destroy all associated viewports and viewport managers
	viewDestroy(s_pView);
unLoadRes();
	ULONG radiallinespositions_size = 192000 / 4;
		LoadRes(radiallinespositions_size, "data/radiallinesallpositions.bin");
}
