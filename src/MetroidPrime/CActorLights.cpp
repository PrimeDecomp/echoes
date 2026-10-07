#include "MetroidPrime/CActorLights.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/PVS/CPVSVisSet.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDynamicLight.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "WorldFormat/CPVSAreaSet.hpp"
#include "WorldFormat/CWorldLight.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

#include <alloca.h>
#include <float.h>

const float CActorLights::kDefaultMinPosChange = 0.1f;
const uint CActorLights::kInvalidShadowLightIndex = -1;
int CActorLights::sFrameSchedulerCount = 0;
static bool sUseOverflowLight = true;

struct SLightValue {
  uint mAreaLightIdx;
  CVector3f mColor;
  float mColorMag;
  float mAccumulatedMag;
  EPVSVisSetState mVisibility;

  SLightValue(uint idx, const CVector3f& color, EPVSVisSetState visibility)
  : mAreaLightIdx(idx)
  , mColor(color)
  , mColorMag(color.Magnitude())
  , mAccumulatedMag(0.f)
  , mVisibility(visibility) {}

  struct CPredicate {
    bool operator()(const SLightValue& a, const SLightValue& b) const {
      return a.mColorMag > b.mColorMag;
    }
  };
};
CHECK_SIZEOF(SLightValue, 0x1c);

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
  if (mInArea && (!mHasAreaLights || mAmbientOnly)) {
    gpRender->SetAmbientColor(CColor::White());
    CGraphics::DisableAllLights();
    return;
  }

  float ambientR = mAmbientColor.GetRed() + mDynamicAmbientColor.GetRed();
  float ambientG = mAmbientColor.GetGreen() + mDynamicAmbientColor.GetGreen();
  float ambientB = mAmbientColor.GetBlue() + mDynamicAmbientColor.GetBlue();

  int lightIdx = 0;
  if (!mAreaLights.empty()) {
    if (mBrightLightLag != 0 && mUseBrightLightLag) {
      CLight light = mAreaLights[0];
      const CColor& color = light.GetColor();
      const float level = 1.f - static_cast< int >(mBrightLightLag) / 15.f;
      light.SetColor(
          CColor(color.GetRed() * level, color.GetGreen() * level, color.GetBlue() * level, 1.f));
      CGraphics::LoadLight(kLight0, light);
    } else {
      CGraphics::LoadLight(kLight0, mAreaLights[0]);
    }
    lightIdx = 1;
    for (int i = 1; i < mAreaLights.size(); ++i, ++lightIdx) {
      CGraphics::LoadLight(static_cast< ERglLight >(lightIdx), mAreaLights[i]);
    }
  }

  for (int i = 0; i < mDynamicLights.size(); ++i, ++lightIdx) {
    const CLight& light = mDynamicLights[i];
    if (light.GetType() == kLT_Hard) {
      float intensity = 1.f;
      const float distance = (mLastActorPos - light.GetPosition()).Magnitude();
      if (light.GetRadius() > 0.001f) {
        const float fraction = distance / light.GetRadius();
        if (!(fraction < 0.8f)) {
          const float faded = 1.f - 5.f * (fraction - 0.8f);
          intensity = faded > 0.f ? faded : 0.f;
        }
      }
      ambientR += 0.1f;
      ambientG += 0.1f;
      ambientB += 0.1f;
      const CLight converted = CLight::BuildCustom(light.GetPosition(), CVector3f::Up(),
                                                   CColor(intensity, intensity, intensity, 1.f),
                                                   1.f, 0.f, 0.f, 1.f, 0.f, 0.f);
      CGraphics::LoadLight(static_cast< ERglLight >(lightIdx), converted);
    } else {
      CGraphics::LoadLight(static_cast< ERglLight >(lightIdx), light);
    }
  }

  if (lightIdx > 0) {
    CGraphics::SetLightState(static_cast< uchar >((1 << lightIdx) - 1));
  } else {
    CGraphics::DisableAllLights();
  }

  if (mDisableWorldLights) {
    const uchar value = CCast::ToUint8(255.f * mWorldLightingLevel);
    gpRender->SetAmbientColor(CColor::Black());
    gpRender->SetGXRegister1Color(CColor(value, value, value, uchar(255)));
  } else {
    gpRender->SetAmbientColor(CColor(ambientR > 1.f ? 1.f : ambientR,
                                     ambientG > 1.f ? 1.f : ambientG,
                                     ambientB > 1.f ? 1.f : ambientB, 1.f));
  }
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
  const rstl::vector< CWorldLight >& initialLights =
      mLayer2 ? area.GetPostConstructed()->mLightsB : area.GetPostConstructed()->mLightsA;
  mHasAreaLights = !initialLights.empty();
  if (!mHasAreaLights || !mInArea) {
    if (mDisableWorldLights) {
      mWorldLightingLevel = area.GetPostConstructed()->mWorldLightingLevel;
    }
    mShadowLightArrIdx = kInvalidShadowLightIndex;
    return true;
  }

  CVector3f position = CVector3f::Zero();
  if (!mDirty && mAid == area.GetId()) {
    if (mgr.GetRenderFrameIndex() - mLastUpdateFrame < mAreaUpdateFramePeriod) {
      return false;
    }
    mLastUpdateFrame = mgr.GetRenderFrameIndex();
    position = bounds.GetCenterPoint() + mLightingPositionOffset;
    if (mWorldLightingLevel == area.GetPostConstructed()->mWorldLightingLevel &&
        (mLastActorPos - position).MagSquared() < mActorPositionDeltaUpdateThreshold) {
      return false;
    }
    mLastActorPos = position;
  } else {
    if (mAid != area.GetId()) {
      mBrightLightIdx = -1;
    }
    mLastUpdateFrame = sFrameSchedulerCount + mgr.GetRenderFrameIndex();
    position = bounds.GetCenterPoint() + mLightingPositionOffset;
    mLastActorPos = position;
  }
  mWorldLightingLevel = area.GetPostConstructed()->mWorldLightingLevel;
  mDirty = false;
  mAid = area.GetId();
  mShadowLightArrIdx = kInvalidShadowLightIndex;

  const rstl::vector< CWorldLight >& worldLights =
      mLayer2 ? area.GetPostConstructed()->mLightsB : area.GetPostConstructed()->mLightsA;
  const rstl::vector< CLight >& lights =
      mLayer2 ? area.GetPostConstructed()->mGfxLightsB : area.GetPostConstructed()->mGfxLightsA;
  SLightValue* values =
      static_cast< SLightValue* >(alloca(worldLights.size() * sizeof(SLightValue)));
  int valueCount = 0;
  CVector3f localAmbient = CVector3f::Zero();
  const CPVSAreaSet* areaPVS = area.GetPostConstructed()->mPvs.get();
  const bool usePVS = areaPVS != nullptr && gkPVSEnabled == 1;
  const bool useSecondLayer = mLayer2 ? (usePVS ? areaPVS->GetNum2ndLights() != 0 : true) : false;
  CPVSVisSet centerSet(kVSS_OutOfBounds);
  CPVSVisSet maxSet(kVSS_OutOfBounds);
  CPVSVisSet minSet(kVSS_OutOfBounds);
  if (usePVS) {
    centerSet =
        areaPVS->GetVisOctree().GetVisSet(area.GetPostConstructed()->mInverseTransform * position);
    maxSet = areaPVS->GetVisOctree().GetVisSet(area.GetPostConstructed()->mInverseTransform *
                                               bounds.GetMaxPoint());
    minSet = areaPVS->GetVisOctree().GetVisSet(area.GetPostConstructed()->mInverseTransform *
                                               bounds.GetMinPoint());
  }

  for (int i = 0; i < lights.size(); ++i) {
    const CLight& light = lights[i];
    if (light.GetType() == kLT_LocalAmbient) {
      localAmbient = light.GetNormalIndependentLightingAtPoint(position);
      continue;
    }
    EPVSVisSetState visibility = kVSS_OutOfBounds;
    if (usePVS && worldLights[i].DoesCastShadows()) {
      const int feature =
          useSecondLayer ? area.Get2ndPVSLightFeature(i) : area.Get1stPVSLightFeature(i);
      if (feature != -1) {
        visibility = centerSet.GetVisible(feature);
        if (visibility != kVSS_NodeFound) {
          visibility = CPVSVisSet::CombineStates(visibility, maxSet.GetVisible(feature));
        }
        if (visibility != kVSS_NodeFound) {
          visibility = CPVSVisSet::CombineStates(visibility, minSet.GetVisible(feature));
        }
      }
    }
    if (visibility != kVSS_EndOfTree &&
        CollisionUtil::AABoxSphereIntersection(
            bounds, CSphere(light.GetPosition(), light.GetRadius() * 2.f))) {
      values[valueCount] =
          SLightValue(i, light.GetNormalIndependentLightingAtPoint(position), visibility);
      ++valueCount;
    }
  }

  rstl::sort(values, values + valueCount, SLightValue::CPredicate());
  if (mFindShadowLight) {
    float magnitude = localAmbient.Magnitude();
    for (int i = valueCount - 1; i >= 0; --i) {
      magnitude += values[i].mColorMag;
      values[i].mAccumulatedMag = magnitude;
    }
  }

  CVector3f overflowAmbient = CVector3f::Zero();
  CLight overflowLight = CLight::BuildCustom(CVector3f::Zero(), CVector3f::Zero(), CColor::Black(),
                                             0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
  CVector3f overflowColor = CVector3f::Zero();
  float overflowMagnitude = 0.f;
  const bool useOverflowLight = !mAmbienceGenerated && sUseOverflowLight;
  const int maxAreaLights = useOverflowLight ? mMaxAreaLights - 1 : mMaxAreaLights;
  mAreaLights.clear();
  int mostSignificantLight = 0;
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid),
      CMaterialList(kMT_Projectile, kMT_ProjectilePassthrough, kMT_SeeThrough));

  for (int i = 0; i < valueCount; ++i) {
    const SLightValue& value = values[i];
    if (mAreaLights.size() < maxAreaLights) {
      bool contact = true;
      const int lightIndex = value.mAreaLightIdx;
      const bool castsShadows =
          worldLights[lightIndex].DoesCastShadows() == true && mCastShadows == true;
      const bool outOfBounds = usePVS && value.mVisibility == kVSS_OutOfBounds;
      if (castsShadows) {
        const CLight& light = lights[lightIndex];
        const CVector3f rayStart = position;
        CVector3f delta = light.GetPosition() - rayStart;
        const float distance = delta.Magnitude();
        const bool shadowCandidate = mFindShadowLight &&
                                     static_cast< uint >(mShadowLightArrIdx) ==
                                         static_cast< uint >(kInvalidShadowLightIndex) &&
                                     light.GetType() != kLT_LocalAmbient && distance > 2.f &&
                                     !bounds.PointInside(light.GetPosition());
        bool useShadow = shadowCandidate;
        if (shadowCandidate) {
          const bool significant =
              mAreaLights.empty() ||
              (mAreaLights.size() == 1 &&
               value.mColorMag / values[mostSignificantLight].mColorMag > 0.5f);
          useShadow = significant;
          if (significant) {
            useShadow = value.mColorMag / value.mAccumulatedMag >
                        mShadowDynamicRangeThreshold / (1.f + mShadowDynamicRangeThreshold);
          }
        }
        if (useShadow) {
          mShadowLightArrIdx = mAreaLights.size();
          mShadowLightIdx = lightIndex;
        } else if (!outOfBounds) {
          delta *= 1.f / distance;
          contact =
              CGameCollision::RayStaticLineOfSightTest(area, rayStart, delta, distance, filter);
          if (i == 0) {
            if (contact) {
              mInBrightLight = true;
            } else {
              mInBrightLight = false;
            }
            if (mBrightLightIdx != lightIndex) {
              if (contact) {
                mBrightLightLag = 0;
              } else {
                mBrightLightLag = 15;
              }
              mBrightLightIdx = lightIndex;
            }
            mUseBrightLightLag = false;
            contact = true;
          }
        }
      }
      if (contact) {
        if (mAreaLights.empty()) {
          mostSignificantLight = i;
        }
        mAreaLights.push_back(lights[lightIndex]);
      }
    } else if (useOverflowLight && value.mColorMag > 0.001f) {
      MergeOverflowLight(overflowLight, overflowColor, lights[value.mAreaLightIdx],
                         value.mColorMag);
      overflowMagnitude += value.mColorMag;
    } else {
      overflowAmbient += value.mColor;
    }
  }

  if (useOverflowLight) {
    AddOverflowToLights(overflowLight, overflowColor, overflowMagnitude);
  } else {
    MoveAmbienceToLights(overflowAmbient);
  }
  if (mDisableAmbientLights) {
    mAmbientColor = CColor::Black();
  } else {
    if (localAmbient.GetX() > 1.f) {
      localAmbient.SetX(1.f);
    }
    if (localAmbient.GetY() > 1.f) {
      localAmbient.SetY(1.f);
    }
    if (localAmbient.GetZ() > 1.f) {
      localAmbient.SetZ(1.f);
    }
    mAmbientColor = CColor(localAmbient.GetX(), localAmbient.GetY(), localAmbient.GetZ(), 1.f);
  }
  if (area.GetPostConstructed()->mWorldLightingLevel < 1.f) {
    MultiplyLightingLevels(area.GetPostConstructed()->mWorldLightingLevel);
  }
  return true;
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
    const float red = rstl::min_val(1.f, (1.f / 3.f) * color.GetX() + mAmbientColor.GetRed());
    const float green = rstl::min_val(1.f, (1.f / 3.f) * color.GetY() + mAmbientColor.GetGreen());
    const float blue = rstl::min_val(1.f, (1.f / 3.f) * color.GetZ() + mAmbientColor.GetBlue());
    mAmbientColor.Set(red, green, blue, 1.f);
    return;
  }

  CLight& light = mAreaLights[0];
  float r, g, b;
  light.GetColor().Get(r, g, b);
  CVector3f useColor = color + CVector3f(r, g, b);
  float maxComponent = rstl::max_val(useColor.GetX(), useColor.GetY());
  maxComponent = rstl::max_val(maxComponent, useColor.GetZ());
  if (maxComponent > FLT_EPSILON) {
    useColor *= 1.f / maxComponent;
  }
  light.SetColor(CColor(useColor.GetX(), useColor.GetY(), useColor.GetZ(), 1.f));
}

