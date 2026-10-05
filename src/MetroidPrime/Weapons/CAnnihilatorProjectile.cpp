#include "MetroidPrime/Weapons/CAnnihilatorProjectile.hpp"

const CMaterialFilter CAnnihilatorProjectile::kTargetFilter = CMaterialFilter::MakeIncludeExclude(
    CMaterialList(kMT_SeekerTarget), CMaterialList(kMT_NoPlatformCollision, kMT_Trigger));
const CMaterialFilter CAnnihilatorProjectile::kRayFilter = CMaterialFilter::MakeIncludeExclude(
    CMaterialList(kMT_Unknown59), CMaterialList(kMT_NoPlatformCollision));
float CAnnihilatorProjectile::sNextTargetSeekOffset = 0.f;

CAnnihilatorProjectile::CAnnihilatorProjectile(
    const TToken< CWeaponDescription >& description, EWeaponType type, const CTransform4f& xf,
    EMaterialTypes excludeMaterial, const CDamageInfo& damage, TUniqueId uid, TAreaId areaId,
    TUniqueId owner, TUniqueId homingTarget, uint attributes, bool underwater,
    const CVector3f& scale, float projectileSpeed, float projectileTurnRate)
: CEnergyProjectile(true, description, type, xf, excludeMaterial, damage, uid, areaId, owner,
                    homingTarget, attributes, underwater, scale, CImpactVisorEffect(), false, true,
                    false, 1.f, 4.f, 4.f) {}

CAnnihilatorProjectile::~CAnnihilatorProjectile() {}

void CAnnihilatorProjectile::Think(float dt, CStateManager& mgr) {}

void CAnnihilatorProjectile::GatherTargetCandidates(
    CStateManager& mgr, rstl::vector< STargetCandidate >& candidates, const CActor& source,
    const CTransform4f& xf, TUniqueId owner, const CTransform4f* projection, float projectileSpeed,
    float projectileTurnRate, float radius, float height, float turnTestDistance) {}

bool CAnnihilatorProjectile::CanTargetActor(CStateManager& mgr, TUniqueId owner,
                                            const CActor& target) {}
