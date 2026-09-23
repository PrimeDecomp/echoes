#include "Kyoto/Streams/CBitStreamWriter.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Streams/COutputStream.hpp"
#include "Kyoto/Streams/StreamSupport.hpp"

CBitStreamWriter::CBitStreamWriter(COutputStream& stream)
: x0_stream(stream), x4_shiftRegister(0), x8_shiftRegisterOffset(32) {}

CBitStreamWriter::~CBitStreamWriter() { Flush(); }

uint CBitStreamWriter::GetWrittenBits() const {
  return x0_stream.GetWrittenBytes() * 8 + 32 - x8_shiftRegisterOffset;
}

void CBitStreamWriter::Flush() {
  if (x8_shiftRegisterOffset < 32) {
    x0_stream.Put(&x4_shiftRegister, min_containing_bytes(32 - x8_shiftRegisterOffset));
    x4_shiftRegister = 0;
    x8_shiftRegisterOffset = 32;
  }
}

void CBitStreamWriter::WriteBits(const uint val, const uint bitCount) {

  const uint registerOffset = x8_shiftRegisterOffset;
  if (registerOffset >= bitCount) {
    const uint off = registerOffset - bitCount;
    x4_shiftRegister |= (val & (bitCount != 32 ? (1 << bitCount) - 1 : 0xffffffff)) << off;
    x8_shiftRegisterOffset -= bitCount;
  } else {
    const uint shiftAmt = bitCount - registerOffset;
    const uint shiftA = val >> shiftAmt;

    x4_shiftRegister |= shiftA & (registerOffset != 0x20 ? (1 << registerOffset) - 1 : 0xffffffff);
    x8_shiftRegisterOffset = 0;
    Flush();
    const uint shift = 32 - shiftAmt;
    x4_shiftRegister = (val & (shiftAmt != 32 ? (1 << shiftAmt) - 1 : 0xffffffff)) << shift;
    x8_shiftRegisterOffset -= shiftAmt;
  }
}

void CBitStreamWriter::FlushAll() {
  Flush();
  x0_stream.Flush();
}

COutputStream& CBitStreamWriter::GetOutputStream() {
  Flush();
  return x0_stream;
}