// Guessed name.
bool CActorLights::IsLightExcluded(const CStateManager& mgr, TUniqueId id) const {
  if (id != kInvalidUniqueId) {
    if (const CScriptDynamicLight* light =
            TCastToConstPtr< CScriptDynamicLight >(mgr.GetObjectById(TUniqueId(id)))) {
      if (!((mLayer2 && light->UsesLayerTwo()) || (!mLayer2 && light->UsesLayerOne()))) {
        return true;
      }
    }
  }
  return false;
}

void CActorLights::BuildDynamicLightList(const CStateManager& mgr, const CAABox& bounds) {
  UpdateBrightLight();
  mAmbientOnly = false;
  mDynamicLights.clear();
  mDynamicAmbientColor = CColor::Black();

  CVector3f ambient = CVector3f::Zero();
  const CVector3f lightingPos = bounds.GetCenterPoint() + mLightingPositionOffset;
  for (int i = 0; i < mExplicitLightIds.size() && mDynamicLights.size() < 4; ++i) {
    const CScriptDynamicLight* gameLight =
        TCastToConstPtr< CScriptDynamicLight >(mgr.GetObjectById(mExplicitLightIds[i]));
    if (gameLight == nullptr || !gameLight->GetActive()) {
      continue;
    }
    const CLight light = gameLight->GetLight();
    if (light.GetType() == kLT_LocalAmbient) {
      ambient = light.GetNormalIndependentLightingAtPoint(lightingPos);
    } else if (CollisionUtil::AABoxSphereIntersection(
                   bounds, CSphere(light.GetPosition(), light.GetRadius()))) {
      mDynamicLights.push_back(light);
    }
  }

  const rstl::vector< rstl::pair< TUniqueId, CLight > >& lights = mgr.GetDynamicActorLights();
  if (!mFindNearestDynamicLights) {
    for (int i = 0; i < lights.size() && mDynamicLights.size() < mMaxDynamicLights; ++i) {
      if (IsLightExcluded(mgr, lights[i].first)) {
        continue;
      }
      const CLight& light = lights[i].second;
      if (light.GetType() == kLT_Hard && mExcludeSpecialDynamicLights) {
        continue;
      }
      if (light.GetType() == kLT_LocalAmbient) {
        if (!mDisableAmbientLights) {
          ambient = light.GetNormalIndependentLightingAtPoint(lightingPos);
        }
      } else if (CollisionUtil::AABoxSphereIntersection(
                     bounds, CSphere(light.GetPosition(), light.GetRadius()))) {
        mDynamicLights.push_back(light);
      }
    }
  } else {
    int ids[4];
    float radii[4] = {-1.f, -1.f, -1.f, -1.f};
    const int explicitCount = mDynamicLights.size();
    for (int i = 0; i < lights.size() && mDynamicLights.size() < 4; ++i) {
      if (IsLightExcluded(mgr, lights[i].first)) {
        continue;
      }
      const CLight& light = lights[i].second;
      if (light.GetType() == kLT_Hard && mExcludeSpecialDynamicLights) {
        continue;
      }
      if (light.GetType() == kLT_LocalAmbient) {
        if (!mDisableAmbientLights) {
          ambient = light.GetNormalIndependentLightingAtPoint(lightingPos);
        }
        continue;
      }

      bool handled = false;
      for (int j = explicitCount; j < mDynamicLights.size(); ++j) {
        if (ids[j] == light.GetId()) {
          const float radius = CollisionUtil::AABoxSphereIntersectionRadius(
              bounds, CSphere(light.GetPosition(), light.GetRadius()));
          if (radius >= 0.f && radii[j] > radius) {
            radii[j] = radius;
            mDynamicLights[j] = light;
            handled = true;
          }
          break;
        }
      }
      if (!handled) {
        const int idx = mDynamicLights.size();
        radii[idx] = CollisionUtil::AABoxSphereIntersectionRadius(
            bounds, CSphere(light.GetPosition(), light.GetRadius()));
        if (radii[idx] >= 0.f) {
          ids[idx] = light.GetId();
          mDynamicLights.push_back(light);
        }
      }
    }
  }

  mDynamicAmbientColor.Set(ambient.GetX() > 1.f ? 1.f : ambient.GetX(),
                           ambient.GetY() > 1.f ? 1.f : ambient.GetY(),
                           ambient.GetZ() > 1.f ? 1.f : ambient.GetZ(), 1.f);
}

