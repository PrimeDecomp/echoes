#include "Kyoto/Streams/CBitStreamReader.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/StreamSupport.hpp"

CBitStreamReader::CBitStreamReader(CInputStream& stream)
: x0_stream(stream), x4_bitWord(0), x8_bitOffset(0) {}

CBitStreamReader::~CBitStreamReader() {}

uint CBitStreamReader::ReadBits(uint bitCount) {
  if (x8_bitOffset >= bitCount) {
    uint mask = 0xffffffff;
    uint bwShift = 32 - bitCount;
    if (bitCount != 0x20) {
      mask = (1 << bitCount) - 1;
    }
    uint ret = mask & (x4_bitWord >> bwShift);

    x8_bitOffset -= bitCount;
    x4_bitWord <<= bitCount;
    return ret;
  }

  uint shiftAmt = bitCount - x8_bitOffset;

  uint ret = 0;
  {
    uint mask = 0xffffffff;
    uint bwShift = 32 - x8_bitOffset;
    if (x8_bitOffset != 0x20) {
      mask = (1 << x8_bitOffset) - 1;
    }
    ret = (mask & (x4_bitWord >> bwShift)) << shiftAmt;
  }

  uint len = min_containing_bytes(shiftAmt);
  x8_bitOffset = 0;
  x0_stream.Get(&x4_bitWord, len);

  {
    uint mask = 0xffffffff;
    uint bwShift = 32 - shiftAmt;
    if (shiftAmt != 0x20) {
      mask = (1 << shiftAmt) - 1;
    }
    ret |= ((mask & (x4_bitWord >> bwShift)) << x8_bitOffset);
  }

  x8_bitOffset = len * 8;
  x8_bitOffset -= shiftAmt;
  x4_bitWord <<= shiftAmt;
  return ret;
}

void CBitStreamReader::Flush() { x8_bitOffset = 0; }

CInputStream& CBitStreamReader::GetInputStream() {
  Flush();
  return x0_stream;
}
