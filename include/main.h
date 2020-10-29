
#include <ace/managers/state.h>
#include <ace/utils/extview.h>

#include <proto/exec.h>
#include <proto/dos.h>

#define GAME_STATE_COUNT 6

extern tStateManager *g_pGameStateManager;
extern tState *g_pGameStates[];

int g_iChan3Played;
int g_iChan4Played;

UBYTE* g_pBuffer ;
ULONG g_ulBufferLength ;
UBYTE* LoadRes(ULONG ,char* );
void unLoadRes();

extern tView *g_tViewLateDestroy;

#define myChangeState(var)                                                         \
	if (g_pGameStateManager->pCurrent && g_pGameStateManager->pCurrent->cbDestroy) \
	{                                                                              \
		g_pGameStateManager->pCurrent->cbDestroy();                                \
	}                                                                              \
	g_pGameStateManager->pCurrent = g_pGameStates[var];                            \
	if (g_pGameStateManager->pCurrent && g_pGameStateManager->pCurrent->cbCreate)  \
	{                                                                              \
		g_pGameStateManager->pCurrent->cbCreate();                                 \
	}                                                                              
	