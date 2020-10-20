
#include <ace/managers/state.h>
#include <ace/utils/extview.h>

#include <proto/exec.h>
#include <proto/dos.h>

#define GAME_STATE_COUNT 5

extern tStateManager *g_pGameStateManager;
extern tState *g_pGameStates[];

int g_iChan3Played;
int g_iChan4Played;

UBYTE* g_pBuffer;

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
	