#ifndef _CBITSTREAMREADER
#define _CBITSTREAMREADER

#include "types.h"

class CInputStream;

class CBitStreamReader {
  CInputStream& mStream;
  uint mBitWord;
  uint mBitOffset;

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
