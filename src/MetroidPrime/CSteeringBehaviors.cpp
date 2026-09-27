#include "MetroidPrime/CSteeringBehaviors.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"

CSteeringBehaviors::CSteeringBehaviors() : x0_(M_PIF / 2.f) {}

CVector3f CSteeringBehaviors::Flee(const CPhysicsActor& actor, const CVector3f& position) const {
  const CVector3f delta = actor.GetTranslation() - position;
  if (delta.CanBeNormalized()) {
    return delta.AsNormalized();
  }
  return actor.GetTransform().GetForward();
}

CVector3f CSteeringBehaviors::Seek(const CPhysicsActor& actor,
                                   const CVector3f& destination) const {
  const CVector3f delta = destination - actor.GetTranslation();
  if (delta.CanBeNormalized()) {
    return delta.AsNormalized();
  }
  return CVector3f::Zero();
}

CVector3f CSteeringBehaviors::Arrival(const CPhysicsActor& actor, const CVector3f& destination,
                                      float dampingRadius) const {
  const CVector3f delta = destination - actor.GetTranslation();
  if (!delta.CanBeNormalized()) {
    return CVector3f::Zero();
  }

  const float distanceSquared = delta.MagSquared();
  const float radiusSquared = dampingRadius * dampingRadius;
  const float weight = distanceSquared < radiusSquared ? distanceSquared / radiusSquared : 1.f;
  return weight * delta.AsNormalized();
}

CVector3f CSteeringBehaviors::Separation(const CPhysicsActor& actor, const CVector3f& position,
                                         float separation) const {
  const CVector3f delta = actor.GetTranslation() - position;
  const float distanceSquared = delta.MagSquared();
  const float radiusSquared = separation * separation;
  if (distanceSquared < radiusSquared) {
    if (delta.CanBeNormalized()) {
      return delta.AsNormalized() * (1.f - distanceSquared / radiusSquared);
    }
    return actor.GetTransform().GetForward();
  }
  return CVector3f::Zero();
}

CVector3f CSteeringBehaviors::Alignment(const CPhysicsActor& actor,
                                        rstl::reserved_vector< TUniqueId, 1024 >& list,
                                        const CStateManager& mgr) const {
  CVector3f direction = CVector3f::Zero();
  if (!list.empty()) {
    for (int i = 0; i < list.size(); ++i) {
      if (const CActor* neighbor = static_cast< const CActor* >(mgr.GetObjectById(list[i]))) {
        direction += neighbor->GetTransform().GetForward();
      }
    }
    direction *= 1.f / list.size();
  }

  const float angle = CVector3f::GetAngleDiff(actor.GetTransform().GetForward(), direction);
  return direction * (angle / M_PIF);
}

CVector3f CSteeringBehaviors::Cohesion(const CPhysicsActor& actor,
                                       rstl::reserved_vector< TUniqueId, 1024 >& list,
                                       float dampingRadius, const CStateManager& mgr) const {
  CVector3f destination = CVector3f::Zero();
  if (list.empty()) {
    return destination;
  }

  for (int i = 0; i < list.size(); ++i) {
    if (const CActor* neighbor = static_cast< const CActor* >(mgr.GetObjectById(list[i]))) {
      destination += neighbor->GetTranslation();
    }
  }
  destination *= 1.f / list.size();
  return Arrival(actor, destination, dampingRadius);
}

CVector2f CSteeringBehaviors::Separation2D(const CPhysicsActor& actor, const CVector2f& position,
                                           float separation) const {
  const CVector2f delta = actor.GetTranslation().ToVec2f() - position;
  const float distanceSquared = delta.MagSquared();
  const float radiusSquared = separation * separation;
  if (distanceSquared < radiusSquared) {
    if (distanceSquared > FLT_EPSILON) {
      return delta.AsNormalized() * (1.f - distanceSquared / radiusSquared);
    }
    return actor.GetTransform().GetForward().ToVec2f();
  }
  return CVector2f::Zero();
}

bool CSteeringBehaviors::ProjectLinearIntersection(const CVector3f& origin, float speed,
                                                   const CVector3f& position,
                                                   const CVector3f& velocity,
                                                   CVector3f& intersection) {
  const CVector3f delta = position - origin;
  float positive, negative;
  if (CMath::SolveQuadratic(velocity.MagSquared() - speed * speed,
                            2.f * CVector3f::Dot(velocity, delta), delta.MagSquared(), positive,
                            negative) &&
      negative > 0.f) {
    intersection = position + velocity * negative;
    return true;
  }
  return false;
}

bool CSteeringBehaviors::ProjectLinearIntersection(const CVector3f& origin, float speed,
                                                   const CVector3f& position,
                                                   const CVector3f& velocity,
                                                   const CVector3f& acceleration,
                                                   CVector3f& intersection) {
  // TODO: Use the shared quartic solver and evaluate the positive interception times.
  return false;
}

bool CSteeringBehaviors::ProjectOrbitalIntersection(const CVector3f& origin, float speed,
                                                    float dt, const CVector3f& position,
                                                    const CVector3f& velocity,
                                                    const CVector3f& orbitPoint,
                                                    CVector3f& intersection) {
  // TODO: Step the radial/tangential motion and test projectile arrival time.
  return false;
}

bool CSteeringBehaviors::ProjectOrbitalIntersection(const CVector3f& origin, float speed,
                                                    float dt, const CVector3f& position,
                                                    const CVector3f& velocity,
                                                    const CVector3f& acceleration,
                                                    const CVector3f& orbitPoint,
                                                    CVector3f& intersection) {
  // TODO: Include acceleration in orbital stepping and the linear fallback.
  return false;
}

CVector3f CSteeringBehaviors::ProjectOrbitalPosition(const CVector3f& position,
                                                     const CVector3f& velocity,
                                                     const CVector3f& orbitPoint, float dt,
                                                     float preThinkDt) {
  // TODO: Advance the position while preserving radial and tangential velocity components.
  return position;
}
