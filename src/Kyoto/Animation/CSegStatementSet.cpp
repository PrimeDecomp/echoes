#include "Kyoto/Animation/CSegStatementSet.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"

#include <dolphin/os/OSCache.h>

// The splits place this constant in the CSegStatementSet .sdata2 range.
const ushort CSoundPOINode::skExtendedVersion = 1;

namespace {
const int kSegmentCount = 100;
const int kSegmentSetSize = kSegmentCount * sizeof(CSegStatement);
const int kCacheSlotCount = 3;
int sFreeSegments = (1 << kCacheSlotCount) - 1;

inline void* AllocateSegment() {
  LCQueueWait(0);
  if (sFreeSegments) {
    for (uint i = 0; i < kCacheSlotCount; ++i) {
      if ((sFreeSegments & (1 << i)) != 0) {
        sFreeSegments ^= 1 << i;
        char* base = static_cast< char* >(LCGetBase());
        base += i * kSegmentSetSize;
        return base;
      }
    }
  }

  return CMemory::Alloc(kSegmentSetSize);
}

inline void FreeSegment(CSegStatement* seg) {
  char* base = static_cast< char* >(LCGetBase());
  char* ptr = reinterpret_cast< char* >(seg);
  if (ptr >= base && ptr < base + kCacheSlotCount * kSegmentSetSize) {
    int index = (ptr - base) / kSegmentSetSize;
    sFreeSegments |= 1 << index;
  } else {
    CMemory::Free(seg);
  }
}
} // namespace

CSegStatementSet::CSegStatementSet(void* storage)
: mSegData(static_cast< CSegStatement* >(storage)) {
  for (int i = 0; i < kSegmentCount; ++i) {
    new (&mSegData[i]) CSegStatement;
  }
}

CStackSegStatementSet::CStackSegStatementSet() : CSegStatementSet(AllocateSegment()) {}

CStackSegStatementSet::~CStackSegStatementSet() { FreeSegment(mSegData); }
