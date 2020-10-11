#ifndef _ACE_PHYSICS_H_
#define _ACE_PHYSICS_H_

#include "ace/types.h"
#include "vectors.h"

typedef struct _tMover
{
    UBYTE ubSpriteIndex;
    v2d tLocation;
    v2d tVelocity;
    v2d tAccelleration;

    v2d tPrevLocation;

    fix16_t tMass;

    uint8_t ubLocked;
#ifdef TRAIL
    struct Queue* tQueue;
#endif
    //struct Queue *tQueue;
} tMover;

void spriteVectorInit(tMover* ,const UBYTE ,const int , const int , const fix16_t , const int ,const unsigned int );
void moverBounce(tMover* );
UBYTE moverMove(tMover);

UBYTE g_ubVBounceEnabled;
UBYTE g_ubHBounceEnabled;

inline static void spriteVectorApplyForce(tMover* pMover,v2d* pForce)
{
  // Force must be divided by mass because of Newton second law (A = F/M)
  // So the accelleration is Force/Mass og the object I am moving
  v2d tNewForce;
  v2d_div_s(&tNewForce,pForce,pMover->tMass);
  v2d_add(&pMover->tAccelleration,&pMover->tAccelleration,&tNewForce);
}
inline static void moverAddAccellerationToVelocity(tMover* pMover)
{
  v2d_add(&pMover->tVelocity,&pMover->tVelocity,&pMover->tAccelleration);
}

inline static void moverAddVelocityToLocation(tMover* pMover)
{

  // Save previous location before updating
  pMover->tPrevLocation.x = pMover->tLocation.x;
  pMover->tPrevLocation.y = pMover->tLocation.y;

  // Do the update
  v2d_add(&pMover->tLocation,&pMover->tLocation,&pMover->tVelocity);
}

inline static void spriteVectorResetAccelleration(tMover* pMover)
{
  pMover->tAccelleration.x=0;
  pMover->tAccelleration.y=0;
}
#endif