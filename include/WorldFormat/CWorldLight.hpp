#ifndef _CWORLDLIGHT
#define _CWORLDLIGHT

#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/construct.hpp"

class CInputStream;
class CWorldLight {
  static const CVector3f kDefaultPosition;
  static const CVector3f kDefaultDirection;

public:
  enum EWorldLightType {
    kWLT_LocalAmbient,
    kWLT_Directional,
    kWLT_Custom,
    kWLT_Spot,
    kWLT_Spot2,
    kWLT_LocalAmbient2
  };

  explicit CWorldLight(CInputStream& in);
  CLight GetAsCGraphicsLight() const;
  const CVector3f& GetPosition() const { return mPosition; }
  bool DoesCastShadows() const { return mCastShadows; }

private:
  EWorldLightType mType;
  CVector3f mColor;
  CVector3f mPosition;
  CVector3f mDirection;
  float mQ;
  float mCutoffAngle;
  float x30_;
  bool mCastShadows;
  float x38_;
  EFalloffType mFalloff;
  float x40_;
  uint x44_;
};
CHECK_SIZEOF(CWorldLight, 0x48)

namespace rstl {
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(CWorldLight)
RSTL_DECLARE_BITWISE_CONSTRUCTION(CWorldLight)
}

#endif // _CWORLDLIGHT
