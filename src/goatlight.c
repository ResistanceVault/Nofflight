/********************************************/
/* Nofflight Amiga demo for Flashparty 2021 */
/* Author : Ozzyboshi                       */
/********************************************/

#include "goatlight.h"
#include <ace/managers/key.h>                   // Keyboard processing
#include <ace/managers/game.h>                  // For using gameExit
#include <ace/managers/system.h>                // For systemUnuse and systemUse
#include <ace/managers/viewport/simplebuffer.h> // Simple buffer
#include <ace/managers/blit.h>                  // for blitwait
#include <fixmath/fix16.h>
#include "../include/main.h"

#include "simplebuffertest.h"
#include "sprites.h"
#include "../_res/ball2bpl16x16_frame1.h"

#include "globals.h"

#include "../include/goatfonts.h"

#define BITPLANES 5 // 4 bitplanes
#define YTEXTCOORDINATE 175

#define copSetWaitBackAndFront(var, var2)                    \
    copSetWait(&pCmdListBack[ubCopIndex].sWait, var, var2);  \
    copSetWait(&pCmdListFront[ubCopIndex].sWait, var, var2); \
    ubCopIndex++;

#define copSetMoveBackAndFront(var, var2)                    \
    mycopSetMove(&pCmdListBack[ubCopIndex].sMove, (void*)var, var2);  \
    mycopSetMove(&pCmdListFront[ubCopIndex].sMove, (void*)var, var2); \
    ubCopIndex++;

#define copSetMoveBack(var, var2)                           \
    mycopSetMove((tCopMoveCmd *)&pCmdListBack[ubCopIndex].sMove,(void*) var, var2); \
    ubCopIndex++;

static void mycopSetMove(tCopMoveCmd *pMoveCmd, void *pAddr, UWORD uwValue) {
        pMoveCmd->bfUnused = 0;
        pMoveCmd->bfDestAddr = (ULONG)pAddr - (ULONG)((UBYTE *)g_pCustom);
        pMoveCmd->bfValue = uwValue;
}


static tView *s_pView;    // View containing all the viewports
static tVPort *s_pVpMain; // Viewport for playfield
static tSimpleBufferTestManager *s_pMainBuffer;
static UWORD s_uwCopRawOffs = 0;
static fix16_t sg_tVelocity;
static fix16_t sg_tVelocityIncrementer;
static UBYTE ubCopSpritesColorIndex = 0;

void updateCamera2(BYTE);
UWORD getBarColor(const UBYTE);
UWORD getBarColorPerspective(const UBYTE);
UWORD getBarColorPerspectiveBack(const UBYTE);
void setSpriteBallsCopperlistBack();

/*void MaskScreen(UBYTE);
void unMaskScreen(UBYTE);
UBYTE unMaskIntro(UBYTE, UBYTE);*/
void printPerspectiveRow2(tSimpleBufferTestManager *s_pMainBuffer, const UWORD, const UWORD, const UWORD);

UBYTE buildPerspectiveCopperlist(UBYTE);

void setHiddenRightBarColors(UWORD, UWORD, UWORD);

#define MAXCOLORS 12

// Color sequence palette for non perspective vertical rectangles
static UWORD s_pBarColors2[MAXCOLORS] = {
    0x0C87, // color of first col
    0x0AE8, // color of the second col
    0x08BC, // color of the third col
    0x0000, // color of the fourth col

    0x0C87, // color of first col
    0x0AE8, // color of the second col
    0x08BC, // color of the third col
    0x0000, // color of the fourth col

    0x0C87, // color of first col
    0x0AE8, // color of the second col
    0x08BC, // color of the third col
    0x0000, // color of the fourth col
};

// Color sequence palette for perspective rectangles
static UWORD s_pBarColorsPerspective2[MAXCOLORS] = {
    0x095A, // color of first col
    0x0694, // color of the second col
    0x087D, // color of the third col
    0x0000, // color of the fourth col

    0x095A, // color of first col
    0x0694, // color of the second col
    0x087D, // color of the third col
    0x0000, // color of the fourth col

    0x095A, // color of first col
    0x0694, // color of the second col
    0x087D, // color of the third col
    0x0000, // color of the fourth col
};

// Color sequence palette for perspective back rectangles
static UWORD s_pBarColorsPerspectiveBack2[MAXCOLORS] = {
    0x0733, // color of first col
    0x0463, // color of the second col
    0x0338, // color of the third col
    0x0000, // color of the fourth col

    0x0733, // color of first col
    0x0463, // color of the second col
    0x0338, // color of the third col
    0x0000, // color of the fourth col

    0x0733, // color of first col
    0x0463, // color of the second col
    0x0338, // color of the third col
    0x0000, // color of the fourth col
};

#ifdef STARTWITHBLACKBARS

// Actual Palette for non perspective vertical rectangles - first time they are all black for intro
static UWORD s_pBarColors[MAXCOLORS] = {
    0x0000, // color of first col
    0x0000, // color of the second col
    0x0000, // color of the third col
    0x0000, // color of the fourth col

    0x0000, // color of first col
    0x0000, // color of the second col
    0x0000, // color of the third col
    0x0000, // color of the fourth col

    0x0000, // color of first col
    0x0000, // color of the second col
    0x0000, // color of the third col
    0x0000, // color of the fourth col
};

// Actual Palette for perspective rectangles - first time they are all black for intro
static UWORD s_pBarColorsPerspective[MAXCOLORS] = {
    0x0000, // color of first col
    0x0000, // color of the second col
    0x0000, // color of the third col
    0x0000, // color of the fourth col

    0x0000, // color of first col
    0x0000, // color of the second col
    0x0000, // color of the third col
    0x0000, // color of the fourth col

    0x0000, // color of first col
    0x0000, // color of the second col
    0x0000, // color of the third col
    0x0000, // color of the fourth col
};

// Actual Palette for perspective rectangles - first time they are all black for intro
static UWORD s_pBarColorsPerspectiveBack[MAXCOLORS] = {
    0x0000, // color of first col
    0x0000, // color of the second col
    0x0000, // color of the third col
    0x0000, // color of the fourth col

    0x0000, // color of first col
    0x0000, // color of the second col
    0x0000, // color of the third col
    0x0000, // color of the fourth col

    0x0000, // color of first col
    0x0000, // color of the second col
    0x0000, // color of the third col
    0x0000, // color of the fourth col
};

#else
static UWORD s_pBarColors[MAXCOLORS] = {
    0x0C87, // color of first col
    0x0AE8, // color of the second col
    0x08BC, // color of the third col 0x08BC
    0x0000, // color of the fourth col

    0x0C87, // color of first col
    0x0AE8, // color of the second col
    0x08BC, // color of the third col
    0x0000, // color of the fourth col

    0x0C87, // color of first col
    0x0AE8, // color of the second col
    0x08BC, // color of the third col
    0x0000, // color of the fourth col
};

// Color sequence palette for perspective rectangles
static UWORD s_pBarColorsPerspective[MAXCOLORS] = {
    0x095A, // color of first col
    0x0694, // color of the second col
    0x087D, // color of the third col
    0x0000, // color of the fourth col

    0x095A, // color of first col
    0x0694, // color of the second col
    0x087D, // color of the third col
    0x0000, // color of the fourth col

    0x095A, // color of first col
    0x0694, // color of the second col
    0x087D, // color of the third col
    0x0000, // color of the fourth col
};

// Color sequence palette for perspective rectangles
static UWORD s_pBarColorsPerspectiveBack[MAXCOLORS] = {
    0x0733, // color of first col
    0x0463, // color of the second col
    0x0338, // color of the third col
    0x0000, // color of the fourth col

    0x0733, // color of first col
    0x0463, // color of the second col
    0x0338, // color of the third col
    0x0000, // color of the fourth col

    0x0733, // color of first col
    0x0463, // color of the second col
    0x0338, // color of the third col
    0x0000, // color of the fourth col
};
#endif

#define MAXCOLORSEFFECT 4 * 3
static UWORD s_pBarColorEffect[MAXCOLORSEFFECT] = {
    0x0111, 0x0222, 0x0333,
    0x0AAA, 0x0BBB, 0x0CCC,
    0x0444, 0x0555, 0x0666,
    0x0777, 0x0888, 0x0999};

static UBYTE s_ubBarColorsCopPositions[MAXCOLORS];
static UBYTE s_ubBarColorsCopPositionsBorder[MAXCOLORS];
static UBYTE s_ubBarColorsCopPositionsPerspective[MAXCOLORS];
static UBYTE s_ubBarColorsCopPositionsPerspectiveBack[MAXCOLORS];

static UBYTE s_ubColorIndex = 0;

#define SETBARCOLORSFRONTANDBACK                                                          \
    for (UBYTE ubCounter = 0; ubCounter < 4; ubCounter++)                                 \
    {                                                                                     \
        s_ubBarColorsCopPositions[ubCounter] = ubCopIndex;                                \
        copSetMoveBackAndFront(&g_pCustom->color[ubCounter + 1], getBarColor(ubCounter)); \
    }                                                                                     \
    for (UBYTE ubCounter = 4; ubCounter < 8; ubCounter++)                                 \
    {                                                                                     \
        s_ubBarColorsCopPositions[ubCounter] = ubCopIndex;                                \
        copSetMoveBackAndFront(&g_pCustom->color[ubCounter + 5], getBarColor(ubCounter)); \
    }                                                                                     \
    for (UBYTE ubCounter = 8; ubCounter < 12; ubCounter++)                                \
    {                                                                                     \
        s_ubBarColorsCopPositions[ubCounter] = ubCopIndex;                                \
        copSetMoveBackAndFront(&g_pCustom->color[ubCounter + 9], getBarColor(ubCounter)); \
    }

#define SETBARCOLORSFRONTANDBACKBORDER                                                    \
    for (UBYTE ubCounter = 0; ubCounter < MAXCOLORS; ubCounter++)                         \
    {                                                                                     \
        s_ubBarColorsCopPositionsBorder[ubCounter] = ubCopIndex;                          \
        copSetMoveBackAndFront(&g_pCustom->color[ubCounter + 1], getBarColor(ubCounter)); \
    }

#define SETBARCOLORSFRONTANDBACKPERSPECTIVE                                                          \
    for (UBYTE ubCounter = 0; ubCounter < MAXCOLORS; ubCounter++)                                    \
    {                                                                                                \
        s_ubBarColorsCopPositionsPerspective[ubCounter] = ubCopIndex;                                \
        copSetMoveBackAndFront(&g_pCustom->color[ubCounter + 1], getBarColorPerspective(ubCounter)); \
    }

#define SETBARCOLORSFRONTANDBACKPERSPECTIVEBACK                                                          \
    for (UBYTE ubCounter = 0; ubCounter < MAXCOLORS; ubCounter++)                                        \
    {                                                                                                    \
        s_ubBarColorsCopPositionsPerspectiveBack[ubCounter] = ubCopIndex;                                \
        copSetMoveBackAndFront(&g_pCustom->color[ubCounter + 1], getBarColorPerspectiveBack(ubCounter)); \
    }

#define SETBARCOLORSBACK                                                          \
    for (UBYTE ubCounter = 0; ubCounter < 4; ubCounter++)                         \
    {                                                                             \
        copSetMoveBack(&g_pCustom->color[ubCounter + 1], getBarColor(ubCounter)); \
    }                                                                             \
    for (UBYTE ubCounter = 4; ubCounter < 8; ubCounter++)                         \
    {                                                                             \
        copSetMoveBack(&g_pCustom->color[ubCounter + 5], getBarColor(ubCounter)); \
    }                                                                             \
    for (UBYTE ubCounter = 8; ubCounter < 12; ubCounter++)                        \
    {                                                                             \
        copSetMoveBack(&g_pCustom->color[ubCounter + 9], getBarColor(ubCounter)); \
    }

