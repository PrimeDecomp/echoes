#include <dolphin/card.h>
#include <dolphin/dsp.h>
#include <dolphin/dvd.h>
#include <dolphin/os.h>

#include <dolphin/CARDPriv.h>

static void BlockReadCallback(s32 chan, s32 result) {
  CARDControl* card;
  CARDCallback callback;

  card = &__CARDBlock[chan];
  if (result < 0) {
    goto error;
  }

  card->xferred += CARD_SEG_SIZE;

  card->addr += CARD_SEG_SIZE;
  (u8*)card->buffer += CARD_SEG_SIZE;
  if (--card->repeat <= 0) {
    goto error;
  }

  result = __CARDReadSegment(chan, BlockReadCallback);
  if (result < 0) {
    goto error;
  }
  return;

error:
  if (card->apiCallback == 0) {
    __CARDPutControlBlock(card, result);
  }
  callback = card->xferCallback;
  if (callback) {
    card->xferCallback = 0;
    callback(chan, result);
  }
}

#line 88
s32 __CARDRead(s32 chan, u32 addr, s32 length, void* dst, CARDCallback callback) {
  CARDControl* card;

  ASSERT(0 < length && length % CARD_SEG_SIZE == 0);
  ASSERT(0 <= chan && chan < 2);

  card = &__CARDBlock[chan];
  if (!card->attached) {
    return CARD_RESULT_NOCARD;
  }

  card->xferCallback = callback;
  card->repeat = (int)(length / CARD_SEG_SIZE);
  card->addr = addr;
  card->buffer = dst;

  return __CARDReadSegment(chan, BlockReadCallback);
}

static void BlockWriteCallback(s32 chan, s32 result) {
  CARDControl* card;
  CARDCallback callback;

  card = &__CARDBlock[chan];
  if (result < 0) {
    goto error;
  }

  card->xferred += card->pageSize;

  card->addr += card->pageSize;
  (u8*)card->buffer += card->pageSize;
  if (--card->repeat <= 0) {
    goto error;
  }

  result = __CARDWritePage(chan, BlockWriteCallback);
  if (result < 0) {
    goto error;
  }
  return;

error:
  if (card->apiCallback == 0) {
    __CARDPutControlBlock(card, result);
  }
  callback = card->xferCallback;
  if (callback) {
    card->xferCallback = 0;
    callback(chan, result);
  }
}

#line 149
s32 __CARDWrite(s32 chan, u32 addr, s32 length, void* dst, CARDCallback callback) {
  CARDControl* card;

  card = &__CARDBlock[chan];
  ASSERT(0 < length && length % card->pageSize == 0);
  ASSERT(0 <= chan && chan < 2);
  if (!card->attached) {
    return CARD_RESULT_NOCARD;
  }

  card->xferCallback = callback;
  card->repeat = (int)(length / card->pageSize);
  card->addr = addr;
  card->buffer = dst;

  return __CARDWritePage(chan, BlockWriteCallback);
}

#line 182
s32 CARDGetXferredBytes(s32 chan) {
  ASSERT(0 <= chan && chan < 2);
  return __CARDBlock[chan].xferred;
}
