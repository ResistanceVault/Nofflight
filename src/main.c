
#include <ace/generic/main.h>
#include <ace/managers/key.h>
#include <ace/managers/state.h>

// Without it compiler will yell about undeclared gameGsCreate etc
#include "../include/main.h"
#include "../include/flash2021.h"
#include "../include/resistancelogo.h"
#include "../include/metaballschunky.h"
#include "../include/Dirty_Tricks.h"
//#include "player610.6.no_cia.bin.h"
//#include "testmod.p61.h"

tStateManager *g_pGameStateManager = 0;
tState *g_pGameStates[GAME_STATE_COUNT] = {0};

long mt_init(const unsigned char *);
void mt_music();
void mt_end();



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
    g_pCustom->intreq=(1<<INTB_VERTB); g_pCustom->intreq=(1<<INTB_VERTB);
    //p61Music();
    mt_music();
  }
    //chan3played();
}

void genericCreate(void)
{
  mt_init(Dirty_Tricks_data);
  //p61Init(testmod_data);

  // Here goes your startup code
  logWrite("Hello, Amiga!\n");
  keyCreate(); // We'll use keyboard
  g_pGameStateManager = stateManagerCreate();
  g_pGameStates[0] = stateCreate(resistanceLogoGsCreate, resistanceLogoGsLoop, resistanceLogoGsDestroy, 0, 0, 0);
  g_pGameStates[1] = stateCreate(gameGsCreate, gameGsLoop, gameGsDestroy, 0, 0, 0);
  g_pGameStates[2] = stateCreate(metaballsGsCreate, metaballsGsLoop, metaballsGsDestroy, 0, 0, 0);
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

  keyDestroy(); // We don't need it anymore
  logWrite("Goodbye, Amiga!\n");
}
