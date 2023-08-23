#ifndef _CSCRIPTAREAPROPERTIES
#define _CSCRIPTAREAPROPERTIES

#include "MetroidPrime/CEntity.hpp"

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/Graphics/CColor.hpp"

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
  CEntity* TypesMatch(EEntityType) override;
  ~CScriptAreaProperties() override;

  void AcceptScriptMsg(CStateManager& mgr, CScriptMsg&) override;
};
CHECK_SIZEOF(CScriptAreaProperties, 0x5c)

#endif // _CSCRIPTAREAPROPERTIES
