#ifndef _CSTEERINGBEHAVIORS
#define _CSTEERINGBEHAVIORS

#include "Kyoto/Math/CVector3f.hpp"

class CPhysicsActor;

class CSteeringBehaviors {
public:
  CSteeringBehaviors();
  CVector3f Seek(const CPhysicsActor& actor, const CVector3f& destination) const;
  CVector3f Flee(const CPhysicsActor& actor, const CVector3f& position) const;

private:
  float x0_;
};
CHECK_SIZEOF(CSteeringBehaviors, 0x4)

#endif
