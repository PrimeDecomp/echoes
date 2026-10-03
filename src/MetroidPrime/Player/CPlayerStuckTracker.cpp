#include "MetroidPrime/Player/CPlayerStuckTracker.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "rstl/math.hpp"
#include "rstl/optional_object.hpp"
#include <float.h>

CPlayerStuckTracker::CPlayerStuckTracker() : mUpdateCount(0) {}

void CPlayerStuckTracker::AddState(EPlayerState state, const CVector3f& position,
                                  const CVector3f& velocity, const CVector2f& input) {
  if (state == kPS_StartingJump) {
    ++mUpdateCount;
    mPositions.AddValue(position);
    mSpeeds.AddValue(velocity.Magnitude());
    mInputs.AddValue(input);
  } else {
    ++mUpdateCount;
  }
}

// Prime-correlated helper name; native hidden-result storage contains value and validity.
template < typename T, int N >
rstl::optional_object< T > _getElementBoundsCheck(const rstl::reserved_vector< T, N >& v,
                                               int index);

template <>
rstl::optional_object< CVector2f >
_getElementBoundsCheck(const rstl::reserved_vector< CVector2f, 20 >& v, int index) {
  if (index >= v.size()) {
    return rstl::optional_object_null();
  }
  return v[index];
}

template <>
rstl::optional_object< float >
_getElementBoundsCheck(const rstl::reserved_vector< float, 20 >& v, int index) {
  if (index >= v.size()) {
    return rstl::optional_object_null();
  }
  return v[index];
}

template <>
rstl::optional_object< CVector3f >
_getElementBoundsCheck(const rstl::reserved_vector< CVector3f, 20 >& v, int index) {
  if (index >= v.size()) {
    return rstl::optional_object_null();
  }
  return v[index];
}

bool CPlayerStuckTracker::IsPlayerStuck() const {
  if (mUpdateCount >= 20 && mPositions.size() == 20) {
    float distance = 0.f;
    CAABox inputBounds(CVector3f(*_getElementBoundsCheck(mInputs, 0), 0.f),
                       CVector3f(*_getElementBoundsCheck(mInputs, 0), 0.f));
    float minSpeed = *_getElementBoundsCheck(mSpeeds, 0);

    for (int i = 1; i < 20; ++i) {
      const CVector3f delta =
          *_getElementBoundsCheck(mPositions, i - 1) - *_getElementBoundsCheck(mPositions, i);
      const float deltaSq = delta.MagSquared();
      if (deltaSq > FLT_EPSILON) {
        distance += CMath::FastSqrtF(deltaSq);
      }
      minSpeed = rstl::min_val(*_getElementBoundsCheck(mSpeeds, i), minSpeed);
      inputBounds.AccumulateBounds(CVector3f(*_getElementBoundsCheck(mInputs, i), 0.f));
    }

    const bool stopped = distance < 1.f / 30.f || distance < (1.f / 30.f) * minSpeed;
    if (stopped) {
      const float inputRange = (inputBounds.GetMaxPoint() - inputBounds.GetMinPoint()).Magnitude();
      CAABox inputWithZero(inputBounds.GetMinPoint(), inputBounds.GetMaxPoint());
      inputWithZero.AccumulateBounds(CVector3f::Zero());
      const float inputExtent =
          (inputWithZero.GetMaxPoint() - inputWithZero.GetMinPoint()).Magnitude();
      const bool unusualInput = inputExtent < 0.01f || inputRange > 1.5f;
      if (unusualInput) {
        return true;
      }
    }
  }
  return false;
}

void CPlayerStuckTracker::ResetStats() {
  mPositions.clear();
  mSpeeds.clear();
  mInputs.clear();
  mUpdateCount = 0;
}
