#ifndef _CDAMAGEEFFECT
#define _CDAMAGEEFFECT

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"

#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class CElementGen;
class CGenDescription;

class CDamageEffect : public CActor {
public:
  CDamageEffect(const TToken< CGenDescription >& effect, TUniqueId uid, TAreaId areaId, bool active,
                TUniqueId owner, const CTransform4f& xf, const CDamageInfo& damage,
                const CAABox& bounds, float timeScale, const CVector3f& scale, bool affectsVisor,
                float visorAlpha, CAssetId visorTexture, float visorFadeIn, float visorFadeOut,
                bool showInCombat, bool showInDark, bool showInEcho);

  // CEntity
  ~CDamageEffect() override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

private:
  rstl::single_ptr< CElementGen > mParticle;
  TUniqueId mOwner;
  CDamageInfo mDamage;
  CDamageInfo mScaledDamage;
  rstl::optional_object< CAABox > mBounds;
  float mTimeScale;
  bool mShowInCombat : 1;
  bool mShowInDark : 1;
  bool mShowInEcho : 1;
  bool mShowAlways : 1;
  bool x1b8_28_ : 1;
  bool mAffectsVisor : 1;
  float mVisorAlpha;
  CAssetId mVisorTexture;
  float mVisorFadeIn;
  float mVisorFadeOut;
  float mTimer;
};
CHECK_SIZEOF(CDamageEffect, 0x1d0)

#endif // _CDAMAGEEFFECT
