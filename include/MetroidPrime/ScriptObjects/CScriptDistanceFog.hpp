#ifndef _CSCRIPTDISTANCEFOG
#define _CSCRIPTDISTANCEFOG

#include "MetroidPrime/CEntity.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CVector2f.hpp"

// Guessed name: Prime's CScriptDistanceFog. In Echoes the thermal and X-ray fades are replaced by a
// single light target and speed, and the WorldLightFader loader builds the same class.
class CScriptDistanceFog : public CEntity {
public:
  CScriptDistanceFog(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     ERglFogMode mode, const CColor& color, const CVector2f& range,
                     float colorDelta, CVector2f rangeDelta, float lightTarget, float lightSpeed,
                     bool explicitFog);

  // CEntity
  ~CScriptDistanceFog() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

private:
  ERglFogMode mMode;
  CColor mColor;
  CVector2f mRange;
  float mColorDelta;
  CVector2f mRangeDelta;
  float mLightTarget;
  float mLightSpeed;
  bool mExplicit;
  bool mNonZero;
};
CHECK_SIZEOF(CScriptDistanceFog, 0x4c)

#endif // _CSCRIPTDISTANCEFOG
