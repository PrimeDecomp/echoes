#ifndef _RSTL_OPTIONAL_STORAGE
#define _RSTL_OPTIONAL_STORAGE

#include "types.h"

#include "rstl/construct.hpp"

namespace rstl {
// Guessed name. Inline storage that a caller placement-constructs on demand.
// Unlike optional_object, native code reads and writes the validity flag as a
// one-bit field, and destruction clears it.
template < typename T >
class optional_storage {
public:
  optional_storage() : m_valid(false) {}
  ~optional_storage() {
    if (m_valid) {
      rstl::destroy(get_ptr());
    }
    m_valid = false;
  }

  T* prepare_emplace() {
    if (m_valid) {
      rstl::destroy(get_ptr());
    }
    m_valid = true;
    return get_ptr();
  }

  T* get_ptr() { return reinterpret_cast< T* >(m_data); }
  const T* get_ptr() const { return reinterpret_cast< const T* >(m_data); }
  bool valid() const { return m_valid; }
  operator bool() const { return m_valid; }

  T& operator*() { return *get_ptr(); }
  const T& operator*() const { return *get_ptr(); }

private:
  ALIGNAS(T) uchar m_data[sizeof(T)];
  bool m_valid : 1;
};
} // namespace rstl

#endif // _RSTL_OPTIONAL_STORAGE
