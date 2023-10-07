#include <dolphin/card.h>
#include <dolphin/dsp.h>
#include <dolphin/dvd.h>
#include <dolphin/os.h>

#include <dolphin/CARDPriv.h>

#line 77
s32 CARDRenameAsync(s32 chan, const char* old, const char* new, CARDCallback callback) {
  CARDControl* card;
  CARDDir* dir;
  CARDDir* ent;
  s32 result;
  int fileNo;
  int newNo;
  int oldNo;

  ASSERT(0 <= chan && chan < 2);
  ASSERT(*old != 0xff && *new != 0xff);
  ASSERT(*old != 0x00 && *new != 0x00);

  if (*old == 0xff || *new == 0xff || *old == 0x00 || *new == 0x00) {
    return CARD_RESULT_FATAL_ERROR;
  }
  if (CARD_FILENAME_MAX < (u32)strlen(old) || CARD_FILENAME_MAX < (u32)strlen(new)) {
    return CARD_RESULT_NAMETOOLONG;
  }
  result = __CARDGetControlBlock(chan, &card);
  if (result < 0) {
    return result;
  }

  newNo = oldNo = -1;
  dir = __CARDGetDirBlock(card);
  for (fileNo = 0; fileNo < CARD_MAX_FILE; fileNo++) {
    ent = &dir[fileNo];
    if (ent->gameName[0] == 0xff) {
      continue;
    }

    if (memcmp(ent->gameName, card->diskID->gameName, sizeof(ent->gameName)) != 0 ||
        memcmp(ent->company, card->diskID->company, sizeof(ent->company)) != 0) {
      continue;
    }

    if (__CARDCompareFileName(ent, old)) {
      oldNo = fileNo;
    }
    if (__CARDCompareFileName(ent, new)) {
      newNo = fileNo;
    }
  }

  if (oldNo == -1) {
    return __CARDPutControlBlock(card, CARD_RESULT_NOFILE);
  }
  if (newNo != -1) {
    return __CARDPutControlBlock(card, CARD_RESULT_EXIST);
  }

  ent = &dir[oldNo];
  result = __CARDIsWritable(card, ent);
  if (result < 0) {
    return __CARDPutControlBlock(card, result);
  }

  strncpy((char*)ent->fileName, new, CARD_FILENAME_MAX);

  ent->time = (u32)OSTicksToSeconds(OSGetTime());
  result = __CARDUpdateDir(chan, callback);
  if (result < 0) {
    __CARDPutControlBlock(card, result);
  }
  return result;
}

s32 CARDRename(s32 chan, const char* old, const char* new) {
  s32 result;
  result = CARDRenameAsync(chan,  old, new, __CARDSyncCallback);
  if (result < 0) {
    return result;
  }

  return __CARDSync(chan);
}
