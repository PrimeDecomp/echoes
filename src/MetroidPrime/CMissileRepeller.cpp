#include "MetroidPrime/CMissileRepeller.hpp"

CMissileRepeller::CMissileRepeller(float radius, float deflectionRate, ushort soundId,
                                 float spaceWarpStrength, const CVector3f& offset)
: mRadius(radius)
, mDeflectionRate(deflectionRate)
, mSoundId(soundId)
, mSpaceWarpStrength(spaceWarpStrength)
, mOffset(offset)
, mDeflectedProjectiles(kInvalidUniqueId)
, mSoundHandle()
, mSoundTime(0.f)
, mActive(true) {}

CMissileRepeller::~CMissileRepeller() {}

void CMissileRepeller::Update(CStateManager& mgr, const CActor& actor, float dt) {}

void CMissileRepeller::Render(const CStateManager& mgr, const CActor& actor) const {}
