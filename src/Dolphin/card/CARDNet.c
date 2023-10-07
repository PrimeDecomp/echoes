#include <dolphin/card.h>
#include <dolphin/dsp.h>
#include <dolphin/dvd.h>
#include <dolphin/os.h>

#include <dolphin/CARDPriv.h>
#include <dolphin/OSRtcPriv.h>

u16 __CARDVendorID = 0xffff;
u8 __CARDPermMask = CARD_DIR_PERM_PUBLIC | CARD_DIR_PERM_NO_COPY | CARD_DIR_PERM_NO_MOVE;

u16 CARDSetVendorID(u16 vendorID) {
  u16 prevID;
  prevID = __CARDVendorID;
  __CARDVendorID = vendorID;
  return prevID;
}

u16 CARDGetVendorID() { return __CARDVendorID; }

#line 98
s32 CARDGetSerialNo(s32 chan, u64* serialNo) {
  CARDControl* card;
  s32 result;
  CARDID* id;
  u64 code;
  int i;

  ASSERT(0 <= chan && chan < 2);

  if (!(0 <= chan && chan < 2)) {
    return CARD_RESULT_FATAL_ERROR;
  }

  result = __CARDGetControlBlock(chan, &card);
  if (result < 0) {
    return result;
  }

  id = (CARDID*)card->workArea;
  for (code = 0, i = 0; i < sizeof(id->serial) / sizeof(u64); ++i) {
    code ^= *(u64*)&id->serial[sizeof(u64) * i];
  }
  *serialNo = code;

  return __CARDPutControlBlock(card, CARD_RESULT_READY);
}

#line 141
s32 CARDGetUniqueCode(s32 chan, u64* uniqueCode) {
  CARDControl* card;
  s32 result;
  OSSramEx* sram;

  ASSERT(0 <= chan && chan < 2);
  if (!(0 <= chan && chan < 2)) {
    return CARD_RESULT_FATAL_ERROR;
  }

  result = __CARDGetControlBlock(chan, &card);
  if (result < 0) {
    return result;
  }

  sram = __OSLockSramEx();
  memcpy(uniqueCode, sram->flashID[chan] + 4, 8);
  __OSUnlockSramEx(FALSE);

  return __CARDPutControlBlock(card, CARD_RESULT_READY);
}

s32 CARDGetAttributes(s32 chan, s32 fileNo, u8* attr) {
  CARDDir dirent;
  s32 result;

  result = __CARDGetStatusEx(chan, fileNo, &dirent);
  if (result == CARD_RESULT_READY) {
    *attr = dirent.permission;
  }
  return result;
}

s32 CARDSetAttributesAsync(s32 chan, s32 fileNo, u8 attr, CARDCallback callback) {
  CARDDir dirent;
  s32 result;

  if (attr & ~__CARDPermMask) {
    return CARD_RESULT_NOPERM;
  }

  result = __CARDGetStatusEx(chan, fileNo, &dirent);
  if (result < 0) {
    return result;
  }

  if ((dirent.permission & CARD_DIR_PERM_GLOBAL) && !(attr & CARD_DIR_PERM_GLOBAL) ||
      (dirent.permission & CARD_DIR_PERM_COMPANY) && !(attr & CARD_DIR_PERM_COMPANY)) {
    return CARD_RESULT_NOPERM;
  }

  if ((attr & CARD_DIR_PERM_GLOBAL) && (attr & CARD_DIR_PERM_COMPANY) ||
      (attr & CARD_DIR_PERM_GLOBAL) && (dirent.permission & CARD_DIR_PERM_COMPANY) ||
      (attr & CARD_DIR_PERM_COMPANY) && (dirent.permission & CARD_DIR_PERM_GLOBAL)) {
    return CARD_RESULT_NOPERM;
  }

  dirent.permission = attr;
  return __CARDSetStatusExAsync(chan, fileNo, &dirent, callback);
}

s32 CARDSetAttributes(s32 chan, s32 fileNo, u8 attr) {
  s32 result;

  result = CARDSetAttributesAsync(chan, fileNo, attr, __CARDSyncCallback);
  if (result < 0) {
    return result;
  }
  return __CARDSync(chan);
}

static BOOL __CARDEnablePerm(u8 perm, BOOL enable) {
  BOOL prev;

  prev = (__CARDPermMask & perm) ? TRUE : FALSE;
  if (enable) {
    __CARDPermMask |= perm;
  } else {
    __CARDPermMask &= ~perm;
  }
  return prev;
}

BOOL __CARDEnableGlobal(BOOL enable) { return __CARDEnablePerm(CARD_DIR_PERM_GLOBAL, enable); }

BOOL __CARDEnableCompany(BOOL enable) { return __CARDEnablePerm(CARD_DIR_PERM_COMPANY, enable); }
