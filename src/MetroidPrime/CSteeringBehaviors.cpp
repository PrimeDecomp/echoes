#include "MetroidPrime/CSteeringBehaviors.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "rstl/math.hpp"

// Native polynomial solver used by the accelerated interception path.
extern "C" int fn_802CB918(const float* coefficients, float* roots);

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
  const CVector3f delta = position - origin;
  float coefficients[5];
  coefficients[0] = delta.MagSquared();
  coefficients[1] = 2.f * CVector3f::Dot(delta, velocity);
  coefficients[2] = velocity.MagSquared() + CVector3f::Dot(delta, acceleration) - speed * speed;
  coefficients[3] = CVector3f::Dot(velocity, acceleration);
  coefficients[4] = 0.25f * acceleration.MagSquared();

  float roots[4];
  const int count = fn_802CB918(coefficients, roots);
  bool found = false;
  for (int i = 0; i < count; ++i) {
    const float time = roots[i];
    if (time > 0.f) {
      found = true;
      intersection = position + velocity * time + 0.5f * time * time * acceleration;
    }
  }
  return found;
}

bool CSteeringBehaviors::ProjectOrbitalIntersection(const CVector3f& origin, float speed, float dt,
                                                    const CVector3f& position,
                                                    const CVector3f& velocity,
                                                    const CVector3f& orbitPoint,
                                                    CVector3f& intersection) {
  if (speed > 0.f) {
    if (velocity.CanBeNormalized()) {
      CVector3f radial((position - orbitPoint).DropZ());
      if (radial.CanBeNormalized()) {
        CVector3f currentPosition = position;
        CVector3f currentVelocity = velocity;
        CVector3f delta = currentPosition - origin;
        float travelTime = delta.Magnitude() / speed;
        float elapsed = 0.f;
        float previousRemaining = FLT_MAX;
        float remaining = travelTime - elapsed;
        CVector3f radialUnit = radial.AsNormalized();
        CVector3f tangent = CVector3f::Cross(radialUnit, CVector3f::Up());
        float tangentialSpeed = CVector3f::Dot(currentVelocity, tangent);
        float radialSpeed = CVector3f::Dot(currentVelocity, radialUnit);

        while (remaining < previousRemaining && elapsed < 4.f) {
          if (close_enough(remaining, dt) || remaining < 0.f) {
            intersection = currentPosition;
            return true;
          }

          currentPosition += dt * currentVelocity;
          previousRemaining = remaining;
          radial = (currentPosition - orbitPoint).DropZ();
          if (!radial.CanBeNormalized()) {
            break;
          }

          radialUnit = radial.AsNormalized();
          CVector3f tangent = CVector3f::Cross(radialUnit, CVector3f::Up());
          currentVelocity = tangentialSpeed * tangent + radialSpeed * radialUnit;
          delta = currentPosition - origin;
          travelTime = delta.Magnitude() / speed;
          elapsed += dt;
          remaining = travelTime - elapsed;
        }
      } else {
        return ProjectLinearIntersection(origin, speed, position, velocity, intersection);
      }
    } else {
      intersection = position;
      return true;
    }
  }
  return false;
}

bool CSteeringBehaviors::ProjectOrbitalIntersection(const CVector3f& origin, float speed, float dt,
                                                    const CVector3f& position,
                                                    const CVector3f& velocity,
                                                    const CVector3f& acceleration,
                                                    const CVector3f& orbitPoint,
                                                    CVector3f& intersection) {
  bool found = false;
  if (speed > 0.f) {
    CVector3f radial((position - orbitPoint).DropZ());
    if (velocity.CanBeNormalized() && radial.CanBeNormalized()) {
      CVector3f currentPosition = position;
      CVector3f currentVelocity = velocity;
      CVector3f delta = currentPosition - origin;
      float travelTime = delta.Magnitude() / speed;
      float elapsed = 0.f;
      float previousRemaining = FLT_MAX;
      float remaining = travelTime - elapsed;
      CVector3f radialUnit = radial.AsNormalized();
      CVector3f tangent = CVector3f::Cross(radialUnit, CVector3f::Up());
      float tangentialSpeed = CVector3f::Dot(currentVelocity, tangent);
      float radialSpeed = CVector3f::Dot(currentVelocity, radialUnit);

      while (remaining < previousRemaining && elapsed < 4.f) {
        if (close_enough(remaining, dt) || remaining < 0.f) {
          intersection = currentPosition;
          found = true;
          break;
        }

        currentPosition += dt * currentVelocity;
        previousRemaining = remaining;
        delta = currentPosition - origin;
        travelTime = delta.Magnitude() / speed;
        elapsed += dt;
        remaining = travelTime - elapsed;
        radial = (currentPosition - orbitPoint).DropZ();
        if (!radial.CanBeNormalized()) {
          break;
        }

        radialUnit = radial.AsNormalized();
        CVector3f tangent = CVector3f::Cross(radialUnit, CVector3f::Up());
        currentVelocity = CVector3f(0.f, 0.f, currentVelocity.GetZ()) + dt * acceleration;
        currentVelocity += tangentialSpeed * tangent + radialSpeed * radialUnit;
      }
    } else {
      return ProjectLinearIntersection(origin, speed, position, velocity, acceleration,
                                       intersection);
    }
  }

  return found;
}

CVector3f CSteeringBehaviors::ProjectOrbitalPosition(const CVector3f& position,
                                                     const CVector3f& velocity,
                                                     const CVector3f& orbitPoint, float dt,
                                                     float preThinkDt) {
  CVector3f currentPosition = position;
  if (velocity.CanBeNormalized()) {
    CVector3f radial((position - orbitPoint).DropZ());
    if (radial.CanBeNormalized()) {
      CVector3f currentVelocity = velocity;
      float elapsed = 0.f;
      CVector3f radialUnit = radial.AsNormalized();
      CVector3f tangent = CVector3f::Cross(radialUnit, CVector3f::Up());
      float tangentialSpeed = CVector3f::Dot(currentVelocity, tangent);
      float radialSpeed = CVector3f::Dot(currentVelocity, radialUnit);

      while (elapsed < dt) {
        currentPosition += preThinkDt * currentVelocity;
        radial = (currentPosition - orbitPoint).DropZ();
        if (radial.CanBeNormalized()) {
          radialUnit = radial.AsNormalized();
          CVector3f tangent = CVector3f::Cross(radialUnit, CVector3f::Up());
          currentVelocity = tangentialSpeed * tangent + radialSpeed * radialUnit;
        }

        float step = dt - elapsed;
        if (step > preThinkDt) {
          step = preThinkDt;
        }
        elapsed += step;
      }
    }
  }
  return currentPosition;
}
