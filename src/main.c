
#include <ace/generic/main.h>
#include <ace/managers/key.h>
#include <ace/managers/state.h>

#include "../include/main.h"
#include "../include/flash2021.h"
#include "../include/demointro.h"
#include "../include/mivampiralogo.h"
#include "../include/metaballschunky.h"
#include "../include/goatblocks.h"
#include "slidingtext.h"
#include "flashimage.h"

#include "radiallineshiddenpart.h"
//#include "../include/chippy_wip_3.h"
//#include "../include/findaway.h"
//#include "../include/Dirty_Tricks.h"
//#include "player610.6.no_cia.bin.h"
//#include "testmod.p61.h"
//#include "../include/amazed_by_the_pokey.h"

tStateManager *g_pGameStateManager = 0;
tState *g_pGameStates[GAME_STATE_COUNT] = {0};

long mt_init(const unsigned char *);
void mt_music();
void mt_end();
int chan1played();
int chan2played();
int chan3played();
int chan4played();

/*int p61Init(const void* module) { // returns 0 if success, non-zero otherwise
	register volatile const void* _a0 __asm("a0") = module;
	register volatile const void* _a1 __asm("a1") = NULL;
	register volatile const void* _a2 __asm("a2") = NULL;
	register volatile const void* _a3 __asm("a3") = player;
	         register int         _d0 __asm("d0"); // return value
	__asm volatile (
		"movem.l %%d1-%%d7/%%a4-%%a6,-(%%sp)\n"
		"jsr 0(%%a3)\n"
		"movem.l (%%sp)+,%%d1-%%d7/%%a4-%%a6"
	: "=r" (_d0), "+rf"(_a0), "+rf"(_a1), "+rf"(_a2), "+rf"(_a3)
	:
	: "cc", "memory");
	return _d0;
}

void p61Music() {
	register volatile const void* _a3 __asm("a3") = player;
	register volatile const void* _a6 __asm("a6") = (void*)0xdff000;
	__asm volatile (
		"movem.l %%d0-%%d7/%%a0-%%a2/%%a4-%%a5,-(%%sp)\n"
		"jsr 4(%%a3)\n"
		"movem.l (%%sp)+,%%d0-%%d7/%%a0-%%a2/%%a4-%%a5"
	: "+rf"(_a3), "+rf"(_a6)
	:
	: "cc", "memory");
}*/

static void INTERRUPT interruptHandlerMusic2()
{
  if ((g_pCustom->intreqr >> 5) & 1U)
  {
    g_pCustom->intreq = (1 << INTB_VERTB);
    g_pCustom->intreq = (1 << INTB_VERTB);
    //p61Music();
    mt_music();
    g_iChan1Played = chan1played();
    g_iChan2Played = chan2played();
    g_iChan3Played = chan3played();
    g_iChan4Played = chan4played();
  }
  //chan3played();
}

void genericCreate(void)
{
  // Load first asset (resistance logo)
  /*systemUseNoInts2();
  BPTR file = Open((STRPTR)"data/resistance_final.raw", MODE_OLDFILE);
  if (!file)
    gameExit();
  g_pBuffer = AllocMem(51200, MEMF_CHIP);
  Read(file, g_pBuffer, 51200);
  Close(file);
  systemUnuseNoInts2();*/

  //LoadRes(51200,"data/resistance_final.raw");
  //LoadRes(26880, "data/VampireItalialogo.raw");

  LoadRes(62592, "data/amazed_by_pokey-fartfixed.mod");
  g_pMusic = memAlloc(62592, MEMF_CHIP);
  memcpy(g_pMusic,g_pBuffer,62592);
  unLoadRes();

  LoadRes(35840, "data/piston_logo.raw");

  // Init music
  //mt_init(findaway_data);
  //mt_init(chippy_wip_3_data);
  //p61Init(testmod_data);
  mt_init(g_pMusic);

  // Here goes your startup code
  keyCreate(); // We'll use keyboard
  g_pGameStateManager = stateManagerCreate();
  g_pGameStates[0] = stateCreate(introGsCreate, introGsLoop, introGsDestroy, 0, 0, 0);
  g_pGameStates[1] = stateCreate(gameGsCreate, gameGsLoop, gameGsDestroy, 0, 0, 0);
  g_pGameStates[2] = stateCreate(flashimageGsCreate, flashimageGsLoop, flashimageGsDestroy, 0, 0, 0);
  g_pGameStates[3] = stateCreate(metaballsGsCreate, metaballsGsLoop, metaballsGsDestroy, 0, 0, 0);
  g_pGameStates[4] = stateCreate(mivampiraLogoGsCreate, mivampiraLogoGsLoop, mivampiraLogoGsDestroy, 0, 0, 0);
  g_pGameStates[5] = stateCreate(radialLinesGsCreate, radialLinesGsLoop, radialLinesGsDestroy, 0, 0, 0);
  g_pGameStates[6] = stateCreate(goatblocksGsCreate, goatblocksGsLoop, goatblocksGsDestroy, 0, 0, 0);

  stateChange(g_pGameStateManager, g_pGameStates[0]);
  systemSetInt(INTB_VERTB, interruptHandlerMusic2, 0);
}

void genericProcess(void)
{
  // Here goes code done each game frame
  keyProcess();
  stateProcess(g_pGameStateManager);
}

void genericDestroy(void)
{
  // Here goes your cleanup code

  stateManagerDestroy(g_pGameStateManager);
  stateDestroy(g_pGameStates[0]);
  stateDestroy(g_pGameStates[1]);
  stateDestroy(g_pGameStates[2]);
  stateDestroy(g_pGameStates[3]);
  stateDestroy(g_pGameStates[4]);
  stateDestroy(g_pGameStates[5]);
  stateDestroy(g_pGameStates[6]);

  keyDestroy(); // We don't need it anymore
}

UBYTE *LoadRes(ULONG ulSize, char *pFile)
{
  BPTR file2;
  //g_pBuffer = AllocMem(ulSize, MEMF_CHIP);
  g_pBuffer = memAlloc(ulSize, MEMF_CHIP);
  if (g_pBuffer == NULL)
    return NULL;
  systemUseNoInts2();
  file2 = Open((CONST_STRPTR)pFile, MODE_OLDFILE);
  if (file2 == 0)
    gameExit();
  Read(file2, g_pBuffer, ulSize);
  Close(file2);
  systemUnuseNoInts2();
  g_ulBufferLength = ulSize;
  return g_pBuffer;
}

void unLoadRes()
{
  //FreeMem(g_pBuffer,g_ulBufferLength);
  memFree(g_pBuffer, g_ulBufferLength);
  g_ulBufferLength = 0;
}
