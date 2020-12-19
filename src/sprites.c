#include "sprites.h"
#include <stdlib.h>

FUBYTE copRawDisableSprites2(tCopList *pList, FUBYTE fubSpriteMask, FUWORD fuwCmdOffs);

tCopBlock *copBlockEnableSpriteFull(tCopList *pList, FUBYTE fubSpriteIndex, UBYTE *pSpriteData, ULONG ulSpriteSize)
{
  static tCopBlock *pBlockSprites = NULL;
  tCopMoveCmd *pMoveCmd = NULL;

  if (pBlockSprites == NULL)
  {
    pBlockSprites = copBlockDisableSprites(pList, 0xFF);
    systemSetDma(DMAB_SPRITE, 1);

    // Reset tAceSprite array
    for (UBYTE ubIterator = 0; ubIterator < ACE_MAXSPRITES; ubIterator++)
    {
      s_pAceSprites[ubIterator].pSpriteData = NULL;
      s_pAceSprites[ubIterator].ulSpriteSize = 0;
      s_pAceSprites[ubIterator].bTrailingSpriteIndex = -1;
      s_pAceSprites[ubIterator].uwSpriteHeight = 0;
      s_pAceSprites[ubIterator].uwSpriteCenter = 0;
      s_pAceSprites[ubIterator].iBounceBottomLimit = 0;
      s_pAceSprites[ubIterator].iBounceRightLimit = 0;
    }
  }
  if (s_pAceSprites[fubSpriteIndex].pSpriteData)
  {
    FreeMem(s_pAceSprites[fubSpriteIndex].pSpriteData, s_pAceSprites[fubSpriteIndex].ulSpriteSize);
    s_pAceSprites[fubSpriteIndex].pSpriteData = NULL;
    s_pAceSprites[fubSpriteIndex].ulSpriteSize = 0;
    s_pAceSprites[fubSpriteIndex].bTrailingSpriteIndex = -1;
    s_pAceSprites[fubSpriteIndex].uwSpriteHeight = 0;
    s_pAceSprites[fubSpriteIndex].uwSpriteCenter = 0;
    s_pAceSprites[fubSpriteIndex].iBounceBottomLimit = 0;
    s_pAceSprites[fubSpriteIndex].iBounceRightLimit = 0;
  }

  // Bounce limits
  s_pAceSprites[fubSpriteIndex].iBounceBottomLimit = 255 - ulSpriteSize / 4;
  s_pAceSprites[fubSpriteIndex].iBounceRightLimit = 319 - 16;

  s_pAceSprites[fubSpriteIndex].uwSpriteHeight = ulSpriteSize / 4;
  s_pAceSprites[fubSpriteIndex].uwSpriteCenter = 8;

  //Make some room for sprite extra information
  ulSpriteSize += 8;

  s_pAceSprites[fubSpriteIndex].pSpriteData = (UBYTE *)AllocMem(ulSpriteSize, MEMF_CHIP);
  memset(s_pAceSprites[fubSpriteIndex].pSpriteData, 0, ulSpriteSize);

  const UBYTE ubVStart = 0x30;
  const UBYTE ubHStart = 0x90;
  s_pAceSprites[fubSpriteIndex].pSpriteData[0] = ubVStart; // ubVStart
  s_pAceSprites[fubSpriteIndex].pSpriteData[1] = ubHStart; // ubHstart
  s_pAceSprites[fubSpriteIndex].pSpriteData[2] = (UBYTE)((ulSpriteSize - 8) / 4) + ubVStart;
  s_pAceSprites[fubSpriteIndex].pSpriteData[3] = 0x00;

  s_pAceSprites[fubSpriteIndex].ulSpriteSize = ulSpriteSize - 8;

  // For each line of the sprite
  UWORD ubImgOffset = 0;
  UWORD ubHalfOffset = (UWORD)((ulSpriteSize - 8) / 2);

  for (UWORD ubImgCount = 0; 0 && ubImgCount < (UWORD)((ulSpriteSize - 8) / 4); ubImgCount++)
  {
    // First two bytes from first bitplane
    memcpy(s_pAceSprites[fubSpriteIndex].pSpriteData + 4 + ubImgOffset, pSpriteData + ubImgCount * 2, 2);
    ubImgOffset += 2;

    // Other two bytes from second bitplane
    memcpy(s_pAceSprites[fubSpriteIndex].pSpriteData + 4 + ubImgOffset, pSpriteData + ubHalfOffset + ubImgCount * 2, 2);
    ubImgOffset += 2;
  }

  // Terminator
  s_pAceSprites[fubSpriteIndex].pSpriteData[ulSpriteSize - 1] = 0x00;
  s_pAceSprites[fubSpriteIndex].pSpriteData[ulSpriteSize - 2] = 0x00;
  s_pAceSprites[fubSpriteIndex].pSpriteData[ulSpriteSize - 3] = 0x00;
  s_pAceSprites[fubSpriteIndex].pSpriteData[ulSpriteSize - 4] = 0x00;

  //ULONG ulAddr = (ULONG)pSpriteData;
  ULONG ulAddr = (ULONG)s_pAceSprites[fubSpriteIndex].pSpriteData;

  if (fubSpriteIndex == 0)
  {
    pMoveCmd = (tCopMoveCmd *)&pBlockSprites->pCmds[0];
    pMoveCmd->bfValue = ulAddr >> 16;
    logWrite("move command : %hx\n", pMoveCmd->bfDestAddr);

    pMoveCmd = (tCopMoveCmd *)&pBlockSprites->pCmds[1];
    pMoveCmd->bfValue = ulAddr & 0xFFFF;
    logWrite("move command : %hx\n", pMoveCmd->bfDestAddr);
  }

  // Start of sprite 1
  if (fubSpriteIndex == 1)
  {
    pMoveCmd = (tCopMoveCmd *)&pBlockSprites->pCmds[2];
    pMoveCmd->bfValue = ulAddr >> 16;

    pMoveCmd = (tCopMoveCmd *)&pBlockSprites->pCmds[3];
    pMoveCmd->bfValue = ulAddr & 0xFFFF;
    // end of sprite 1
  }

  if (fubSpriteIndex == 2)
  {
    // Start of sprite 2

    pMoveCmd = (tCopMoveCmd *)&pBlockSprites->pCmds[4];
    pMoveCmd->bfValue = ulAddr >> 16;

    pMoveCmd = (tCopMoveCmd *)&pBlockSprites->pCmds[5];
    pMoveCmd->bfValue = ulAddr & 0xFFFF;
  }

  if (fubSpriteIndex == 3)
  {
    // Start of sprite 3

    pMoveCmd = (tCopMoveCmd *)&pBlockSprites->pCmds[6];
    pMoveCmd->bfValue = ulAddr >> 16;

    pMoveCmd = (tCopMoveCmd *)&pBlockSprites->pCmds[7];
    pMoveCmd->bfValue = ulAddr & 0xFFFF;
  }

  if (fubSpriteIndex == 4)
  {
    // Start of sprite 4

    pMoveCmd = (tCopMoveCmd *)&pBlockSprites->pCmds[8];
    pMoveCmd->bfValue = ulAddr >> 16;

    pMoveCmd = (tCopMoveCmd *)&pBlockSprites->pCmds[9];
    pMoveCmd->bfValue = ulAddr & 0xFFFF;
  }

  if (fubSpriteIndex == 5)
  {
    // Start of sprite 5

    pMoveCmd = (tCopMoveCmd *)&pBlockSprites->pCmds[10];
    pMoveCmd->bfValue = ulAddr >> 16;

    pMoveCmd = (tCopMoveCmd *)&pBlockSprites->pCmds[11];
    pMoveCmd->bfValue = ulAddr & 0xFFFF;
  }

  // Start of sprite 6
  if (fubSpriteIndex == 6)
  {
    pMoveCmd = (tCopMoveCmd *)&pBlockSprites->pCmds[12];
    pMoveCmd->bfValue = ulAddr >> 16;
    pMoveCmd = (tCopMoveCmd *)&pBlockSprites->pCmds[13];
    pMoveCmd->bfValue = ulAddr & 0xFFFF;
  }

  // Start of sprite 7
  if (fubSpriteIndex == 7)
  {
    pMoveCmd = (tCopMoveCmd *)&pBlockSprites->pCmds[14];
    pMoveCmd->bfValue = ulAddr >> 16;
    pMoveCmd = (tCopMoveCmd *)&pBlockSprites->pCmds[15];
    pMoveCmd->bfValue = ulAddr & 0xFFFF;
  }

  //SETSPRITEIMG(fubSpriteIndex,pSpriteData,ulSpriteSize)

  return NULL;
}

