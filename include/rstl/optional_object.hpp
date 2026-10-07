#ifndef _RSTL_OPTIONAL_OBJECT
#define _RSTL_OPTIONAL_OBJECT

#include "types.h"

#include "rstl/construct.hpp"

namespace rstl {
struct optional_object_null {};

template < typename T >
class optional_object {
public:
  optional_object() : m_valid(false) {}
  optional_object(optional_object_null) : m_valid(false) {}
  optional_object(const T& item) : m_valid(true) { rstl::construct< T >(m_data, item); }
  optional_object(const optional_object& other) : m_valid(other.valid()) {
    if (other.valid()) {
      construct< T >(m_data, other.data());
    }
  }
  ~optional_object() {
    // clear();
    // Makes ~CScriptHudMemo match
    if (m_valid) {
      rstl::destroy(get_ptr());
    }
  }

  optional_object& operator=(const optional_object& other) {
    if (this == &other) {
      return *this;
    }
    if (other.valid()) {
      assign(other.data());
    } else {
      clear();
    }
    return *this;
  }
  optional_object& operator=(const T& item) {
    assign(item);
    return *this;
  }

  T& data() { return *get_ptr(); }
  const T& data() const { return *get_ptr(); }
  template < typename A1, typename A2, typename A3, typename A4 >
  T& emplace(const A1& a1, const A2& a2, const A3& a3, const A4& a4) {
    clear();
    new (m_data) T(a1, a2, a3, a4);
    m_valid = true;
    return data();
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
  operator bool() const { return m_valid; } // replace with valid()?
  void clear() {
    if (m_valid) {
      rstl::destroy(get_ptr());
    }
    m_valid = false;
  }

  T& operator*() { return data(); }
  T* operator->() { return &data(); }

  const T& operator*() const { return data(); }
  const T* operator->() const { return &data(); }

private:
  ALIGNAS(T) uchar m_data[sizeof(T)];
  bool m_valid ATTRIBUTE_ALIGN(4);

  void assign(const T& item) {
    if (!m_valid) {
      construct< T >(m_data, item);
      m_valid = true;
      return;
    }
    *get_ptr() = item;
  }
};

} // namespace rstl

#endif // _RSTL_OPTIONAL_OBJECT
