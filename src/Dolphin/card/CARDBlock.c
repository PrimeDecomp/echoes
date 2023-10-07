#include <dolphin/card.h>
#include <dolphin/dsp.h>
#include <dolphin/dvd.h>
#include <dolphin/os.h>

#include <dolphin/CARDPriv.h>

#line 56
u16* __CARDGetFatBlock(CARDControl* card) {
  ASSERT(card->currentFat);
  return card->currentFat;
}

#line 72
static void WriteCallback(s32 chan, s32 result) {
  CARDControl* card;
  CARDCallback callback;
  u16* fat0;
  u16* fat1;

  card = &__CARDBlock[chan];
  if (result >= 0) {
    fat0 = (u16*)((u8*)card->workArea + 0x6000);
    fat1 = (u16*)((u8*)card->workArea + 0x8000);
    ASSERT(card->currentFat);

    if (card->currentFat == fat0) {

      card->currentFat = fat1;
      memcpy(fat1, fat0, 0x2000);
    } else {

      ASSERT(card->currentFat == fat1);
      card->currentFat = fat0;
      memcpy(fat0, fat1, 0x2000);
    }
  }

  if (card->apiCallback == NULL) {
    __CARDPutControlBlock(card, result);
  }

  callback = card->eraseCallback;
  if (callback) {
    card->eraseCallback = NULL;
    callback(chan, result);
  }
}

static void EraseCallback(s32 chan, s32 result) {
  CARDControl* card;
  CARDCallback callback;
  u16* fat;
  u32 addr;

  card = &__CARDBlock[chan];
  if (result < 0) {
    goto error;
  }

  fat = __CARDGetFatBlock(card);
  addr = ((u32)fat - (u32)card->workArea) / CARD_SYSTEM_BLOCK_SIZE * card->sectorSize;
  result = __CARDWrite(chan, addr, CARD_SYSTEM_BLOCK_SIZE, fat, WriteCallback);
  if (result < 0) {
    goto error;
  }

  return;

error:
  if (card->apiCallback == NULL) {
    __CARDPutControlBlock(card, result);
  }
  callback = card->eraseCallback;
  if (callback) {
    card->eraseCallback = NULL;
    callback(chan, result);
  }
}

#line 174
s32 __CARDAllocBlock(s32 chan, u32 cBlock, CARDCallback callback) {
  CARDControl* card;
  u16* fat;
  u16 iBlock;
  u16 startBlock;
  u16 prevBlock;
  u16 count;

  ASSERT(0 < cBlock);
  ASSERT(0 <= chan && chan < 2);
  card = &__CARDBlock[chan];
  if (!card->attached) {
    return CARD_RESULT_NOCARD;
  }

  fat = __CARDGetFatBlock(card);
  if (fat[3] < cBlock) {
    return CARD_RESULT_INSSPACE;
  }

  fat[3] -= cBlock;
  startBlock = 0xFFFF;
  iBlock = fat[4];
  count = 0;
  while (0 < cBlock) {
    if (card->cBlock - 5 < ++count) {
      return CARD_RESULT_BROKEN;
    }

    iBlock++;
    if (!CARDIsValidBlockNo(card, iBlock)) {
      iBlock = 5;
    }

    if (fat[iBlock] == 0x0000u) {
      if (startBlock == 0xFFFF) {
        startBlock = iBlock;
      } else {
        fat[prevBlock] = iBlock;
      }
      prevBlock = iBlock;
      fat[iBlock] = 0xFFFF;
      --cBlock;
    }
  }
  fat[4] = iBlock;
  card->startBlock = startBlock;

  return __CARDUpdateFatBlock(chan, fat, callback);
}

#line 248
s32 __CARDFreeBlock(s32 chan, u16 nBlock, CARDCallback callback) {
  CARDControl* card;
  u16* fat;
  u16 nextBlock;

  ASSERT(0 <= chan && chan < 2);
  card = &__CARDBlock[chan];
  if (!card->attached) {
    return CARD_RESULT_NOCARD;
  }

  fat = __CARDGetFatBlock(card);
  while (nBlock != 0xFFFF) {
    if (!CARDIsValidBlockNo(card, nBlock)) {
      return CARD_RESULT_BROKEN;
    }

    nextBlock = fat[nBlock];
    fat[nBlock] = 0;
    nBlock = nextBlock;
    ++fat[3];
  }

  return __CARDUpdateFatBlock(chan, fat, callback);
}

#line 291
s32 __CARDUpdateFatBlock(s32 chan, u16* fat, CARDCallback callback) {
  CARDControl* card;
  u32 addr;

  ASSERT(0 <= chan && chan < 2);
  card = &__CARDBlock[chan];
  ++fat[2];
  __CARDCheckSum(fat + 2, 0x1FFC, fat, fat + 1);
  DCStoreRange(fat, 0x2000);
  card->eraseCallback = callback;

  addr = (((u32)fat - (u32)card->workArea) / 8192u) * card->sectorSize;
  return __CARDEraseSector(chan, addr, EraseCallback);
}
