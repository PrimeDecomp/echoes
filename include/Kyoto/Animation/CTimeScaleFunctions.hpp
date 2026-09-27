#ifndef _CTIMESCALEFUNCTIONS
#define _CTIMESCALEFUNCTIONS

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "rstl/ownership_transfer.hpp"

enum EVaryingAnimationTimeScaleType { kVATST_Constant, kVATST_Linear };

class IVaryingAnimationTimeScale {
public:
  virtual EVaryingAnimationTimeScaleType GetType() const = 0;
  virtual float VTimeScaleIntegral(const float& lowerLimit, const float& upperLimit) const = 0;
  virtual float VFindUpperLimit(const float& lowerLimit, const float& root) const = 0;
  virtual rstl::ownership_transfer< IVaryingAnimationTimeScale > VClone() const = 0;
  virtual rstl::ownership_transfer< IVaryingAnimationTimeScale >
  VGetFunctionMirrored(const float& value) const = 0;

  rstl::ownership_transfer< IVaryingAnimationTimeScale > Clone() const { return VClone(); }
};
CHECK_SIZEOF(IVaryingAnimationTimeScale, 0x4)

class CConstantAnimationTimeScale : public IVaryingAnimationTimeScale {
public:
  explicit CConstantAnimationTimeScale(float scale) : mScale(scale) {}

  // IVaryingAnimationTimeScale
  EVaryingAnimationTimeScaleType GetType() const override { return kVATST_Constant; }
  float VTimeScaleIntegral(const float& lowerLimit, const float& upperLimit) const override;
  float VFindUpperLimit(const float& lowerLimit, const float& root) const override;
  rstl::ownership_transfer< IVaryingAnimationTimeScale > VClone() const override;
  rstl::ownership_transfer< IVaryingAnimationTimeScale >
  VGetFunctionMirrored(const float& value) const override;

private:
  float mScale;
};
CHECK_SIZEOF(CConstantAnimationTimeScale, 0x8)

#endif // _CTIMESCALEFUNCTIONS
