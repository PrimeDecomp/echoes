#include "Kyoto/Streams/COutputStream.hpp"

#include "Kyoto/Streams/StreamSupport.hpp"

#include "Kyoto/Alloc/CMemory.hpp"

#include <stdint.h>
#include <string.h>

COutputStream::COutputStream(const int len)
: mUnwrittenLen(0)
, mBufLen(len)
, mBufPtr(len > 64 ? rs_new uchar[len]
                   : &mScratch[32 - reinterpret_cast< uintptr_t >(mScratch) % 31])
, mNumWrites(0) {}

COutputStream::~COutputStream() {
  if (mBufLen > 64) {
    delete[] static_cast< const uchar* >(mBufPtr);
  }
}

void COutputStream::DoPut(const void* ptr, const size_t len) {
  uint offset = 0;
  const uchar* offsetPtr = nullptr;
  uint tempLen = len;
  if (tempLen != 0) {
    mNumWrites += tempLen;
    if (tempLen + mUnwrittenLen <= mBufLen) {
      memcpy(static_cast< uchar* >(mBufPtr) + mUnwrittenLen, ptr, tempLen);
      mUnwrittenLen += tempLen;
      return;
    }

    offsetPtr = static_cast< const uchar* >(ptr) + tempLen;
    while (tempLen != 0) {
      const uint count = mBufLen - mUnwrittenLen;
      offset = count;
      if (tempLen < count) {
        offset = tempLen;
      }
      if (offset != 0) {
        memcpy(static_cast< uchar* >(mBufPtr) + mUnwrittenLen, offsetPtr - tempLen, offset);

        tempLen -= offset;
        mUnwrittenLen += offset;
      } else {
        DoFlush();
      }
    }
  }
}

void COutputStream::DoFlush() {
  if (mUnwrittenLen != 0) {
    Write(mBufPtr, mUnwrittenLen);
    mUnwrittenLen = 0;
  }
}
