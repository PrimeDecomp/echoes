#ifndef _CBITSTREAMWRITER
#define _CBITSTREAMWRITER

#include "types.h"

class COutputStream;

class CBitStreamWriter {
  COutputStream& x0_stream;
  uint x4_shiftRegister;
  volatile uint x8_shiftRegisterOffset;

public:
  explicit CBitStreamWriter(COutputStream& stream);
  ~CBitStreamWriter();

  uint GetWrittenBits() const;
  void Flush();
  void FlushAll();
  void WriteBits(uint value, uint bitCount);
  COutputStream& GetOutputStream();
};

CHECK_SIZEOF(CBitStreamWriter, 0xc)

#endif // _CBITSTREAMWRITER
