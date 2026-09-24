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
: x0_heapSize(heapSize)
, x4_(unk1)
, x8_(unk2)
, xc_(unk3)
, x10_(unk4)
, x14_heapSize2(heapSize2)
, x18_(unk5)
, x1c_(unk6)
, x20_(unk7)
, x24_(unk8)
, x28_(unk9)
, x2c_(unk10)
, x30_smallNumAllocs(smallAllocNumAllocs)
, x34_smallAllocatedSize(smallAllocAllocatedSize)
, x38_smallRemainingSize(smallAllocRemainingSize)
, x3c_mediumNumAllocs(mediumAllocNumAllocs)
, x40_mediumAllocatedSize(mediumAllocAllocatedSize)
, x44_mediumBlocksAvailable(mediumAllocBlocksAvailable)
, x48_(unk11)
, x4c_(unk12)
, x50_(unk13)
, x54_mediumTotalAllocated(mediumAllocTotalAllocated)
, x58_fakeStatics(fakeStatics) {}

IAllocator::~IAllocator() {}
