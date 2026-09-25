#ifndef _CTIMEREMAINDERANDFRACTION
#define _CTIMEREMAINDERANDFRACTION

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "rstl/math.hpp"

class CIntegerTimeAndRemainder {
public:
  CIntegerTimeAndRemainder(const CCharAnimTime& time, const CCharAnimTime& interval);

  const float& RealTime() const { return mRealTime; }

  const uint& IntegerTime() const { return mIntegerTime; }

  const float& Remainder() const { return mRemainder; }

private:
  float mRealTime;
  uint mIntegerTime;
  float mRemainder;
};
CHECK_SIZEOF(CIntegerTimeAndRemainder, 0xc)

class CTimeRemainderAndFraction : public CIntegerTimeAndRemainder {
public:
  CTimeRemainderAndFraction(const CCharAnimTime& time, const CCharAnimTime& interval);

  const float& FinestSample() const { return mFinestSample; }

private:
  float mFraction;
  float mFinestSample;
};
CHECK_SIZEOF(CTimeRemainderAndFraction, 0x14)

#endif
