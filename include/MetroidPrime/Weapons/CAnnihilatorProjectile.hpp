#ifndef _CANNIHILATORPROJECTILE
#define _CANNIHILATORPROJECTILE

#include "Collision/CMaterialFilter.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

class CGameCamera;

// Guessed class name
class CAnnihilatorProjectile : public CEnergyProjectile {
public:
  CAnnihilatorProjectile(const TToken< CWeaponDescription >& description, EWeaponType type,
                         const CTransform4f& xf, EMaterialTypes excludeMaterial,
                         const CDamageInfo& damage, TUniqueId uid, TAreaId areaId, TUniqueId owner,
                         TUniqueId homingTarget, uint attributes, bool underwater,
                         const CVector3f& scale, float projectileSpeed, float projectileTurnRate);

  // CEntity
  ~CAnnihilatorProjectile() override;
  void Think(float dt, CStateManager& mgr) override;

private:
  // Guessed name
  struct STargetCandidate {
    CActor* mActor;                  // Guessed name
    float mDistance;                 // Guessed name
    CVector3f mProjectedAimPosition; // Guessed name
    float x14_;
  };

  float mTargetSeekTimer;    // Guessed name
  float mProjectileSpeed;    // Guessed name
  float mProjectileTurnRate; // Guessed name

  static const CMaterialFilter kTargetFilter; // Guessed name
  static const CMaterialFilter kRayFilter;    // Guessed name
  static float sNextTargetSeekOffset;         // Guessed name

  // Guessed name
  static void GatherTargetCandidates(CStateManager& mgr,
                                     rstl::vector< STargetCandidate >& candidates,
                                     const CActor& source, const CTransform4f& xf, TUniqueId owner,
                                     const CGameCamera* projection, float projectileSpeed,
                                     float projectileTurnRate, float radius, float height,
                                     float turnTestDistance);
  static bool CanTargetActor(CStateManager& mgr, TUniqueId owner,
                             const CActor& target); // Guessed name
};
CHECK_SIZEOF(CAnnihilatorProjectile, 0x578)

#endif // _CANNIHILATORPROJECTILE
