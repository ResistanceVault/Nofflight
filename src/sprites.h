#ifndef _ACE_SPRITES_H_
#define _ACE_SPRITES_H_

#include "ace/types.h"
#include "ace/managers/copper.h"
#include "ace/managers/system.h"

#include "physics.h"

#define ACE_MAXSPRITES 8
#define ACE_SPRITES_COPPERLIST_SIZE 16

typedef struct _tAceSprite
{
  UBYTE *pSpriteData;
  ULONG ulSpriteSize;
  BYTE bTrailingSpriteIndex;
  UWORD uwSpriteHeight; // Height of the sprite in pixel (ulSpriteSize/4)
  UWORD uwSpriteCenter; // value 8 for single sprite (since a sprite is 16 bits wide) or 16 for side by side sprite

  int iBounceBottomLimit;
  int iBounceRightLimit;

} tAceSprite;
tAceSprite s_pAceSprites[ACE_MAXSPRITES];

//tCopBlock *pBlockSprites = NULL;

tCopBlock *copBlockEnableSpriteFull(tCopList *, FUBYTE, UBYTE *, ULONG);
void copBlockEnableSpriteRaw(tCopList *, FUBYTE , UBYTE *, ULONG , FUWORD );
void memBitmapToSprite(UBYTE* , const size_t );
void copBlockSpritesFree();
void spriteMove3(FUBYTE,WORD,WORD);
void disableSpritesAll();
#endif
