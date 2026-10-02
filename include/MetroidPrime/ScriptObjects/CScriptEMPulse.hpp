#ifndef _CSCRIPTEMPULSE
#define _CSCRIPTEMPULSE

#include "MetroidPrime/CActor.hpp"

class CElementGen;
class CGenDescription;

class CScriptEMPulse : public CActor {
public:
  CScriptEMPulse(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                 const CTransform4f& xf, float initialRadius, float finalRadius, float duration,
                 float minHudDisableTime, float maxHudDisableTime, float minHudDisableAmount,
                 float maxHudDisableAmount, CAssetId particleId);

  // CEntity
  ~CScriptEMPulse() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

private:
  CAABox CalculateBoundingBox() const;

  float mDuration;
  float mFinalRadius;
  float mCurrentRadius;
  float mInitialRadius;
  float mMinHudDisableTime;
  float mMaxHudDisableTime;
  float mMinHudDisableAmount;
  float mMaxHudDisableAmount;
  TLockedToken< CGenDescription > mParticleDesc;
  rstl::single_ptr< CElementGen > mParticleGen;
};
CHECK_SIZEOF(CScriptEMPulse, 0x188)

#endif // _CSCRIPTEMPULSE
