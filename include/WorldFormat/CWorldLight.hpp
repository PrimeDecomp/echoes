#ifndef _CWORLDLIGHT
#define _CWORLDLIGHT

#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CInputStream;
class CWorldLight {
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

#endif // _CWORLDLIGHT
