#ifndef _CSCRIPTVISORFLARE
#define _CSCRIPTVISORFLARE

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CVisorFlare.hpp"

class CScriptVisorFlare : public CActor {
public:
  CScriptVisorFlare(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                    const CVector3f& pos, CVisorFlare::EBlendMode blendMode, bool distanceScaled,
                    float fadeTime, float angularFalloff, float rotationScale, uint darkVisorMode,
                    uint combatVisorMode, const rstl::vector< CVisorFlare::CFlareDef >& flares,
                    bool smallOcclusionTest, bool noOcclusionTest);

  // CEntity
  ~CScriptVisorFlare() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;

private:
  mutable CVisorFlare mFlare; // 0x158
};
CHECK_SIZEOF(CScriptVisorFlare, 0x268)

#endif // _CSCRIPTVISORFLARE
