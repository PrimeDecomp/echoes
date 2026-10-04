#ifndef _CLIGHTCOMBOPROJECTILE
#define _CLIGHTCOMBOPROJECTILE

#include "MetroidPrime/Weapons/CBeamInfo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"

// Guessed name
class CLightComboProjectile : public CEnergyProjectile {
public:
  CLightComboProjectile(const TToken< CWeaponDescription >& description, EWeaponType type,
                        const CTransform4f& xf, EMaterialTypes material, const CDamageInfo& damage,
                        TUniqueId uid, TAreaId areaId, TUniqueId owner, TUniqueId homingTarget,
                        bool underwater, uint attribs, float radius);

  // CEntity
  ~CLightComboProjectile() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  // Guessed names
  bool CanCreateRay() const;
  void RequestRayResets(CStateManager& mgr, bool includeUntargeted, bool fullReset);
  void FreeRays(CStateManager& mgr);
  void RequestRayReset(CStateManager& mgr, TUniqueId rayId, bool fullReset);
  bool UpdateRayTarget(CStateManager& mgr, TUniqueId rayId, const CVector3f& position);
  TUniqueId CreateRay(float resetDelay, CStateManager& mgr, TUniqueId target,
                      const CVector3f& position);
  TUniqueId CreateRay(float resetDelay, CStateManager& mgr, const CVector3f& position);
  TUniqueId CreateRay(float resetDelay, CStateManager& mgr, TUniqueId target);

private:
  // Guessed name
  struct SRayInfo {
    SRayInfo(TUniqueId target, const CVector3f& position, float resetDelay)
    : mTargetId(target)
    , mTargetPosition(position)
    , mResetDelay(resetDelay)
    , mResetRequested(false)
    , mFullReset(false) {}

    TUniqueId mTargetId;
    CVector3f mTargetPosition;
    float mResetDelay;
    bool mResetRequested : 1;
    bool mFullReset : 1;
  };

  typedef rstl::pair< TUniqueId, SRayInfo > TRay;
  typedef rstl::reserved_vector< TRay, 8 > TRays;
  typedef rstl::pair< TUniqueId, TUniqueId > TTargetRay;

  float mRadius;
  float mRaySpawnTimer;
  CProjectileInfo mRayProjectile;
  CBeamInfo mRayBeamInfo;
  TRays mRays;
  rstl::reserved_vector< TTargetRay, 8 > mTargetRays;
  bool mRayListsDirty : 1;
};
CHECK_SIZEOF(CLightComboProjectile, 0x6f0)
NESTED_CHECK_SIZEOF(CLightComboProjectile, SRayInfo, 0x18)
NESTED_CHECK_SIZEOF(CLightComboProjectile, TRay, 0x1c)

#endif // _CLIGHTCOMBOPROJECTILE
