#include "Kyoto/Streams/CInputStream.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "rstl/math.hpp"
#include "string.h"

CInputStream::CInputStream(const void* ptr, unsigned long len)
: x4_buffer(const_cast< uchar* >(reinterpret_cast< const uchar* >(ptr)))
, xc_length(len)
, x10_owned(false) {
  x8_ptr = x4_buffer;
}

CInputStream::CInputStream(const void* ptr, unsigned long len, bool owned)
: x4_buffer(const_cast< uchar* >(reinterpret_cast< const uchar* >(ptr)))
, xc_length(len)
, x10_owned(owned) {
  x8_ptr = x4_buffer;
}

CInputStream::CInputStream(const rstl::pair< const void*, unsigned long >& buffer, bool owned)
: x4_buffer(const_cast< uchar* >(reinterpret_cast< const uchar* >(buffer.first)))
, xc_length(buffer.second)
, x10_owned(owned) {
  x8_ptr = x4_buffer;
}

CInputStream::~CInputStream() {
  if (x10_owned) {
    delete[] x4_buffer;
  }
}

void CInputStream::Get(void* dest, unsigned long len) {
  memcpy(dest, x8_ptr, len);
  x8_ptr += len;
}

const void* CInputStream::Get(unsigned long len) {
  const void* result = x8_ptr;
  x8_ptr += len;
  return result;
}

size_t CInputStream::ReadBytes(void* dest, size_t len) {
  size_t count = xc_length - (x8_ptr - x4_buffer);
  if (len < count) {
    count = len;
  }
  count = rstl::max_val(size_t(0), count);
  if (count != 0) {
    if (dest != nullptr) {
      memcpy(dest, x8_ptr, count);
    }
    x8_ptr += count;
  }
  return count;
}

float CInputStream::ReadFloat() {
  static float f;
  *reinterpret_cast< uint* >(&f) = ReadInt32();
  return f;
}

rstl::auto_ptr< uchar > CInputStream::ReleaseBuffer() {
  x10_owned = false;
  return rstl::auto_ptr< uchar >(x4_buffer);
}
