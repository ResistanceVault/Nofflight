#include "../include/resistancelogo.h"

#include <ace/managers/game.h>                  // For using gameClose
#include <ace/managers/system.h>                // For systemUnuse and systemUse
#include <ace/managers/viewport/simplebuffer.h> // Simple buffer
#include <ace/utils/palette.h>
#include <ace/managers/blit.h>
#include <ace/managers/key.h> // Keyboard processing

#include <proto/exec.h>
#include <proto/dos.h>

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

void resistanceLogoGsCreate(void)
{
    ULONG ulRawSize = (SimpleBufferTestGetRawCopperlistInstructionCount(5) + 3);

    s_pView = viewCreate(0,
                         TAG_VIEW_GLOBAL_CLUT, 1, // Same Color LookUp Table for all viewports
                         TAG_VIEW_COPLIST_MODE, VIEW_COPLIST_MODE_RAW,
                         TAG_VIEW_COPLIST_RAW_COUNT, ulRawSize,
                         TAG_END); // Must always end with TAG_END or synonym: TAG_DONE

    s_pVpMain = vPortCreate(0,
                            TAG_VPORT_VIEW, s_pView,
                            TAG_VPORT_BPP, 5, // 5 bits per pixel, 32 colors

                            TAG_END);
    s_pMainBuffer = simpleBufferCreate(0,
                                       TAG_SIMPLEBUFFER_VPORT, s_pVpMain, // Required: parent viewport
                                       TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR,
                                       TAG_SIMPLEBUFFER_COPLIST_OFFSET, 0,

                                       TAG_END);

    UWORD s_uwCopRawOffs = SimpleBufferTestGetRawCopperlistInstructionCount(5);

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

    s_pVpMain->pPalette[16] = 0x0FEC;
    s_pVpMain->pPalette[17] = 0x0FD9;
    s_pVpMain->pPalette[18] = 0x0EC8;
    s_pVpMain->pPalette[19] = 0x0DB7;
    s_pVpMain->pPalette[20] = 0x0CA6;
    s_pVpMain->pPalette[21] = 0x0B95;
    s_pVpMain->pPalette[22] = 0x0A85;
    s_pVpMain->pPalette[23] = 0x0974;

    s_pVpMain->pPalette[24] = 0x0863;
    s_pVpMain->pPalette[25] = 0x0753;
    s_pVpMain->pPalette[26] = 0x0642;
    s_pVpMain->pPalette[27] = 0x0531;
    s_pVpMain->pPalette[28] = 0x0430;
    s_pVpMain->pPalette[29] = 0x0321;
    s_pVpMain->pPalette[30] = 0x0211;
    s_pVpMain->pPalette[31] = 0x0100;

    //tCopList *pCopList = s_pMainBuffer->sCommon.pVPort->pView->pCopList;
    //tCopCmd *pCmdListBack = &pCopList->pBackBfr->pList[s_uwCopRawOffs];
    //tCopCmd *pCmdListFront = &pCopList->pFrontBfr->pList[s_uwCopRawOffs];

    pBuffer = AllocMem(51200, MEMF_CHIP);

    // We don't need anything from OS anymore
    systemUnuse();

    // Load the view
    viewLoad(s_pView);

// Start music
#ifdef MUSIC_ON

    mt_init(Dirty_Tricks_data);
    systemSetInt(INTB_VERTB, interruptHandlerMusic, 0);

#endif

#if 0
    systemUseNoInts2();
    Execute("copy data/resistance_final.plt to ram:resistance_final.plt", 0, 0);
    Execute("copy data/resistance_final.raw to ram:resistance_final.raw", 0, 0);
    systemUnuseNoInts2();



    static BPTR file = 0;
    systemUseNoInts();
    file = Open("ram:resistance_final.raw", MODE_OLDFILE);
    Read(file, buf, 200);
    Close(file);
    systemUnuseNoInts();
#endif
}

//char buf[51200];

