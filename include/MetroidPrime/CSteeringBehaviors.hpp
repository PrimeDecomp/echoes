#ifndef _CSTEERINGBEHAVIORS
#define _CSTEERINGBEHAVIORS

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/reserved_vector.hpp"

class CPhysicsActor;
class CStateManager;

class CSteeringBehaviors {
public:
  CSteeringBehaviors();
  CVector3f Seek(const CPhysicsActor& actor, const CVector3f& destination) const;
  CVector3f Flee(const CPhysicsActor& actor, const CVector3f& position) const;
  CVector3f Arrival(const CPhysicsActor& actor, const CVector3f& destination,
                    float dampingRadius) const;
  CVector3f Separation(const CPhysicsActor& actor, const CVector3f& position,
                       float separation) const;
  CVector3f Alignment(const CPhysicsActor& actor, rstl::reserved_vector< TUniqueId, 1024 >& list,
                      const CStateManager& mgr) const;
  CVector3f Cohesion(const CPhysicsActor& actor, rstl::reserved_vector< TUniqueId, 1024 >& list,
                     float dampingRadius, const CStateManager& mgr) const;
  CVector2f Separation2D(const CPhysicsActor& actor, const CVector2f& position,
                         float separation) const;

  static bool ProjectLinearIntersection(const CVector3f& origin, float speed,
                                        const CVector3f& position, const CVector3f& velocity,
                                        CVector3f& intersection);
  static bool ProjectLinearIntersection(const CVector3f& origin, float speed,
                                        const CVector3f& position, const CVector3f& velocity,
                                        const CVector3f& acceleration, CVector3f& intersection);
  static bool ProjectOrbitalIntersection(const CVector3f& origin, float speed, float dt,
                                         const CVector3f& position, const CVector3f& velocity,
                                         const CVector3f& orbitPoint, CVector3f& intersection);
  static bool ProjectOrbitalIntersection(const CVector3f& origin, float speed, float dt,
                                         const CVector3f& position, const CVector3f& velocity,
                                         const CVector3f& acceleration,
                                         const CVector3f& orbitPoint, CVector3f& intersection);
  static CVector3f ProjectOrbitalPosition(const CVector3f& position, const CVector3f& velocity,
                                          const CVector3f& orbitPoint, float dt, float preThinkDt);

private:
  float x0_; // Initialized to pi/2; no consumer has established its purpose yet.
};
CHECK_SIZEOF(CSteeringBehaviors, 0x4)

#endif // _CSTEERINGBEHAVIORS