#define SETBARCOLORSBACKREDUCED                                                   \
    for (UBYTE ubCounter = 0; ubCounter < 4; ubCounter++)                         \
    {                                                                             \
        copSetMoveBack(&g_pCustom->color[ubCounter + 1], getBarColor(ubCounter)); \
    }
/*copSetMoveBack(&g_pCustom->color[9], getBarColor(7));                         \
    copSetMoveBack(&g_pCustom->color[10], getBarColor(8));*/

#define SETBARCOLORSBACKPERSPECTIVE                                                          \
    for (UBYTE ubCounter = 0; ubCounter < MAXCOLORS; ubCounter++)                            \
    {                                                                                        \
        copSetMoveBack(&g_pCustom->color[ubCounter + 1], getBarColorPerspective(ubCounter)); \
    }

#define SETBARCOLORSBACKPERSPECTIVEBACK                                                          \
    for (UBYTE ubCounter = 0; ubCounter < MAXCOLORS; ubCounter++)                                \
    {                                                                                            \
        copSetMoveBack(&g_pCustom->color[ubCounter + 1], getBarColorPerspectiveBack(ubCounter)); \
    }

#define SETBARCOLORSBACKBORDERSTD \
    for (UBYTE ubCounter = 0; ubCounter < MAXCOLORS; ubCounter++)                 \
    {                                                                             \
        copSetMoveBack(&g_pCustom->color[ubCounter + 1], getBarColor(ubCounter)); \
    }

#define VBARHEIGHT 207
#define PERSECTIVEBARHEIGHT 3
#define PERSPECTIVEBARSNUMBER 12    // How many perspective bars?
#define PERSPECTIVEBARSNUMBERBACK 4 // How many back perspective bars?
#define PERSPECTIVEBLOCKSIZE 7      // how many copper instruction for each perspective block?
typedef struct _tPerspectiveBar
{
    UBYTE ubCopIndex;
    UBYTE ubScrollCounter;
    UBYTE pScrollFlags2[32];
    UBYTE ubCopIndex2;
} tPerspectiveBar;

static tPerspectiveBar tPerspectiveBarArray[PERSPECTIVEBARSNUMBER + PERSPECTIVEBARSNUMBERBACK];

UBYTE ubCopIndexFirstLine = 0;

#define INITSCROLLFLAG(var, var0, var1, var2, var3, var4, var5, var6, var7, var8, var9, var10, var11, var12, var13, var14, var15, var16, var17, var18, var19, var20, var21, var22, var23, var24, var25, var26, var27, var28, var29, var30, var31) \
    tPerspectiveBarArray[var].pScrollFlags2[0] = var0;                                                                                                                                                                                            \
    tPerspectiveBarArray[var].pScrollFlags2[1] = var1;                                                                                                                                                                                            \
    tPerspectiveBarArray[var].pScrollFlags2[2] = var2;                                                                                                                                                                                            \
    tPerspectiveBarArray[var].pScrollFlags2[3] = var3;                                                                                                                                                                                            \
    tPerspectiveBarArray[var].pScrollFlags2[4] = var4;                                                                                                                                                                                            \
    tPerspectiveBarArray[var].pScrollFlags2[5] = var5;                                                                                                                                                                                            \
    tPerspectiveBarArray[var].pScrollFlags2[6] = var6;                                                                                                                                                                                            \
    tPerspectiveBarArray[var].pScrollFlags2[7] = var7;                                                                                                                                                                                            \
    tPerspectiveBarArray[var].pScrollFlags2[8] = var8;                                                                                                                                                                                            \
    tPerspectiveBarArray[var].pScrollFlags2[9] = var9;                                                                                                                                                                                            \
    tPerspectiveBarArray[var].pScrollFlags2[10] = var10;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[11] = var11;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[12] = var12;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[13] = var13;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[14] = var14;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[15] = var15;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[16] = var16;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[17] = var17;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[18] = var18;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[19] = var19;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[20] = var20;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[21] = var21;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[22] = var22;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[23] = var23;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[24] = var24;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[25] = var25;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[26] = var26;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[27] = var27;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[28] = var28;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[29] = var29;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[30] = var30;                                                                                                                                                                                          \
    tPerspectiveBarArray[var].pScrollFlags2[31] = var31;

// Start of sprites data
tMover g_Sprite1Vector, g_Sprite0Vector;

#define MAXBALLS 7
tMover g_pBallsMovers[MAXBALLS];

v2d g_Gravity;
#define LITTLE_BALLS_MASS 2

//Music
/*long mt_init(const unsigned char *);
int chan2played();
int chan3played();
void mt_music();
void mt_end();*/

static tBitMap *g_pBitmapHelper1, *g_pBitmapHelper2, *g_pBitmapHelperTmp;
static UBYTE ubTxtOverlayFlag = 0;
static UBYTE s_ubMoveBalls = 0;

void printTxtChar(UWORD uwXPos, UBYTE ubLetter);
void printTxtCharEmpty(UWORD uwXPos);
void printTextMsg(BYTE);
void blitBitPlane(UBYTE *, UBYTE *);
void setSpriteBallsCopperlist();

static UWORD uwChan2PlayedArray[15];
static UWORD uwChan3PlayedArray[15];
static UWORD uwChan4PlayedArray[15];

