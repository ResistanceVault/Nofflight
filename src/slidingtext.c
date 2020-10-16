#include "slidingtext.h"
#include "../include/main.h"

#include <ace/managers/key.h>                   // Keyboard processing
#include <ace/managers/game.h>                  // For using gameClose
#include <ace/managers/system.h>                // For systemUnuse and systemUse
#include <ace/managers/viewport/simplebuffer.h> // Simple buffer
#include <ace/managers/blit.h>

#include "fonts.h"

#include <proto/exec.h>
#include <proto/dos.h>

static tView *s_pView;    // View containing all the viewports
static tVPort *s_pVpMain; // Viewport for playfield
static tSimpleBufferManager *s_pMainBuffer;
static char g_cPhrase[] = {"THANKS TO OFFENCE AND ATLANTIS FOR THEIR GOATLIGHT DEMO, AWESOME!!!!        \n"};
void printCharToRight(char);
void scorri();
BPTR file2;

void slidingTxtGsCreate(void)
{
 /* systemUseNoInts2();
    file2 = Open("data/resistance_final.raw", MODE_OLDFILE);
     Close(file2);
    systemUnuseNoInts2();*/
  // Create a view - first arg is always zero, then it's option-value
  s_pView = viewCreate(0,
                       TAG_VIEW_GLOBAL_CLUT, 1, // Same Color LookUp Table for all viewports
                       TAG_END);                // Must always end with TAG_END or synonym: TAG_DONE

  // Now let's do the same for main playfield
  s_pVpMain = vPortCreate(0,
                          TAG_VPORT_VIEW, s_pView,
                          TAG_VPORT_BPP, 1, // 2 bits per pixel, 4 colors
                          // We won't specify height here - viewport will take remaining space.
                          TAG_END);
  s_pMainBuffer = simpleBufferCreate(0,
                                     TAG_SIMPLEBUFFER_VPORT, s_pVpMain, // Required: parent viewport
                                     TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR,
                                     TAG_END);

  // Since we've set up global CLUT, palette will be loaded from first viewport
  // Colors are 0x0RGB, each channel accepts values from 0 to 15 (0 to F).
  s_pVpMain->pPalette[0] = 0x0000; // First color is also border color
  s_pVpMain->pPalette[1] = 0x0888; // Gray
  s_pVpMain->pPalette[2] = 0x0800; // Red - not max, a bit dark
  s_pVpMain->pPalette[3] = 0x0008; // Blue - same brightness as red

  //systemUnuse();

  // Load the view
  viewLoad(s_pView);

  
}

void slidingTxtGsLoop(void)
{
  

  if (keyCheck(KEY_C))
  {
    stateChange(g_pGameStateManager, g_pGameStates[3]);
  }
  else
  {
    static UBYTE ubCounter = 0;
    if (ubCounter == 0)
    {
      static char *s_pPhrasePointer = &g_cPhrase[0];
      printCharToRight(*s_pPhrasePointer);
      s_pPhrasePointer++;
      if (*s_pPhrasePointer == '\n')
      {
        s_pPhrasePointer = &g_cPhrase[0];
        
        //stateChange(g_pGameStateManager, g_pGameStates[3]);
        myChangeState(3);
      }
    }
    scorri();

    ubCounter++;
    if (ubCounter > 15)
      ubCounter = 0;
  }
  vPortWaitForEnd(s_pVpMain);
}

void slidingTxtGsDestroy(void)
{

  // This will also destroy all associated viewports and viewport managers
 // viewDestroy(s_pView);
 g_tViewLateDestroy = (void*)s_pView;
}

void printCharToRight(char carToPrint)
{
  int carToPrintOffset = ((int)carToPrint - 0x20) * 40; // Prima tolgo 0x20 perché il primo carattere nel file di ramjam è uno spazio, poi moltiplico per 40 bytes perché i caratteri sono 2 bytes*20 righe
  UBYTE *firstBitPlane = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[0]);

  // vogliamo stampare all'estrema destra quindi aggiungiamo 38
  firstBitPlane += 38;

  for (int i = 0; i < 20; i++)
  {
    *firstBitPlane = (UBYTE)fonts_data[carToPrintOffset];
    firstBitPlane++;
    *firstBitPlane = (UBYTE)fonts_data[carToPrintOffset + 1];
    firstBitPlane++;
    carToPrintOffset += 2;
    firstBitPlane += 38;
  }
}

void scorri()
{

  blitWait();

  g_pCustom->bltcon0 = 0x19f0;
  g_pCustom->bltcon1 = 0x0002;

  g_pCustom->bltafwm = 0xffff;
  g_pCustom->bltalwm = 0x7fff;

  g_pCustom->bltamod = 0x0000;
  g_pCustom->bltbmod = 0x0000;
  g_pCustom->bltcmod = 0x0000;
  g_pCustom->bltdmod = 0x0000;

  UBYTE *firstBitPlane = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[0]) + ((20 * 20 - 1) * 2);

  g_pCustom->bltdpt = firstBitPlane;
  g_pCustom->bltapt = firstBitPlane;

  g_pCustom->bltsize = 0x0514;
}
