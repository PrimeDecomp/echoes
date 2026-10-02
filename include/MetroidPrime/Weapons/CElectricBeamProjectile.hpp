#ifndef _CELECTRICBEAMPROJECTILE
#define _CELECTRICBEAMPROJECTILE

#include "MetroidPrime/Weapons/CBeamProjectile.hpp"

#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class CElectricDescription;
class CElementGen;
class CGenDescription;
class CParticleElectric;

class CElectricBeamInfo {
public:
  const TToken< CElectricDescription >& GetElectricDescription() const {
    return mElectricDescription;
  }
  float GetLength() const { return mLength; }
  float GetRadius() const { return mRadius; }
  float GetTravelSpeed() const { return mTravelSpeed; }
  CAssetId GetParticleId() const { return mParticleId; }
  float GetFadeSpeed() const { return mFadeSpeed; }
  float GetDamageInterval() const { return mDamageInterval; }

private:
  TToken< CElectricDescription > mElectricDescription;
  float mLength;
  float mRadius;
  float mTravelSpeed;
  CAssetId mParticleId;
  float mFadeSpeed;
  float mDamageInterval;
};
CHECK_SIZEOF(CElectricBeamInfo, 0x20)

class CElectricBeamProjectile : public CBeamProjectile {
public:
  CElectricBeamProjectile(const TToken< CWeaponDescription >& description, EWeaponType type,
                          const CElectricBeamInfo& beamInfo, const CTransform4f& xf,
                          EMaterialTypes material, const CDamageInfo& damage, TUniqueId uid,
                          TAreaId areaId, TUniqueId owner, uint attribs);

  // CEntity
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;

  // CBeamProjectile
  void UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr) override;
  void ResetBeam(CStateManager& mgr, bool fullReset) override;
  void Fire(const CTransform4f& xf, CStateManager& mgr, bool flag) override;

private:
  rstl::single_ptr< CParticleElectric > mElectric;
  rstl::optional_object< TLockedToken< CGenDescription > > mGenDescription;
  rstl::single_ptr< CElementGen > mElementGen;
  float mFadeSpeed;
  float mIntensity;
  float mDamageTimer;
  float mDamageInterval;
  bool mFiring;
};
CHECK_SIZEOF(CElectricBeamProjectile, 0x5c8)

#endif // _CELECTRICBEAMPROJECTILE
