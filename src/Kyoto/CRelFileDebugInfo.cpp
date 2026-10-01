#include "Kyoto/CRelFileDebugInfo.hpp"

#include "Kyoto/Alloc/CMemory.hpp"

CRelFileDebugInfo* CRelFileDebugInfo::mspHead = nullptr;

CRelFileDebugInfo::CRelFileDebugInfo()
: mName(nullptr), mStart(0), mSize(0), mNext(nullptr), mPrev(nullptr) {}

CRelFileDebugInfo::~CRelFileDebugInfo() { Unregister(); }

void CRelFileDebugInfo::Register(const char* name, const void* start, uint size) {
  if (mNext != nullptr) {
    return;
  }
  mName = name;
  mStart = reinterpret_cast< uint >(start);
  mSize = size;
  mNext = mspHead;
  if (mNext != nullptr) {
    mNext->mPrev = this;
  }
  mspHead = this;
}

void CRelFileDebugInfo::Unregister() {
  if (mspHead == this) {
    mspHead = mNext;
  }
  if (mNext != nullptr) {
    mNext->mPrev = mPrev;
  }
  if (mPrev != nullptr) {
    mPrev->mNext = mNext;
  }
  mNext = nullptr;
  mPrev = nullptr;
}

bool CRelFileDebugInfo::Contains(int address) const { return address - mStart < mSize; }

CRelFileDebugInfo* CRelFileDebugInfo::FindByAddress(int address) {
  for (CRelFileDebugInfo* info = mspHead; info != nullptr; info = info->mNext) {
    if (info->Contains(address)) {
      return info;
    }
  }
  return nullptr;
}
