#ifndef _CACTORLIGHTS
#define _CACTORLIGHTS

// TODO: look for Echoes differences

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CGameArea;
class CStateManager;
class CAABox;

class CActorLights {
public:
  static const float kDefaultPositionUpdateThreshold;

  CActorLights(const uint areaUpdateFramePeriod, CVector3f lightingPositionOffset,
               const int maxDynamicLights, const int maxAreaLights,
               float positionUpdateThreshold = kDefaultPositionUpdateThreshold,
               const bool ambientChannelOverflow = false, const bool useLightSet2 = false,
               const bool disableWorldLights = false, const bool unk = false);
  ~CActorLights();

  void BuildConstantAmbientLighting();
  void BuildConstantAmbientLighting(const CColor&);
  bool BuildAreaLightList(const CStateManager& mgr, const CGameArea& area, const CAABox& bounds);
  void BuildDynamicLightList(const CStateManager& mgr, const CAABox& bounds);
  void BuildFakeLightList(const rstl::vector< CLight >&, const CColor&);
  void BuildFaceLightList(const CStateManager& mgr, const CGameArea& area, const CAABox& aabb);

  void ActivateLights() const;
  uint GetActiveLightCount() const;

  bool GetNeedsRelight() const { return mDirty == TRUE; }
  bool HasShadowLight() const { return mShadowLightArrIdx != -1; }
  int GetShadowLightIndex() const { return mShadowLightIdx; }

  void SetAmbientColor(const CColor& color);
  void SetCastShadows(bool v) { mCastShadows = v; }
  void SetFindShadowLight(bool v) { mFindShadowLight = v; }
  void SetShadowDynamicRangeThreshold(float t) { mShadowDynamicRangeThreshold = t; }

private:
  rstl::reserved_vector< CLight, 4 > mAreaLights;
  rstl::reserved_vector< CLight, 4 > mDynamicLights;
  CVector3f mAmbientColor;
  TAreaId mAid;
  bool mDirty : 1;
  bool mCastShadows : 1;
  bool mHasAreaLights : 1;
  bool mFindShadowLight : 1;
  bool mInArea : 1;
  bool mAmbienceGenerated : 1;
  bool mLayer2 : 1;
  bool mDisableWorldLights : 1;
  bool mInBrightLight : 1;
  bool mUseBrightLightLag : 1;
  bool mAmbientOnly : 1;
  bool mFindNearestDynamicLights;
  int mShadowLightArrIdx;
  int mShadowLightIdx;
  uint mLastUpdateFrame;
  uint mAreaUpdateFramePeriod;
  CVector3f mLightingPositionOffset;
  int mMaxAreaLights;
  int mMaxDynamicLights;
  CVector3f mLastActorPos;
  float mActorPositionDeltaUpdateThreshold;
  float mShadowDynamicRangeThreshold;
  float mWorldLightingLevel;
  int mBrightLightIdx;
  uint mBrightLightLag;

  int prime2addition; // TODO: figure out where this is
};
CHECK_SIZEOF(CActorLights, 0x2e4)

#endif // _CACTORLIGHTS