void gameGsCreate(void)
{
    // ULONG ulRawSize = SimpleBufferTestGetRawCopperlistInstructionCount(BITPLANES)+16;
    ULONG ulRawSize = (SimpleBufferTestGetRawCopperlistInstructionCount(BITPLANES) + MAXBALLS * 2 + ACE_SPRITES_COPPERLIST_SIZE + 10 + MAXCOLORS * 4 + PERSPECTIVEBLOCKSIZE * (PERSPECTIVEBARSNUMBER + PERSPECTIVEBARSNUMBERBACK) + 1
                       /*                   3 * 3 + // 32 bars - each consists of WAIT + 3 MOVE instruction
        1 +     // Final WAIT
        1       // Just to be sure*/
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

    s_pMainBuffer = SimpleBufferTestCreate(0,
                                           TAG_SIMPLEBUFFER_VPORT, s_pVpMain, // Required: parent viewport
                                           TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR,
                                           TAG_SIMPLEBUFFER_BOUND_WIDTH, 320 + 32 * 2,
                                           TAG_SIMPLEBUFFER_USE_X_SCROLLING, 0,
                                           TAG_SIMPLEBUFFER_COPLIST_OFFSET, 0,
                                           TAG_SIMPLEBUFFER_IS_DBLBUF, 0,
                                           TAG_END);

    s_uwCopRawOffs = SimpleBufferTestGetRawCopperlistInstructionCount(BITPLANES);

    // We don't need anything from OS anymore
     //systemUnuse();

    for (UBYTE ubBallIndex = 0; ubBallIndex < MAXBALLS; ubBallIndex++)
    {
        spriteVectorInit(&g_pBallsMovers[ubBallIndex], ubBallIndex, -16 - ubBallIndex * 9, 50 + ubBallIndex * 20, fix16_div(fix16_from_int((int)ubBallIndex + 4), fix16_from_int((int)ubBallIndex + 2)), 0, LITTLE_BALLS_MASS);
        copBlockEnableSpriteRaw(s_pView->pCopList, ubBallIndex, (UBYTE *)ball2bpl16x16_frame1_data, sizeof(ball2bpl16x16_frame1_data), s_uwCopRawOffs);
        moverMove(g_pBallsMovers[ubBallIndex]);
    }

    s_uwCopRawOffs += ACE_SPRITES_COPPERLIST_SIZE;

    // Draw rectangles

    // Build perspective row function: scheme following
    /*
ColUMN            0   1   2   3        4   5   6   7        8   9   10   11

Bitplane 0 -      1   0   1   0        1   0   1   0        1   0    1    0
Bitplane 1 -      0   1   1   0        0   1   1   0        0   1    1    0
Bitplane 2 -      0   0   0   1        0   0   0   1        0   0    0    1
Bitplane 3 -      0   0   0   0        1   1   1   1        0   0    0    0
Bitplane 4 -      0   0   0   0        0   0   0   0        1   1    1    1

Bitplane 4 - (quello per il testo , 1 testo on, 2 testo off)*/

    blitRect(s_pMainBuffer->pBack, 0 * 32, 0, 24, VBARHEIGHT, 1); // first column col 1 (index 0)
    blitRect(s_pMainBuffer->pBack, 1 * 32, 0, 24, VBARHEIGHT, 2); // second column col 2
    blitRect(s_pMainBuffer->pBack, 2 * 32, 0, 24, VBARHEIGHT, 3); // third column col 3
    blitRect(s_pMainBuffer->pBack, 3 * 32, 0, 24, VBARHEIGHT, 4); // fourth column col 4
    // ----------- end of first group -------------------------

    blitRect(s_pMainBuffer->pBack, 4 * 32, 0, 24, VBARHEIGHT, 9);  // first column col 9 (index 4)
    blitRect(s_pMainBuffer->pBack, 5 * 32, 0, 24, VBARHEIGHT, 10); // second column col 10
    blitRect(s_pMainBuffer->pBack, 6 * 32, 0, 24, VBARHEIGHT, 11); // third column col 11
    blitRect(s_pMainBuffer->pBack, 7 * 32, 0, 24, VBARHEIGHT, 12); // fourth column col 12
    // ----------- end of second group -------------------------

    blitRect(s_pMainBuffer->pBack, 8 * 32, 0, 24, VBARHEIGHT, 17);  // first column col 9 (index 8)
    blitRect(s_pMainBuffer->pBack, 9 * 32, 0, 24, VBARHEIGHT, 18);  // second column col 10
    blitRect(s_pMainBuffer->pBack, 10 * 32, 0, 24, VBARHEIGHT, 19); // third column col 11
    blitRect(s_pMainBuffer->pBack, 11 * 32, 0, 24, VBARHEIGHT, 20); // fourth column col 12

    blitRect(s_pMainBuffer->pBack, 0 * 32, VBARHEIGHT, 24, 1, 1); // first column col 1 (index 0)
    blitRect(s_pMainBuffer->pBack, 1 * 32, VBARHEIGHT, 24, 1, 2); // second column col 2
    blitRect(s_pMainBuffer->pBack, 2 * 32, VBARHEIGHT, 24, 1, 3); // third column col 3
    blitRect(s_pMainBuffer->pBack, 3 * 32, VBARHEIGHT, 24, 1, 4); // fourth column col 4

    blitRect(s_pMainBuffer->pBack, 4 * 32, VBARHEIGHT, 24, 1, 5); // first column col 1 (index 0)
    blitRect(s_pMainBuffer->pBack, 5 * 32, VBARHEIGHT, 24, 1, 6); // second column col 2
    blitRect(s_pMainBuffer->pBack, 6 * 32, VBARHEIGHT, 24, 1, 7); // third column col 3
    blitRect(s_pMainBuffer->pBack, 7 * 32, VBARHEIGHT, 24, 1, 8); // fourth column col 4

    blitRect(s_pMainBuffer->pBack, 8 * 32, VBARHEIGHT, 24, 1, 9);   // first column col 1 (index 0)
    blitRect(s_pMainBuffer->pBack, 9 * 32, VBARHEIGHT, 24, 1, 10);  // second column col 2
    blitRect(s_pMainBuffer->pBack, 10 * 32, VBARHEIGHT, 24, 1, 11); // third column col 3
    blitRect(s_pMainBuffer->pBack, 11 * 32, VBARHEIGHT, 24, 1, 12); // fourth column col 4

    // ----------- end of third group -------------------------

    // Start building/drawing perspective rows
    UWORD uwRowWidth = 25;
    for (UWORD uwCounter = 208; uwCounter < 208 + PERSPECTIVEBARSNUMBER * PERSECTIVEBARHEIGHT; uwCounter++)
    {
        printPerspectiveRow2(s_pMainBuffer, uwCounter, 48, uwRowWidth);
        if ((uwCounter % PERSECTIVEBARHEIGHT) == 0)
            uwRowWidth += 3;
    }
    uwRowWidth -= 3;
    for (UWORD uwCounter = 244; uwCounter < 244 + PERSPECTIVEBARSNUMBERBACK * PERSECTIVEBARHEIGHT; uwCounter++)
    {
        printPerspectiveRow2(s_pMainBuffer, uwCounter, 48, uwRowWidth - 3);
        if ((uwCounter % PERSECTIVEBARHEIGHT) == 0)
            uwRowWidth -= 3;
    }

    tCopList *pCopList = s_pMainBuffer->sCommon.pVPort->pView->pCopList;
    tCopCmd *pCmdListBack = &pCopList->pBackBfr->pList[s_uwCopRawOffs];
    tCopCmd *pCmdListFront = &pCopList->pFrontBfr->pList[s_uwCopRawOffs];

    UBYTE ubCopIndex = 0;

    copSetWaitBackAndFront(0, 43);

    SETBARCOLORSFRONTANDBACK;

    /*copSetWaitBackAndFront(0, 200);
    copSetMoveBackAndFront(&g_pCustom->color[0], 0x0000);
    copSetMoveBackAndFront(&g_pCustom->color[0], 0x0000);
    copSetMoveBackAndFront(&g_pCustom->color[0], 0x0000);*/

    // Sprites colors
    // Sprite 0 and 1
    ubCopSpritesColorIndex = ubCopIndex;
    // create space in copperlist for sprites - for now just a dumb set on color 5 that is unused
    for (UBYTE ubDumb = 0; ubDumb < 16; ubDumb++)
    {
        copSetMoveBackAndFront(&g_pCustom->color[5], 0x000F);
    }

#if 0

#endif

    ubCopIndex = buildPerspectiveCopperlist(ubCopIndex);

    copSetWaitBackAndFront(7, 43);
    copSetMoveBackAndFront(&g_pCustom->color[31], 0x0FF0);

    // set default velocity to 1
    sg_tVelocity = fix16_div(fix16_from_int(2), fix16_from_int(1));
    sg_tVelocityIncrementer = fix16_div(fix16_from_int(1), fix16_from_int(10));

    // Init the gravity force
    g_Gravity.x = 0; //fix16_div(fix16_from_int(1), fix16_from_int(1000));
    g_Gravity.y = fix16_div(fix16_from_int(1), fix16_from_int(5));

    // Enable bounce
    g_ubHBounceEnabled = 0;
    g_ubVBounceEnabled = 1;

    // Inizio test text overlay
    g_pBitmapHelper1 = bitmapCreate(384 * 1, 256, 1, BMF_CLEAR);
    blitBitPlane((UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[3]), g_pBitmapHelper1->Planes[0]);

    g_pBitmapHelper2 = bitmapCreate(384 * 1, 256, 1, BMF_CLEAR);

    g_pBitmapHelperTmp = bitmapCreate(320 * 1, 32, 2, BMF_CLEAR);

    // Init music
    //mt_init(discocrazy_data);

    uwChan3PlayedArray[6] = 0x0C87;
    uwChan3PlayedArray[5] = 0x0CA7;
    uwChan3PlayedArray[4] = 0x0CB7;
    uwChan3PlayedArray[3] = 0x0CC7;
    uwChan3PlayedArray[2] = 0x0CD7;
    uwChan3PlayedArray[1] = 0x0CE7;
    uwChan3PlayedArray[0] = 0x0CF7;

    uwChan4PlayedArray[6] = 0x0AE8;
    uwChan4PlayedArray[5] = 0x0BE9;
    uwChan4PlayedArray[4] = 0x0CEA;
    uwChan4PlayedArray[3] = 0x0DEB;
    uwChan4PlayedArray[2] = 0x0EFC;
    uwChan4PlayedArray[1] = 0x0FFD;
    uwChan4PlayedArray[0] = 0x0FFF;

    uwChan2PlayedArray[6] = 0x08BC;
    uwChan2PlayedArray[5] = 0x07AB;
    uwChan2PlayedArray[4] = 0x069B;
    uwChan2PlayedArray[3] = 0x048A;
    uwChan2PlayedArray[2] = 0x037A;
    uwChan2PlayedArray[1] = 0x0269;
    uwChan2PlayedArray[0] = 0x0069;

    // Load the view
    viewLoad(s_pView);

    //bitmapSave(s_pMainBuffer->pBack, "bitplanes.raw");
}

void gameGsLoop(void)
{
    static UWORD uwFrameNo = 0;

#ifdef COLORDEBUG
    g_pCustom->color[0] = 0x0FF0;
#endif
    static BYTE bIsExiting = 0;
    static BYTE bXCamera = 0;

#ifdef AUTOSCROLLING
    uwFrameNo++;

    // Manage col change
    if (uwFrameNo >= 300 && bIsExiting == 0)
    {
        static BYTE bChan3Index = 0;
        static BYTE bChan4Index = 0;
#if 1
        //int g_iChan4Played = chan2played();
        //g_iChan4Played=uwFrameNo%10;

        if (g_iChan4Played)
            bChan3Index = 0;
        else
        {
            bChan3Index++;
            if (bChan3Index > 6)
                bChan3Index = 6;
        }
        s_pBarColors[0] = uwChan3PlayedArray[bChan3Index];
        s_pBarColors[4] = uwChan3PlayedArray[bChan3Index];
        s_pBarColors[8] = uwChan3PlayedArray[bChan3Index];

        if (g_iChan4Played)
            bChan4Index = 0;
        else
        {
            bChan4Index++;
            if (bChan4Index > 6)
                bChan4Index = 6;
        }
        s_pBarColors[1] = uwChan4PlayedArray[bChan4Index];
        s_pBarColors[5] = uwChan4PlayedArray[bChan4Index];
        s_pBarColors[9] = uwChan4PlayedArray[bChan4Index];

        s_pBarColors[2] = uwChan2PlayedArray[bChan4Index];
        s_pBarColors[6] = uwChan2PlayedArray[bChan4Index];
        s_pBarColors[10] = uwChan2PlayedArray[bChan4Index];
#endif
    }

    static fix16_t tCameraFrame = 0;

    tCameraFrame = fix16_add(tCameraFrame, sg_tVelocity);

    bXCamera = (BYTE)fix16_to_int(tCameraFrame);

    if (bXCamera >= 32)
    {
        bXCamera = 0;
        tCameraFrame = 0;
        s_ubColorIndex++;
        if (s_ubColorIndex >= MAXCOLORS)
            s_ubColorIndex = 0;
    }
    else if (bXCamera < 0)
    {
        bXCamera = 31;
        tCameraFrame = fix16_from_int(31);
        if (s_ubColorIndex == 0)
            s_ubColorIndex = MAXCOLORS - 1;
        else
            s_ubColorIndex--;
    }

    // Control starts here

    // Slow down
    if (uwFrameNo >= 600 && uwFrameNo <= 900 && (uwFrameNo % 4) == 0)
    {
        sg_tVelocity = fix16_sub(sg_tVelocity, sg_tVelocityIncrementer);
    }

    // Accellerate
    else if (uwFrameNo >= 1000 && uwFrameNo <= 1550 && (uwFrameNo % 6) == 0)
    {
        sg_tVelocity = fix16_add(sg_tVelocity, sg_tVelocityIncrementer);
    }

    // Show balls
    if (uwFrameNo == 300 || uwFrameNo == 301)
    {
        g_pCustom->bplcon0=0x4200;   // disables 5th bitplane when ball bouncing to save dma cycles
        setSpriteBallsCopperlistBack();
        s_ubMoveBalls = 1;
    }
    // Balls falls down
    else if (uwFrameNo == 1700)
    {
        g_ubVBounceEnabled = 0;
    }

    // Print text
    else if (uwFrameNo == 2000)
    {
        disableSpritesAll();
        g_pCustom->bplcon0=0x5200; // re-enables 5th bitplane for txt
        s_ubMoveBalls = 0;
        ubTxtOverlayFlag = 1;
    }
    else if (uwFrameNo == 3600)
    {

        bIsExiting = 1;
        ubTxtOverlayFlag = 0;

        tCopList *pCopList = s_pMainBuffer->sCommon.pVPort->pView->pCopList;
        tCopCmd *pCmdListBack = &pCopList->pBackBfr->pList[s_uwCopRawOffs];
        tCopCmd *pCmdListFront = &pCopList->pFrontBfr->pList[s_uwCopRawOffs];

        UBYTE ubCopIndex = 1;

        SETBARCOLORSFRONTANDBACK;

        for (UBYTE ubDumb = 0; ubDumb < 16; ubDumb++)
        {
            copSetMoveBackAndFront(&g_pCustom->color[5], 0x000F);
        }
    }

    updateCamera2(bXCamera);
    

#endif
#if 0

    if (keyCheck(KEY_T))
    {
        ubTxtOverlayFlag = 1;
    }

    if (keyCheck(KEY_ESCAPE))
    {
        gameExit();
        return;
    }

    // Color effect
    if (keyCheck(KEY_SPACE))
    {
        static UBYTE ubColorEffectIndex = 0;

//ubColorEffectIndex=3;
#ifdef ACE_DEBUG
        logWrite("Setting new colors for index %u (%x %x %x )\n", ubColorEffectIndex, s_pBarColorEffect[ubColorEffectIndex], s_pBarColorEffect[ubColorEffectIndex + 1], s_pBarColorEffect[ubColorEffectIndex + 2]);
#endif
        setHiddenRightBarColors(s_pBarColorEffect[ubColorEffectIndex], s_pBarColorEffect[ubColorEffectIndex + 1], s_pBarColorEffect[ubColorEffectIndex + 2]);
        ubColorEffectIndex += 3;
        if (ubColorEffectIndex > MAXCOLORSEFFECT - 3)
            ubColorEffectIndex = 0;
    }

    // Exits with black bars
    if (keyCheck(KEY_Q))
    {
        bIsExiting = 1;
        ubTxtOverlayFlag = 0;
    }

    if (keyUse(KEY_V))
    {
        bXCamera = 0;
        s_ubColorIndex = 0;
        updateCamera2(bXCamera);
    }
    if (keyUse(KEY_B))
    {
        bXCamera++;

        if (bXCamera >= 32)
        {
            bXCamera = 0;
            s_ubColorIndex++;
            if (s_ubColorIndex >= MAXCOLORS)
                s_ubColorIndex = 0;
        }

        updateCamera2(bXCamera);
    }

    if (keyUse(KEY_C))
    {
        bXCamera--;
        if (bXCamera < 0)
        {
            bXCamera = 31;
            if (s_ubColorIndex == 0)
                s_ubColorIndex = MAXCOLORS - 1;
            else
                s_ubColorIndex--;
        }
        updateCamera2(bXCamera);
    }

    if (keyUse(KEY_D))
    {
        tCopBfr *pCopBfr = s_pView->pCopList->pBackBfr;
        copDumpBfr(pCopBfr);

        pCopBfr = s_pView->pCopList->pFrontBfr;
        copDumpBfr(pCopBfr);
    }

    // Increase speed in autoscrolling mode
    if (keyUse(KEY_O))
    {
        sg_tVelocity = fix16_add(sg_tVelocity, sg_tVelocityIncrementer);
    }
    // Decrease speed in autoscolling mode
    if (keyUse(KEY_P))
    {
        //if (sg_tVelocity > 0)
        sg_tVelocity = fix16_sub(sg_tVelocity, sg_tVelocityIncrementer);
    }

    if (keyUse(KEY_1))
    {
        setSpriteBallsCopperlist();
        s_ubMoveBalls = 1;
    }
#endif
    if (s_ubMoveBalls)
    {
        for (UBYTE ubBallIndex = 0; ubBallIndex < MAXBALLS; ubBallIndex++)
        {
            //g_pBallsMovers
            tMover *g_SpriteVector = &g_pBallsMovers[ubBallIndex];

            if (!(*g_SpriteVector).ubLocked)
            {

                spriteVectorApplyForce(g_SpriteVector, &g_Gravity);
                
                v2d lol;
                v2d_add(&lol,&g_SpriteVector->tVelocity,&g_SpriteVector->tAccelleration);
                g_SpriteVector->tVelocity=lol;
                //moverAddAccellerationToVelocity(g_SpriteVector);
                //moverAddVelocityToLocation(g_SpriteVector);
                v2d_add(&g_SpriteVector->tLocation,&g_SpriteVector->tLocation,&g_SpriteVector->tVelocity);

                UWORD uwLocationX = (UWORD)fix16_to_int(g_SpriteVector->tLocation.x);
                UWORD uwLocationY = (UWORD)fix16_to_int(g_SpriteVector->tLocation.y);
                if (uwLocationY > 255)
                {
                    // Just a random point off screen
                    spriteMove3((FUBYTE)g_SpriteVector->ubSpriteIndex, (UWORD)0, (UWORD)300);
                    (*g_SpriteVector).ubLocked=1;
                }
                spriteMove3((FUBYTE)g_SpriteVector->ubSpriteIndex, uwLocationX, uwLocationY);

                /*if (moverMove(*g_SpriteVector))
                    (*g_SpriteVector).ubLocked = 1;*/

                int y = fix16_to_int(g_SpriteVector->tLocation.y);
                if (g_ubVBounceEnabled && (y > s_pAceSprites[g_SpriteVector->ubSpriteIndex].iBounceBottomLimit - 30 || y < 10))
                {
                    g_SpriteVector->tVelocity.y = fix16_mul(g_SpriteVector->tVelocity.y, fix16_from_int(-1));
                    g_SpriteVector->tVelocity.y = fix16_sub(g_SpriteVector->tVelocity.y, g_SpriteVector->tAccelleration.y);
                }
                g_SpriteVector->tAccelleration.x = 0;
                g_SpriteVector->tAccelleration.y = 0;
            }
        }
    }

#ifdef COLORDEBUG
    g_pCustom->color[0] = 0x0000;
#endif

    vPortWaitForEnd(s_pVpMain);
    copSwapBuffers();

    if (ubTxtOverlayFlag)
    {
        printTextMsg(bXCamera);
 
    }

    static BYTE bIntroColorIndex = 11;
    static UBYTE lastBx = 0;
    if (bXCamera > 0)
        lastBx = 1;
    if (bXCamera == 0 && bIntroColorIndex >= 0 && lastBx == 1)
    {
        BYTE bColorIndex = MAXCOLORS - 1 + s_ubColorIndex;
        while (bColorIndex > MAXCOLORS - 1)
            bColorIndex -= MAXCOLORS;
#ifdef ACE_DEBUG
        logWrite("Setting intro color %u to index number %u\n", s_pBarColors2[bColorIndex], bColorIndex);
#endif
        s_pBarColors[bColorIndex] = s_pBarColors2[bColorIndex];
        s_pBarColorsPerspective[bColorIndex] = s_pBarColorsPerspective2[bColorIndex];
        s_pBarColorsPerspectiveBack[bColorIndex] = s_pBarColorsPerspectiveBack2[bColorIndex];
        bIntroColorIndex--;
        lastBx = 0;
    }

    // Manage exit
    static BYTE bExitSequence = 11;
    if (bXCamera == 0 && bIsExiting && lastBx == 1)
    {
        setHiddenRightBarColors(0x0000, 0x0000, 0x0000);
#ifdef ACE_DEBUG
        logWrite("Setting intro color to zero to index number\n");
#endif

        if (bExitSequence < 0)
        {
            //gameExit();
            myChangeState(2);
            return;
        }
        bExitSequence--;
        lastBx = 0;
    }
}

void gameGsDestroy(void)
{
    //mt_end();

    bitmapDestroy(g_pBitmapHelper1);
    bitmapDestroy(g_pBitmapHelper2);
    bitmapDestroy(g_pBitmapHelperTmp);

    // Cleanup when leaving this gamestate
    //systemUse();

    // Free sprite stuff
    copBlockSpritesFree();

    // This will also destroy all associated viewports and viewport managers
    viewDestroy(s_pView);
}

void updateCamera2(BYTE bX)
{
#ifdef ACE_DEBUG
    logWrite("UpdateCamera2 input %u\n", bX);
#endif
    UBYTE ubCopIndex;
    tCopList *pCopList = s_pMainBuffer->sCommon.pVPort->pView->pCopList;
    tCopCmd *pCmdList = &pCopList->pBackBfr->pList[s_pMainBuffer->uwCopperOffset];
    tCopCmd *pCmdListBack = &pCopList->pBackBfr->pList[s_uwCopRawOffs];

    UWORD uwShift = (16 - (bX & 0xF)) & 0xF;
    uwShift = (uwShift << 4) | uwShift;
#ifdef ACE_DEBUG
    logWrite("UpdateCamera2 shift  %u\n", uwShift);
#endif

    UBYTE isBitplanesShifted = 0;

    ULONG ulPlaneAddr = (ULONG)((ULONG)s_pMainBuffer->pBack->Planes[0]);
    ULONG ulPlaneAddr2 = (ULONG)((ULONG)s_pMainBuffer->pBack->Planes[1]);
    ULONG ulPlaneAddr3 = (ULONG)((ULONG)s_pMainBuffer->pBack->Planes[2]);
    ULONG ulPlaneAddr4 = (ULONG)((ULONG)s_pMainBuffer->pBack->Planes[3]);
    ULONG ulPlaneAddr5 = (ULONG)((ULONG)s_pMainBuffer->pBack->Planes[4]);

    if (ubTxtOverlayFlag || s_ubMoveBalls)
    {
        //printTextMsg(bX);
#if 1
        static UBYTE ubLastPosition = 0;
        static UWORD uwCycleCounter = 0;

        BYTE bCurrentPosition = bX - ubLastPosition;
#ifdef ACE_DEBUG
        logWrite("UpdateCamera2 Velocity: %u\n", bCurrentPosition);
#endif

        // If bCurrentposition is negative, one full cycle has been done
        if (bCurrentPosition < 0)
        {
            uwCycleCounter++;
            /*{
                static UBYTE ubTxtArrayOffset = 0;
                static UBYTE pTxtArray[] = {1, 0, 1, 0, 1};
                UBYTE *pPtrTxtArray = pTxtArray + ubTxtArrayOffset;
                ubTxtArrayOffset++;

                for (UWORD uwCycleCont = 0; uwCycleCont < uwCycleCounter; uwCycleCont++)
                {
                    if ((*pPtrTxtArray) == 0)
                        printTxtCharEmpty(40 - uwCycleCont * 4);
                    else
                        printTxtChar(40 - uwCycleCont * 4);
                    pPtrTxtArray--;
                }
            }*/

#ifdef ACE_DEBUG
            logWrite("UpdateCamera2 full cycle detected: %u\n", uwCycleCounter);
#endif
        }
        /*if (uwCycleCounter == 12)
            gameExit();*/

        ubLastPosition = bX;
#endif

        /*if (bX == 0 && s_ulTxtPlaneCounter)
            s_ulTxtPlaneCounter += 2;*/
        /*if (bX > 16)
        {
            s_ulTxtPlaneCounter += 2;
            while (ubTxtFrameCounter>16) ubTxtFrameCounter-=16;
        }*/

        ///while (ubTxtFrameCounter>31) ubTxtFrameCounter-=31;

        UWORD uwOffset = uwCycleCounter << 2;
        uwOffset = 0;
        ulPlaneAddr4 = (ULONG)((ULONG)g_pBitmapHelper1->Planes[0] + uwOffset);
        ulPlaneAddr5 = (ULONG)g_pBitmapHelper2->Planes[0] + uwOffset;
        if (bX > 16)
        {
#ifdef ACE_DEBUG
            logWrite("UpdateCamera2 extra bitplane increase\n");
#endif
            ulPlaneAddr4 += 2;
            ulPlaneAddr5 += 2;
        }

        //tCopList *pCopList = s_pMainBuffer->sCommon.pVPort->pView->pCopList;
        //tCopCmd *pCmdListBack = &pCopList->pBackBfr->pList[s_uwCopRawOffs];
        static UBYTE txtcopinit = 0;
        if (txtcopinit == 0 && s_ubMoveBalls == 0)
        {
            tCopCmd *pCmdListFront = &pCopList->pFrontBfr->pList[s_uwCopRawOffs];

            ubCopIndex = ubCopSpritesColorIndex;

            copSetMoveBackAndFront(&g_pCustom->color[16], 0x00CF);
            copSetMoveBackAndFront(&g_pCustom->color[17], 0x00CF);
            copSetMoveBackAndFront(&g_pCustom->color[18], 0x00CF);
            copSetMoveBackAndFront(&g_pCustom->color[19], 0x00CF);

            copSetMoveBackAndFront(&g_pCustom->color[20], 0x00CF);
            copSetMoveBackAndFront(&g_pCustom->color[21], 0x00CF);
            copSetMoveBackAndFront(&g_pCustom->color[22], 0x00CF);
            copSetMoveBackAndFront(&g_pCustom->color[23], 0x00CF);

            copSetMoveBackAndFront(&g_pCustom->color[24], 0x055F);
            copSetMoveBackAndFront(&g_pCustom->color[25], 0x055F);
            copSetMoveBackAndFront(&g_pCustom->color[26], 0x055F);
            copSetMoveBackAndFront(&g_pCustom->color[27], 0x055F);

            copSetMoveBackAndFront(&g_pCustom->color[28], 0x055F);
            copSetMoveBackAndFront(&g_pCustom->color[29], 0x055F);
            copSetMoveBackAndFront(&g_pCustom->color[30], 0x055F);
            copSetMoveBackAndFront(&g_pCustom->color[31], 0x055F);

            ubCopIndex = 7;
            copSetMoveBackAndFront(&g_pCustom->color[7], 0x0FFF);
            copSetMoveBackAndFront(&g_pCustom->color[8], 0x0FFF);
            copSetMoveBackAndFront(&g_pCustom->color[9], 0x0FFF);
            copSetMoveBackAndFront(&g_pCustom->color[10], 0x0FFF);
            copSetMoveBackAndFront(&g_pCustom->color[11], 0x0FFF);
            copSetMoveBackAndFront(&g_pCustom->color[12], 0x0FFF);

            txtcopinit = 1;
        }
    }

    if (bX > 16)
    {
#ifdef ACE_DEBUG
        logWrite("Aggiungo 2\n");
#endif
        ulPlaneAddr += 2;
        ulPlaneAddr2 += 2;
        ulPlaneAddr3 += 2;
        if (!ubTxtOverlayFlag && s_ubMoveBalls == 0)
        {
            ulPlaneAddr4 += 2;
            ulPlaneAddr5 += 2;
        }
        isBitplanesShifted = 1;
    }

    // Move the upper vertical bars
    copSetMove(&pCmdList[6].sMove, &g_pBplFetch[0].uwHi, ulPlaneAddr >> 16);
    copSetMove(&pCmdList[7].sMove, &g_pBplFetch[0].uwLo, ulPlaneAddr & 0xFFFF);

    copSetMove(&pCmdList[8].sMove, &g_pBplFetch[1].uwHi, ulPlaneAddr2 >> 16);
    copSetMove(&pCmdList[9].sMove, &g_pBplFetch[1].uwLo, ulPlaneAddr2 & 0xFFFF);

    copSetMove(&pCmdList[10].sMove, &g_pBplFetch[2].uwHi, ulPlaneAddr3 >> 16);
    copSetMove(&pCmdList[11].sMove, &g_pBplFetch[2].uwLo, ulPlaneAddr3 & 0xFFFF);

    copSetMove(&pCmdList[12].sMove, &g_pBplFetch[3].uwHi, ulPlaneAddr4 >> 16);
    copSetMove(&pCmdList[13].sMove, &g_pBplFetch[3].uwLo, ulPlaneAddr4 & 0xFFFF);

    copSetMove(&pCmdList[14].sMove, &g_pBplFetch[4].uwHi, ulPlaneAddr5 >> 16);
    copSetMove(&pCmdList[15].sMove, &g_pBplFetch[4].uwLo, ulPlaneAddr5 & 0xFFFF);

    if (bX > 0)
    {
#ifdef ACE_DEBUG
        logWrite("Caso con fetch modificato a 30\n");
#endif
        copSetMove(&pCmdList[2].sMove, &g_pCustom->ddfstrt, 0x0030);
        copSetMove(&pCmdList[3].sMove, &g_pCustom->bpl1mod, 0x0006);
        copSetMove(&pCmdList[4].sMove, &g_pCustom->bpl2mod, 0x0006);
        copSetMove(&pCmdList[5].sMove, &g_pCustom->bplcon1, uwShift);

        // start of perspective
        UWORD uwBplMods = 0x0006;
        UBYTE ubLastBplMods = 0;

        // Each perspective row must be modifield in its copperlist block - cycle each of them
        for (UBYTE ubPerspectiveRowCounter = 0; ubPerspectiveRowCounter < PERSPECTIVEBARSNUMBER + PERSPECTIVEBARSNUMBERBACK; ubPerspectiveRowCounter++)
        {
            struct _tPerspectiveBar *tBar = &tPerspectiveBarArray[ubPerspectiveRowCounter];
            struct _tPerspectiveBar *tBarPrev = &tPerspectiveBarArray[ubPerspectiveRowCounter - 1];

            tBar->ubScrollCounter = tBar->pScrollFlags2[bX];
#ifdef ACE_DEBUG
            logWrite("pscrollflags set to %u\n", tBar->ubScrollCounter);
#endif

            // }

            // This will contain the left shift position absolute
            UBYTE ubAbsShift = bX - 1 + tBar->ubScrollCounter;
            if (isBitplanesShifted)
                ubAbsShift = bX - 16 - 1 + tBar->ubScrollCounter;

            // Now we must calculate how many words we are left shifting because eventually we must change bplmods and then the remainder for bplcon1
            UBYTE ubAbsBplMods = ubAbsShift >> 4;
            UBYTE ubAbsRemainder = ubAbsShift % 16;

            // Now it's time to calculate the reg values - to get the new mod we must take into account what was the last mod and subtract it
            // For the bplcon1 we must take FF (max shifting) and subtract for the remainder
            UWORD uwFinalMods = 0x0006 + ubAbsBplMods * 2 - ubLastBplMods;
            UWORD uwFinalBplCon1 = 0x00FF - ubAbsRemainder * 17;

            // Save ubLastBplMods
            ubLastBplMods = ubAbsBplMods * 2;

#ifdef ACE_DEBUG
            /*logWrite("abs - Processing row %u\n", ubPerspectiveRowCounter);
            logWrite("abs - Ok now we are on position %u so we must absolute shift this perspective bar to the left for %u positions: mods %u and bpl1con to %u\n", bX, ubAbsShift, ubAbsBplMods, ubAbsRemainder);
            logWrite("abs - New registers : mods : %u, bplcon1: %u\n", uwFinalMods, uwFinalBplCon1);*/
#endif

            copSetMove(&pCmdListBack[0 + tBar->ubCopIndex].sMove, &g_pCustom->bplcon1, uwFinalBplCon1);
            copSetMove(&pCmdListBack[1 + tBar->ubCopIndex].sMove, &g_pCustom->bpl1mod, uwBplMods);
            copSetMove(&pCmdListBack[2 + tBar->ubCopIndex].sMove, &g_pCustom->bpl2mod, uwBplMods);
            if (ubPerspectiveRowCounter > 0)
            {
                /*copSetMove(&pCmdListBack[4 + tBarPrev->ubCopIndex].sMove, &g_pCustom->bpl1mod, uwFinalMods);
                copSetMove(&pCmdListBack[5 + tBarPrev->ubCopIndex].sMove, &g_pCustom->bpl2mod, uwFinalMods);*/

                copSetMove(&pCmdListBack[0 + tBarPrev->ubCopIndex2].sMove, &g_pCustom->bpl1mod, uwFinalMods);
                copSetMove(&pCmdListBack[1 + tBarPrev->ubCopIndex2].sMove, &g_pCustom->bpl2mod, uwFinalMods);
            }
            else
            {
#ifdef ACE_DEBUG
                logWrite("abs - Here i should be moving the first chunk and in am not doing it!!!!!\n");
#endif

                copSetMove(&pCmdListBack[ubCopIndexFirstLine].sMove, &g_pCustom->bpl1mod, uwFinalMods);
                copSetMove(&pCmdListBack[ubCopIndexFirstLine + 1].sMove, &g_pCustom->bpl2mod, uwFinalMods);
            }
        }
    }
    else
    {
        copSetMove(&pCmdList[2].sMove, &g_pCustom->ddfstrt, 0x0038);
        copSetMove(&pCmdList[3].sMove, &g_pCustom->bpl1mod, 0x0008);
        copSetMove(&pCmdList[4].sMove, &g_pCustom->bpl2mod, 0x0008);
        copSetMove(&pCmdList[5].sMove, &g_pCustom->bplcon1, uwShift);

        for (UBYTE ubPerspectiveRowCounter = 0; ubPerspectiveRowCounter < PERSPECTIVEBARSNUMBER + PERSPECTIVEBARSNUMBERBACK; ubPerspectiveRowCounter++)
        {
            struct _tPerspectiveBar *tBar = &tPerspectiveBarArray[ubPerspectiveRowCounter];
            struct _tPerspectiveBar *tBarPrev = &tPerspectiveBarArray[ubPerspectiveRowCounter - 1];

            copSetMove(&pCmdListBack[0 + tBar->ubCopIndex].sMove, &g_pCustom->bplcon1, 0x0000);
            copSetMove(&pCmdListBack[1 + tBar->ubCopIndex].sMove, &g_pCustom->bpl1mod, 0x0008);
            copSetMove(&pCmdListBack[2 + tBar->ubCopIndex].sMove, &g_pCustom->bpl2mod, 0x0008);

            tPerspectiveBarArray[ubPerspectiveRowCounter].ubScrollCounter = 0;

            if (ubPerspectiveRowCounter > 0)
            {
                /*copSetMove(&pCmdListBack[4 + tBarPrev->ubCopIndex].sMove, &g_pCustom->bpl1mod, 0x0008);
                copSetMove(&pCmdListBack[5 + tBarPrev->ubCopIndex].sMove, &g_pCustom->bpl2mod, 0x0008);*/

                copSetMove(&pCmdListBack[0 + tBarPrev->ubCopIndex2].sMove, &g_pCustom->bpl1mod, 0x0008);
                copSetMove(&pCmdListBack[1 + tBarPrev->ubCopIndex2].sMove, &g_pCustom->bpl2mod, 0x0008);
            }
            else
            {
                copSetMove(&pCmdListBack[ubCopIndexFirstLine].sMove, &g_pCustom->bpl1mod, 0x0008);
                copSetMove(&pCmdListBack[ubCopIndexFirstLine + 1].sMove, &g_pCustom->bpl2mod, 0x0008);
            }
        }
    }

    ubCopIndex = 1; // must be one because at zero we would get wait instruction

    // if da togliere è solo un test
    if (ubTxtOverlayFlag == 0 && s_ubMoveBalls == 0)
    {
        SETBARCOLORSBACK;
    }
    else
    {
        SETBARCOLORSBACKREDUCED;
    }

    ubCopIndex = s_ubBarColorsCopPositionsBorder[0];
    /*pCmdListBack[ubCopIndex].sMove.bfValue = getBarColor(0);
    pCmdListBack[ubCopIndex].sMove.bfValue = getBarColor(1);
    pCmdListBack[ubCopIndex].sMove.bfValue = getBarColor(2);
    pCmdListBack[ubCopIndex].sMove.bfValue = getBarColor(3);
    pCmdListBack[ubCopIndex].sMove.bfValue = getBarColor(4);
    pCmdListBack[ubCopIndex].sMove.bfValue = getBarColor(5);
    pCmdListBack[ubCopIndex].sMove.bfValue = getBarColor(6);
    pCmdListBack[ubCopIndex].sMove.bfValue = getBarColor(7);
    pCmdListBack[ubCopIndex].sMove.bfValue = getBarColor(8);
    pCmdListBack[ubCopIndex].sMove.bfValue = getBarColor(9);
    pCmdListBack[ubCopIndex].sMove.bfValue = getBarColor(10);
    pCmdListBack[ubCopIndex].sMove.bfValue = getBarColor(11);*/
    SETBARCOLORSBACKBORDERSTD;

    ubCopIndex = s_ubBarColorsCopPositionsPerspective[0];
    SETBARCOLORSBACKPERSPECTIVE;

    ubCopIndex = s_ubBarColorsCopPositionsPerspectiveBack[0];
    SETBARCOLORSBACKPERSPECTIVEBACK;

    //copSwapBuffers();
}



UWORD getBarColor(const UBYTE ubColNo)
{
    UBYTE ubColorRealIndex = ubColNo + s_ubColorIndex;
    while (ubColorRealIndex >= MAXCOLORS)
        ubColorRealIndex -= MAXCOLORS;
    return s_pBarColors[ubColorRealIndex];
}

UWORD getBarColorPerspective(const UBYTE ubColNo)
{
    UBYTE ubColorRealIndex = ubColNo + s_ubColorIndex;
    while (ubColorRealIndex >= MAXCOLORS)
        ubColorRealIndex -= MAXCOLORS;
    return s_pBarColorsPerspective[ubColorRealIndex];
}

UWORD getBarColorPerspectiveBack(const UBYTE ubColNo)
{
    UBYTE ubColorRealIndex = ubColNo + s_ubColorIndex;
    while (ubColorRealIndex >= MAXCOLORS)
        ubColorRealIndex -= MAXCOLORS;
    return s_pBarColorsPerspectiveBack[ubColorRealIndex];
}

// Build perspective row function: scheme following
/*
ColUMN            0   1   2   3        4   5   6   7        8   9   10   11

Bitplane 0 -      1   0   1   0        1   0   1   0        1   0    1    0
Bitplane 1 -      0   1   1   0        0   1   1   0        0   1    1    0
Bitplane 2 -      0   0   0   1        1   1   1   0        0   0    0    1
Bitplane 3 -      0   0   0   0        0   0   0   1        1   1    1    1


*/

void printPerspectiveRow2(tSimpleBufferTestManager *s_pMainBuffer, const UWORD uwRowNo, const UWORD uwBytesPerRow, const UWORD uwBarWidth)
{
#ifdef ACE_DEBUG
    UBYTE ubDebug = 0;
#endif
    typedef struct _tBitplanes
    {
        UBYTE *p_ubBitplaneStartPointer;
        UBYTE *p_ubBitplaneEndPointer;
        UBYTE *p_ubBitplanePointer;
    } tBitplanes;

    tBitplanes bitplanes[BITPLANES];

    //ulValue = 0b00000001 11111111 1111111 111111111 00000000;

    UWORD uwSpaceBetweenCols = 8;

    UBYTE *p_ubBitplane0StartPointer = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[0]);
    UBYTE *p_ubBitplane1StartPointer = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[1]);
    UBYTE *p_ubBitplane2StartPointer = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[2]);
    UBYTE *p_ubBitplane3StartPointer = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[3]);

    p_ubBitplane0StartPointer = p_ubBitplane0StartPointer + uwBytesPerRow * uwRowNo; // vertical position
    UBYTE *p_ubBitplane0Pointer = p_ubBitplane0StartPointer + 4 * 4 + 3;             // go to last byte of 5th  bar

    p_ubBitplane1StartPointer = p_ubBitplane1StartPointer + uwBytesPerRow * uwRowNo; // vertical position
    UBYTE *p_ubBitplane1Pointer = p_ubBitplane1StartPointer + 4 * 4 + 3;             // go to 5th  bar

    p_ubBitplane2StartPointer = p_ubBitplane2StartPointer + uwBytesPerRow * uwRowNo; // vertical position
    UBYTE *p_ubBitplane2Pointer = p_ubBitplane2StartPointer + 4 * 4 + 3;             // go to 5th  bar

    p_ubBitplane3StartPointer = p_ubBitplane3StartPointer + uwBytesPerRow * uwRowNo; // vertical position
    UBYTE *p_ubBitplane3Pointer = p_ubBitplane3StartPointer + 4 * 4 + 3;             // go to 5th  bar

    bitplanes[0].p_ubBitplaneStartPointer = p_ubBitplane0StartPointer;
    bitplanes[0].p_ubBitplaneEndPointer = p_ubBitplane0StartPointer + 48;
    bitplanes[0].p_ubBitplanePointer = p_ubBitplane0Pointer;

    bitplanes[1].p_ubBitplaneStartPointer = p_ubBitplane1StartPointer;
    bitplanes[1].p_ubBitplaneEndPointer = p_ubBitplane1StartPointer + 48;
    bitplanes[1].p_ubBitplanePointer = p_ubBitplane1Pointer;

    bitplanes[2].p_ubBitplaneStartPointer = p_ubBitplane2StartPointer;
    bitplanes[2].p_ubBitplaneEndPointer = p_ubBitplane2StartPointer + 48;
    bitplanes[2].p_ubBitplanePointer = p_ubBitplane2Pointer;

    bitplanes[3].p_ubBitplaneStartPointer = p_ubBitplane3StartPointer;
    bitplanes[3].p_ubBitplaneEndPointer = p_ubBitplane3StartPointer + 48;
    bitplanes[3].p_ubBitplanePointer = p_ubBitplane3Pointer;

    for (UBYTE ubBitplaneCounter = 0; ubBitplaneCounter < BITPLANES - 1; ubBitplaneCounter++)
    {
        UWORD uwSpaceBetweenColsCounter = 0;
        UWORD uwBarWidthCounter = 0;
        BYTE bBytePos = 0;
        UBYTE ubBarCounter = 4;

        // start bitplane 0
        while (bitplanes[ubBitplaneCounter].p_ubBitplanePointer >= bitplanes[ubBitplaneCounter].p_ubBitplaneStartPointer)
        {
            if (uwSpaceBetweenColsCounter < uwSpaceBetweenCols)
            {
#ifdef ACE_DEBUG
                if (ubDebug)
                    logWrite("Setting space (%d-%d-bytepos :%d)\n", uwSpaceBetweenColsCounter, uwSpaceBetweenCols, bBytePos);
#endif

                uwSpaceBetweenColsCounter++;
                bBytePos++;
                if (bBytePos >= 8)
                {
                    bBytePos = 0;
                    bitplanes[ubBitplaneCounter].p_ubBitplanePointer--;
#ifdef ACE_DEBUG
                    if (ubDebug)
                        logWrite("Byte ended!!! decrementing p_ubBitplane0Pointer\n");
#endif
                }
            }
            else
            {
                // Actual colum writing
                if (uwBarWidthCounter < uwBarWidth)
                {
#ifdef ACE_DEBUG
                    if (ubDebug)
                        logWrite("Setting bar (%d-%d-bytepos :%d)\n", uwBarWidthCounter, uwBarWidth, bBytePos);
#endif
                    // Column (starting at 4)    0   1   2   3        4
                    // Bitplane 0 -              1   0   1   0        1   0   1   0        1   0    1    0
                    if (ubBitplaneCounter == 0)
                    {
                        if ((ubBarCounter % 2) == 0)
                            *bitplanes[ubBitplaneCounter].p_ubBitplanePointer |= BV(bBytePos);
                    }

                    // Column (starting at 4)    0   1   2   3        4
                    // Bitplane 1 -              0   1   1   0        0   1   1   0        0   1    1    0
                    else if (ubBitplaneCounter == 1)
                    {
                        if ((ubBarCounter % 4) == 1 || (ubBarCounter % 4) == 2)
                            *bitplanes[ubBitplaneCounter].p_ubBitplanePointer |= BV(bBytePos);
                    }

                    // Column (starting at 4)    0   1   2   3        4
                    // Bitplane 2 -              0   0   0   1        1   1   1   0        0   0    0    1
                    else if (ubBitplaneCounter == 2)
                    {
                        if (ubBarCounter >= 3)
                            *bitplanes[ubBitplaneCounter].p_ubBitplanePointer |= BV(bBytePos);
                    }

                    // Column (starting at 4)    0   1   2   3        4
                    // Bitplane 3 -      0   0   0   0        0   0   0   1        1   1    1    1
                    else if (ubBitplaneCounter == 3)
                    {
                        // we never have to paint something here
                    }

                    bBytePos++;
                    if (bBytePos >= 8)
                    {
                        bBytePos = 0;
                        bitplanes[ubBitplaneCounter].p_ubBitplanePointer--;
#ifdef ACE_DEBUG
                        if (ubDebug)
                            logWrite("Byte ended!!! decrementing p_ubBitplane0Pointer\n");
#endif
                    }

                    uwBarWidthCounter++;
                }
                else
                {
#ifdef ACE_DEBUG
                    if (ubDebug)
                        logWrite("Bar cycle ended, resetting counters and incrementing the wait space\n");
#endif
                    uwSpaceBetweenColsCounter = 0;
                    uwBarWidthCounter = 0;
                    uwSpaceBetweenCols = 8;
                    ubBarCounter--;
                }
            }
            // Compose the byte
        }

// start of the right part of the screen
// restore pointers
#ifdef ACE_DEBUG
        if (ubDebug)
            logWrite("Start right side... repositioning\n");
#endif
        bitplanes[ubBitplaneCounter].p_ubBitplanePointer = bitplanes[ubBitplaneCounter].p_ubBitplaneStartPointer + 4 * 5;
        uwSpaceBetweenColsCounter = 0;
        uwBarWidthCounter = 0;
        uwSpaceBetweenCols = 8;
        ubBarCounter = 5;
        bBytePos = 7;

        while (bitplanes[ubBitplaneCounter].p_ubBitplanePointer < bitplanes[ubBitplaneCounter].p_ubBitplaneEndPointer)
        {

            if (uwBarWidthCounter < uwBarWidth)
            {
#ifdef ACE_DEBUG
                if (ubDebug)
                    logWrite("Setting bar (%d-%d-bytepos :%d)\n", uwBarWidthCounter, uwBarWidth, bBytePos);
#endif
                // Column (starting at 5)                             5   6   7        8   9   10   11
                // Bitplane 0 -              1   0   1   0        1   0   1   0        1   0    1    0
                if (ubBitplaneCounter == 0)
                {
                    if ((ubBarCounter % 2) == 0)
                        *bitplanes[ubBitplaneCounter].p_ubBitplanePointer |= BV(bBytePos);
                }

                // Column (starting at 5)                             5   6   7        8   9   10   11
                // Bitplane 1 -              0   1   1   0        0   1   1   0        0   1    1    0
                else if (ubBitplaneCounter == 1)
                {
                    if ((ubBarCounter % 4) == 1 || (ubBarCounter % 4) == 2)
                        *bitplanes[ubBitplaneCounter].p_ubBitplanePointer |= BV(bBytePos);
                }

                // Column (starting at 5)                             5   6   7        8   9   10   11
                // Bitplane 2 -              0   0   0   1        1   1   1   0        0   0    0    1
                else if (ubBitplaneCounter == 2)
                {
                    if (ubBarCounter == 5 || ubBarCounter == 6 || ubBarCounter == 11)
                        *bitplanes[ubBitplaneCounter].p_ubBitplanePointer |= BV(bBytePos);
                }

                // Column (starting at 5)                             5   6   7        8   9   10   11
                // Bitplane 3 -              0   0   0   0        0   0   0   1        1   1    1    1
                else if (ubBitplaneCounter == 3)
                {
                    if (ubBarCounter >= 7)
                        *bitplanes[ubBitplaneCounter].p_ubBitplanePointer |= BV(bBytePos);
                }

                bBytePos--;
                if (bBytePos < 0)
                {
                    bBytePos = 7;
                    bitplanes[ubBitplaneCounter].p_ubBitplanePointer++;
#ifdef ACE_DEBUG
                    if (ubDebug)
                        logWrite("Byte ended!!! incrementing p_ubBitplane0Pointer\n");
#endif
                }

                uwBarWidthCounter++;
            }
            else
            {
                if (uwSpaceBetweenColsCounter < uwSpaceBetweenCols)
                {
#ifdef ACE_DEBUG
                    if (ubDebug)
                        logWrite("Setting space (%d-%d-bytepos :%d)\n", uwSpaceBetweenColsCounter, uwSpaceBetweenCols, bBytePos);
#endif

                    uwSpaceBetweenColsCounter++;
                    bBytePos--;
                    if (bBytePos < 0)
                    {
                        bBytePos = 7;
                        bitplanes[ubBitplaneCounter].p_ubBitplanePointer++;
#ifdef ACE_DEBUG
                        if (ubDebug)
                            logWrite("Byte ended!!! incrementing p_ubBitplane0Pointer\n");
#endif
                    }
                }
                else
                {
#ifdef ACE_DEBUG
                    if (ubDebug)
                        logWrite("Bar cycle ended, resetting counters and incrementing the wait space\n");
#endif
                    uwSpaceBetweenColsCounter = 0;
                    uwBarWidthCounter = 0;
                    uwSpaceBetweenCols = 8;
                    ubBarCounter++;
                }
            }
        }
    }
}

