#ifndef _CATOMICBETA
#define _CATOMICBETA

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"

class CElectricDescription;
class CWeaponDescription;

// Prime 1 has the same class. It hovers along a restricted flight path, fires three electric
// beams from its bomb locators while the player is in range and speeds up while any player is
// charging a beam.
class CAtomicBeta : public CPatterned {
public:
  CAtomicBeta(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
              const CTransform4f& xf, const CModelData& modelData,
              const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
              CAssetId electricId, CAssetId weaponId, const CDamageInfo& beamDamage,
              CAssetId particleId, float beamFadeSpeed, float beamRadius, float beamDamageInterval,
              const CDamageVulnerability& frozenVulnerability, float moveSpeed, float minSpeed,
              float maxSpeed, ushort flySound, ushort chargedFlySound, ushort electricitySound,
              float speedStep);

  // CEntity
  ~CAtomicBeta() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& other, CStateManager& mgr) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& position,
                                                         const CVector3f& direction,
                                                         const CWeaponMode& mode,
                                                         int attributes) const override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;

private:
  void CreateBeams(CStateManager& mgr);
  void UpdateBeams(CStateManager& mgr, bool fire);
  void DestroyBeams(CStateManager& mgr);
  void PlayLoopedSound(CSfxHandle& handle, ushort sfxId, const CVector3f position,
                       uchar volume) const;
  void StopLoopedSound(CSfxHandle& handle) const;

  static bool IsCharging(const CStateManager& mgr);

  rstl::reserved_vector< TUniqueId, 3 > mBeamIds;
  bool mBeamFired;
  float mMinSpeed;
  float mMaxSpeed;
  float mSpeedStep;
  float mCurrentSpeed;
  CDamageVulnerability mFrozenVulnerability;
  float mMoveSpeed;
  CVector3f mDirection;
  TToken< CElectricDescription > mElectricDescription;
  TToken< CWeaponDescription > mWeaponDescription;
  CDamageInfo mBeamDamage;
  CAssetId mBeamParticle;
  float mBeamFadeSpeed;
  float mBeamRadius;
  float mBeamDamageInterval;
  float mStaticInterferenceStrength; // Guessed name
  float mStaticInterferenceRadius;   // Guessed name
  ushort mFlySound;
  ushort mChargedFlySound;
  ushort mElectricitySound;
  CSfxHandle mFlySoundHandle;
  CSfxHandle mChargedFlySoundHandle;
  CSfxHandle mElectricitySoundHandle;
  float mTouchRadius;
};
CHECK_SIZEOF(CAtomicBeta, 0x880)

#endif // _CATOMICBETA
