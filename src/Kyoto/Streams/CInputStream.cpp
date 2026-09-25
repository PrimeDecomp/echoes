#include "Kyoto/Streams/CInputStream.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "rstl/math.hpp"
#include "string.h"

CInputStream::CInputStream(const void* ptr, unsigned long len)
: mBuffer(const_cast< uchar* >(reinterpret_cast< const uchar* >(ptr)))
, mLength(len)
, mOwned(false) {
  mPtr = mBuffer;
}

CInputStream::CInputStream(const void* ptr, unsigned long len, bool owned)
: mBuffer(const_cast< uchar* >(reinterpret_cast< const uchar* >(ptr)))
, mLength(len)
, mOwned(owned) {
  mPtr = mBuffer;
}

CInputStream::CInputStream(const SBufferAndSize& buffer, bool owned)
: mBuffer(const_cast< uchar* >(reinterpret_cast< const uchar* >(buffer.mBuffer)))
, mLength(buffer.mSize)
, mOwned(owned) {
  mPtr = mBuffer;
}

CInputStream::~CInputStream() {
  if (mOwned) {
    delete[] mBuffer;
  }
}

void CInputStream::Get(void* dest, unsigned long len) {
  memcpy(dest, mPtr, len);
  mPtr += len;
}

const void* CInputStream::Get(unsigned long len) {
  const void* result = mPtr;
  mPtr += len;
  return result;
}

size_t CInputStream::ReadBytes(void* dest, size_t len) {
  size_t count = mLength - (mPtr - mBuffer);
  if (len < count) {
    count = len;
  }
  count = rstl::max_val(size_t(0), count);
  if (count != 0) {
    if (dest != nullptr) {
      memcpy(dest, mPtr, count);
    }
    mPtr += count;
  }
  return count;
}

float CInputStream::ReadFloat() {
  static float f;
  *reinterpret_cast< uint* >(&f) = ReadInt32();
  return f;
}

rstl::auto_ptr< uchar > CInputStream::ReleaseBuffer() {
  mOwned = false;
  return rstl::auto_ptr< uchar >(mBuffer);
}
