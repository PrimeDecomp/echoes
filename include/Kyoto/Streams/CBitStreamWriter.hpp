#ifndef _CBITSTREAMWRITER
#define _CBITSTREAMWRITER

#include "types.h"

class COutputStream;

class CBitStreamWriter {
  COutputStream& mStream;
  uint mShiftRegister;
  volatile uint mShiftRegisterOffset;

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