void resistanceLogoGsLoop(void)
{
    static int iFrameNo = 0;
    static BYTE bDimCounter = 0;
    static BYTE bDimCounter2 = 15;

    /*if (keyCheck(KEY_ESCAPE))
    {
        gameExit();
    }*/

    static int copy = 0;
    if (!copy)
    {
#if 1
        systemUseNoInts2();
        Execute("copy data/resistance_final.raw to ram:resistance_final.raw", 0, 0);
        copy = 1;
        systemUnuseNoInts2();
        //sleep(10);
        //return ;
        //#else

#endif
    }

    else if (copy == 1)
    {
        copy = 2;
        BPTR file = 0;
        systemUseNoInts();
        file = Open("ram:resistance_final.raw", MODE_OLDFILE);
        if (!file)
            gameExit();
        Read(file, pBuffer, 51200);
        Close(file);
        unlink("ram:resistance_final.raw");
        systemUnuseNoInts();
    }
    else if (copy == 2)
    {
        //copyToMainBpl((unsigned char*)buf, 0, 0);
        UBYTE *lol = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[0]);
        memcpy(lol, pBuffer, 10240);
        lol = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[1]);
        memcpy(lol, pBuffer + 10240, 10240);

        lol = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[2]);
        memcpy(lol, pBuffer + 10240 * 2, 10240);

        lol = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[3]);
        memcpy(lol, pBuffer + 10240 * 3, 10240);

        lol = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[4]);
        memcpy(lol, pBuffer + 10240 * 4, 10240);

        copy = 3;
    }

    if (bDimCounter <= 15)
    {
        /*s_pVpMain->pPalette[1] = paletteColorDim(0x0600, bDimCounter);
        s_pVpMain->pPalette[2] = paletteColorDim(0x0300, bDimCounter);
        s_pVpMain->pPalette[3] = paletteColorDim(0x0900, bDimCounter);
        s_pVpMain->pPalette[4] = paletteColorDim(0x0200, bDimCounter);
        s_pVpMain->pPalette[5] = paletteColorDim(0x0800, bDimCounter);
        s_pVpMain->pPalette[6] = paletteColorDim(0x0500, bDimCounter);
        s_pVpMain->pPalette[7] = paletteColorDim(0x0b00, bDimCounter);*/

        s_pVpMain->pPalette[0] = paletteColorDim(0x0000, bDimCounter);
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
        s_pVpMain->pPalette[15] = paletteColorDim(0x0011, bDimCounter);

        s_pVpMain->pPalette[16] = paletteColorDim(0x0FEC, bDimCounter);
        s_pVpMain->pPalette[17] = paletteColorDim(0x0FD9, bDimCounter);
        s_pVpMain->pPalette[18] = paletteColorDim(0x0EC8, bDimCounter);
        s_pVpMain->pPalette[19] = paletteColorDim(0x0DB7, bDimCounter);
        s_pVpMain->pPalette[20] = paletteColorDim(0x0CA6, bDimCounter);
        s_pVpMain->pPalette[21] = paletteColorDim(0x0B95, bDimCounter);
        s_pVpMain->pPalette[22] = paletteColorDim(0x0A85, bDimCounter);
        s_pVpMain->pPalette[23] = paletteColorDim(0x0974, bDimCounter);

        s_pVpMain->pPalette[24] = paletteColorDim(0x0863, bDimCounter);
        s_pVpMain->pPalette[25] = paletteColorDim(0x0753, bDimCounter);
        s_pVpMain->pPalette[26] = paletteColorDim(0x0642, bDimCounter);
        s_pVpMain->pPalette[27] = paletteColorDim(0x0531, bDimCounter);
        s_pVpMain->pPalette[28] = paletteColorDim(0x0430, bDimCounter);
        s_pVpMain->pPalette[29] = paletteColorDim(0x0321, bDimCounter);
        s_pVpMain->pPalette[30] = paletteColorDim(0x0211, bDimCounter);
        s_pVpMain->pPalette[31] = paletteColorDim(0x0100, bDimCounter);

        if ((iFrameNo % 10) == 0)
            bDimCounter++;
    }

    if (iFrameNo > 400)
    {
        if (bDimCounter2 >= 0)
        {
            /*s_pVpMain->pPalette[1] = paletteColorDim(0x0600, bDimCounter2);
            s_pVpMain->pPalette[2] = paletteColorDim(0x0300, bDimCounter2);
            s_pVpMain->pPalette[3] = paletteColorDim(0x0900, bDimCounter2);
            s_pVpMain->pPalette[4] = paletteColorDim(0x0200, bDimCounter2);
            s_pVpMain->pPalette[5] = paletteColorDim(0x0800, bDimCounter2);
            s_pVpMain->pPalette[6] = paletteColorDim(0x0500, bDimCounter2);
            s_pVpMain->pPalette[7] = paletteColorDim(0x0b00, bDimCounter2);*/

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

            s_pVpMain->pPalette[16] = paletteColorDim(0x0FEC, bDimCounter2);
            s_pVpMain->pPalette[17] = paletteColorDim(0x0FD9, bDimCounter2);
            s_pVpMain->pPalette[18] = paletteColorDim(0x0EC8, bDimCounter2);
            s_pVpMain->pPalette[19] = paletteColorDim(0x0DB7, bDimCounter2);
            s_pVpMain->pPalette[20] = paletteColorDim(0x0CA6, bDimCounter2);
            s_pVpMain->pPalette[21] = paletteColorDim(0x0B95, bDimCounter2);
            s_pVpMain->pPalette[22] = paletteColorDim(0x0A85, bDimCounter2);
            s_pVpMain->pPalette[23] = paletteColorDim(0x0974, bDimCounter2);

            s_pVpMain->pPalette[24] = paletteColorDim(0x0863, bDimCounter2);
            s_pVpMain->pPalette[25] = paletteColorDim(0x0753, bDimCounter2);
            s_pVpMain->pPalette[26] = paletteColorDim(0x0642, bDimCounter2);
            s_pVpMain->pPalette[27] = paletteColorDim(0x0531, bDimCounter2);
            s_pVpMain->pPalette[28] = paletteColorDim(0x0430, bDimCounter2);
            s_pVpMain->pPalette[29] = paletteColorDim(0x0321, bDimCounter2);
            s_pVpMain->pPalette[30] = paletteColorDim(0x0211, bDimCounter2);
            s_pVpMain->pPalette[31] = paletteColorDim(0x0100, bDimCounter2);

            if ((iFrameNo % 10) == 0)
                bDimCounter2--;
        }

        //gameExit();return ;
    }
    if (bDimCounter2 < 0)
    {
        stateChange(g_pGameStateManager, g_pGameStates[1]);
        //gameExit();
    }
    if (copy >= 3)
        iFrameNo++;
    viewUpdateCLUT(s_pView);
    vPortWaitForEnd(s_pVpMain);
}

void resistanceLogoGsDestroy(void)
{
    FreeMem(pBuffer, 51200);


    // This will also destroy all associated viewports and viewport managers
    viewDestroy(s_pView);

}

