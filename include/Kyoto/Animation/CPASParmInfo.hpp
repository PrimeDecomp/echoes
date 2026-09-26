#ifndef _CPASPARMINFO
#define _CPASPARMINFO

#include "Kyoto/Animation/CPASAnimParm.hpp"
#include "rstl/construct.hpp"

class CInputStream;
class CPASParmInfo {
public:
  enum EWeightFunction {
    kWF_Invalid = -1,
    kWF_ExactMatch,
    kWF_PercentError,
    kWF_AngularPercent,
    kWF_NoWeight,
  };

  CPASParmInfo(CInputStream& in);

  CPASAnimParm::EParmType GetParameterType() const { return mType; }
  EWeightFunction GetWeightFunction() const { return mWeightFunction; }
  float GetParameterWeight() const { return mWeight; }
  CPASAnimParm::UParmValue GetParameterMinValue() const { return mMin; }
  CPASAnimParm::UParmValue GetParameterMaxValue() const { return mMax; }

private:
  CPASAnimParm::EParmType mType;
  EWeightFunction mWeightFunction;
  float mWeight;
  CPASAnimParm::UParmValue mMin;
  CPASAnimParm::UParmValue mMax;
};

namespace rstl {
template <>
struct is_trivially_destructible< CPASParmInfo > {
  enum { value = true };
};

template <>
inline void construct< CPASParmInfo >(void* dest, const CPASParmInfo& src) {
  *static_cast< CPASParmInfo* >(dest) = src;
}
} // namespace rstl

CHECK_SIZEOF(CPASParmInfo, 0x14)

#endif // _CPASPARMINFO