// Build the tPerspectiveBarArray array to manage the copperlist
UBYTE buildPerspectiveCopperlist(UBYTE ubCopIndex)
{
    tCopList *pCopList = s_pMainBuffer->sCommon.pVPort->pView->pCopList;
    tCopCmd *pCmdListBack = &pCopList->pBackBfr->pList[s_uwCopRawOffs];
    tCopCmd *pCmdListFront = &pCopList->pFrontBfr->pList[s_uwCopRawOffs];

    UBYTE ubWaitCount = 0;
    UBYTE ubSpecialWaitSet = 0;

    copSetWaitBackAndFront(0, 209 + 43 + ubWaitCount - 1);

    // Change palette for perspective rows
    SETBARCOLORSFRONTANDBACKPERSPECTIVE

    ubCopIndexFirstLine = ubCopIndex;
    copSetMoveBackAndFront(&g_pCustom->color[15], 0x0000);
    copSetMoveBackAndFront(&g_pCustom->color[15], 0x0000);

    for (UBYTE ubCount = 0; ubCount < PERSPECTIVEBARSNUMBER + PERSPECTIVEBARSNUMBERBACK; ubCount++)
    {
#ifdef ACE_DEBUG
        logWrite("setting wait for row %u\n", 209 + 43 + ubWaitCount);
#endif
        copSetWaitBackAndFront(0, 209 + 43 + ubWaitCount);

        //s_ubPerspectiveBarCopPositions[ubCount] = ubCopIndex;
        tPerspectiveBarArray[ubCount].ubCopIndex = ubCopIndex;
        tPerspectiveBarArray[ubCount].ubScrollCounter = 0;

        copSetMoveBackAndFront(&g_pCustom->bplcon1, 0x0000);
        copSetMoveBackAndFront(&g_pCustom->bpl1mod, 0x0008);
        copSetMoveBackAndFront(&g_pCustom->bpl2mod, 0x0008);

        // test di oggi
        if (ubCount == PERSPECTIVEBARSNUMBER - 2)
        {
#ifdef ACE_DEBUG
            logWrite("setting palette change for border at %u \n", ubCount);
#endif

            // Change palette for perspective back rows
            SETBARCOLORSFRONTANDBACKBORDER
        }
        // fine test di oggi

        if (ubCount == PERSPECTIVEBARSNUMBER - 1)
        {
#ifdef ACE_DEBUG
            logWrite("setting palette change for back at %u \n", ubCount);
#endif

            // Change palette for perspective back rows
            SETBARCOLORSFRONTANDBACKPERSPECTIVEBACK
        }

        if (209 + 43 + ubWaitCount + PERSECTIVEBARHEIGHT - 1 > 255 && ubSpecialWaitSet == 0)
        {
#ifdef ACE_DEBUG
            logWrite("setting special wait to go under 255 (1) \n");
#endif
            copSetWaitBackAndFront(0xdf, 0xff);
            ubSpecialWaitSet = 1;
        }

        UWORD uwLastWait = 209 + 43 + ubWaitCount + PERSECTIVEBARHEIGHT - 1;
        copSetWaitBackAndFront(0, uwLastWait);
#ifdef ACE_DEBUG
        logWrite("setting copperlist last row with wait %u (%u) \n", uwLastWait, ubWaitCount);
#endif

        if ((ubCount % 2) == 0)
        {

            tPerspectiveBarArray[ubCount].ubCopIndex2 = ubCopIndex;

            copSetMoveBackAndFront(&g_pCustom->color[0], 0x0000);
            copSetMoveBackAndFront(&g_pCustom->color[0], 0x0000);
        }
        else
        {
            tPerspectiveBarArray[ubCount].ubCopIndex2 = ubCopIndex;

            copSetMoveBackAndFront(&g_pCustom->color[0], 0x0000);
            copSetMoveBackAndFront(&g_pCustom->color[0], 0x0000);
        }
        if (209 + 43 + ubWaitCount > 255 - PERSECTIVEBARHEIGHT && ubSpecialWaitSet == 0)
        {
#ifdef ACE_DEBUG
            logWrite("setting special wait to go under 255 (2)\n");
#endif
            copSetWaitBackAndFront(0xdf, 0xff);
            ubSpecialWaitSet = 1;
        }

        ubWaitCount += PERSECTIVEBARHEIGHT;
    }

    INITSCROLLFLAG(0,
                   0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0,
                   1, 1, 1, 1, 1, 1, 1, 1,
                   1, 1, 1, 1, 1, 1, 1, 1);

    INITSCROLLFLAG(1,
                   0, 0, 0, 0, 0, 0, 1, 1,
                   1, 1, 1, 1, 1, 1, 1, 2,
                   2, 2, 2, 2, 2, 2, 2, 2,
                   2, 2, 2, 2, 4, 4, 4, 4);

    INITSCROLLFLAG(2,
                   0, 0, 0, 1, 1, 1, 1, 1,
                   1, 2, 2, 2, 2, 2, 3, 3,
                   4, 3, 4, 4, 4, 4, 4, 4,
                   4, 5, 5, 5, 6, 8, 8, 8);

    INITSCROLLFLAG(3,
                   0, 0, 1, 1, 1, 1, 2, 2,
                   2, 2, 3, 3, 3, 4, 4, 4,
                   5, 5, 5, 6, 6, 6, 6, 6,
                   7, 7, 7, 8, 9, 10, 10, 10);

    INITSCROLLFLAG(4,
                   0, 0, 1, 1, 1, 2, 2, 3,
                   3, 3, 4, 4, 4, 5, 5, 6,
                   6, 6, 8, 8, 8, 8, 8, 9,
                   9, 10, 11, 10, 12, 13, 13, 14);

    INITSCROLLFLAG(5,
                   0, 0, 1, 1, 2, 2, 3, 3,
                   4, 4, 5, 5, 5, 6, 6, 7,
                   7, 8, 9, 9, 10, 10, 11, 11,
                   11, 12, 13, 13, 14, 15, 16, 17);

    INITSCROLLFLAG(6,
                   0, 1, 1, 2, 2, 3, 3, 4,
                   4, 5, 6, 6, 7, 7, 8, 8,
                   9, 10, 11, 11, 12, 13, 13, 13,
                   14, 14, 16, 16, 17, 18, 19, 20);

    INITSCROLLFLAG(7,
                   0, 1, 1, 2, 3, 3, 4, 4,
                   5, 6, 6, 7, 8, 8, 9, 10,
                   10, 11, 13, 13, 14, 15, 15, 16,
                   16, 17, 19, 18, 20, 21, 22, 23);

    INITSCROLLFLAG(8,
                   0, 1, 1, 2, 3, 4, 4, 5,
                   6, 7, 7, 8, 9, 10, 10, 11,
                   12, 13, 14, 15, 16, 17, 18, 19,
                   19, 19, 21, 21, 22, 23, 24, 25);

    INITSCROLLFLAG(9,
                   0, 1, 2, 2, 3, 4, 5, 6,
                   7, 7, 8, 9, 10, 11, 12, 12,
                   13, 14, 16, 17, 18, 19, 20, 21,
                   21, 22, 24, 24, 25, 26, 27, 28);

    INITSCROLLFLAG(10,
                   0, 1, 2, 3, 4, 5, 5, 6,
                   7, 8, 9, 10, 11, 12, 13, 15,
                   15, 16, 18, 19, 20, 21, 22, 23,
                   23, 25, 26, 26, 28, 29, 30, 31);

    INITSCROLLFLAG(11,
                   0, 1, 2, 3, 4, 5, 6, 7,
                   8, 9, 10, 11, 12, 13, 14, 16,
                   16, 17, 19, 20, 22, 23, 24, 25,
                   25, 27, 28, 29, 30, 31, 32, 33);

    //Here starts back perspective

    INITSCROLLFLAG(12,
                   0, 1, 2, 3, 4, 5, 5, 6,
                   7, 8, 9, 10, 11, 12, 13, 15,
                   15, 16, 18, 19, 20, 21, 22, 23,
                   23, 25, 26, 26, 28, 29, 30, 31);

    INITSCROLLFLAG(13,
                   0, 1, 2, 2, 3, 4, 5, 6,
                   7, 7, 8, 9, 10, 11, 12, 12,
                   13, 14, 16, 17, 18, 19, 20, 21,
                   21, 22, 24, 24, 25, 26, 27, 28);

    INITSCROLLFLAG(14,
                   0, 1, 1, 2, 3, 4, 4, 5,
                   6, 7, 7, 8, 9, 10, 10, 11,
                   12, 13, 14, 15, 16, 17, 18, 19,
                   19, 19, 21, 21, 22, 23, 24, 25);

    INITSCROLLFLAG(15,
                   0, 1, 1, 2, 3, 3, 4, 4,
                   5, 6, 6, 7, 8, 8, 9, 10,
                   10, 11, 13, 13, 14, 15, 15, 16,
                   16, 17, 19, 18, 20, 21, 22, 23);

    return ubCopIndex;
}

