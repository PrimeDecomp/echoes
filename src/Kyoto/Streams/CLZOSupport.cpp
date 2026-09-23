#include "Kyoto/Streams/CLZOSupport.hpp"

#include "LZO/lzo1x.h"

extern "C" int __lzo_init_done;

void CLZOSupport::Inflate(const uchar* source, uint sourceLen, uchar* dest, uint& destLen) {
  if (!__lzo_init_done) {
    lzo_init();
  }
  lzo1x_decompress(source, sourceLen, dest, &destLen, nullptr);
}
