#ifndef _CLZOINPUTSTREAM
#define _CLZOINPUTSTREAM

#include "types.h"

#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/auto_ptr.hpp"

class CLZOInputStream : public CInputStream {
public:
  CLZOInputStream(const rstl::auto_ptr< CInputStream >& in, unsigned long compressedLen,
                  unsigned long decompressedLen);
  ~CLZOInputStream() override;
};

CHECK_SIZEOF(CLZOInputStream, 0x14)

#endif // _CLZOINPUTSTREAM