void setHiddenRightBarColors(UWORD ubColorRectangle, UWORD ubColorPerspective, UWORD ubColorPerspectiveBack)
{
    const BYTE bMaxCol = MAXCOLORS - 1;
    BYTE bColorIndex = bMaxCol + s_ubColorIndex;
    while (bColorIndex > bMaxCol)
        bColorIndex -= MAXCOLORS;
    s_pBarColors[bColorIndex] = ubColorRectangle;
    s_pBarColorsPerspective[bColorIndex] = ubColorPerspective;
    s_pBarColorsPerspectiveBack[bColorIndex] = ubColorPerspectiveBack;
}

void printTxtChar(UWORD uwXPos, UBYTE ubLetter)
{
    ubLetter -= 64;
    UBYTE *goatfonts_data_bpl1 = (UBYTE *)(goatfonts_data + 128 * ubLetter);
    UBYTE *goatfonts_data_bpl2 = (UBYTE *)(goatfonts_size / 2 + goatfonts_data + 128 * ubLetter);
    for (int i = 0; i < 32; i++)
    {
        memcpy(g_pBitmapHelper2->Planes[0] + YTEXTCOORDINATE * 48 + uwXPos + i * 48, &goatfonts_data_bpl1[i * 4], 4);
        memcpy(g_pBitmapHelper1->Planes[0] + YTEXTCOORDINATE * 48 + uwXPos + i * 48, &goatfonts_data_bpl2[i * 4], 4);
    }
}

