#include "MetroidPrime/CRainSplashGenerator.hpp"

#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"

#include "Kyoto/Animation/CSkinRules.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"

#include "dolphin/gx/GXVert.h"

static const GXVtxDescList skSplashVertexDesc[] = {
    {GX_VA_POS, GX_DIRECT}, {GX_VA_CLR0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};

int CRainSplashGenerator::GetNextBestPt(int point, const CSkinnedModel& model,
                                        const SSkinningWorkspace& workspace, int count,
                                        CRandom16& random, float minZ) {
  int nextPoint = point;
  float maxDistance = 0.f;
  const CVector3f reference = model.GetSkinnedPosition(workspace, point);
  for (int i = 0; i < 3; ++i) {
    int index = random.Range(0, count - 1);
    const CVector3f vertex = model.GetSkinnedPosition(workspace, index);
    const float distance = (reference - vertex).MagSquared();
    const CVector3f normal = model.GetSkinnedNormal(workspace, index);
    const float normalDot = CVector3f::Dot(normal, CVector3f::Up());
    const bool goodNormal = normalDot >= 0.f && normalDot <= 1.f;
    const bool goodHeight = minZ > 0.f ? vertex.GetZ() > minZ : true;
    if (distance > maxDistance && goodNormal && goodHeight) {
      nextPoint = index;
      maxDistance = distance;
    }
  }
  return nextPoint;
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
    mRainSplashes.push_back_unsafe(SRainSplash());
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
  const int numPoints = model.GetSkinRules()->GetNumPoints();
  if (mRaining && mGenerateTimer > mGenerateInterval) {
    int point = mCurrentPoint;
    for (int i = 0; i < mGenerationRate; ++i) {
      if (mQueueSize >= mRainSplashes.size()) {
        break;
      }
      const int nextPoint = GetNextBestPt(point, model, workspace, numPoints, mRandom, mMinZ);
      AddPoint(
          CVector3f::ByElementMultiply(mScale, model.GetSkinnedPosition(workspace, nextPoint)));
      point = nextPoint;
    }
    mCurrentPoint = point;
    mGenerateTimer = 0.f;
  }
}

CVector3f CRainSplashGenerator::GeneratePoint(const CSkinnedModel& model,
                                              const SSkinningWorkspace& workspace) {
  mCurrentPoint = GetNextBestPt(mCurrentPoint, model, workspace,
                                model.GetSkinRules()->GetNumPoints(), mRandom, mMinZ);
  return CVector3f::ByElementMultiply(mScale, model.GetSkinnedPosition(workspace, mCurrentPoint));
}

void CRainSplashGenerator::UpdateRainSplashRange(CStateManager& mgr, const int start, int end,
                                                 float dt) {
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
    if (neededFx != kEFX_None && envFx.IsSplashActive()) {
      const float rainMag = envFx.GetRainMagnitude();
      float zero = 0.f;
      if (rainMag != zero) {
        switch (neededFx) {
        case kEFX_Rain:
          raining = true;
          magnitude = rainMag;
          break;
        }
      }
    }
  }

  if (raining) {
    UpdateRainSplashes(mgr, magnitude, dt);
    mRaining = true;
  } else {
    mRaining = false;
  }
}

void CRainSplashGenerator::Draw(const CTransform4f& xf) const {
  if (mRaining) {
    DoDraw(xf);
  }
}

void CRainSplashGenerator::DoDraw(const CTransform4f& xf) const {
  if (mDt <= 0.f) {
    return;
  }
  CGX::SetVtxDescv(skSplashVertexDesc);
  CGX::SetNumChans(1);
  CGX::SetNumTevStages(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  CGX::SetNumTexGens(0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
  CGX::SetZMode(true, GX_LEQUAL, false);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  CGraphics::SetModelMatrix(xf);
  if (mQueueSize > 0) {
    if (mQueueTail <= mQueueHead) {
      for (int i = mQueueHead; i < mRainSplashes.size(); ++i) {
        const SRainSplash& splash = mRainSplashes[i];
        splash.Draw(mAlpha, mDt, splash.mPosition);
      }
      for (int i = 0; i < mQueueTail; ++i) {
        const SRainSplash& splash = mRainSplashes[i];
        splash.Draw(mAlpha, mDt, splash.mPosition);
      }
    } else {
      for (int i = mQueueHead; i < mQueueTail; ++i) {
        const SRainSplash& splash = mRainSplashes[i];
        splash.Draw(mAlpha, mDt, splash.mPosition);
      }
    }
  }
  CGX::SetLineWidth(6, GX_TO_ZERO);
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
  if (mTime > 0.f) {
    float delta = dt * mSpeed;
    const float trailTime = delta * CCast::ToReal32(mLength);
    float t = mTime - trailTime;
    if (t < 0.f) {
      t = 0.f;
    }
    int vertexCount = static_cast< int >((mTime - t) / delta + 1.f);
    CGX::SetLineWidth(mLineWidth * 6, GX_TO_ZERO);
    CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, vertexCount);
    for (int i = 0; i < vertexCount; ++i) {
      const float height = -4.f * t * (t - 1.f) * mParabolaHeight;
      GXPosition3f32(t * mEndX + position.GetX(), t * mEndY + position.GetY(),
                     height + position.GetZ());
      GXColor1u32(static_cast< uint >(t * alpha) | 0xffffff00);
      t += delta;
    }
    CGX::End();
  }
}

const uchar kSplashLineWidth = 3;

CRainSplashGenerator::SRainSplash::SRainSplash()
: mLines(4, SSplashLine()), mPosition(CVector3f::Zero()), x70_(0.f) {}

void CRainSplashGenerator::SRainSplash::Update(float dt, CStateManager& mgr) {
  for (rstl::reserved_vector< SSplashLine, 4 >::iterator it = mLines.begin(); it != mLines.end();
       ++it) {
    it->Update(dt, mgr);
  }
}

void CRainSplashGenerator::SRainSplash::Draw(float alpha, float dt,
                                             const CVector3f& position) const {
  for (rstl::reserved_vector< SSplashLine, 4 >::const_iterator it = mLines.begin();
       it != mLines.end(); ++it) {
    it->Draw(alpha, dt, position);
  }
}

const bool CRainSplashGenerator::SRainSplash::IsActive() const {
  bool active = false;
  for (rstl::reserved_vector< SSplashLine, 4 >::const_iterator it = mLines.begin();
       it != mLines.end(); ++it) {
    active |= it->mActive;
  }
  return active;
}

void CRainSplashGenerator::SRainSplash::SetPoint(const CVector3f& position) {
  for (rstl::reserved_vector< SSplashLine, 4 >::iterator it = mLines.begin(); it != mLines.end();
       ++it) {
    it->SetActive();
  }
  mPosition = position;
}
