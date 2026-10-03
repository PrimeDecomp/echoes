#include "Kyoto/Audio/CSfxPitchBend.hpp"

#include "Kyoto/Basics/CCast.hpp"

#include "rstl/math.hpp"

CSfxPitchBend::CSfxPitchBend(const CSfxHandle& handle, ushort start, ushort target, float duration)
: mHandle(handle), mPitch(start), mTargetPitch(target), mTimeRemaining(duration) {}

void CSfxPitchBend::Update(float dt) {
  if (mTimeRemaining > 0.f) {
    short difference = mTargetPitch - mPitch;
    const float pitchRate = CCast::StoF(difference) / mTimeRemaining;
    short step = CCast::FtoS(pitchRate * dt);

    if (mPitch < mTargetPitch) {
      mPitch = rstl::min_val< int >(mPitch + step, mTargetPitch);
    } else if (mPitch > mTargetPitch) {
      mPitch = rstl::max_val< int >(mPitch + step, mTargetPitch);
    }

    mTimeRemaining -= dt;
  }
}

bool CSfxPitchBend::IsFinished() const { return mTimeRemaining <= 0.f; }