void printTxtCharEmpty(UWORD uwXPos)
{
    for (int i = 0; i < 32; i++)
    {
        memset(g_pBitmapHelper2->Planes[0] + YTEXTCOORDINATE * 48 + uwXPos + i * 48, 0, 4);
        memset(g_pBitmapHelper1->Planes[0] + YTEXTCOORDINATE * 48 + uwXPos + i * 48, 0, 4);
    }
}

void printTextMsg(BYTE bX)
{
    static UBYTE ubLastPosition = 0;
    static UWORD uwCycleCounter = 0;
    static UBYTE next = 0;

    BYTE bCurrentPosition = bX - ubLastPosition;
#ifdef ACE_DEBUG
    logWrite("UpdateCamera2 Velocity: %u\n", bCurrentPosition);
#endif

    // If bCurrentposition is negative, one full cycle has been done
    /*if (bCurrentPosition < 0)
    {
        g_pCustom->color[0]=0X0f00;
        next = 1;
    }*/
    if (next==1)
    {
        blitWait();
        g_pCustom->bltcon0 = 0x09F0;
        g_pCustom->bltcon1 = 0x0000;
        g_pCustom->bltafwm = 0xFFFF;
        g_pCustom->bltalwm = 0xFFFF;
        g_pCustom->bltamod = 0x0008;
        g_pCustom->bltdmod = 0x0000;
        g_pCustom->bltapt = (UBYTE *)((ULONG)g_pBitmapHelper1->Planes[0] + 4 + (48 * YTEXTCOORDINATE));
        g_pCustom->bltdpt = (UBYTE *)((ULONG)g_pBitmapHelperTmp->Planes[0]);
        g_pCustom->bltsize = 0x0814;
        // copy to g_pBitmapHelperTmp

        blitWait();
        g_pCustom->bltcon0 = 0x09F0;
        g_pCustom->bltcon1 = 0x0000;
        g_pCustom->bltafwm = 0xFFFF;
        g_pCustom->bltalwm = 0xFFFF;
        g_pCustom->bltamod = 0x0008;
        g_pCustom->bltdmod = 0x0000;
        g_pCustom->bltapt = (UBYTE *)((ULONG)g_pBitmapHelper2->Planes[0] + 4 + (48 * YTEXTCOORDINATE));
        g_pCustom->bltdpt = (UBYTE *)((ULONG)g_pBitmapHelperTmp->Planes[1]);
        g_pCustom->bltsize = 0x0814;

        next = 0;
    }
    if (bCurrentPosition < 0)
    {

        blitWait();
        g_pCustom->bltcon0 = 0x09f0;
        g_pCustom->bltcon1 = 0x0000;
        g_pCustom->bltafwm = 0xFFFF;
        g_pCustom->bltalwm = 0xFFFF;
        g_pCustom->bltamod = 0x0000;
        g_pCustom->bltdmod = 0x0008;
        g_pCustom->bltapt = (UBYTE *)((ULONG)g_pBitmapHelperTmp->Planes[0]);
        g_pCustom->bltdpt = (UBYTE *)((ULONG)g_pBitmapHelper1->Planes[0] + (48 * YTEXTCOORDINATE));
        g_pCustom->bltsize = 0x0814;
        // copy to g_pBitmapHelperTmp

        blitWait();
        g_pCustom->bltcon0 = 0x09f0;
        g_pCustom->bltcon1 = 0x0000;
        g_pCustom->bltafwm = 0xFFFF;
        g_pCustom->bltalwm = 0xFFFF;
        g_pCustom->bltamod = 0x0000;
        g_pCustom->bltdmod = 0x0008;
        g_pCustom->bltapt = (UBYTE *)((ULONG)g_pBitmapHelperTmp->Planes[1]);
        g_pCustom->bltdpt = (UBYTE *)((ULONG)g_pBitmapHelper2->Planes[0] + (48 * YTEXTCOORDINATE));
        g_pCustom->bltsize = 0x0814;

        next = 1;
        uwCycleCounter++;
        {
            static UBYTE ubTxtArrayOffset = 0;
            static UBYTE pTxtArray[] = {'N', '@', 'O', '@', 'F', '@', 'F', '@', 'L', '@', 'I', '@', 'G', '@', 'H', '@', 'T', '@', '@', '@',
                                        'I', '@', 'S', '@', '@', '@',
                                        'A', '@', '@', '@',
                                        'T', '@', 'R', '@', 'I', '@', 'B', '@', 'U', '@', 'T', '@', 'E', '@', '@', '@',
                                        'T', '@', 'O', '@', '@', '@',
                                        'N', '@', 'O', '@', 'I', '@', 'C', '@', 'E', '@', '@', '@', '@',
                                        'O', '@', 'F', '@', 'F', '@', 'E', '@', 'N', '@', 'C', '@', 'E', '@', '@', '@',
                                        'F', '@', 'A', '@', 'I', '@', 'R', '@', 'L', '@', 'I', '@', 'G', '@', 'H', '@', 'T', '@', '@', '@',
                                        /*'R', '@', 'E', '@', 'L', '@', 'E', '@', 'A', '@', 'S', '@', 'E', '@', 'D', '@', '@', '@',
                                        'A', '@', 'T', '@', '@', '@',*/
                                        'F', '@', 'O', '@', 'R', '@', '@', '@', '@',
                                        'F', '@', 'L', '@', 'A', '@', 'S', '@', 'H', '@', 'P', '@', 'A', '@', 'R', '@', 'T', '@', 'Y', '@', '@', '@',
                                        ']', '@', '[',  '@', ']', '@', '\\',  '@', '@', '@', '@',    
                                        '@', '@', '@', '@', '@', '@', '@', '@'};
            UBYTE *pPtrTxtArray = pTxtArray + ubTxtArrayOffset;
            ubTxtArrayOffset++;

            if (ubTxtArrayOffset >= sizeof(pTxtArray))
            {
                ubTxtArrayOffset = sizeof(pTxtArray) - 1; // repeat last char forever
            }

            if (uwCycleCounter > 10)
                uwCycleCounter = 10;

            printTxtChar(40, *pPtrTxtArray);
            pPtrTxtArray--;
        }

#ifdef ACE_DEBUG
        logWrite("UpdateCamera2 full cycle detected: %u\n", uwCycleCounter);
#endif
    }

    ubLastPosition = bX;

   /* blitWait();
        g_pCustom->bltcon0 = 0x09F0;
        g_pCustom->bltcon1 = 0x0000;
        g_pCustom->bltafwm = 0xFFFF;
        g_pCustom->bltalwm = 0xFFFF;
        g_pCustom->bltamod = 0x0008;
        g_pCustom->bltdmod = 0x0000;
        g_pCustom->bltapt = (UBYTE *)((ULONG)g_pBitmapHelper1->Planes[0] + 4 + (48 * YTEXTCOORDINATE));
        g_pCustom->bltdpt = (UBYTE *)((ULONG)g_pBitmapHelperTmp->Planes[0]);
        g_pCustom->bltsize = 0x0814;
        // copy to g_pBitmapHelperTmp

        blitWait();
        g_pCustom->bltcon0 = 0x09F0;
        g_pCustom->bltcon1 = 0x0000;
        g_pCustom->bltafwm = 0xFFFF;
        g_pCustom->bltalwm = 0xFFFF;
        g_pCustom->bltamod = 0x0008;
        g_pCustom->bltdmod = 0x0000;
        g_pCustom->bltapt = (UBYTE *)((ULONG)g_pBitmapHelper2->Planes[0] + 4 + (48 * YTEXTCOORDINATE));
        g_pCustom->bltdpt = (UBYTE *)((ULONG)g_pBitmapHelperTmp->Planes[1]);
        g_pCustom->bltsize = 0x0814;
        blitWait();
        vPortWaitForEnd(s_pVpMain);*/
}

