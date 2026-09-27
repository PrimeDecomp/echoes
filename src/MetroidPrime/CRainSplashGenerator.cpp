#include "MetroidPrime/CRainSplashGenerator.hpp"

#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"

#include "Kyoto/Basics/CCast.hpp"

int CRainSplashGenerator::GetNextBestPt(int point, const CSkinnedModel& model,
                                        const SSkinningWorkspace& workspace, int count,
                                        CRandom16& random, float minZ) {
  // TODO: Sample three skinned vertices and normals; choose the farthest eligible point.
  return point;
}

CRainSplashGenerator::CRainSplashGenerator(const CVector3f& scale, int maxSplashes,
                                           int generationRate, float minZ, float alpha)
: mScale(scale)
, mGenerateTimer(0.f)
, mDt(0.f)
, mMinZ(minZ)
, mAlpha(alpha > 1.f ? 255.f : alpha * 255.f)
, mCurrentPoint(0)
, mQueueTail(0)
, mQueueHead(0)
, mQueueSize(0)
, mGenerationRate(generationRate > maxSplashes ? maxSplashes : generationRate)
, x48_24_(false)
, mRaining(true)
, mForceRaining(false) {
  mRainSplashes.reserve(maxSplashes);
  for (int i = 0; i < maxSplashes; ++i) {
    mRainSplashes.push_back(SRainSplash());
  }
}

void CRainSplashGenerator::AddPoint(const CVector3f& position) {
  if (mQueueTail >= mRainSplashes.size()) {
    mQueueTail = 0;
  }
  mRainSplashes[mQueueTail].SetPoint(position);
  ++mQueueSize;
  ++mQueueTail;
}

void CRainSplashGenerator::GeneratePoints(const CSkinnedModel& model,
                                          const SSkinningWorkspace& workspace) {
  // TODO: Fill the splash queue from skinned model samples when the generation interval elapses.
}

CVector3f CRainSplashGenerator::GeneratePoint(const CSkinnedModel& model,
                                              const SSkinningWorkspace& workspace) {
  // TODO: Select a skinned point, update mCurrentPoint, and return its scaled position.
  return CVector3f::Zero();
}

void CRainSplashGenerator::UpdateRainSplashRange(CStateManager& mgr, int start, int end, float dt) {
  for (int i = start; i < end; ++i) {
    SRainSplash& splash = mRainSplashes[i];
    splash.Update(dt, mgr);
    if (!splash.IsActive()) {
      --mQueueSize;
      ++mQueueHead;
      if (mQueueHead >= mRainSplashes.size()) {
        mQueueHead = 0;
      }
    }
  }
}

void CRainSplashGenerator::UpdateRainSplashes(CStateManager& mgr, float magnitude, float dt) {
  mGenerateTimer += dt;
  mGenerateInterval = 1.f / (70.f * magnitude);
  if (mQueueSize > 0) {
    if (mQueueTail <= mQueueHead) {
      UpdateRainSplashRange(mgr, mQueueHead, mRainSplashes.size(), dt);
      UpdateRainSplashRange(mgr, 0, mQueueTail, dt);
    } else {
      UpdateRainSplashRange(mgr, mQueueHead, mQueueTail, dt);
    }
  }
}

void CRainSplashGenerator::Update(float dt, CStateManager& mgr) {
  mDt = dt;
  bool raining = mForceRaining;
  float magnitude = 10.f;
  if (!raining) {
    const int neededFx = mgr.GetWorld()->GetNeededEnvFx();
    const CEnvFxManager& envFx = *mgr.GetEnvFxManager();
    if (neededFx != kEFX_None && envFx.IsSplashActive() && envFx.GetRainMagnitude() != 0.f &&
        neededFx == kEFX_Rain) {
      raining = true;
      magnitude = envFx.GetRainMagnitude();
    }
  }

  if (raining) {
    UpdateRainSplashes(mgr, magnitude, dt);
  }
  mRaining = raining;
}

void CRainSplashGenerator::Draw(const CTransform4f& xf) const {
  if (mRaining) {
    DoDraw(xf);
  }
}

void CRainSplashGenerator::DoDraw(const CTransform4f& xf) const {
  // TODO: Set up line rendering and draw the active ring-buffer ranges in model space.
}

void CRainSplashGenerator::SSplashLine::SetActive() { mActive = true; }

void CRainSplashGenerator::SSplashLine::Update(float dt, CStateManager& mgr) {
  if (!mActive) {
    return;
  }

  if (mTime <= 0.8f) {
    mLineWidth = CCast::ToUint8(5.f * (1.f - mTime) + 3.f * mTime);
    mTime += dt * mSpeed;
  } else if (mLength != 0) {
    --mLength;
  } else {
    mActive = false;
    mTime = 0.f;
    mSpeed = mgr.Random()->Range(4.f, 8.f);
    mParabolaHeight = mgr.Random()->Range(0.015625f, 0.03125f);
    mEndX = mgr.Random()->Range(-0.125f, 0.125f);
    mEndY = mgr.Random()->Range(-0.125f, 0.125f);
    mLength = static_cast< uchar >(mgr.Random()->Range(1, 2));
  }
}

void CRainSplashGenerator::SSplashLine::Draw(float alpha, float dt,
                                             const CVector3f& position) const {
  // TODO: Draw the fading parabolic line strip, using mLength for the trail duration.
}

CRainSplashGenerator::SRainSplash::SRainSplash()
: mLines(SSplashLine()), mPosition(CVector3f::Zero()), x70_(0.f) {}

void CRainSplashGenerator::SRainSplash::Update(float dt, CStateManager& mgr) {
  for (int i = 0; i < mLines.size(); ++i) {
    mLines[i].Update(dt, mgr);
  }
}

void CRainSplashGenerator::SRainSplash::Draw(float alpha, float dt,
                                             const CVector3f& position) const {
  for (int i = 0; i < mLines.size(); ++i) {
    mLines[i].Draw(alpha, dt, position);
  }
}

bool CRainSplashGenerator::SRainSplash::IsActive() const {
  bool active = false;
  for (int i = 0; i < mLines.size(); ++i) {
    active |= mLines[i].mActive;
  }
  return active;
}

void CRainSplashGenerator::SRainSplash::SetPoint(const CVector3f& position) {
  for (int i = 0; i < mLines.size(); ++i) {
    mLines[i].SetActive();
  }
  mPosition = position;
}
