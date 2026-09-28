#ifndef _CTWEAKTARGETING
#define _CTWEAKTARGETING

#include "rstl/single_ptr.hpp"

struct SLdrTweakTargeting;

class CTweakTargeting {
public:
  explicit CTweakTargeting(const SLdrTweakTargeting& data) : mData(&data) {}

private:
  const SLdrTweakTargeting* mData;
};
CHECK_SIZEOF(CTweakTargeting, 0x4)

extern rstl::single_ptr< CTweakTargeting > gpTweakTargeting;

#endif // _CTWEAKTARGETING
