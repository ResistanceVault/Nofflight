//#include "../include/resistancelogo.h"

#include <ace/managers/game.h>					// For using gameClose
#include <ace/managers/system.h>				// For systemUnuse and systemUse
#include <ace/managers/viewport/simplebuffer.h> // Simple buffer
#include <ace/utils/palette.h>
#include <ace/managers/blit.h>
#include <ace/managers/key.h> // Keyboard processing
#include "../include/mivampirademocolors.h"
#include "../include/mivampira_zoomoutplt.h"
#include "../include/main.h"
#include "vectors.h"

inline void v2d_zero(v2d *dest)
{
	dest->x = 0;
	dest->y = 0;
}

typedef struct TCurtain
{
	UBYTE ubPosition; // Position of the courtain inside the mask list
	UBYTE ubOffset;	  // where in the bitplanes print the courtain (in bytes from 0 to 40)
	UBYTE ubIncrementer;
	UBYTE ubSkipFramesCounter;
	v2d tLocation;
	v2d tVelocity;
	v2d tAccelleration;
	UBYTE ubCountBouncer;
	UBYTE *s_pSource;
} TCurtain;

#define MAXCOURTAINS 4
static TCurtain s_pCurtains[MAXCOURTAINS];

#define DELAYTIME 30
#define GRAVITY 900
#define ANIMATIONWAIT 100

static tVPort *s_pVpMain; // Viewport for playfield
static tSimpleBufferManager *s_pMainBuffer;

static tView *s_pView; // View containing all the viewports

static tVPort *s_pVpMain; // Viewport for playfield
static tSimpleBufferManager *s_pMainBuffer;
static UBYTE *pBuffer;

static UBYTE *s_pMask = NULL;
static v2d g_Gravity;

void createMask();
void blitBlack(UWORD);
static UBYTE reverse(UBYTE);
static void clearHr(UWORD);
static void clearBpl();
static void copyToMainBplZoom();

static UBYTE *imgPointer;
static UBYTE *imgPointerStart;
static UWORD *pltPointer;

inline static void courtain_init(UBYTE ubIndex, UBYTE ubAlloc, UBYTE ubOffset, UBYTE ubSkipFramesCounter)
{
	s_pCurtains[ubIndex].ubOffset = ubOffset;
	s_pCurtains[ubIndex].ubPosition = 0;
	s_pCurtains[ubIndex].ubIncrementer = 1;
	s_pCurtains[ubIndex].ubSkipFramesCounter = ubSkipFramesCounter;
	if (ubAlloc)
		s_pCurtains[ubIndex].s_pSource = AllocMem(10, MEMF_CHIP);
	s_pCurtains[ubIndex].ubCountBouncer = 0;

	v2d_zero(&s_pCurtains[ubIndex].tLocation);
	v2d_zero(&s_pCurtains[ubIndex].tVelocity);
	v2d_zero(&s_pCurtains[ubIndex].tAccelleration);
}

void mivampiraLogoGsCreate(void)
{
	pBuffer = g_pBuffer;
	//LoadRes(38400, "data/mivampirademo.raw");
	if (pBuffer == NULL)
		gameExit();

	s_pView = viewCreate(0,
						 TAG_VIEW_GLOBAL_CLUT, 1, // Same Color LookUp Table for all viewports
						 TAG_END);				  // Must always end with TAG_END or synonym: TAG_DONE

	s_pVpMain = vPortCreate(0,
							TAG_VPORT_VIEW, s_pView,
							TAG_VPORT_BPP, 5,
							TAG_END);

	s_pMainBuffer = simpleBufferCreate(0,
									   TAG_SIMPLEBUFFER_VPORT, s_pVpMain, // Required: parent viewport
									   TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR,
									   TAG_END);

	UBYTE ubCount = 0;
	UWORD *p_uwPalette = (UWORD *)mivampirademocolors_data;
	for (ubCount = 0; ubCount < 16; ubCount++)
	{
		UWORD newColor = *p_uwPalette;

		s_pVpMain->pPalette[ubCount] = newColor;
		p_uwPalette++;
	}

	for (UBYTE ubCount = 16; ubCount < 32; ubCount++)
		s_pVpMain->pPalette[ubCount] = 0x0001;

	// Create mask list
	createMask();

	// Create curtains
	courtain_init(0, 1, 0, 1 * DELAYTIME);
	courtain_init(1, 1, 10, 2 * DELAYTIME);
	courtain_init(2, 1, 20, 3 * DELAYTIME);
	courtain_init(3, 1, 30, 4 * DELAYTIME);

	// Gravity setup
	g_Gravity.y = 0; //fix16_div(fix16_from_int(1), fix16_from_int(1000));
	g_Gravity.x = fix16_div(fix16_from_int(1), fix16_from_int(GRAVITY));

	// We don't need anything from OS anymore
	//systemUnuse();

	memset((UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[4]), 0xFF, 40 * 256);

	// Load the view
	viewLoad(s_pView);
}

