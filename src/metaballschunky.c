#include "../include/metaballschunky.h"
#include <ace/managers/key.h>                   // Keyboard processing
#include <ace/managers/game.h>                  // For using gameExit
#include <ace/managers/system.h>                // For systemUnuse and systemUse
#include <ace/managers/viewport/simplebuffer.h> // Simple buffer
#include <ace/managers/blit.h>

#include "../include/colors.h"

#define BITPLANES 5

#define XRES 20
#define YRES 16

//#define COLORDEBUG

#define copSetWaitBackAndFront(var, var2, var3)            \
  copSetWait(&pCmdListBack[ubCopIndex].sWait, var, var2);  \
  copSetWait(&pCmdListFront[ubCopIndex].sWait, var, var2); \
  if (var3)                                                \
    g_sWaitPositions[ubWaitCounter++] = ubCopIndex;        \
  ubCopIndex++;

#define copSetMoveBackAndFront(var, var2)                  \
  copSetMove(&pCmdListBack[ubCopIndex].sMove, var, var2);  \
  copSetMove(&pCmdListFront[ubCopIndex].sMove, var, var2); \
  ubCopIndex++;

static tView *s_pView;    // View containing all the viewports
static tVPort *s_pVpMain; // Viewport for playfield
static tSimpleBufferManager *s_pMainBuffer;
static UWORD s_uwCopRawOffs = 0;
static tCopCmd *pCopCmds;
static UWORD g_sWaitPositions[YRES];

void setPxColor(UBYTE ubX, UBYTE ubY, UWORD uwValue);

void metaballsGsCreate(void)
{
  ULONG ulRawSize = (simpleBufferGetRawCopperlistInstructionCount(BITPLANES) +
                     YRES + 1 +    // yres is 16 so we need 16 waits + 1 for checking the 255th line
                     XRES * YRES + //reserve space for 20 colors for each YRES
                     1 +           // Final WAIT
                     1             // Just to be sure
  );

  // Create a view - first arg is always zero, then it's option-value
  s_pView = viewCreate(0,
                       TAG_VIEW_GLOBAL_CLUT, 1, // Same Color LookUp Table for all viewports
                       TAG_VIEW_COPLIST_MODE, VIEW_COPLIST_MODE_RAW,
                       TAG_VIEW_COPLIST_RAW_COUNT, ulRawSize,
                       TAG_END); // Must always end with TAG_END or synonym: TAG_DONE

  // Now let's do the same for main playfield
  s_pVpMain = vPortCreate(0,
                          TAG_VPORT_VIEW, s_pView,
                          TAG_VPORT_BPP, BITPLANES, // 4 bits per pixel, 16 colors
                          // We won't specify height here - viewport will take remaining space.
                          TAG_END);
  s_pMainBuffer = simpleBufferCreate(0,
                                     TAG_SIMPLEBUFFER_VPORT, s_pVpMain, // Required: parent viewport
                                     TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR,
                                     TAG_SIMPLEBUFFER_COPLIST_OFFSET, 0,
                                     TAG_SIMPLEBUFFER_IS_DBLBUF, 0,
                                     TAG_END);

  s_uwCopRawOffs = simpleBufferGetRawCopperlistInstructionCount(BITPLANES);
  tCopBfr *pCopBfr = s_pView->pCopList->pBackBfr;
  pCopCmds = &pCopBfr->pList[s_uwCopRawOffs];

  // Since we've set up global CLUT, palette will be loaded from first viewport
  // Colors are 0x0RGB, each channel accepts values from 0 to 15 (0 to F).
  s_pVpMain->pPalette[0] = 0x0000; // First color is also border color
  s_pVpMain->pPalette[1] = 0x0888; // Gray
  s_pVpMain->pPalette[2] = 0x0800; // Red - not max, a bit dark
  s_pVpMain->pPalette[3] = 0x0008; // Blue - same brightness as red
  s_pVpMain->pPalette[4] = 0x00FF;
  s_pVpMain->pPalette[5] = 0x00FA;
  s_pVpMain->pPalette[6] = 0x00F1;

  s_pVpMain->pPalette[7] = 0x00F2;

  s_pVpMain->pPalette[8] = 0x00F3;

  s_pVpMain->pPalette[9] = 0x00F4;

  s_pVpMain->pPalette[10] = 0x00F5;

  s_pVpMain->pPalette[11] = 0x00F6;

  s_pVpMain->pPalette[12] = 0x00F7;

  s_pVpMain->pPalette[13] = 0x00F8;

  s_pVpMain->pPalette[14] = 0x00F9;

  s_pVpMain->pPalette[15] = 0x00AA;

  s_pVpMain->pPalette[16] = 0x0011;
  s_pVpMain->pPalette[17] = 0x0022;
  s_pVpMain->pPalette[18] = 0x0033;
  s_pVpMain->pPalette[19] = 0x0044;
  s_pVpMain->pPalette[20] = 0x0055;
  s_pVpMain->pPalette[21] = 0x0066;

  systemSetDma(DMAB_SPRITE, 0);


  /*
ColUMN            0   1   2   3        4   5   6   7        8   9   10   11   12   13

Bitplane 0 -      1   0   1   0        1   0   1   0        1   0    1    0    1    0
Bitplane 1 -      0   1   1   0        0   1   1   0        0   1    1    0    0    1
Bitplane 2 -      0   0   0   1        1   1   1   0        0   0    0    1    1    1
Bitplane 3 -      0   0   0   0        0   0   0   1        1   1    1    1    1    1
Bitplane 4 -      0   0   0   0        0   0   0   0        0   0    0    0    0    0

*/

  for (UBYTE ubCont = 0; ubCont < 20; ubCont++)
  {
    blitRect(s_pMainBuffer->pBack, ubCont * 16, 0, 16, 256, ubCont + 1);
  }

  tCopList *pCopList = s_pMainBuffer->sCommon.pVPort->pView->pCopList;
  tCopCmd *pCmdListBack = &pCopList->pBackBfr->pList[s_uwCopRawOffs];
  tCopCmd *pCmdListFront = &pCopList->pFrontBfr->pList[s_uwCopRawOffs];

  UWORD ubCopIndex = 0;
  UBYTE ubWaitCounter = 0;

  // Start of opperlist - wait + setting colors
  UBYTE ubSpecialWait = 0;
  for (UBYTE ubCount = 0; ubCount < YRES; ubCount++)
  {
    UBYTE ubXWait = 44 + ubCount * YRES;
    copSetWaitBackAndFront(0, ubXWait, 1);
    copSetMoveBackAndFront(&g_pCustom->color[1], 0x0888);
    copSetMoveBackAndFront(&g_pCustom->color[2], 0x0AAA);
    copSetMoveBackAndFront(&g_pCustom->color[3], 0x0BBB);

    copSetMoveBackAndFront(&g_pCustom->color[4], 0x0111);
    copSetMoveBackAndFront(&g_pCustom->color[5], 0x0222);
    copSetMoveBackAndFront(&g_pCustom->color[6], 0x0333);

    copSetMoveBackAndFront(&g_pCustom->color[7], 0x0444);
    copSetMoveBackAndFront(&g_pCustom->color[8], 0x0555);
    copSetMoveBackAndFront(&g_pCustom->color[9], 0x0666);

    copSetMoveBackAndFront(&g_pCustom->color[10], 0x0444);
    copSetMoveBackAndFront(&g_pCustom->color[11], 0x0555);
    copSetMoveBackAndFront(&g_pCustom->color[12], 0x0666);

    copSetMoveBackAndFront(&g_pCustom->color[13], 0x0777);
    copSetMoveBackAndFront(&g_pCustom->color[14], 0x0888);
    copSetMoveBackAndFront(&g_pCustom->color[15], 0x0999);

    copSetMoveBackAndFront(&g_pCustom->color[16], 0x0AAA);
    copSetMoveBackAndFront(&g_pCustom->color[17], 0x0BBB);
    copSetMoveBackAndFront(&g_pCustom->color[18], 0x0CCC);

    copSetMoveBackAndFront(&g_pCustom->color[19], 0x0444);
    copSetMoveBackAndFront(&g_pCustom->color[20], 0x0555);
    //copSetMoveBackAndFront(&g_pCustom->color[21], 0x0666)   ;

    if ((UWORD)ubXWait + YRES > 255 && ubSpecialWait == 0)
    {
      copSetWaitBackAndFront(0xdf, 0xff, 0);
      ubSpecialWait = 1;
    }
  }

  // Load the view
  viewLoad(s_pView);
}

