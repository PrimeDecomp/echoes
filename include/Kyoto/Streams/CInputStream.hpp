#ifndef _CINPUTSTREAM
#define _CINPUTSTREAM

#include "types.h"

#include "stddef.h"

#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"

class CInputStream;
template < typename T >
struct TType {};

template < typename T >
inline TType< T > TGetType(const T&) {
  return TType< T >();
}

class CInputStream {
public:
  struct SBufferAndSize {
    const void* mBuffer;
    unsigned long mSize;

    SBufferAndSize(const void* buffer, unsigned long size) : mBuffer(buffer), mSize(size) {}
  };

  CInputStream(const void* ptr, unsigned long len);
  CInputStream(const void* ptr, unsigned long len, bool owned);
  CInputStream(const SBufferAndSize& buffer, bool owned);
  virtual ~CInputStream();

  float ReadFloat();
  size_t ReadBytes(void* dest, size_t len);
  void Get(void* dest, unsigned long len);
  const void* Get(unsigned long len);
  rstl::auto_ptr< uchar > ReleaseBuffer();

  template < typename T >
  T Get(const TType< T >& type = TType< T >());

  int ReadInt32() {
    int* result = reinterpret_cast< int* >(mPtr);
    mPtr = reinterpret_cast< uchar* >(result + 1);
    return *result;
  }
  u16 ReadUint16() {
    u16* result = reinterpret_cast< u16* >(mPtr);
    mPtr = reinterpret_cast< uchar* >(result + 1);
    return *result;
  }
  short ReadInt16() { return static_cast< short >(ReadUint16()); }
  u8 ReadUint8() {
    u8* result = mPtr;
    mPtr = result + 1;
    return *result;
  }
  char ReadInt8() { return static_cast< char >(ReadUint8()); }
  bool ReadBool() { return ReadUint8() != 0; }
  uint GetReadPosition() const { return mPtr - mBuffer; }

private:
  uchar* mBuffer;
  uchar* mPtr;
  unsigned long mLength;
  bool mOwned;
};

CHECK_SIZEOF(CInputStream, 0x14)

template < typename T >
inline T CInputStream::Get(const TType< T >& type) {
  return T(*this);
}

template <>
inline bool CInputStream::Get< bool >(const TType< bool >& type) {
  return ReadBool();
}

template <>
inline char CInputStream::Get< char >(const TType< char >& type) {
  return ReadInt8();
}

template <>
inline unsigned char CInputStream::Get< unsigned char >(const TType< unsigned char >& type) {
  return ReadUint8();
}

template <>
inline signed char CInputStream::Get< signed char >(const TType< signed char >& type) {
  return ReadInt8();
}

template <>
inline int CInputStream::Get< int >(const TType< int >& type) {
  return ReadInt32();
}

template <>
inline uint CInputStream::Get< uint >(const TType< uint >& type) {
  return ReadInt32();
}

template <>
inline unsigned long CInputStream::Get< unsigned long >(const TType< unsigned long >& type) {
  return ReadInt32();
}

template <>
inline u64 CInputStream::Get< u64 >(const TType< u64 >& type) {
  const uint high = ReadInt32();
  const uint low = ReadInt32();
  return (static_cast< u64 >(high) << 32) | low;
}

template <>
inline float CInputStream::Get< float >(const TType< float >& type) {
  return ReadFloat();
}

template <>
inline short CInputStream::Get< short >(const TType< short >& type) {
  return ReadInt16();
}

template <>
inline ushort CInputStream::Get< ushort >(const TType< ushort >& type) {
  return ReadUint16();
}

// rstl
template < typename L, typename R >
inline rstl::pair< L, R >::pair(CInputStream& in)
: first(in.Get(TGetType(first))), second(in.Get(TGetType(second))) {}

#include "rstl/vector.hpp"
template < typename T, typename Alloc >
rstl::vector< T, Alloc >::vector(CInputStream& in, const Alloc& allocator)
: mCount(0), mCapacity(0), mItems(nullptr) {
  int count = in.ReadInt32();
  reserve(count);
  for (int i = 0; i < count; i++) {
    push_back_unsafe(in.Get< T >());
  }
}

#include "rstl/reserved_vector.hpp"
template < typename T, int N >
inline rstl::reserved_vector< T, N >::reserved_vector(CInputStream& in) : mCount(in.ReadInt32()) {
  for (int i = 0; i < mCount; i++) {
    construct(&data()[i], in.Get(TType< T >()));
  }
}

#include "rstl/red_black_tree.hpp"
template < typename T, typename P, int U, typename S, typename Cmp, typename Alloc >
inline rstl::red_black_tree< T, P, U, S, Cmp, Alloc >::red_black_tree(
    CInputStream& in, const S& selector, const Cmp& cmp, const Alloc& alloc)
: mSelector(selector), mCmp(cmp), mAllocator(alloc), mCount(0) {
  const int count = in.Get< int >();
  for (int i = 0; i < count; ++i) {
    insert(in.Get< P >());
  }
}

#endif // _CINPUTSTREAM