//char buf[51200];

void mivampiraLogoGsLoop(void)
{
	static int iFrameNo = 0;
	/*static BYTE bDimCounter = 0;
	static BYTE bDimCounter2 = 15;*/

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

		unLoadRes();
		LoadRes(51200, "data/mivampira_zoomout.raw");

		pltPointer = (UWORD *)mivampira_zoomoutplt_data;
		imgPointer = (UBYTE *)g_pBuffer;
		imgPointerStart = imgPointer;
	}
	else if (copy == 3)
	{
		for (UBYTE ubCourtainCounter = 0; ubCourtainCounter < MAXCOURTAINS; ubCourtainCounter++)
		{
			if (s_pCurtains[ubCourtainCounter].ubSkipFramesCounter > 0)
			{
				s_pCurtains[ubCourtainCounter].ubSkipFramesCounter--;
			}
			else
			{
				blitBlack(ubCourtainCounter);
				if (s_pCurtains[ubCourtainCounter].ubCountBouncer < 10)
				{
					v2d_add(&s_pCurtains[ubCourtainCounter].tAccelleration, &s_pCurtains[ubCourtainCounter].tAccelleration, &g_Gravity);
					v2d_add(&s_pCurtains[ubCourtainCounter].tVelocity, &s_pCurtains[ubCourtainCounter].tVelocity, &s_pCurtains[ubCourtainCounter].tAccelleration);
					v2d_add(&s_pCurtains[ubCourtainCounter].tLocation, &s_pCurtains[ubCourtainCounter].tLocation, &s_pCurtains[ubCourtainCounter].tVelocity);
					s_pCurtains[ubCourtainCounter].ubPosition = (UBYTE)fix16_to_int(s_pCurtains[ubCourtainCounter].tLocation.x);
					if (s_pCurtains[ubCourtainCounter].ubPosition >= 38)
					{
						s_pCurtains[ubCourtainCounter].tVelocity.x = fix16_mul(s_pCurtains[ubCourtainCounter].tVelocity.x, fix16_from_int(-1));
						v2d_add(&s_pCurtains[ubCourtainCounter].tAccelleration, &s_pCurtains[ubCourtainCounter].tAccelleration, &g_Gravity);
						v2d_add(&s_pCurtains[ubCourtainCounter].tVelocity, &s_pCurtains[ubCourtainCounter].tVelocity, &s_pCurtains[ubCourtainCounter].tAccelleration);
						v2d_add(&s_pCurtains[ubCourtainCounter].tLocation, &s_pCurtains[ubCourtainCounter].tLocation, &s_pCurtains[ubCourtainCounter].tVelocity);
						s_pCurtains[ubCourtainCounter].ubPosition = (UBYTE)fix16_to_int(s_pCurtains[ubCourtainCounter].tLocation.x);
						s_pCurtains[ubCourtainCounter].ubCountBouncer++;
					}
				}

				// End of transition here
				else if (ubCourtainCounter == 3)
				{
					copy = 4;
				}
			}
		}
	}

