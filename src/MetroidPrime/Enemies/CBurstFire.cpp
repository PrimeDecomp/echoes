#include "MetroidPrime/Enemies/CBurstFire.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

#include "Kyoto/Math/CRelAngle.hpp"

#include "rstl/math.hpp"

CBurstFire::CBurstFire(const SBurst** burstDefs, int firstBurstCount)
: mBurstType(-1)
, mAngleIdx(-1)
, mTimeToNextShot(0.f)
, mFirstBurstIdx(0)
, mFirstBurstCounter(firstBurstCount)
, mShouldFire(false)
, mAvoidAccuracy(false)
, mCurBursts(nullptr) {
  while (*burstDefs) {
    mBurstDefs.push_back(*burstDefs);
    ++burstDefs;
  }
}

void CBurstFire::Start(CStateManager& mgr) {
  const SBurst* bursts = mBurstDefs[mBurstType];
  int burstIdx = -1;

  if (mFirstBurstCounter-- > 0) {
    burstIdx = mFirstBurstIdx >= 0 ? mFirstBurstIdx : 0;
  } else {
    int random = mgr.Random()->Range(0, 100);
    int advanceAccum = 0;
    do {
      ++burstIdx;
      int advanceWeight = bursts[burstIdx].mRandomSelectionWeight;
      if (advanceWeight == 0) {
        advanceAccum = 100;
        --burstIdx;
      }
      advanceAccum += advanceWeight;
    } while (random > advanceAccum);
  }

  mCurBursts = &bursts[burstIdx];
  mAngleIdx = -1;
  mTimeToNextShot = 0.f;
  mShouldFire = false;
}

void CBurstFire::Update(CStateManager& mgr, float dt) {
  mShouldFire = false;
  if (!mCurBursts) {
    return;
  }

  mTimeToNextShot -= dt;
  if (mTimeToNextShot < 0.f) {
    ++mAngleIdx;
    if (mCurBursts->mShotAngles[mAngleIdx] > 0) {
      mShouldFire = true;
      mTimeToNextShot = mCurBursts->mTimeToNextShot;
      mTimeToNextShot += (mgr.Random()->Float() - 0.5f) * mCurBursts->mTimeToNextShotVariance;
    } else {
      mCurBursts = nullptr;
    }
  }
}

CVector3f CBurstFire::GetError(float xMag, float zMag) const {
  CVector3f result = CVector3f::Zero();

  if (mShouldFire && mCurBursts) {
    int shotAngle = mCurBursts->mShotAngles[mAngleIdx];
    if (mAvoidAccuracy && (shotAngle == 4 || shotAngle == 12)) {
      shotAngle = mCurBursts->mShotAngles[mAngleIdx > 0 ? mAngleIdx - 1 : mAngleIdx + 1];
    }

    if (shotAngle > 0) {
      const float angleStep = CRelAngle::FromDegrees(-22.5f).AsRadians();
      float angle = angleStep * shotAngle;
      result.SetX(CMath::FastCosR(angle) * xMag);
      result.SetZ(CMath::FastSinR(angle) * zMag);
    }
  }

  return result;
}

CVector3f CBurstFire::GetDistanceCompensatedError(float dist, float maxErrDist) const {
  float xErr = GetMaxXError();
  float zErr = GetMaxZError();
  float div = dist / maxErrDist;
  xErr = rstl::min_val(div * xErr, xErr);
  zErr = rstl::min_val(div * zErr, zErr);
  return GetError(xErr, zErr);
}

float CBurstFire::GetMaxXError() const { return gpTweakPlayerA->GetPlayerRadius() * 3.625f + 0.2f; }

float CBurstFire::GetMaxZError() const { return gpTweakPlayerA->GetEyeOffset(); }
