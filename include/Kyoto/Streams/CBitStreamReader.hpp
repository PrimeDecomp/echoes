#ifndef _CBITSTREAMREADER
#define _CBITSTREAMREADER

#include "types.h"

class CInputStream;

class CBitStreamReader {
  CInputStream& x0_stream;
  uint x4_bitWord;
  uint x8_bitOffset;

public:
  explicit CBitStreamReader(CInputStream& stream);
  ~CBitStreamReader();

  uint ReadBits(uint bitCount);
  void Flush();
  CInputStream& GetInputStream();
  bool ReadPackedBool() { return ReadBits(1) != 0; }
};

CHECK_SIZEOF(CBitStreamReader, 0xc)

#endif // _CBITSTREAMREADER
