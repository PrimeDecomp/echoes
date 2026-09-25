#include "Kyoto/Streams/CBitStreamReader.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/StreamSupport.hpp"

CBitStreamReader::CBitStreamReader(CInputStream& stream)
: mStream(stream), mBitWord(0), mBitOffset(0) {}

CBitStreamReader::~CBitStreamReader() {}

uint CBitStreamReader::ReadBits(uint bitCount) {
  if (mBitOffset >= bitCount) {
    uint mask = 0xffffffff;
    uint bwShift = 32 - bitCount;
    if (bitCount != 0x20) {
      mask = (1 << bitCount) - 1;
    }
    uint ret = mask & (mBitWord >> bwShift);

    mBitOffset -= bitCount;
    mBitWord <<= bitCount;
    return ret;
  }

  uint shiftAmt = bitCount - mBitOffset;

  uint ret = 0;
  {
    uint mask = 0xffffffff;
    uint bwShift = 32 - mBitOffset;
    if (mBitOffset != 0x20) {
      mask = (1 << mBitOffset) - 1;
    }
    ret = (mask & (mBitWord >> bwShift)) << shiftAmt;
  }

  uint len = min_containing_bytes(shiftAmt);
  mBitOffset = 0;
  mStream.Get(&mBitWord, len);

  {
    uint mask = 0xffffffff;
    uint bwShift = 32 - shiftAmt;
    if (shiftAmt != 0x20) {
      mask = (1 << shiftAmt) - 1;
    }
    ret |= ((mask & (mBitWord >> bwShift)) << mBitOffset);
  }

  mBitOffset = len * 8;
  mBitOffset -= shiftAmt;
  mBitWord <<= shiftAmt;
  return ret;
}

void CBitStreamReader::Flush() { mBitOffset = 0; }

CInputStream& CBitStreamReader::GetInputStream() {
  Flush();
  return mStream;
}