void blitBitPlane(UBYTE *pSrc, UBYTE *pDst)
{
    blitWait();
    g_pCustom->bltcon0 = 0x09f0;
    g_pCustom->bltcon1 = 0x0000;
    g_pCustom->bltafwm = 0xFFFF;
    g_pCustom->bltalwm = 0xFFFF;
    g_pCustom->bltamod = 0x0000;
    g_pCustom->bltdmod = 0x0000;
    g_pCustom->bltapt = pSrc + VBARHEIGHT * 48;
    g_pCustom->bltdpt = pDst + VBARHEIGHT * 48;
    g_pCustom->bltsize = 0x0C58;
}

void setSpriteBallsCopperlist()
{
    tCopList *pCopList = s_pMainBuffer->sCommon.pVPort->pView->pCopList;
    tCopCmd *pCmdListBack = &pCopList->pBackBfr->pList[s_uwCopRawOffs];
    tCopCmd *pCmdListFront = &pCopList->pFrontBfr->pList[s_uwCopRawOffs];
    UBYTE ubCopIndex = ubCopSpritesColorIndex;

    copSetMoveBackAndFront(&g_pCustom->color[16], 0x000F);
    copSetMoveBackAndFront(&g_pCustom->color[17], 0x0edd);
    copSetMoveBackAndFront(&g_pCustom->color[18], 0x0922);
    copSetMoveBackAndFront(&g_pCustom->color[19], 0x0b77);

    // Sprites 2 and 3
    copSetMoveBackAndFront(&g_pCustom->color[20], 0x000F);
    copSetMoveBackAndFront(&g_pCustom->color[21], 0x0edd);
    copSetMoveBackAndFront(&g_pCustom->color[22], 0x0922);
    copSetMoveBackAndFront(&g_pCustom->color[23], 0x0b77);

    // Sprites 4 and 5
    copSetMoveBackAndFront(&g_pCustom->color[24], 0x000F);
    copSetMoveBackAndFront(&g_pCustom->color[25], 0x0edd);
    copSetMoveBackAndFront(&g_pCustom->color[26], 0x0922);
    copSetMoveBackAndFront(&g_pCustom->color[27], 0x0b77);

    // Sprites 6 and 7 (7 is unused)
    copSetMoveBackAndFront(&g_pCustom->color[28], 0x000F);
    copSetMoveBackAndFront(&g_pCustom->color[29], 0x0edd);
    copSetMoveBackAndFront(&g_pCustom->color[30], 0x0922);
    copSetMoveBackAndFront(&g_pCustom->color[31], 0x0b77);
    // Sprites colors end
}

