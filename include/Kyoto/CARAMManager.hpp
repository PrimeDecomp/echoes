#ifndef _CARAMMANAGER
#define _CARAMMANAGER

#include "types.h"

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

  static void Shutdown();
  static void CollectGarbage();
  static void PreInitializeAlloc(uint size) { mPreInitializeAlloc += size; }
  static void Initialize(uint);
  static void WaitForAllDMAsToComplete();
  static const void* GetInvalidAlloc();// { return (const void*)kInvalidAlloc; }
  static const uint GetInvalidDMAHandle() { return kInvalidHandle; }
  static bool CancelDMA(uint);
  static void WaitForDMACompletion(uint);
  static bool IsDMACompleted(uint handle);
  static void* Alloc(uint len, const unkptr = nullptr);
  static void Free(const void* ptr, const unkptr = nullptr);
  static int DMAToARAM(void*, void*, uint, EDMAPriority);
  static int DMAToMRAM(void*, void*, uint, EDMAPriority);
  // TODO: Verify name against behavior
  static bool IsAllocValid(const void* ptr);
private:
  static uint mPreInitializeAlloc;
  static const int kInvalidAlloc;
  static const int kInvalidHandle;
};

#endif // _CARAMMANAGER
