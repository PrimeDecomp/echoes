#include "MetroidPrime/CActorLights.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "rstl/math.hpp"

#include <float.h>

const float CActorLights::kDefaultMinPosChange = 0.1f;
const int CActorLights::kInvalidShadowLightIndex = -1;
int CActorLights::sFrameSchedulerCount = 0;
static bool sUseOverflowLight = true;

CActorLights::CActorLights(const uint areaUpdateFramePeriod, CVector3f lightingPositionOffset,
                           const int maxDynamicLights, const int maxAreaLights,
                           float positionUpdateThreshold, const bool ambientChannelOverflow,
                           const bool useLightSet2, const bool disableWorldLights,
                           const bool disableAmbientLights)
: mAmbientColor(CColor::Black())
, mDynamicAmbientColor(CColor::Black())
, mAid(kInvalidAreaId)
, mDirty(true)
, mCastShadows(true)
, mHasAreaLights(false)
, mFindShadowLight(false)
, mInArea(!disableWorldLights && maxAreaLights > 0)
, mAmbienceGenerated(ambientChannelOverflow)
, mLayer2(useLightSet2)
, mDisableWorldLights(disableWorldLights)
, mInBrightLight(true)
, mUseBrightLightLag(false)
, mAmbientOnly(false)
, mFindNearestDynamicLights(false)
, mDisableAmbientLights(disableAmbientLights)
, mExcludeSpecialDynamicLights(false)
, mShadowLightArrIdx(kInvalidShadowLightIndex)
, mShadowLightIdx(kInvalidShadowLightIndex)
, mLastUpdateFrame(0)
, mAreaUpdateFramePeriod(areaUpdateFramePeriod)
, mLightingPositionOffset(lightingPositionOffset)
, mMaxAreaLights(maxAreaLights)
, mMaxDynamicLights(maxDynamicLights)
, mLastActorPos(CVector3f::Zero())
, mActorPositionDeltaUpdateThreshold(positionUpdateThreshold * positionUpdateThreshold)
, mShadowDynamicRangeThreshold(0.f)
, mWorldLightingLevel(1.f)
, mBrightLightIdx(-1)
, mBrightLightLag(0) {
  ++sFrameSchedulerCount;
  sFrameSchedulerCount &= 7;
}

CActorLights::~CActorLights() {}

uint CActorLights::GetActiveLightCount() const {
  if (mInArea) {
    return mAreaLights.size() + mDynamicLights.size();
  }
  return mDynamicLights.size();
}

const CLight& CActorLights::GetLight(uint idx) const {
  if (mInArea) {
    if (idx < mAreaLights.size()) {
      return mAreaLights[idx];
    }
    return mDynamicLights[idx - mAreaLights.size()];
  }
  return mDynamicLights[idx];
}

void CActorLights::ActivateLights() const {
  // TODO: load the selected lights, fade the brightest light, and combine the two ambient
  // colors. Echoes also converts special dynamic lights before updating renderer state.
}

void CActorLights::UpdateBrightLight() {
  if (static_cast< int >(mBrightLightLag) > 0 && mInBrightLight) {
    --mBrightLightLag;
  } else if (mBrightLightLag < 15 && !mInBrightLight) {
    ++mBrightLightLag;
  }
  mUseBrightLightLag = true;
}

void CActorLights::MergeOverflowLight(CLight& out, CVector3f& color, const CLight& in, float mag) {
  CVector3f lightColor = CVector3f::Zero();
  in.GetColor().Get(lightColor[kDX], lightColor[kDY], lightColor[kDZ]);
  color += mag * lightColor;

  out.SetAngleAttenuation(
      in.GetAngleAttenuationConstant() * mag + out.GetAngleAttenuationConstant(),
      in.GetAngleAttenuationLinear() * mag + out.GetAngleAttenuationLinear(),
      in.GetAngleAttenuationQuadratic() * mag + out.GetAngleAttenuationQuadratic());
  out.SetAttenuation(in.GetAttenuationConstant() * mag + out.GetAttenuationConstant(),
                     in.GetAttenuationLinear() * mag + out.GetAttenuationLinear(),
                     in.GetAttenuationQuadratic() * mag + out.GetAttenuationQuadratic());
  out.SetPosition(out.GetPosition() + in.GetPosition() * mag);
  out.SetDirection(out.GetDirection() + in.GetDirection() * mag);
}