void setSpriteBallsCopperlistBack()
{
    tCopList *pCopList = s_pMainBuffer->sCommon.pVPort->pView->pCopList;
    tCopCmd *pCmdListBack = &pCopList->pBackBfr->pList[s_uwCopRawOffs];
    //tCopCmd *pCmdListFront = &pCopList->pFrontBfr->pList[s_uwCopRawOffs];
    UBYTE ubCopIndex = ubCopSpritesColorIndex;

    copSetMoveBack(&g_pCustom->color[16], 0x000F);
    copSetMoveBack(&g_pCustom->color[17], 0x0edd);
    copSetMoveBack(&g_pCustom->color[18], 0x0922);
    copSetMoveBack(&g_pCustom->color[19], 0x0b77);

    // Sprites 2 and 3
    copSetMoveBack(&g_pCustom->color[20], 0x000F);
    copSetMoveBack(&g_pCustom->color[21], 0x0edd);
    copSetMoveBack(&g_pCustom->color[22], 0x0922);
    copSetMoveBack(&g_pCustom->color[23], 0x0b77);

    // Sprites 4 and 5
    copSetMoveBack(&g_pCustom->color[24], 0x000F);
    copSetMoveBack(&g_pCustom->color[25], 0x0edd);
    copSetMoveBack(&g_pCustom->color[26], 0x0922);
    copSetMoveBack(&g_pCustom->color[27], 0x0b77);

    // Sprites 6 and 7 (7 is unused)
    copSetMoveBack(&g_pCustom->color[28], 0x000F);
    copSetMoveBack(&g_pCustom->color[29], 0x0edd);
    copSetMoveBack(&g_pCustom->color[30], 0x0922);
    copSetMoveBack(&g_pCustom->color[31], 0x0b77);
    // Sprites colors end
}
