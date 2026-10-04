#include "MetroidPrime/CMemoryDrawEnum.hpp"

#include "Kyoto/Alloc/IAllocator.hpp"

uint CMemoryDrawEnum::mWorldMemory = 0;

IAllocator::SMetrics::SMetrics(const SMetrics& other)
: mHeapSize(other.mHeapSize)
, x4_(other.x4_)
, x8_(other.x8_)
, xc_(other.xc_)
, x10_(other.x10_)
, mHeapSize2(other.mHeapSize2)
, x18_(other.x18_)
, x1c_(other.x1c_)
, x20_(other.x20_)
, x24_(other.x24_)
, x28_(other.x28_)
, x2c_(other.x2c_)
, mSmallNumAllocs(other.mSmallNumAllocs)
, mSmallAllocatedSize(other.mSmallAllocatedSize)
, mSmallRemainingSize(other.mSmallRemainingSize)
, mMediumNumAllocs(other.mMediumNumAllocs)
, mMediumAllocatedSize(other.mMediumAllocatedSize)
, mMediumBlocksAvailable(other.mMediumBlocksAvailable)
, x48_(other.x48_)
, x4c_(other.x4c_)
, x50_(other.x50_)
, mMediumTotalAllocated(other.mMediumTotalAllocated)
, mFakeStatics(other.mFakeStatics) {}

NESTED_CHECK_SIZEOF(IAllocator, SMetrics, 0x5c)
