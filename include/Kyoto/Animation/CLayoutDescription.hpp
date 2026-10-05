#ifndef _CLAYOUTDESCRIPTION
#define _CLAYOUTDESCRIPTION

#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"

class CCharLayoutInfo;
class CLayoutScaleInfo;

class CLayoutDescription {
public:
  explicit CLayoutDescription(const TLockedToken< CCharLayoutInfo >& layout)
  : mLayoutToken(layout) {}

  class CScaledLayoutDescription {
  public:
    typedef rstl::pair< rstl::pair< const CLayoutScaleInfo*, const CLayoutScaleInfo* >, float >
        ScaleInfo;

    const TLockedToken< CCharLayoutInfo >& ScaledLayout() const { return mLayoutToken; }

  private:
    TLockedToken< CCharLayoutInfo > mLayoutToken;
    float mScale;
    rstl::optional_object< ScaleInfo > mScaleInfo;
  };

  uchar GetNumSegments() const;

  bool UsesScale() const { return mScaled.valid(); }

  const TLockedToken< CCharLayoutInfo >& ScaledLayout() const {
    return UsesScale() ? mScaled->ScaledLayout() : mLayoutToken;
  }

private:
  TLockedToken< CCharLayoutInfo > mLayoutToken;
  rstl::optional_object< CScaledLayoutDescription > mScaled;
};

CHECK_SIZEOF(CLayoutDescription, 0x30)
NESTED_CHECK_SIZEOF(CLayoutDescription, CScaledLayoutDescription, 0x20)

#endif // _CLAYOUTDESCRIPTION
