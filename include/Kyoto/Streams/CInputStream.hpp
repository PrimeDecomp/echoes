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
T cinput_stream_helper(const TType< T >& type, CInputStream& in);

template < typename T >
inline TType< T > TGetType(const T&) {
  return TType< T >();
}

class CInputStream {
public:
  struct SBufferAndSize {
    const void* x0_buffer;
    unsigned long x4_size;

    SBufferAndSize(const void* buffer, unsigned long size) : x0_buffer(buffer), x4_size(size) {}
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
  T Get(const TType< T >& type = TType< T >()) {
    return cinput_stream_helper(TType< T >(), *this);
  }

  int ReadInt32() {
    int* result = reinterpret_cast< int* >(x8_ptr);
    x8_ptr = reinterpret_cast< uchar* >(result + 1);
    return *result;
  }
  u16 ReadUint16() {
    u16* result = reinterpret_cast< u16* >(x8_ptr);
    x8_ptr = reinterpret_cast< uchar* >(result + 1);
    return *result;
  }
  short ReadInt16() {
    short* result = reinterpret_cast< short* >(x8_ptr);
    x8_ptr = reinterpret_cast< uchar* >(result + 1);
    return *result;
  }
  u8 ReadUint8() {
    u8* result = x8_ptr;
    x8_ptr = result + 1;
    return *result;
  }
  char ReadInt8() {
    char* result = reinterpret_cast< char* >(x8_ptr);
    x8_ptr = reinterpret_cast< uchar* >(result + 1);
    return *result;
  }
  bool ReadBool() { return ReadUint8() != 0; }
  uint GetReadPosition() const { return x8_ptr - x4_buffer; }

private:
  uchar* x4_buffer;
  uchar* x8_ptr;
  unsigned long xc_length;
  bool x10_owned;
};

CHECK_SIZEOF(CInputStream, 0x14)

template < typename T >
inline T cinput_stream_helper(const TType< T >& type, CInputStream& in) {
  return T(in);
}
template <>
inline bool cinput_stream_helper(const TType< bool >& type, CInputStream& in) {
  return in.ReadBool();
}
template <>
inline char cinput_stream_helper(const TType< char >& type, CInputStream& in) {
  return in.ReadInt8();
}

template <>
inline unsigned char cinput_stream_helper(const TType< unsigned char >& type, CInputStream& in) {
  return in.ReadUint8();
}

template <>
inline signed char cinput_stream_helper(const TType< signed char >& type, CInputStream& in) {
  return in.ReadInt8();
}

template <>
inline int cinput_stream_helper(const TType< int >& type, CInputStream& in) {
  return in.ReadInt32();
}
template <>
inline uint cinput_stream_helper(const TType< uint >& type, CInputStream& in) {
  return in.ReadInt32();
}
template <>
inline unsigned long cinput_stream_helper(const TType< unsigned long >& type, CInputStream& in) {
  return in.ReadInt32();
}
template <>
inline float cinput_stream_helper(const TType< float >& type, CInputStream& in) {
  return in.ReadFloat();
}
template <>
inline short cinput_stream_helper(const TType< short >& type, CInputStream& in) {
  return in.ReadInt16();
}
template <>
inline ushort cinput_stream_helper(const TType< ushort >& type, CInputStream& in) {
  return in.ReadUint16();
}

// rstl
template < typename L, typename R >
inline rstl::pair< L, R >::pair(CInputStream& in)
: first(in.Get(TGetType(first))), second(in.Get(TGetType(second))) {}

#include "rstl/vector.hpp"
template < typename T, typename Alloc >
rstl::vector< T, Alloc >::vector(CInputStream& in, const Alloc& allocator)
: x4_count(0), x8_capacity(0), xc_items(nullptr) {
  int count = in.ReadInt32();
  reserve(count);
  for (int i = 0; i < count; i++) {
    push_back_unsafe(in.Get< T >());
  }
}

#include "rstl/reserved_vector.hpp"
template < typename T, int N >
inline rstl::reserved_vector< T, N >::reserved_vector(CInputStream& in) : x0_count(in.ReadInt32()) {
  for (int i = 0; i < x0_count; i++) {
    construct(&data()[i], in.Get(TType< T >()));
  }
}

#endif // _CINPUTSTREAM
