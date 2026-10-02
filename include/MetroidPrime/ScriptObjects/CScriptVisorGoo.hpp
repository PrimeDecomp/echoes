#ifndef _CSCRIPTVISORGOO
#define _CSCRIPTVISORGOO

#include "MetroidPrime/CActor.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/TToken.hpp"

class CGenDescription;
class CElectricDescription;

class CScriptVisorGoo : public CActor {
public:
  CScriptVisorGoo(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                  const CTransform4f& xf, CAssetId particle, CAssetId electric,
                  const CColor& color, float minRange, float maxRange, float chanceMinRange,
                  float chanceMaxRange, int sfx, bool noViewCheck, bool persistent,
                  bool deleteOnDeactivate);

  // CEntity
  ~CScriptVisorGoo() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& other, CStateManager& mgr) override;

  const TToken< CGenDescription >& GetParticleDesc() const { return mParticleDesc; }
  const TToken< CElectricDescription >& GetElectricDesc() const { return mElectricDesc; }

private:
  TToken< CGenDescription > mParticleDesc; // 0x158
  TToken< CElectricDescription > mElectricDesc; // 0x160
  ushort mSfx;
  CAssetId mParticleId;
  CAssetId mElectricId;
  TUniqueId mEffectId;
  float mMinRange;
  float mMaxRange;
  float mChanceMinRange;
  float mChanceMaxRange;
  CColor mColor;
  bool mViewCheck : 1;
  bool mPersistent : 1;
  bool mDeleteOnDeactivate : 1;
};
CHECK_SIZEOF(CScriptVisorGoo, 0x190)

#endif // _CSCRIPTVISORGOO
