#include "Kyoto/CFrameDelayedKiller.hpp"

#include "Kyoto/Particles/CElementAllocationChunk.hpp"
#include "Kyoto/Particles/IElement.hpp"

#include <dolphin/gx/GXManage.h>
#include <rstl/list.hpp>

static uint sCurList = 0;
static bool sInitialized = false;
static rstl::list< void* > sFrameDelayedList[2];

void CFrameDelayedKiller::Initialize() {
  sInitialized = true;
  StallAndFlushAllAllocations();
}

void CFrameDelayedKiller::ShutDown() {
  StallAndFlushAllAllocations();
  sInitialized = false;
}

void CFrameDelayedKiller::FlushAllAllocations() {
  for (int i = 0; i < 2; ++i) {
    FlushAllocationsForFrame();
  }
}

void CFrameDelayedKiller::StallAndFlushAllAllocations() {
  GXDrawDone();
  FlushAllAllocations();
}

void CFrameDelayedKiller::ScheduleDeletion(const EWhichFrame whichFrame, void* victim) {
  if (victim == nullptr) {
    return;
  }
  if (!sInitialized) {
    GXDrawDone();
    CMemory::Free(victim);
    return;
  }

  uint index = whichFrame == kWhichFrame_NextFrame ? sCurList : sCurList ^ 1;

  sFrameDelayedList[index].push_back(victim);
}

void CFrameDelayedKiller::FlushAllocationsForFrame() {
  sCurList ^= 1;
  rstl::list< void* >& list = sFrameDelayedList[sCurList];
  for (rstl::list< void* >::iterator t = list.begin(); t != list.end(); ++t) {
    CMemory::Free(*t);
  }

  list.clear();
}

CElementAllocationChunk::CElementAllocationChunk()
: mCapacity(256), mAllocatedWords(0), mAllocationCount(0) {}

bool CElementAllocationChunk::CanAllocate(uint size) const {
  return mCapacity > mAllocatedWords + (size + 3) / 4;
}

bool CElementAllocationChunk::Contains(const void* ptr) const {
  int offset = static_cast< const char* >(ptr) - reinterpret_cast< const char* >(mData);
  int index = offset / 4;
  return mCapacity > index;
}

void* CElementAllocationChunk::Allocate(uint size) {
  void* ptr = &mData[mAllocatedWords];
  mAllocatedWords += (size + 3) / 4;
  ++mAllocationCount;
  return ptr;
}

void CElementAllocationChunk::Free(void*) { --mAllocationCount; }

void CElementAllocationChunk::Rewind(uint size) {
  uint words = (size + 3) / 4;
  if (words > mAllocatedWords) {
    mAllocatedWords = 0;
  } else {
    mAllocatedWords -= words;
  }
}

uint CElementAllocationChunk::GetAllocatedSize() const { return mAllocatedWords * 4; }

uint CElementAllocationChunk::GetAllocationCount() const { return mAllocationCount; }

void* IElement::operator new(size_t sz, const char* fileAndLine, const char* type) {
  return CElementAllocator::Alloc(sz, fileAndLine, type);
}

void IElement::operator delete(void* ptr, const size_t sz) { CElementAllocator::Free(ptr, sz); }
