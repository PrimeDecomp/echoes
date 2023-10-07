#include <dolphin/card.h>
#include <dolphin/dsp.h>
#include <dolphin/dvd.h>
#include <dolphin/os.h>

#include <dolphin/CARDPriv.h>

#line 79
s32 __CARDGetStatusEx(s32 chan, s32 fileNo, CARDDir* dirent) {
  CARDControl* card;
  CARDDir* dir;
  CARDDir* ent;
  s32 result;

  ASSERT(0 <= chan && chan < 2);
  ASSERT(0 <= fileNo && fileNo < CARD_MAX_FILE);
  if (fileNo < 0 || CARD_MAX_FILE <= fileNo) {
    return CARD_RESULT_FATAL_ERROR;
  }
  result = __CARDGetControlBlock(chan, &card);
  if (result < 0) {
    return result;
  }

  dir = __CARDGetDirBlock(card);
  ent = &dir[fileNo];
  result = __CARDIsReadable(card, ent);
  if (0 <= result) {
    memcpy(dirent, ent, sizeof(CARDDir));
  }
  return __CARDPutControlBlock(card, result);
}

#line 135
s32 __CARDSetStatusExAsync(s32 chan, s32 fileNo, CARDDir* dirent, CARDCallback callback) {
  CARDControl* card;
  CARDDir* dir;
  CARDDir* ent;
  s32 result;
  u8* p;

  ASSERT(0 <= fileNo && fileNo < CARD_MAX_FILE);
  ASSERT(0 <= chan && chan < 2);
  ASSERT(*dirent->fileName != 0xff && *dirent->fileName != 0x00);

  //
  //
  //
  // clang-format off
  ASSERTMSG(dirent->iconAddr == 0xffffffff || 
            dirent->iconAddr < CARD_READ_SIZE, 
            "CARDSetStatus[Async](): stat->iconAddr must be 0xffffffff or less than CARD_READ_SIZE.");
  ASSERTMSG(dirent->commentAddr == 0xffffffff || 
            dirent->commentAddr % CARD_SYSTEM_BLOCK_SIZE <=CARD_SYSTEM_BLOCK_SIZE - CARD_COMMENT_SIZE, 
            "CARDSetStatus[Async](): comment strings (set by stat->commentAddr) must not cross 8KB byte boundary.");
  // clang-format on

  if (fileNo < 0 || CARD_MAX_FILE <= fileNo || *dirent->fileName == 0xff ||
      *dirent->fileName == 0x00) {
    return CARD_RESULT_FATAL_ERROR;
  }

  result = __CARDGetControlBlock(chan, &card);
  if (result < 0) {
    return result;
  }

  dir = __CARDGetDirBlock(card);
  ent = &dir[fileNo];
  result = __CARDIsWritable(card, ent);
  if (result < 0) {
    return __CARDPutControlBlock(card, result);
  }

  for (p = dirent->fileName; p < &dirent->fileName[CARD_FILENAME_MAX]; ++p) {
    if (*p == 0x00) {
      while (++p < &dirent->fileName[CARD_FILENAME_MAX]) {
        *p = 0x00;
      }
      break;
    }
  }

  if (dirent->permission & CARD_DIR_PERM_GLOBAL) {
    memset(dirent->gameName, 0, sizeof(ent->gameName));
    memset(dirent->company, 0, sizeof(ent->company));
  }

  if (dirent->permission & CARD_DIR_PERM_COMPANY) {
    memset(dirent->gameName, 0, sizeof(ent->gameName));
  }

  if (memcmp(ent->fileName, dirent->fileName, CARD_FILENAME_MAX) != 0 ||
      memcmp(ent->gameName, dirent->gameName, sizeof(ent->gameName)) != 0 ||
      memcmp(ent->company, dirent->company, sizeof(ent->company)) != 0) {
    s32 i;

    for (i = 0; i < CARD_MAX_FILE; i++) {
      CARDDir* ent;

      if (i == fileNo) {
        continue;
      }

      ent = &dir[i];
      if (ent->gameName[0] == 0xff) {
        continue;
      }

      if (memcmp(ent->gameName, dirent->gameName, sizeof(ent->gameName)) == 0 &&
          memcmp(ent->company, dirent->company, sizeof(ent->company)) == 0 &&
          memcmp(ent->fileName, dirent->fileName, CARD_FILENAME_MAX) == 0) {
        return __CARDPutControlBlock(card, CARD_RESULT_EXIST);
      }
    }

    memcpy(ent->fileName, dirent->fileName, CARD_FILENAME_MAX);
    memcpy(ent->gameName, dirent->gameName, sizeof(ent->gameName));
    memcpy(ent->company, dirent->company, sizeof(ent->company));
  }

  ent->time = dirent->time;

  ent->bannerFormat = dirent->bannerFormat;
  ent->iconAddr = dirent->iconAddr;
  ent->iconFormat = dirent->iconFormat;
  ent->iconSpeed = dirent->iconSpeed;
  ent->commentAddr = dirent->commentAddr;

  ent->permission = dirent->permission;
  ent->copyTimes = dirent->copyTimes;

  result = __CARDUpdateDir(chan, callback);
  if (result < 0) {
    __CARDPutControlBlock(card, result);
  }
  return result;
}

s32 __CARDSetStatusEx(s32 chan, s32 fileNo, CARDDir* dirent) {
  s32 result;

  result = __CARDSetStatusExAsync(chan, fileNo, dirent, __CARDSyncCallback);
  if (result < 0) {
    return result;
  }
  return __CARDSync(chan);
}
