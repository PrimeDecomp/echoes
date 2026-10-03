#ifndef _CMISSILEREPELLER
#define _CMISSILEREPELLER

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/reserved_vector.hpp"

class CActor;
class CStateManager;

class CMissileRepeller {
public:
  CMissileRepeller(float radius, float deflectionRate, ushort soundId, float spaceWarpStrength,
                  const CVector3f& offset);
  ~CMissileRepeller();

  void Update(CStateManager& mgr, const CActor& actor, float dt);
  void Render(const CStateManager& mgr, const CActor& actor) const;

private:
  // Guessed member names.
  float mRadius;
  float mDeflectionRate;
  ushort mSoundId;
  float mSpaceWarpStrength;
  CVector3f mOffset;
  rstl::reserved_vector< TUniqueId, 10 > mDeflectedProjectiles;
  CSfxHandle mSoundHandle;
  float mSoundTime;
  bool mActive;
};
CHECK_SIZEOF(CMissileRepeller, 0x40)

#endif // _CMISSILEREPELLER