void copBlockEnableSpriteRaw(tCopList *pList, FUBYTE fubSpriteIndex, UBYTE *pSpriteData, ULONG ulSpriteSize, FUWORD fuwCopRawOffs)
{
  static UBYTE ubCopInit = 0;
  if (ubCopInit == 0)
  {
    ubCopInit = 1;
    copRawDisableSprites(pList, 0xFF, fuwCopRawOffs);
    systemSetDma(DMAB_SPRITE, 1);

    // Reset tAceSprite array
    for (UBYTE ubIterator = 0; ubIterator < ACE_MAXSPRITES; ubIterator++)
    {
      s_pAceSprites[ubIterator].pSpriteData = NULL;
      s_pAceSprites[ubIterator].ulSpriteSize = 0;
      s_pAceSprites[ubIterator].bTrailingSpriteIndex = -1;
      s_pAceSprites[ubIterator].uwSpriteHeight = 0;
      s_pAceSprites[ubIterator].uwSpriteCenter = 0;
      s_pAceSprites[ubIterator].iBounceBottomLimit = 0;
      s_pAceSprites[ubIterator].iBounceRightLimit = 0;
    }
  }
  if (s_pAceSprites[fubSpriteIndex].pSpriteData)
  {
    FreeMem(s_pAceSprites[fubSpriteIndex].pSpriteData, s_pAceSprites[fubSpriteIndex].ulSpriteSize);
    s_pAceSprites[fubSpriteIndex].pSpriteData = NULL;
    s_pAceSprites[fubSpriteIndex].ulSpriteSize = 0;
    s_pAceSprites[fubSpriteIndex].bTrailingSpriteIndex = -1;
    s_pAceSprites[fubSpriteIndex].uwSpriteHeight = 0;
    s_pAceSprites[fubSpriteIndex].uwSpriteCenter = 0;
    s_pAceSprites[fubSpriteIndex].iBounceBottomLimit = 0;
    s_pAceSprites[fubSpriteIndex].iBounceRightLimit = 0;
  }

  // Bounce limits
  s_pAceSprites[fubSpriteIndex].iBounceBottomLimit = 255 - ulSpriteSize / 4;
  s_pAceSprites[fubSpriteIndex].iBounceRightLimit = 319 - 16;

  s_pAceSprites[fubSpriteIndex].uwSpriteHeight = ulSpriteSize / 4;
  s_pAceSprites[fubSpriteIndex].uwSpriteCenter = 8;

  //Make some room for sprite extra information
  ulSpriteSize += 8;

  s_pAceSprites[fubSpriteIndex].pSpriteData = (UBYTE *)AllocMem(ulSpriteSize, MEMF_CHIP);
  memset(s_pAceSprites[fubSpriteIndex].pSpriteData, 0, ulSpriteSize);

  const UBYTE ubVStart = 0x60;
  const UBYTE ubHStart = 0x90;
  s_pAceSprites[fubSpriteIndex].pSpriteData[0] = ubVStart; // ubVStart
  s_pAceSprites[fubSpriteIndex].pSpriteData[1] = ubHStart; // ubHstart
  s_pAceSprites[fubSpriteIndex].pSpriteData[2] = (UBYTE)((ulSpriteSize - 8) / 4) + ubVStart;
  s_pAceSprites[fubSpriteIndex].pSpriteData[3] = 0x00;

  s_pAceSprites[fubSpriteIndex].ulSpriteSize = ulSpriteSize - 8;

  // For each line of the sprite
  UWORD ubImgOffset = 0;
  UWORD ubHalfOffset = (UWORD)((ulSpriteSize - 8) / 2);

  for (UWORD ubImgCount = 0; ubImgCount < (UWORD)((ulSpriteSize - 8) / 4); ubImgCount++)
  {
    // First two bytes from first bitplane
    memcpy(s_pAceSprites[fubSpriteIndex].pSpriteData + 4 + ubImgOffset, pSpriteData + ubImgCount * 2, 2);
    ubImgOffset += 2;

    // Other two bytes from second bitplane
    memcpy(s_pAceSprites[fubSpriteIndex].pSpriteData + 4 + ubImgOffset, pSpriteData + ubHalfOffset + ubImgCount * 2, 2);
    ubImgOffset += 2;
  }

  // Terminator
  s_pAceSprites[fubSpriteIndex].pSpriteData[ulSpriteSize - 1] = 0x00;
  s_pAceSprites[fubSpriteIndex].pSpriteData[ulSpriteSize - 2] = 0x00;
  s_pAceSprites[fubSpriteIndex].pSpriteData[ulSpriteSize - 3] = 0x00;
  s_pAceSprites[fubSpriteIndex].pSpriteData[ulSpriteSize - 4] = 0x00;

  ULONG ulAddr = (ULONG)s_pAceSprites[fubSpriteIndex].pSpriteData;

  tCopMoveCmd *pCmd = &pList->pBackBfr->pList[fuwCopRawOffs].sMove;
  tCopMoveCmd *pCmd2 = &pList->pFrontBfr->pList[fuwCopRawOffs].sMove;

  pCmd += fubSpriteIndex * 2;
  pCmd2 += fubSpriteIndex * 2;

  copSetMove(pCmd++, &g_pSprFetch[fubSpriteIndex].uwHi, ulAddr >> 16);
  copSetMove(pCmd, &g_pSprFetch[fubSpriteIndex].uwLo, ulAddr & 0xFFFF);

  copSetMove(pCmd2++, &g_pSprFetch[fubSpriteIndex].uwHi, ulAddr >> 16);
  copSetMove(pCmd2, &g_pSprFetch[fubSpriteIndex].uwLo, ulAddr & 0xFFFF);
}