void CActorLights::BuildFaceLightList(const CStateManager& mgr, const CGameArea& area,
                                      const CAABox& aabb, int playerIndex) {
  const CTransform4f cameraTransform =
      mgr.GetCameraManager(playerIndex)->GetFirstPersonCamera()->GetTransform();
  mHasAreaLights = true;
  mAmbientColor = CColor::Black();
  mDynamicLights.clear();

  const CObjectList& lights = mgr.GetObjectListById(kOL_GameLight);
  CVector3f accumulatedColor = CVector3f::Zero();
  for (int i = lights.GetFirstObjectIndex(); i != -1 && mDynamicLights.size() < mMaxDynamicLights;
       i = lights.GetNextObjectIndex(i)) {
    const CEntity* entity = lights[i];
    if (entity == nullptr || !entity->GetActive()) {
      continue;
    }
    const CGameLight* const gameLight = TCastToConstPtr< CGameLight >(entity);
    const CExplosion* explosion =
        TCastToConstPtr< CExplosion >(mgr.GetObjectById(gameLight->GetParentId()));
    if (explosion == nullptr) {
      continue;
    }

    const CLight& originalLight = gameLight->GetLight();
    CLight reflectedLight(originalLight);
    const float constant = reflectedLight.GetAttenuationConstant() *
                           gpTweakGui->GetFaceReflectionLightFalloffMultConstant();
    const float linear = reflectedLight.GetAttenuationLinear() *
                         gpTweakGui->GetFaceReflectionLightFalloffMultLinear();
    const float quadratic = reflectedLight.GetAttenuationQuadratic() *
                            gpTweakGui->GetFaceReflectionLightFalloffMultQuadratic();
    reflectedLight.SetAttenuation(constant, linear, quadratic);

    CVector3f cameraToExplosion = cameraTransform.TransposeMultiply(explosion->GetTranslation());
    if (CVector3f::Dot(CVector3f::Forward(), cameraToExplosion) < 0.f) {
      continue;
    }
    cameraToExplosion[kDY] =
        -cameraToExplosion[kDY] + CTweakGui::FaceReflectionDistanceDebugValueToActualValue(
                                      gpTweakGui->GetFaceReflectionDistanceDebugValue());
    cameraToExplosion[kDZ] =
        -cameraToExplosion[kDZ] + CTweakGui::FaceReflectionHeightDebugValueToActualValue(
                                      gpTweakGui->GetFaceReflectionHeightDebugValue());
    reflectedLight.SetPosition(cameraTransform * cameraToExplosion);

    if (CollisionUtil::AABoxSphereIntersection(
            aabb, CSphere(originalLight.GetPosition(), originalLight.GetRadius()))) {
      accumulatedColor +=
          reflectedLight.GetNormalIndependentLightingAtPoint(cameraTransform.GetTranslation());
      if (originalLight.GetIntensity() > FLT_EPSILON && originalLight.GetRadius() > FLT_EPSILON) {
        mDynamicLights.push_back(reflectedLight);
      }
    }
  }

  const float grayscale = 0.3f * accumulatedColor.GetX() + 0.6f * accumulatedColor.GetY() +
                          0.1f * accumulatedColor.GetZ();
  if (grayscale < 0.012f) {
    mDynamicLights.clear();
  }
  if (grayscale > 0.03f) {
    const float attenuation = 1.f / (0.03f / grayscale);
    for (rstl::reserved_vector< CLight, 4 >::iterator it = mDynamicLights.begin();
         it != mDynamicLights.end(); ++it) {
      it->SetAttenuation(it->GetAttenuationConstant() * attenuation,
                         it->GetAttenuationLinear() * attenuation,
                         it->GetAttenuationQuadratic() * attenuation);
    }
  }
}

void CActorLights::BuildFakeLightList(const rstl::vector< CLight >& lights, const CColor& color) {
  BuildConstantAmbientLighting(color);
  mAreaLights.clear();
  mDynamicLights.clear();

  for (int i = 0; i < 4; ++i) {
    if (i == lights.size()) {
      break;
    }
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
