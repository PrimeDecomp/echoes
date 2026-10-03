#include "MetroidPrime/CSurfaceAlignmentHelper.hpp"

CVector3f CSurfaceAlignmentHelper::ProjectPointToPlane(const CVector3f& point,
                                                       const CVector3f& origin,
                                                       const CVector3f& normal) const {}
bool CSurfaceAlignmentHelper::PointOnSurface(const CCollisionSurface& surface,
                                             const CVector3f& point) const {}
void CSurfaceAlignmentHelper::OrientToSurfaceNormal(CActor& actor, const CVector3f& normal,
                                                    float dt) const {}
void CSurfaceAlignmentHelper::OrientToStoredSurface(CActor& actor, float dt) const {}
void CSurfaceAlignmentHelper::AlignNearPosition(CActor& actor, CStateManager& mgr,
                                                const CVector3f& position, float dt) {}
bool CSurfaceAlignmentHelper::FindNearestSurface(CStateManager& mgr, const CVector3f& position,
                                                 float radius, CCollisionSurface& surface) {}
void CSurfaceAlignmentHelper::Update(CPhysicsActor& actor, CStateManager& mgr, float dt) {}
CSurfaceAlignmentHelper::CSurfaceAlignmentHelper(const CMaterialFilter& filter) {}
CSurfaceAlignmentHelper::CSurfaceAlignmentHelper() {}
