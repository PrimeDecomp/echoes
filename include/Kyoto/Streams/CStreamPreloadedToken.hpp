#ifndef _CSTREAMPRELOADEDTOKEN
#define _CSTREAMPRELOADEDTOKEN

#include "rstl/string.hpp"
#include "types.h"

class CStreamPreloadedData;

// The original name of this shared, reference-counted file handle is unknown.
class CStreamPreloadedToken {
public:
  CStreamPreloadedToken(const rstl::string& path);
  CStreamPreloadedToken(const CStreamPreloadedToken& other);
  ~CStreamPreloadedToken();

  void operator=(const CStreamPreloadedToken& other);
  bool IsReady() const;
  void Read(void* dest, int offset, int length) const;

private:
  CStreamPreloadedData* mData;
};
CHECK_SIZEOF(CStreamPreloadedToken, 0x4)

#endif // _CSTREAMPRELOADEDTOKEN