void metaballsGsLoop(void)
{
#ifdef COLORDEBUG
  g_pCustom->color[0] = 0x0FF0;
#endif

  static UWORD uwFrameNo = 0;
  static UBYTE *pColorPtr = &colors_data[0];
  

  // This will loop forever until you "pop" or change gamestate
  // or close the game
  if (keyCheck(KEY_ESCAPE))
  {
    gameExit();
  }

  if (keyUse(KEY_D))
  {
    tCopBfr *pCopBfr = s_pView->pCopList->pBackBfr;
    copDumpBfr(pCopBfr);

    pCopBfr = s_pView->pCopList->pFrontBfr;
    copDumpBfr(pCopBfr);
  }

  if (keyUse(KEY_Z))
  {
    setPxColor(0, 0, 0x0F00);
  }

  //if (keyUse(KEY_X))
  if (1)
  {
    //Horizontal

    for (UBYTE ubHorizontalCounter = 0; ubHorizontalCounter < XRES; ubHorizontalCounter++)
    {
      // vertical
      for (UBYTE ubVerticalCounter = 0; ubVerticalCounter < YRES; ubVerticalCounter++)
      {

        UWORD uwColor = (*pColorPtr << 8) | (*(pColorPtr + 1));
        pColorPtr += 2;
        setPxColor(ubHorizontalCounter, ubVerticalCounter, uwColor);
      }
    }
    uwFrameNo++;
  }
  if (uwFrameNo >= 300)
  {
    uwFrameNo = 0;
    pColorPtr = &colors_data[0];
  }

#ifdef COLORDEBUG
  g_pCustom->color[0] = 0x0000;
#endif

  vPortWaitForEnd(s_pVpMain);
  copSwapBuffers();
}

void metaballsGsDestroy(void)
{

  // Cleanup when leaving this gamestate
  systemUse();

  // This will also destroy all associated viewports and viewport managers
  viewDestroy(s_pView);
}
void setPxColor(UBYTE ubX, UBYTE ubY, UWORD uwValue)
{
  tCopList *pCopList = s_pMainBuffer->sCommon.pVPort->pView->pCopList;
  tCopCmd *pCmdListBack = &pCopList->pBackBfr->pList[s_uwCopRawOffs];
  ubX++;

#ifdef ACE_DEBUG
  logWrite("Setting value %x for pixel vertical %u\n", uwValue, ubY);
#endif

  pCmdListBack[g_sWaitPositions[ubY] + ubX].sMove.bfValue = uwValue;
}
