#include <dolphin/card.h>
#include <dolphin/dsp.h>
#include <dolphin/dvd.h>
#include <dolphin/os.h>

#include <dolphin/CARDPriv.h>

#line 55
s32 __CARDRawReadAsync(s32 chan, void* buf, s32 length, s32 offset, CARDCallback callback) {
  CARDControl* card;
  s32 result;

  ASSERT(buf && ((u32)buf % 32) == 0);

  result = __CARDGetControlBlock(chan, &card);

  if (result < 0) {
    return __CARDPutControlBlock(card, result);
  }

  ASSERT(0 < length && (length % CARD_SEG_SIZE) == 0 && length < CARD_MAX_SIZE);
  ASSERT((offset % card->sectorSize) == 0);

  DCInvalidateRange(buf, (u32)length);
  result = __CARDRead(chan, (u32)offset, length, buf, callback);
  if (result < 0) {
    __CARDPutControlBlock(card, result);
  }
  return result;
}

s32 __CARDRawRead(s32 chan, void* buf, s32 length, s32 offset) {
  s32 result;

  result = __CARDRawReadAsync(chan, buf, length, offset, __CARDSyncCallback);
  if (result < 0) {
    return result;
  }
  return __CARDSync(chan);
}

#line 109
static void EraseCallback(s32 chan, s32 result) {
  CARDControl* card;
  CARDCallback callback;

  card = &__CARDBlock[chan];
  callback = card->apiCallback;
  card->apiCallback = 0;
  __CARDPutControlBlock(card, result);
  ASSERT(callback);
  callback(chan, result);
}

s32 __CARDRawEraseAsync(s32 chan, s32 offset, CARDCallback callback) {
  CARDControl* card;
  s32 result;

  result = __CARDGetControlBlock(chan, &card);
  if (result < 0) {
    return __CARDPutControlBlock(card, result);
  }
  if (offset % card->sectorSize != 0) {
    return __CARDPutControlBlock(card, CARD_RESULT_FATAL_ERROR);
  }
  if (card->size * 1024 * 1024 / 8 <= offset) {
    return __CARDPutControlBlock(card, CARD_RESULT_LIMIT);
  }

  card->apiCallback = callback ? callback : __CARDDefaultApiCallback;
  result = __CARDEraseSector(chan, (u32)offset, EraseCallback);
  if (result < 0) {
    __CARDPutControlBlock(card, result);
  }
  return result;
}

s32 __CARDRawErase(s32 chan, s32 offset) {
  s32 result;

  result = __CARDRawEraseAsync(chan, offset, __CARDSyncCallback);
  if (result < 0) {
    return result;
  }
  return __CARDSync(chan);
}
