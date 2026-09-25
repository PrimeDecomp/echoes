#include "Kyoto/Alloc/IAllocator.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/COsContext.hpp"

IAllocator::SMetrics::SMetrics(uint heapSize, uint unk1, uint unk2, uint unk3, uint unk4,
                               uint heapSize2, uint unk5, uint unk6, uint unk7, uint unk8,
                               uint unk9, uint unk10, uint smallAllocNumAllocs,
                               uint smallAllocAllocatedSize, uint smallAllocRemainingSize,
                               uint mediumAllocNumAllocs, uint mediumAllocAllocatedSize,
                               uint mediumAllocBlocksAvailable, uint unk11, uint unk12, uint unk13,
                               uint mediumAllocTotalAllocated, uint fakeStatics)
: mHeapSize(heapSize)
, x4_(unk1)
, x8_(unk2)
, xc_(unk3)
, x10_(unk4)
, mHeapSize2(heapSize2)
, x18_(unk5)
, x1c_(unk6)
, x20_(unk7)
, x24_(unk8)
, x28_(unk9)
, x2c_(unk10)
, mSmallNumAllocs(smallAllocNumAllocs)
, mSmallAllocatedSize(smallAllocAllocatedSize)
, mSmallRemainingSize(smallAllocRemainingSize)
, mMediumNumAllocs(mediumAllocNumAllocs)
, mMediumAllocatedSize(mediumAllocAllocatedSize)
, mMediumBlocksAvailable(mediumAllocBlocksAvailable)
, x48_(unk11)
, x4c_(unk12)
, x50_(unk13)
, mMediumTotalAllocated(mediumAllocTotalAllocated)
, mFakeStatics(fakeStatics) {}

IAllocator::~IAllocator() {}