void spriteMove3(FUBYTE fubSpriteIndex, WORD x, WORD y)
{
  if (s_pAceSprites[fubSpriteIndex].bTrailingSpriteIndex >= 0)
  {
    spriteMove3(s_pAceSprites[fubSpriteIndex].bTrailingSpriteIndex, x + 16, y);
  }

  x += 128;
  int y1;
  y += 0x2C;
  y1 = y + s_pAceSprites[fubSpriteIndex].uwSpriteHeight;

  UBYTE *vStart = s_pAceSprites[fubSpriteIndex].pSpriteData + 0;
  UBYTE *hStart = s_pAceSprites[fubSpriteIndex].pSpriteData + 1;
  UBYTE *vStop = s_pAceSprites[fubSpriteIndex].pSpriteData + 2;
  UBYTE *ctrlByte = s_pAceSprites[fubSpriteIndex].pSpriteData + 3;

  UBYTE oldCtrlByteVal = *ctrlByte;
  UWORD spritePosTmp = x >> 1;
  *hStart = (UBYTE)spritePosTmp & 0xFF;

  if (x & 1)
    oldCtrlByteVal |= 0x01;
  else
    oldCtrlByteVal &= 0xFE;

  *vStart = (UBYTE)y & 0xFF;
  *vStop = (UBYTE)y1 & 0xFF;

  if (y > 255)
    oldCtrlByteVal |= 0x04;
  else
    oldCtrlByteVal &= 0xFB;

  if (y1 > 255)
    oldCtrlByteVal |= 0x02;
  else
    oldCtrlByteVal &= 0xFD;

  *ctrlByte = oldCtrlByteVal;

  return;
}

