#ifndef _CTIMEREMAINDERANDFRACTION
#define _CTIMEREMAINDERANDFRACTION

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "rstl/math.hpp"

class CIntegerTimeAndRemainder {
public:
  CIntegerTimeAndRemainder(const CCharAnimTime& time, const CCharAnimTime& interval);

  const float& RealTime() const { return x0_realTime; }

  const uint& IntegerTime() const { return x4_integerTime; }

  const float& Remainder() const { return x8_remainder; }

private:
  float x0_realTime;
  uint x4_integerTime;
  float x8_remainder;
};
CHECK_SIZEOF(CIntegerTimeAndRemainder, 0xc)

class CTimeRemainderAndFraction : public CIntegerTimeAndRemainder {
public:
  CTimeRemainderAndFraction(const CCharAnimTime& time, const CCharAnimTime& interval);

  const float& FinestSample() const { return x10_finestSample; }

private:
  float xc_fraction;
  float x10_finestSample;
};
CHECK_SIZEOF(CTimeRemainderAndFraction, 0x14)

#endif
