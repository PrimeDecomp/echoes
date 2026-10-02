#ifndef _CSHOCKWAVE
#define _CSHOCKWAVE

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"

class CElementGen;
class CGenDescription;
class CElectricDescription;

class CShockWaveInfo {
public:
  CAssetId GetParticleDescId() const { return mShockWaveEffect; }
  const CDamageInfo& GetDamageInfo() const { return mDamage; }
  float GetInitialRadius() const { return mRadius; }
  float GetWidthPercent() const { return mInnerRadiusRatio; }
  float GetInitialExpansionSpeed() const { return mRadialVelocity; }
  float GetSpeedIncrease() const { return mRadialVelocityAcceleration; }
  CAssetId GetWeaponDescId() const { return mVisorElectricEffect; }
  ushort GetElectrocuteSfx() const { return mVisorElectricSound; }
  float GetHeight() const { return mHeight; }

private:
  CAssetId mShockWaveEffect;
  CDamageInfo mDamage;
  float mRadius;
  float mInnerRadiusRatio;
  float mRadialVelocity;
  float mRadialVelocityAcceleration;
  CAssetId mVisorElectricEffect;
  ushort mVisorElectricSound;
  float mHeight;
};
CHECK_SIZEOF(CShockWaveInfo, 0x3c)

class CShockWave : public CActor {
public:
  CShockWave(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
             const CTransform4f& xf, TUniqueId parent, const CShockWaveInfo& data,
             float minActiveTime, float knockback);

  // CEntity
  ~CShockWave() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  bool WasAlreadyDamaged(TUniqueId uid) const;

private:
  TUniqueId mParentId;
  CDamageInfo mDamageInfo;
  TToken< CGenDescription > mElementGenDesc;
  rstl::single_ptr< CElementGen > mElementGen;
  CShockWaveInfo mShockWaveInfo;
  float mRadius;
  float mExpansionSpeed;
  float mActiveTime;
  float mMinActiveTime;
  float mKnockback;
  float mTimeSinceHitPlayerInAir;
  float mTimeSinceHitPlayer;
  bool mHitPlayerInAir;
  bool mHitPlayer;
  rstl::reserved_vector< TUniqueId, 1024 > mHitIds;
  rstl::optional_object< TToken< CElectricDescription > > mElectricDesc;
  TUniqueId mLightId;
};
CHECK_SIZEOF(CShockWave, 0x9f8)

#endif // _CSHOCKWAVE