FUBYTE copRawDisableSprites2(tCopList *pList, FUBYTE fubSpriteMask, FUWORD fuwCmdOffs)
{
  FUBYTE fubCmdCnt = 0;
  //ULONG ulBlank = (ULONG)s_pBlankSprite;

  // No WAIT - could be done earlier by other stuff
  tCopMoveCmd *pCmd = &pList->pBackBfr->pList[fuwCmdOffs].sMove;
  tCopMoveCmd *pCmd2 = &pList->pFrontBfr->pList[fuwCmdOffs].sMove;
  for (FUBYTE i = 0; i != 8; ++i)
  {
    if (fubSpriteMask & 1)
    {
      /*copSetMove(pCmd++, &g_pSprFetch[i].uwHi, ulBlank >> 16);
      copSetMove(pCmd++, &g_pSprFetch[i].uwLo, ulBlank & 0xFFFF);*/

      copSetMove(pCmd++, &g_pSprFetch[i].uwHi, 0x0000);
      copSetMove(pCmd++, &g_pSprFetch[i].uwLo, 0x0000);

      copSetMove(pCmd2++, &g_pSprFetch[i].uwHi, 0x0000);
      copSetMove(pCmd2++, &g_pSprFetch[i].uwLo, 0x0000);
      fubCmdCnt += 2;
    }
    fubSpriteMask >>= 1;
  }

  // Copy to front buffer
  /*CopyMemQuick(
      &pList->pBackBfr->pList[fuwCmdOffs],
      &pList->pFrontBfr->pList[fuwCmdOffs],
      fubCmdCnt * sizeof(tCopCmd));*/

  return fubCmdCnt;
}

