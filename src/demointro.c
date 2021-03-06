#include "../include/demointro.h"

#include <ace/managers/game.h>                  // For using gameClose
#include <ace/managers/system.h>                // For systemUnuse and systemUse
#include <ace/managers/viewport/simplebuffer.h> // Simple buffer
#include <ace/utils/palette.h>
#include <ace/managers/blit.h>

// Assets
#include "../include/main.h"
// #include "../include/VampireItalialogo.h"

static tVPort *s_pVpMain; // Viewport for playfield
static tSimpleBufferManager *s_pMainBuffer;

static tView *s_pView; // View containing all the viewports

static tVPort *s_pVpMain; // Viewport for playfield
static tSimpleBufferManager *s_pMainBuffer;
void copyToMainBplIntro(const unsigned char *, const UBYTE, const UBYTE);

void introGsCreate(void)
{
    // Create a view - first arg is always zero, then it's option-value
    s_pView = viewCreate(0,
                         TAG_VIEW_GLOBAL_CLUT, 1, // Same Color LookUp Table for all viewports
                         TAG_END);                // Must always end with TAG_END or synonym: TAG_DONE

    // Now let's do the same for main playfield
    s_pVpMain = vPortCreate(0,
                            TAG_VPORT_VIEW, s_pView,
                            TAG_VPORT_BPP, 4, // 4 bits per pixel, 16 colors
                                              // We won't specify height here - viewport will take remaining space.
                            TAG_END);
    s_pMainBuffer = simpleBufferCreate(0,
                                       TAG_SIMPLEBUFFER_VPORT, s_pVpMain, // Required: parent viewport
                                       TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR,
                                       TAG_END);

    // Since we've set up global CLUT, palette will be loaded from first viewport
    // Colors are 0x0RGB, each channel accepts values from 0 to 15 (0 to F).
    /* s_pVpMain->pPalette[0] = 0x0000; // First color is also border color
    s_pVpMain->pPalette[1] = 0x0600; // Gray
    s_pVpMain->pPalette[2] = 0x0300; // Red - not max, a bit dark
    s_pVpMain->pPalette[3] = 0x0900; // Blue - same brightness as red
    s_pVpMain->pPalette[4] = 0x0200;
    s_pVpMain->pPalette[5] = 0x0800;
    s_pVpMain->pPalette[6] = 0x0500;
    s_pVpMain->pPalette[7] = 0x0b00;*/

    s_pVpMain->pPalette[0] = 0x0000;
    s_pVpMain->pPalette[1] = 0x0000;
    s_pVpMain->pPalette[2] = 0x0000;
    s_pVpMain->pPalette[3] = 0x0000;
    s_pVpMain->pPalette[4] = 0x0000;
    s_pVpMain->pPalette[5] = 0x0000;
    s_pVpMain->pPalette[6] = 0x0000;
    s_pVpMain->pPalette[7] = 0x0000;


    UBYTE* vampireitalialogo_data_fast = g_pBuffer;
    copyToMainBplIntro(vampireitalialogo_data_fast, 0, 4);

    // We don't need anything from OS anymore
    systemUnuse();
    //Execute("RUN casentino2020");

    // Load the view
    viewLoad(s_pView);
}

