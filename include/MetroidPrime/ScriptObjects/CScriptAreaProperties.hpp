#ifndef _CSCRIPTAREAPROPERTIES
#define _CSCRIPTAREAPROPERTIES

#include "MetroidPrime/CEntity.hpp"

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"

#include "rstl/optional_object.hpp"


class CModel;
class CScriptAreaProperties : public CEntity {
private:
  bool m_hasSkybox : 1;
  bool m_isDarkWorld : 1;
  int m_environmentEffects;
  float m_density;
  float m_normalLightning;
  CAssetId m_skyBoxAssetId;
  int m_phazonDamage;
  rstl::optional_object< TLockedToken<CModel> > skyBoxModel;
  int x4c;
  float x50;
  float x54;
  CColor m_color;

public:
  CScriptAreaProperties(TUniqueId, const CEntityInfo&, float, float,
                        uint hasSkyBox, bool isDarkWorld, uint, CAssetId skyBoxAssetId, int, int, float, float, const CColor&);
  ~CScriptAreaProperties() override;

  bool GetNeedsSky() const { return m_hasSkybox; }
  int GetEnvFxType() const { return m_environmentEffects; }
  CAssetId GetSkyModel() const { return m_skyBoxAssetId; }
  // Guessed names
  ERglFogMode GetSkyFogMode() const { return static_cast< ERglFogMode >(x4c); }
  float GetSkyFogStart() const { return x50; }
  float GetSkyFogEnd() const { return x54; }
  const CColor& GetSkyFogColor() const { return m_color; }

  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg&) override;
};
CHECK_SIZEOF(CScriptAreaProperties, 0x5c)

#endif // _CSCRIPTAREAPROPERTIES