// Converts a regular bitmap (not interleaved) to sprite format to be pushed into amiga hardware sprites
// THE CONTENTS OF THE MEMORY WILL BE OVERWRITTEN
void memBitmapToSprite(UBYTE *pData, const size_t iDataLength)
{
  // Sprite data must always be even
  if (iDataLength % 2)
    return;

  size_t iBitplaneLength = iDataLength / 2;
  unsigned char *pBuf1 = malloc(iBitplaneLength);
  memcpy(pBuf1, pData, iBitplaneLength);
  unsigned char *pBuf2 = malloc(iBitplaneLength);
  memcpy(pBuf2, pData + iBitplaneLength, iBitplaneLength);

  UWORD ubImgOffset = 0;
  //UWORD ubHalfOffset=(UWORD)((ulSpriteSize-8)/2);

  // For each line of the sprite
  for (UWORD ubImgCount = 0; ubImgCount < (UWORD)(iDataLength / 4); ubImgCount++)
  {
    // First two bytes from first bitplane
    if (ubImgOffset <= iDataLength - 2)
      memcpy(pData + ubImgOffset, pBuf1 + ubImgCount * 2, 2);
    ubImgOffset += 2;

    // Other two bytes from second bitplane
    if (ubImgOffset <= iDataLength - 2)
      memcpy(pData + ubImgOffset, pBuf2 + ubImgCount * 2, 2);
    //memcpy(s_pAceSprites[fubSpriteIndex].pSpriteData+4+ubImgOffset,pSpriteData+ubHalfOffset+ubImgCount*2,2);
    ubImgOffset += 2;
  }
  free(pBuf1);
  free(pBuf2);
  return;
}
void copBlockSpritesFree()
{
  // Reset tAceSprite array
  for (UBYTE ubIterator = 0; ubIterator < ACE_MAXSPRITES; ubIterator++)
  {
    if (s_pAceSprites[ubIterator].pSpriteData)
      FreeMem(s_pAceSprites[ubIterator].pSpriteData, s_pAceSprites[ubIterator].ulSpriteSize);
  }
}

void disableSpritesAll()
{
  systemSetDma(DMAB_SPRITE, 0);

  // Sprite reset
  UWORD *p_Sprites = (UWORD *)0xdff140;
  while (p_Sprites <= (UWORD *)0xdff17E)
  {
    *p_Sprites = 0;
    p_Sprites += 2;
  }
}
