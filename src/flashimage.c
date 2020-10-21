#include "flashimage.h"
#include <ace/managers/key.h>                   // Keyboard processing
#include <ace/managers/game.h>                  // For using gameExit
#include <ace/managers/system.h>                // For systemUnuse and systemUse
#include <ace/managers/viewport/simplebuffer.h> // Simple buffer
#include <ace/managers/blit.h>

#include "../include/main.h"

#include "../include/medioeval_gal_LORES_16.h"
#include "../include/medioeval_gal_LORES_16_plt.h"

#define BITPLANES 4
#define SPEED 8

// All variables outside fns are global - can be accessed in any fn
// Static means here that given var is only for this file, hence 's_' prefix
// You can have many variables with same name in different files and they'll be
// independent as long as they're static
// * means pointer, hence 'p' prefix
static tView *s_pView;    // View containing all the viewports
static tVPort *s_pVpMain; // Viewport for playfield
static tSimpleBufferManager *s_pMainBuffer;
static UWORD *p_uwPalette;
//tView * g_tViewLateDestroy=NULL;

UWORD paletteColorInc(UWORD uwFullColor, UBYTE ubLevel);
void copyToMainBpl(const unsigned char *, const UBYTE, const UBYTE);

UWORD g_uwColors[] = {0x0AAA, 0x0444};

void flashimageGsCreate(void)
{
  systemSetDma(DMAB_SPRITE, 0);

  // Create a view - first arg is always zero, then it's option-value
  s_pView = viewCreate(0,
                       TAG_VIEW_GLOBAL_CLUT, 1, // Same Color LookUp Table for all viewports
                       TAG_END);                // Must always end with TAG_END or synonym: TAG_DONE

  // Now let's do the same for main playfield
  s_pVpMain = vPortCreate(0,
                          TAG_VPORT_VIEW, s_pView,
                          TAG_VPORT_BPP, 4, // 2 bits per pixel, 4 colors
                          // We won't specify height here - viewport will take remaining space.
                          TAG_END);
  s_pMainBuffer = simpleBufferCreate(0,
                                     TAG_SIMPLEBUFFER_VPORT, s_pVpMain, // Required: parent viewport
                                     TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR,
                                     TAG_END);

  /*p_uwPalette = (UWORD *)medioeval_gal_LORES_16_plt_data;
  UBYTE ubCount = 0;
  for (ubCount = 0; ubCount < 16; ubCount++)
  {
    s_pVpMain->pPalette[ubCount] = *p_uwPalette;
    p_uwPalette++;
  }*/

  UBYTE ubCount = 0;
  for (ubCount = 0; ubCount < 16; ubCount++)
    s_pVpMain->pPalette[ubCount] = 0x0000;

  // We don't need anything from OS anymore
  //systemUnuse();

  copyToMainBpl(medioeval_gal_LORES_16_data, 0, 4);

  // Load the view
  viewLoad(s_pView);
}

void flashimageGsLoop(void)
{
  static ULONG ulFrame = 0;
  static UBYTE ubLevel = 0;

  // This will loop forever until you "pop" or change gamestate
  // or close the game
  if (keyCheck(KEY_ESCAPE))
  {
    gameExit();
    return;
  }
  //if (keyUse(KEY_Q))
  if ((ulFrame % SPEED) == 0)
  {

    UBYTE ubCount = 0;
    p_uwPalette = (UWORD *)medioeval_gal_LORES_16_plt_data;
    for (ubCount = 0; ubCount < 16; ubCount++)
    {
      UWORD newColor = paletteColorInc(*p_uwPalette, ubLevel);

      s_pVpMain->pPalette[ubCount] = newColor;
      p_uwPalette++;
    }
    ubLevel++;
  }
  viewUpdateCLUT(s_pView);
  vPortWaitForEnd(s_pVpMain);
  if (ubLevel >= 16)
  {
    myChangeState(3);
  }
  ulFrame++;
}

void flashimageGsDestroy(void)
{
  // Cleanup when leaving this gamestate
  //systemUse();

  // This will also destroy all associated viewports and viewport managers
  //viewDestroy(s_pView);
  g_tViewLateDestroy = (void *)s_pView;
}

UWORD paletteColorInc(UWORD uwFullColor, UBYTE ubLevel)
{
  UBYTE r, g, b;

  if (ubLevel == 0)
    return 0x0FFF;
  if (ubLevel > 16)
    return uwFullColor;

  r = (uwFullColor >> 8) & 0xF;
  g = (uwFullColor >> 4) & 0xF;
  b = (uwFullColor)&0xF;

  // Dim color
  UBYTE ubExtra = (15 - r) * ubLevel;
  ubExtra = ubExtra >> 4;
  r = 15 - ubExtra;
  //logWrite("r color is : %x\n", r);

  ubExtra = (15 - g) * ubLevel;
  ubExtra = ubExtra >> 4;
  g = 15 - ubExtra;

  ubExtra = (15 - b) * ubLevel;
  ubExtra = ubExtra >> 4;
  b = 15 - ubExtra;

  // Output
  return (r << 8) | (g << 4) | b;
}

// Function to copy data to a main bitplane
// Pass ubMaxBitplanes = 0 to use all available bitplanes in the bitmap
void copyToMainBpl(const unsigned char *pData, const UBYTE ubSlot, const UBYTE ubMaxBitplanes)
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
    g_pCustom->bltapt = (UBYTE *)((ULONG)&pData[40 * 256 * ubBitplaneCounter]);
    g_pCustom->bltdpt = (UBYTE *)((ULONG)s_pMainBuffer->pBack->Planes[ubBitplaneCounter] + (40 * ubSlot));
    g_pCustom->bltsize = 0x4014;
    if (ubMaxBitplanes > 0 && ubBitplaneCounter + 1 >= ubMaxBitplanes)
      return;
  }
  return;
}
