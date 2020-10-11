/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _ACE_MANAGERS_VIEWPORT_SIMPLEBUFFERTEST_H_
#define _ACE_MANAGERS_VIEWPORT_SIMPLEBUFFERTEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef AMIGA

/**
 *  Buffer with naive scrolling techniques. Uses loadsa CHIP RAM but there
 *  should'nt be any quirks while using it.
 */

#include <ace/types.h>
#include <ace/managers/viewport/camera.h>
#include <ace/utils/bitmap.h>
#include <ace/utils/tag.h>

// vPort ptr
#define TAG_SIMPLEBUFFERTEST_VPORT          (TAG_USER|1)
// Scrollable area bounds, in pixels
#define TAG_SIMPLEBUFFERTEST_BOUND_WIDTH    (TAG_USER|2)
#define TAG_SIMPLEBUFFERTEST_BOUND_HEIGHT   (TAG_USER|3)
// Buffer bitmap creation flags
#define TAG_SIMPLEBUFFERTEST_BITMAP_FLAGS   (TAG_USER|4)
#define TAG_SIMPLEBUFFERTEST_IS_DBLBUF      (TAG_USER|5)
// If in raw mode, offset on copperlist for placing required copper
// instructions, specified in copper instruction count since beginning.
#define TAG_SIMPLEBUFFERTEST_COPLIST_OFFSET (TAG_USER|6)
#define TAG_SIMPLEBUFFERTEST_USE_X_SCROLLING (TAG_USER|7)

typedef struct _tSimpleBufferTestManager {
	tVpManager sCommon;
	tCameraManager *pCamera;
	// scroll-specific fields
	tBitMap *pFront;       ///< Currently displayed buffer.
	tBitMap *pBack;        ///< Buffer for drawing.
	tCopBlock *pCopBlock;  ///< CopBlock containing modulo/shift/bitplane cmds
	tUwCoordYX uBfrBounds; ///< Buffer bounds in pixels
	UBYTE ubFlags;         ///< Read only. See SIMPLEBUFFERTESTTEST_FLAG_*.
	UWORD uwCopperOffset;  ///< Offset on copperlist in COP_RAW mode.
} tSimpleBufferTestManager;

/**
 *  @brief Creates new simple-scrolled buffer manager along with required buffer
 *  bitmap.
 *  This approach is not suitable for big buffers, because you'll run
 *  out of memory quite easily.
 *
 *  @param pTags Initialization taglist.
 *  @param ...   Taglist passed as va_args.
 *  @return Pointer to newly created buffer manager.
 *
 *  @see SimpleBuffertestDestroy
 *  @see SimpleBuffertestSetBitmap
 */
tSimpleBufferTestManager *SimpleBufferTestCreate(void *pTags,	...);

 /**
 *  @brief Sets new bitmap to be displayed by buffer manager.
 *  If there was buffer created by manager, be sure to intercept & free it.
 *  Also, both buffer bitmaps must have same BPP, as difference would require
 *  copBlock realloc, which is not implemented.
 *  @param pManager The buffer manager, which buffer is to be changed.
 *  @param pBitMap  New bitmap to be used by manager.
 *
 *  @todo Realloc copper buffer to reflect BPP change.
 */
void SimpleBufferTestSetBitmap(tSimpleBufferTestManager *pManager, tBitMap *pBitMap);

void SimpleBufferTestDestroy(tSimpleBufferTestManager *pManager);

void SimpleBufferTestProcess(tSimpleBufferTestManager *pManager);

UBYTE SimpleBufferTestIsRectVisible(
	tSimpleBufferTestManager *pManager,
	UWORD uwX, UWORD uwY, UWORD uwWidth, UWORD uwHeight
);

UBYTE SimpleBufferTestGetRawCopperlistInstructionCount(UBYTE ubBpp);

#endif // AMIGA

#ifdef __cplusplus
}
#endif

#endif // _ACE_MANAGERS_VIEWPORT_SIMPLEBUFFERTEST_H_