#if 0
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
#endif
	if (copy > 3)
		iFrameNo++;

	if (iFrameNo > 300)
	{
#ifdef DISSOLVENZA
		if (bDimCounter2 >= 0)
		{

			UBYTE ubCount = 0;
			UWORD *p_uwPalette = (UWORD *)mivampirademocolors_data;
			for (ubCount = 0; ubCount < 16; ubCount++)
			{
				UWORD newColor = paletteColorInc(*p_uwPalette, bDimCounter2);

				s_pVpMain->pPalette[ubCount] = newColor;
				p_uwPalette++;
			}

			if ((iFrameNo % 10) == 0)
				bDimCounter2--;
		}

//gameExit();return ;
#else
		static ULONG x = 0;
		static ULONG y = 0;
		static UBYTE ubFinish = 0;
		//static UBYTE ubRotoZoom = 0;
		static UWORD uwAniWait = ANIMATIONWAIT;

		if (ubFinish && uwAniWait)
			uwAniWait--;

		else if (ubFinish)
		{
			if (imgPointer == imgPointerStart)
				clearBpl();

			if (imgPointer - imgPointerStart > 51200 - 2048)
			{
				myChangeState(5);
				return;
			}

			copyToMainBplZoom(imgPointer);
			for (UBYTE ubCount = 0; ubCount < 16; ubCount++)
			{
				if (ubCount > 0)
				{
					UWORD newColor = *pltPointer;
					if (*pltPointer == 0x0666)
					{
						s_pVpMain->pPalette[ubCount] = 0x0001;
					}
					else
					{
						s_pVpMain->pPalette[ubCount] = newColor;
					}
				}
				pltPointer++;
			}
		}

		if (x < 320 / 2 - 32)
		{
			blitLine(s_pMainBuffer->pBack, x, 0, x, 255, 0, 0xFFFF, 31);
			blitLine(s_pMainBuffer->pBack, 319 - x, 0, 319 - x, 255, 0, 0xFFFF, 31);
			x++;
		}
		else
			ubFinish = 1;

		if (y < 256 / 2 - 16)
		{
			clearHr(255 - y);
			clearHr(y);
			y++;
		}
#endif
	}
	/*if (bDimCounter2 < 0)
	{
		myChangeState(5);

		return;
	}*/

	viewUpdateCLUT(s_pView);
	vPortWaitForEnd(s_pVpMain);
}

void mivampiraLogoGsDestroy(void)
{
	//unLoadRes();

	FreeMem(s_pMask, 5 * 40);

	FreeMem(s_pCurtains[0].s_pSource, 10);
	FreeMem(s_pCurtains[1].s_pSource, 10);
	FreeMem(s_pCurtains[2].s_pSource, 10);
	FreeMem(s_pCurtains[3].s_pSource, 10);

	// This will also destroy all associated viewports and viewport managers
	viewDestroy(s_pView);
	unLoadRes();
	ULONG radiallinespositions_size = 192000 / 4;
	LoadRes(radiallinespositions_size, "data/radiallinesallpositions.bin");
}

void blitBlack(UWORD uwCourtainIndex)
{
	UBYTE *pTmp = s_pMask + 5 * s_pCurtains[uwCourtainIndex].ubPosition;

	*(s_pCurtains[uwCourtainIndex].s_pSource + 0) = *pTmp;
	*(s_pCurtains[uwCourtainIndex].s_pSource + 9) = reverse(*pTmp);
	pTmp++;

	*(s_pCurtains[uwCourtainIndex].s_pSource + 1) = *pTmp;
	*(s_pCurtains[uwCourtainIndex].s_pSource + 8) = reverse(*pTmp);
	pTmp++;

	*(s_pCurtains[uwCourtainIndex].s_pSource + 2) = *pTmp;
	*(s_pCurtains[uwCourtainIndex].s_pSource + 7) = reverse(*pTmp);
	pTmp++;

	*(s_pCurtains[uwCourtainIndex].s_pSource + 3) = *pTmp;
	*(s_pCurtains[uwCourtainIndex].s_pSource + 6) = reverse(*pTmp);
	pTmp++;

	*(s_pCurtains[uwCourtainIndex].s_pSource + 4) = *pTmp;
	*(s_pCurtains[uwCourtainIndex].s_pSource + 5) = reverse(*pTmp);

	blitWait();
	g_pCustom->bltcon0 = 0x090F;
	g_pCustom->bltcon1 = 0x0000;
	g_pCustom->bltafwm = 0xffff;
	g_pCustom->bltalwm = 0xffff;
	g_pCustom->bltamod = -10;
	g_pCustom->bltdmod = 30;
#ifdef SHAREDPSOURCE
	g_pCustom->bltapt = (UBYTE *)((ULONG)s_pSource);
#else
	g_pCustom->bltapt = (UBYTE *)((ULONG)s_pCurtains[uwCourtainIndex].s_pSource);
#endif
	g_pCustom->bltdpt = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[4] + s_pCurtains[uwCourtainIndex].ubOffset);
	g_pCustom->bltsize = 0x4005;
}

