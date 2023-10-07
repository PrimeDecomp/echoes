
#include <dolphin/card.h>
#include <dolphin/dsp.h>
#include <dolphin/dvd.h>
#include <dolphin/os.h>

#include <dolphin/CARDPriv.h>

static void ProgramCallback(s32 chan, s32 result) {
  CARDControl* card;
  CARDCallback callback;
  u16* fat;
  CARDFileInfo* fileInfo;
  s32 length;

  card = &__CARDBlock[chan];
  if (result < 0) {
    goto error;
  }

  fileInfo = card->fileInfo;
  if (fileInfo->length < 0) {
    result = CARD_RESULT_CANCELED;
    goto error;
  }

  length = (s32)TRUNC(fileInfo->offset + card->sectorSize, card->sectorSize) - fileInfo->offset;
  fileInfo->length -= length;
  if (fileInfo->length <= 0) {
    goto error;
  } else {
    fat = __CARDGetFatBlock(card);
    fileInfo->offset += length;
    fileInfo->iBlock = fat[fileInfo->iBlock];
    if (!CARDIsValidBlockNo(card, fileInfo->iBlock)) {
      result = CARD_RESULT_BROKEN;
      goto error;
    }

#line 94
    ASSERT(OFFSET(fileInfo->length, CARD_PROGRAM_SIZE) == 0);
    ASSERT(OFFSET(fileInfo->offset, card->sectorSize) == 0);
    result =
        __CARDWrite(chan, card->sectorSize * (u32)fileInfo->iBlock,
                    (fileInfo->length < card->sectorSize) ? fileInfo->length : card->sectorSize,
                    card->buffer, ProgramCallback);
  }

  if (result < 0) {
    goto error;
  }

  return;

error:

  callback = card->apiCallback;
  card->apiCallback = 0;
  __CARDPutControlBlock(card, result);

  ASSERT(callback);
  callback(chan, result);
}

#line 140
s32 CARDProgramAsync(CARDFileInfo* fileInfo, void* buf, s32 length, s32 offset,
                     CARDCallback callback) {
  CARDControl* card;
  s32 result;
  CARDDir* dir;
  CARDDir* ent;

  ASSERT(buf && OFFSET(buf, 32) == 0);
  ASSERT(OFFSET(offset, CARD_PROGRAM_SIZE) == 0);
  ASSERT(0 < length && OFFSET(length, CARD_PROGRAM_SIZE) == 0);

  if (OFFSET(offset, CARD_PROGRAM_SIZE) != 0 || OFFSET(length, CARD_PROGRAM_SIZE) != 0) {
    return CARD_RESULT_FATAL_ERROR;
  }
  result = __CARDSeek(fileInfo, length, offset, &card);
  if (result < 0) {
    return result;
  }

  dir = __CARDGetDirBlock(card);
  ent = &dir[fileInfo->fileNo];
  result = __CARDIsWritable(card, ent);
  if (result < 0) {
    return __CARDPutControlBlock(card, result);
  }

  DCStoreRange(buf, (u32)length);
  card->apiCallback = callback ? callback : __CARDDefaultApiCallback;

  offset = (s32)OFFSET(fileInfo->offset, card->sectorSize);
  length = (length < card->sectorSize - offset) ? length : card->sectorSize - offset;
  result = __CARDWrite(fileInfo->chan, card->sectorSize * (u32)fileInfo->iBlock + offset, length,
                       buf, ProgramCallback);
  if (result < 0) {
    __CARDPutControlBlock(card, result);
  }
  return result;
}

s32 CARDProgram(CARDFileInfo* fileInfo, void* buf, s32 length, s32 offset) {
  s32 result;

  result = CARDProgramAsync(fileInfo, buf, length, offset, __CARDSyncCallback);
  if (result < 0) {
    return result;
  }
  return __CARDSync(fileInfo->chan);
}