bool CActorLights::BuildAreaLightList(const CStateManager& mgr, const CGameArea& area,
                                      const CAABox& bounds) {
  // TODO: schedule relighting, collect and sort visible area lights, select a shadow
  // light, and distribute overflow. The packed ambient color replaces Prime's vector.
  return false;
}

void CActorLights::MultiplyLightingLevels(float level) {
  float r, g, b;
  mAmbientColor.Get(r, g, b);
  mAmbientColor.Set(r * level, g * level, b * level, 1.f);

  for (int i = 0; i < mAreaLights.size(); ++i) {
    CColor color = mAreaLights[i].GetColor();
    color.Get(r, g, b);
    color.Set(r * level, g * level, b * level, 1.f);
    mAreaLights[i].SetColor(color);
  }
}

void CActorLights::AddOverflowToLights(const CLight& light, const CVector3f& color, float mag) {
  if (mag >= 0.001f && mMaxAreaLights > 0) {
    mag = 1.f / mag;
    const CVector3f scaledColor = color * mag;
    const CColor useColor(scaledColor.GetX(), scaledColor.GetY(), scaledColor.GetZ(), 1.f);
    const CLight overflowLight = CLight::BuildCustom(
        light.GetPosition() * mag, light.GetDirection() * mag, useColor,
        light.GetAttenuationConstant() * mag, light.GetAttenuationLinear() * mag,
        light.GetAttenuationQuadratic() * mag, light.GetAngleAttenuationConstant() * mag,
        light.GetAngleAttenuationLinear() * mag, light.GetAngleAttenuationQuadratic() * mag);
    mAreaLights.push_back(overflowLight);
  }
}

void CActorLights::MoveAmbienceToLights(const CVector3f& color) {
  if (mAmbienceGenerated || !sUseOverflowLight || mAreaLights.empty()) {
    const CVector3f ambient(mAmbientColor.GetRed(), mAmbientColor.GetGreen(),
                            mAmbientColor.GetBlue());
    const CVector3f combined = ambient + color / 3.f;
    mAmbientColor.Set(rstl::min_val(1.f, combined.GetX()), rstl::min_val(1.f, combined.GetY()),
                      rstl::min_val(1.f, combined.GetZ()), 1.f);
    return;
  }

  CLight& light = mAreaLights[0];
  float r, g, b;
  light.GetColor().Get(r, g, b);
  CVector3f useColor = color + CVector3f(r, g, b);
  const float maxComponent =
      rstl::max_val(rstl::max_val(useColor.GetX(), useColor.GetY()), useColor.GetZ());
  if (maxComponent > FLT_EPSILON) {
    useColor *= 1.f / maxComponent;
  }
  light.SetColor(CColor(useColor.GetX(), useColor.GetY(), useColor.GetZ(), 1.f));
}

// Guessed name.
bool CActorLights::IsLightExcluded(const CStateManager& mgr, TUniqueId id) const {
  // TODO: resolve the light entity's concrete type and test its per-layer enable flags.
  // The invalid ID does not exclude a light; the remaining lookup is not reconstructed yet.
  return false;
}

void CActorLights::BuildDynamicLightList(const CStateManager& mgr, const CAABox& bounds) {
  UpdateBrightLight();
  mAmbientOnly = false;
  mDynamicLights.clear();
  mDynamicAmbientColor = CColor::Black();

  // TODO: collect explicit light IDs before the manager's ID/light pairs, apply layer
  // exclusions and special-light filtering, then pack the local-ambient contribution.
}

void CActorLights::BuildFaceLightList(const CStateManager& mgr, const CGameArea& area,
                                      const CAABox& aabb, int playerIndex) {
  mHasAreaLights = true;
  mAmbientColor = CColor::Black();
  mDynamicLights.clear();

  // TODO: reflect explosion lights relative to this player's first-person camera and
  // scale their attenuation using the face-reflection GUI settings.
}

void CActorLights::BuildFakeLightList(const rstl::vector< CLight >& lights, const CColor& color) {
  BuildConstantAmbientLighting(color);
  mAreaLights.clear();
  mDynamicLights.clear();

  for (int i = 0; i < 4 && i < lights.size(); ++i) {
    mDynamicLights.push_back(lights[i]);
  }
}

void CActorLights::BuildConstantAmbientLighting(const CColor& color) {
  mAmbientOnly = false;
  mAmbientColor = color;
  mAid = kInvalidAreaId;
  mDirty = true;
  mHasAreaLights = true;
  mShadowLightArrIdx = kInvalidShadowLightIndex;
  mShadowLightIdx = kInvalidShadowLightIndex;
}
