#include "Kyoto/CARAMManager.hpp"

#include "Kyoto/Alloc/CMemory.hpp"

#include "rstl/math.hpp"

#include <dolphin/ar.h>
#include <dolphin/arq.h>
#include <dolphin/os.h>

class CARAMManager::CAramPool {
public:
  CAramPool(uint chunkSize, uint numChunks)
  : mpARAMStart(ARAlloc(chunkSize * numChunks))
  , mChunkSize(chunkSize)
  , mNumChunks(numChunks)
  , mChunksAllocated(0)
  , mMaxChunksAllocated(0)
  , mpBookKeepingMemory((uint*)CMemory::Alloc(numChunks * 4, IAllocator::kHI_None,
                                              IAllocator::kSC_Unk1, IAllocator::kTP_Heap,
                                              CCallStack(-1, "\?\?(\?\?)"))) {
    for (uint i = 0; i < mNumChunks; ++i) {
      mpBookKeepingMemory[i] = kFreeChunk;
    }
    CMemory::OffsetFakeStatics(mNumChunks * 4);
  }
  ~CAramPool();

  void* Alloc(uint len);
  uint FindFreeBlocks(uint start, uint end, uint count);
  bool Free(const void* ptr);

private:
  u32 mpARAMStart;
  uint mChunkSize;
  uint mNumChunks;
  uint mChunksAllocated;
  uint mMaxChunksAllocated;
  uint* mpBookKeepingMemory;
};

bool CARAMManager::mbInitialized = false;
uint CARAMManager::mDMAUniqueID = 0;
CARAMManager::CAramPool* CARAMManager::mPools[2];
uint CARAMManager::mPreInitializeAlloc = 16 * 1024;
const uint CARAMManager::kFreeChunk = 0;
const int CARAMManager::kInvalidHandle = -1;

rstl::list< CARAMManager::SAramDMARequest* > CARAMManager::mActiveDMAs;

CARAMManager::CAramPool::~CAramPool() {
  u32 unk = 0;
  CMemory::OffsetFakeStatics(-mNumChunks * 4);
  ARFree(&unk);
  CMemory::Free(mpBookKeepingMemory);
}

void* CARAMManager::CAramPool::Alloc(uint len) {
  uint chunkCount = (mChunkSize - 1 + len) / mChunkSize;
  uint block = FindFreeBlocks(0, mNumChunks, chunkCount);

  if (block == -1) {
    return (void*)-1;
  }

  mChunksAllocated += chunkCount;
  mMaxChunksAllocated = rstl::max_val(mMaxChunksAllocated, mChunksAllocated);
  uint blockOffset = mpARAMStart + block * mChunkSize;
  mpBookKeepingMemory[block] = chunkCount;

  while (--chunkCount != 0) {
    ++block;
    mpBookKeepingMemory[block] = -1;
  }

  return (void*)blockOffset;
}

uint CARAMManager::CAramPool::FindFreeBlocks(uint start, uint end, uint count) {
  while (start < end) {
    if (mpBookKeepingMemory[start] == 0) {
      if (count == 1) {
        return start;
      }

      ++start;
      int found = 1;
      while (start < end) {
        uint tmp = mpBookKeepingMemory[start];
        if (tmp != 0) {
          start += tmp;
          break;
        }

        found++;
        if (found == count) {
          return start - (count - 1);
        }
        ++start;
      }
    } else {
      start += mpBookKeepingMemory[start];
    }
  }
  return -1;
}

bool CARAMManager::CAramPool::Free(const void* ptr) {
  if (!IsAllocValid(ptr)) {
    return false;
  }

  uint blockStart = (reinterpret_cast< u32 >(ptr) - mpARAMStart) / mChunkSize;
  uint blockCount = mpBookKeepingMemory[blockStart];
  mChunksAllocated -= blockCount;
  while (blockCount--) {
    mpBookKeepingMemory[blockStart++] = 0;
  }
  return true;
}


bool CARAMManager::Initialize(uint chunkSize, uint size, uint secondChunkSize) {
  uint aramSize = ARGetSize() - mPreInitializeAlloc;
  uint numChunks = size / chunkSize;
  uint secondNumChunks = (aramSize - size) / secondChunkSize;
  mPools[1] = rs_new CAramPool(chunkSize, numChunks);
  mPools[0] = rs_new CAramPool(secondChunkSize, secondNumChunks);
  mDMAUniqueID = 0;
  mbInitialized = true;
  return true;
}