void introGsLoop(void)
{
    static int iFrameNo = 0;
    static BYTE bDimCounter = 0;
    static BYTE bDimCounter2 = 15;
    if (bDimCounter <= 15)
    {
        #ifdef VAMPIREITALIALOGO

        s_pVpMain->pPalette[1] = paletteColorDim(0x0600, bDimCounter);
        s_pVpMain->pPalette[2] = paletteColorDim(0x0300, bDimCounter);
        s_pVpMain->pPalette[3] = paletteColorDim(0x0900, bDimCounter);
        s_pVpMain->pPalette[4] = paletteColorDim(0x0200, bDimCounter);
        s_pVpMain->pPalette[5] = paletteColorDim(0x0800, bDimCounter);
        s_pVpMain->pPalette[6] = paletteColorDim(0x0500, bDimCounter);
        s_pVpMain->pPalette[7] = paletteColorDim(0x0b00, bDimCounter);
        #else
        s_pVpMain->pPalette[0] = paletteColorDim(0x0134, bDimCounter);
        s_pVpMain->pPalette[1] = paletteColorDim(0x0012, bDimCounter);
        s_pVpMain->pPalette[2] = paletteColorDim(0x0223, bDimCounter);
        s_pVpMain->pPalette[3] = paletteColorDim(0x0334, bDimCounter);
        s_pVpMain->pPalette[4] = paletteColorDim(0x0534, bDimCounter);
        s_pVpMain->pPalette[5] = paletteColorDim(0x0645, bDimCounter);
        s_pVpMain->pPalette[6] = paletteColorDim(0x0756, bDimCounter);

        s_pVpMain->pPalette[7] = paletteColorDim(0x0668, bDimCounter);
        s_pVpMain->pPalette[8] = paletteColorDim(0x0679, bDimCounter);
        s_pVpMain->pPalette[9] = paletteColorDim(0x0689, bDimCounter);
        s_pVpMain->pPalette[10] = paletteColorDim(0x0899, bDimCounter);
        s_pVpMain->pPalette[11] = paletteColorDim(0x0AAA, bDimCounter);
        s_pVpMain->pPalette[12] = paletteColorDim(0x0BBB, bDimCounter);
        s_pVpMain->pPalette[13] = paletteColorDim(0x0CDD, bDimCounter);
        s_pVpMain->pPalette[14] = paletteColorDim(0x0DEE, bDimCounter);
        s_pVpMain->pPalette[15] = paletteColorDim(0x0FFF, bDimCounter);

        #endif

        if ((iFrameNo % 10) == 0)
            bDimCounter++;
    }

    if (iFrameNo > 300)
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

            s_pVpMain->pPalette[0] = paletteColorDim(0x0134, bDimCounter2);
        s_pVpMain->pPalette[1] = paletteColorDim(0x0012, bDimCounter2);
        s_pVpMain->pPalette[2] = paletteColorDim(0x0223, bDimCounter2);
        s_pVpMain->pPalette[3] = paletteColorDim(0x0334, bDimCounter2);
        s_pVpMain->pPalette[4] = paletteColorDim(0x0534, bDimCounter2);
        s_pVpMain->pPalette[5] = paletteColorDim(0x0645, bDimCounter2);
        s_pVpMain->pPalette[6] = paletteColorDim(0x0756, bDimCounter2);

        s_pVpMain->pPalette[7] = paletteColorDim(0x0668, bDimCounter2);
        s_pVpMain->pPalette[8] = paletteColorDim(0x0679, bDimCounter2);
        s_pVpMain->pPalette[9] = paletteColorDim(0x0689, bDimCounter2);
        s_pVpMain->pPalette[10] = paletteColorDim(0x0899, bDimCounter2);
        s_pVpMain->pPalette[11] = paletteColorDim(0x0AAA, bDimCounter2);
        s_pVpMain->pPalette[12] = paletteColorDim(0x0BBB, bDimCounter2);
        s_pVpMain->pPalette[13] = paletteColorDim(0x0CDD, bDimCounter2);
        s_pVpMain->pPalette[14] = paletteColorDim(0x0DEE, bDimCounter2);
        s_pVpMain->pPalette[15] = paletteColorDim(0x0FFF, bDimCounter2);

            if ((iFrameNo % 10) == 0)
                bDimCounter2--;
        }

        //gameExit();return ;
    }
    if (bDimCounter2 < 0)
    {
        myChangeState(1);
        return ;
    }
    iFrameNo++;
    viewUpdateCLUT(s_pView);
    vPortWaitForEnd(s_pVpMain);
}

void introGsDestroy(void)
{
    unLoadRes();
    // This will also destroy all associated viewports and viewport managers
    viewDestroy(s_pView);
}

// Function to copy data to a main bitplane
// Pass ubMaxBitplanes = 0 to use all available bitplanes in the bitmap
void copyToMainBplIntro(const unsigned char *pData, const UBYTE ubSlot, const UBYTE ubMaxBitplanes)
{
    UBYTE ubBitplaneCounter;
    for (ubBitplaneCounter = 0; ubBitplaneCounter < s_pMainBuffer->pBack->Depth; ubBitplaneCounter++)
    {
        blitWait();
        g_pCustom->bltcon0 = 0x09F0;
        g_pCustom->bltcon1 = 0x0000;
        g_pCustom->bltafwm = 0xFFFF;
        g_pCustom->bltalwm = 0xFFFF;
        g_pCustom->bltamod = 0x0000;
        g_pCustom->bltbmod = 0x0000;
        g_pCustom->bltcmod = 0x0000;
        g_pCustom->bltdmod = 0x0000;
        g_pCustom->bltapt = (UBYTE *)((ULONG)&pData[40 * 224 * ubBitplaneCounter]);
        g_pCustom->bltdpt = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[ubBitplaneCounter] + (40 * ubSlot));
        g_pCustom->bltsize = 0x3814;
        if (ubMaxBitplanes > 0 && ubBitplaneCounter + 1 >= ubMaxBitplanes)
            return;
    }
    return;
}