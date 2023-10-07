#include <dolphin/card.h>
#include <dolphin/dsp.h>
#include <dolphin/dvd.h>
#include <dolphin/os.h>

#include <dolphin/CARDPriv.h>

#line 41
static void EraseCallback(s32 chan, s32 result) {
  CARDControl* card;
  CARDCallback callback;
  u16* fat;
  CARDDir* dir;
  CARDDir* ent;
  CARDFileInfo* fileInfo;

  card = &__CARDBlock[chan];
  if (result < 0) {
    goto error;
  }

  fileInfo = card->fileInfo;
  if (fileInfo->length < 0) {
    result = CARD_RESULT_CANCELED;
    goto error;
  }

  fileInfo->length -= card->sectorSize;
  if (fileInfo->length <= 0) {
    dir = __CARDGetDirBlock(card);
    ent = &dir[fileInfo->fileNo];
    ent->time = (u32)OSTicksToSeconds(OSGetTime());
    callback = card->apiCallback;
    card->apiCallback = 0;
    result = __CARDUpdateDir(chan, callback);
  } else {
    fat = __CARDGetFatBlock(card);
    fileInfo->offset += card->sectorSize;
    fileInfo->iBlock = fat[fileInfo->iBlock];
    if (!CARDIsValidBlockNo(card, fileInfo->iBlock)) {
      result = CARD_RESULT_BROKEN;
      goto error;
    }
    result = __CARDEraseSector(chan, card->sectorSize * (u32)fileInfo->iBlock, EraseCallback);
  }

  if (result < 0) {
    goto error;
  }
  return;

error:
  callback = card->apiCallback;
  card->apiCallback = 0;
  __CARDPutControlBlock(card, result);
#line 98
  ASSERT(callback);
  callback(chan, result);
}

#line 126
s32 CARDEraseAsync(CARDFileInfo* fileInfo, s32 length, s32 offset, CARDCallback callback) {
  CARDControl* card;
  s32 result;
  CARDDir* dir;
  CARDDir* ent;

  ASSERT(0 < length);
  result = __CARDSeek(fileInfo, length, offset, &card);
  if (result < 0) {
    return result;
  }

  ASSERT(OFFSET(offset, card->sectorSize) == 0);
  ASSERT(OFFSET(length, card->sectorSize) == 0);
  if (OFFSET(offset, card->sectorSize) != 0 || OFFSET(length, card->sectorSize) != 0) {
    return __CARDPutControlBlock(card, CARD_RESULT_FATAL_ERROR);
  }

  dir = __CARDGetDirBlock(card);
  ent = &dir[fileInfo->fileNo];
  result = __CARDIsWritable(card, ent);
  if (result < 0) {
    return __CARDPutControlBlock(card, result);
  }

  card->apiCallback = callback ? callback : __CARDDefaultApiCallback;
  result =
      __CARDEraseSector(fileInfo->chan, card->sectorSize * (u32)fileInfo->iBlock, EraseCallback);
  if (result < 0) {
    __CARDPutControlBlock(card, result);
  }
  return result;
}

s32 CARDErase(CARDFileInfo* fileInfo, s32 length, s32 offset) {
  s32 result;

  result = CARDEraseAsync(fileInfo, length, offset, __CARDSyncCallback);
  if (result < 0) {
    return result;
  }
  return __CARDSync(fileInfo->chan);
}