void CARAMManager::Shutdown() {
  WaitForAllDMAsToComplete();
  delete mPools[0];
  delete mPools[1];
  mbInitialized = false;
}

void* CARAMManager::Alloc(uint len, int pool) { return mPools[pool]->Alloc(len); }

bool CARAMManager::Free(const void* ptr, int pool) { return mPools[pool]->Free(ptr); }

int CARAMManager::DMAToARAM(void* src, void* dest, uint len, EDMAPriority priority) {
  DCFlushRange(src, len);
  SAramDMARequest* req = rs_new SAramDMARequest();
  req->mComplete = false;
  req->mUniqueID = mDMAUniqueID;
  mActiveDMAs.push_back(req);
  ARQPostRequest(&req->mRequest, req->mUniqueID, ARQ_TYPE_MRAM_TO_ARAM,
                 (priority == kDMAPrio_One) ? ARQ_PRIORITY_HIGH : ARQ_PRIORITY_LOW,
                 reinterpret_cast< u32 >(src), reinterpret_cast< u32 >(dest), len,
                 AramManagerDMACallback);

  GetAndIncrementUniqueID();
  return req->mUniqueID;
}

int CARAMManager::DMAToMRAM(void* src, void* dest, uint len, EDMAPriority priority) {
  DCInvalidateRange(dest, len);
  SAramDMARequest* req = rs_new SAramDMARequest();
  req->mComplete = false;
  req->mUniqueID = mDMAUniqueID;
  mActiveDMAs.push_back(req);
  ARQPostRequest(&req->mRequest, req->mUniqueID, ARQ_TYPE_ARAM_TO_MRAM,
                 (priority == kDMAPrio_One) ? ARQ_PRIORITY_HIGH : ARQ_PRIORITY_LOW,
                 reinterpret_cast< u32 >(src), reinterpret_cast< u32 >(dest), len,
                 AramManagerDMACallback);

  GetAndIncrementUniqueID();
  return req->mUniqueID;
}

bool CARAMManager::IsDMACompleted(uint handle) {
  rstl::list< SAramDMARequest* >::iterator it = mActiveDMAs.begin();
  for (; it != mActiveDMAs.end(); ++it) {
    uint uniqueId = (*it)->mUniqueID;
    if (uniqueId == handle) {
      if ((*it)->mComplete) {
        delete (*it);
        mActiveDMAs.erase(it);
        return true;
      }
      return false;
    }
  }

  return true;
}

void CARAMManager::WaitForDMACompletion(uint handle) {
  rstl::list< SAramDMARequest* >::iterator it = mActiveDMAs.begin();
  for (; it != mActiveDMAs.end(); ++it) {
    uint uniqueId = (*it)->mUniqueID;
    if (uniqueId == handle) {
      // Spin until complete!
      while (!(*it)->mComplete)
        ;
      delete (*it);
      mActiveDMAs.erase(it);
      return;
    }
  }
}

void CARAMManager::WaitForAllDMAsToComplete() {
  while ((int)mActiveDMAs.size() > 0) {
    RefreshActiveDMAList();
  }
}

bool CARAMManager::CancelDMA(uint handle) {
  rstl::list< SAramDMARequest* >::iterator it = mActiveDMAs.begin();
  for (; it != mActiveDMAs.end(); ++it) {
    uint uniqueId = (*it)->mUniqueID;
    if (uniqueId == handle) {
      return (*it)->mComplete != false;
    }
  }
  return true;
}

void CARAMManager::AramManagerDMACallback(u32 result) {
  SAramDMARequest* req = reinterpret_cast< SAramDMARequest* >(result);
  req->mComplete = true;
  if (req->mRequest.type == ARQ_TYPE_ARAM_TO_MRAM) {
    DCInvalidateRange(reinterpret_cast< void* >(req->mRequest.dest), req->mRequest.length);
  }
}

void CARAMManager::RefreshActiveDMAList() {
  rstl::list< SAramDMARequest* >::iterator it = mActiveDMAs.begin();
  while (it != mActiveDMAs.end()) {
    if ((*it)->mComplete) {
      delete (*it);
      it = mActiveDMAs.erase(it);
      continue;
    }
    ++it;
  }
}

void CARAMManager::CollectGarbage() { RefreshActiveDMAList(); }

bool CARAMManager::IsAllocValid(const void* ptr) {
  return reinterpret_cast< const void* >(-1) != ptr;
}

const void* CARAMManager::GetInvalidAlloc() { return reinterpret_cast< const void* >(-1); }
