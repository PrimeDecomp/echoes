#ifndef _CPOWERBOMB
#define _CPOWERBOMB

#include "MetroidPrime/Weapons/CWeapon.hpp"

class CElementGen;
class CGenDescription;

class CPowerBomb : public CWeapon {
public:
  // Guessed names. The native callable's 16-byte payload representation is unresolved.
  struct SCallback {
    typedef void (*FInvoke)(void*, const void*, CStateManager&, CPowerBomb&);

    void operator()(CStateManager& mgr, CPowerBomb& bomb) const {
      mInvoke(mContext, mPayload, mgr, bomb);
    }

    FInvoke mInvoke;
    void* mContext;
    uchar mPayload[16];
  };

  // Guessed names, based on the two observed damage paths.
  enum EFlags {
    kF_NoDamageDelay = 1 << 0,
    kF_InstantDamage = 1 << 1,
  };

  CPowerBomb(TToken< CGenDescription > particle, TUniqueId uid, TAreaId areaId, TUniqueId ownerId,
             EWeaponType type, uint flags, const CTransform4f& xf, const CDamageInfo& damageInfo);

  // CEntity
  ~CPowerBomb() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  float GetCurTime() const { return mBombTime; }
  bool IsEnding() const { return mBombTime > kEndingTime; }
  void ApplyDynamicDamage(const CVector3f& position, CStateManager& mgr);

  static const CColor& FadeColor() { return kFadeColor; }
  static float EndingTime() { return kEndingTime; }

private:
  static CColor kFadeColor;
  static const float kEndingTime;

  bool mCanStartFilter : 1;
  bool mFilterEnabled : 1;
  float mBombTime;
  float mCurRadius;
  float mRadiusIncrement;
  float mDamageStartTime;
  rstl::single_ptr< CElementGen > mParticle;
  TUniqueId mLightId;
  CAssetId mParticleId;
  uint mFlags;
  float mCallbackDelay;
  CSfxHandle mExplosionSound;
  rstl::optional_object< SCallback > mCallback;
};
CHECK_SIZEOF(CPowerBomb, 0x210)
NESTED_CHECK_SIZEOF(CPowerBomb, SCallback, 0x18)

#endif // _CPOWERBOMB