static UBYTE reverse(UBYTE b)
{
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

void createMask()
{
	s_pMask = AllocMem(5 * 40, MEMF_CHIP | MEMF_CLEAR);
	*s_pMask = 0x80;
	*(s_pMask + 5) = 0xC0;
	*(s_pMask + 10) = 0xE0;
	*(s_pMask + 15) = 0xF0;
	*(s_pMask + 20) = 0xF8;
	*(s_pMask + 25) = 0xFC;
	*(s_pMask + 30) = 0xFE;
	*(s_pMask + 35) = 0xFF;

	*(s_pMask + 40) = 0xFF;
	*(s_pMask + 41) = 0x80;
	*(s_pMask + 45) = 0xFF;
	*(s_pMask + 46) = 0xC0;
	*(s_pMask + 50) = 0xFF;
	*(s_pMask + 51) = 0xE0;
	*(s_pMask + 55) = 0xFF;
	*(s_pMask + 56) = 0xF0;
	*(s_pMask + 60) = 0xFF;
	*(s_pMask + 61) = 0xF8;
	*(s_pMask + 65) = 0xFF;
	*(s_pMask + 66) = 0xFC;
	*(s_pMask + 70) = 0xFF;
	*(s_pMask + 71) = 0xFE;
	*(s_pMask + 75) = 0xFF;
	*(s_pMask + 76) = 0xFF;

	*(s_pMask + 80) = 0xFF;
	*(s_pMask + 81) = 0xFF;
	*(s_pMask + 82) = 0x80;
	*(s_pMask + 85) = 0xFF;
	*(s_pMask + 86) = 0xFF;
	*(s_pMask + 87) = 0xC0;
	*(s_pMask + 90) = 0xFF;
	*(s_pMask + 91) = 0xFF;
	*(s_pMask + 92) = 0xE0;
	*(s_pMask + 95) = 0xFF;
	*(s_pMask + 96) = 0xFF;
	*(s_pMask + 97) = 0xF0;
	*(s_pMask + 100) = 0xFF;
	*(s_pMask + 101) = 0xFF;
	*(s_pMask + 102) = 0xF8;
	*(s_pMask + 105) = 0xFF;
	*(s_pMask + 106) = 0xFF;
	*(s_pMask + 107) = 0xFC;
	*(s_pMask + 110) = 0xFF;
	*(s_pMask + 111) = 0xFF;
	*(s_pMask + 112) = 0xFE;
	*(s_pMask + 115) = 0xFF;
	*(s_pMask + 116) = 0xFF;
	*(s_pMask + 117) = 0xFF;

	*(s_pMask + 115) = 0xFF;
	*(s_pMask + 116) = 0xFF;
	*(s_pMask + 117) = 0xFF;
	*(s_pMask + 118) = 0x80;

	*(s_pMask + 120) = 0xFF;
	*(s_pMask + 121) = 0xFF;
	*(s_pMask + 122) = 0xFF;
	*(s_pMask + 123) = 0xC0;

	*(s_pMask + 125) = 0xFF;
	*(s_pMask + 126) = 0xFF;
	*(s_pMask + 127) = 0xFF;
	*(s_pMask + 128) = 0xE0;

	*(s_pMask + 130) = 0xFF;
	*(s_pMask + 131) = 0xFF;
	*(s_pMask + 132) = 0xFF;
	*(s_pMask + 133) = 0xF0;

	*(s_pMask + 135) = 0xFF;
	*(s_pMask + 136) = 0xFF;
	*(s_pMask + 137) = 0xFF;
	*(s_pMask + 138) = 0xF8;

	*(s_pMask + 140) = 0xFF;
	*(s_pMask + 141) = 0xFF;
	*(s_pMask + 142) = 0xFF;
	*(s_pMask + 143) = 0xFC;

	*(s_pMask + 145) = 0xFF;
	*(s_pMask + 146) = 0xFF;
	*(s_pMask + 147) = 0xFF;
	*(s_pMask + 148) = 0xFE;

	*(s_pMask + 150) = 0xFF;
	*(s_pMask + 151) = 0xFF;
	*(s_pMask + 152) = 0xFF;
	*(s_pMask + 153) = 0xFF;

	*(s_pMask + 155) = 0xFF;
	*(s_pMask + 156) = 0xFF;
	*(s_pMask + 157) = 0xFF;
	*(s_pMask + 158) = 0xFF;
	*(s_pMask + 159) = 0x80;

	*(s_pMask + 160) = 0xFF;
	*(s_pMask + 161) = 0xFF;
	*(s_pMask + 162) = 0xFF;
	*(s_pMask + 163) = 0xFF;
	*(s_pMask + 164) = 0xC0;

	*(s_pMask + 165) = 0xFF;
	*(s_pMask + 166) = 0xFF;
	*(s_pMask + 167) = 0xFF;
	*(s_pMask + 168) = 0xFF;
	*(s_pMask + 169) = 0xE0;

	*(s_pMask + 170) = 0xFF;
	*(s_pMask + 171) = 0xFF;
	*(s_pMask + 172) = 0xFF;
	*(s_pMask + 173) = 0xFF;
	*(s_pMask + 174) = 0xF0;

	*(s_pMask + 175) = 0xFF;
	*(s_pMask + 176) = 0xFF;
	*(s_pMask + 177) = 0xFF;
	*(s_pMask + 178) = 0xFF;
	*(s_pMask + 179) = 0xF8;

	*(s_pMask + 180) = 0xFF;
	*(s_pMask + 181) = 0xFF;
	*(s_pMask + 182) = 0xFF;
	*(s_pMask + 183) = 0xFF;
	*(s_pMask + 184) = 0xFC;

	*(s_pMask + 185) = 0xFF;
	*(s_pMask + 186) = 0xFF;
	*(s_pMask + 187) = 0xFF;
	*(s_pMask + 188) = 0xFF;
	*(s_pMask + 189) = 0xFE;

	*(s_pMask + 190) = 0xFF;
	*(s_pMask + 191) = 0xFF;
	*(s_pMask + 192) = 0xFF;
	*(s_pMask + 193) = 0xFF;
	*(s_pMask + 194) = 0xFF;
}

static void clearHr(UWORD uwRow)
{
	ULONG *pRow = (ULONG *)((ULONG)s_pMainBuffer->pBack->Planes[4] + 40 * uwRow);
	for (UBYTE ubCount = 0; ubCount < 10; ubCount++)
	{
		*pRow = 0xFFFFFFFF;
		pRow++;
	}
	return;
}

static void clearBpl()
{
	UBYTE ubBitplaneCounter;
	for (ubBitplaneCounter = 0; ubBitplaneCounter < 5; ubBitplaneCounter++)
	{
		blitWait();
		g_pCustom->bltcon0 = 0x0100;
		g_pCustom->bltcon1 = 0x0000;
		g_pCustom->bltafwm = 0xFFFF;
		g_pCustom->bltalwm = 0xFFFF;
		g_pCustom->bltdmod = 32;
		g_pCustom->bltdpt = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[ubBitplaneCounter] + 16);
		g_pCustom->bltsize = 0x4004;
	}
	return;
}

static void copyToMainBplZoom()
{
	UBYTE ubBitplaneCounter;
	for (ubBitplaneCounter = 0; ubBitplaneCounter < 4; ubBitplaneCounter++)
	{
		blitWait();
		g_pCustom->bltcon0 = 0x09F0;
		g_pCustom->bltcon1 = 0x0000;
		g_pCustom->bltafwm = 0xFFFF;
		g_pCustom->bltalwm = 0xFFFF;
		g_pCustom->bltamod = 0x0000;
		//g_pCustom->bltbmod = 0x0000;
		//g_pCustom->bltcmod = 0x0000;
		g_pCustom->bltdmod = 32;
		g_pCustom->bltapt = (UBYTE *)((ULONG)imgPointer);
		g_pCustom->bltdpt = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[ubBitplaneCounter] + 16 + 40 * 96);
		g_pCustom->bltsize = 0x1004;
		imgPointer += 512;
	}
	return;
}