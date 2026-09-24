#ifndef _CARAMMANAGER
#define _CARAMMANAGER

#include "types.h"

#include <dolphin/arq.h>

#include <rstl/list.hpp>

class CARAMManager {
public:
  enum EDMAPriority {
    kDMAPrio_Zero,
    kDMAPrio_One,
    kDMAPrio_Two,
    kDMAPrio_Three,
    kDMAPrio_Four,
    kDMAPrio_Five,
    kDMAPrio_Six,
  };

  static bool Initialize(uint chunkSize, uint size, uint secondChunkSize);
  static void Shutdown();
  static void* Alloc(uint len, int pool = 0);
  static bool Free(const void* ptr, int pool = 0);
  static int DMAToARAM(void*, void*, uint, EDMAPriority);
  static int DMAToMRAM(void*, void*, uint, EDMAPriority);
  static bool IsDMACompleted(uint handle);
  static void WaitForDMACompletion(uint handle);
  static void WaitForAllDMAsToComplete();
  static bool CancelDMA(uint);
  static void AramManagerDMACallback(u32 result);
  static void RefreshActiveDMAList();
  static void CollectGarbage();
  static bool IsAllocValid(const void* ptr);
  static const void* GetInvalidAlloc();

  static void PreInitializeAlloc(uint size) { mPreInitializeAlloc += size; }
  static const uint GetInvalidDMAHandle() { return kInvalidHandle; }
  static uint GetAndIncrementUniqueID() {
    mDMAUniqueID++;
    if (mDMAUniqueID == GetInvalidDMAHandle()) {
      mDMAUniqueID++;
    }
    return mDMAUniqueID;
  }

private:
  struct SAramDMARequest {
    ARQRequest mRequest;
    uint mUniqueID;
    bool mComplete;
  };

  class CAramPool;
  friend class CAramPool;

  static bool mbInitialized;
  static uint mDMAUniqueID;
  static CAramPool* mPools[2];
  static uint mPreInitializeAlloc;
  static rstl::list< SAramDMARequest* > mActiveDMAs;
  static const uint kFreeChunk;
  static const int kInvalidHandle;
};

#endif // _CARAMMANAGER
