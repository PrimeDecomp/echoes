#ifndef _CFILEPRELOAD
#define _CFILEPRELOAD

#include "types.h"
#include "rstl/string.hpp"

class CFilePreloadData;

// The original name of this shared, reference-counted file handle is unknown.
class CFilePreload {
public:
  CFilePreload(const rstl::string& path);
  CFilePreload(const CFilePreload& other);
  ~CFilePreload();

  void operator=(const CFilePreload& other);
  bool IsReady() const;
  void Read(void* dest, int offset, int length) const;

private:
  CFilePreloadData* x0_data;
};
CHECK_SIZEOF(CFilePreload, 0x4)

#endif // _CFILEPRELOAD
