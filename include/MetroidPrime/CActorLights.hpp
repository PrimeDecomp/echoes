#ifndef _CACTORLIGHTS
#define _CACTORLIGHTS

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
  static const float kDefaultMinPosChange;

  CActorLights(const uint areaUpdateFramePeriod, CVector3f lightingPositionOffset,
               const int maxDynamicLights, const int maxAreaLights,
               float positionUpdateThreshold = kDefaultMinPosChange,
               const bool ambientChannelOverflow = false, const bool useLightSet2 = false,
               const bool disableWorldLights = false, const bool disableAmbientLights = false);
  ~CActorLights();

  void BuildConstantAmbientLighting(const CColor& color);
  bool BuildAreaLightList(const CStateManager& mgr, const CGameArea& area, const CAABox& bounds);
  void BuildDynamicLightList(const CStateManager& mgr, const CAABox& bounds);
  void BuildFakeLightList(const rstl::vector< CLight >& lights, const CColor& color);
  void BuildFaceLightList(const CStateManager& mgr, const CGameArea& area, const CAABox& aabb,
                          int playerIndex);

  void ActivateLights() const;
  uint GetActiveLightCount() const;
  uint GetActiveAreaLightCount() const { return mAreaLights.size(); }
  const CLight& GetLight(uint idx) const;

  bool GetNeedsRelight() const { return mDirty; }
  bool HasShadowLight() const { return mShadowLightArrIdx != kInvalidShadowLightIndex; }
  int GetShadowLightIndex() const { return mShadowLightIdx; }
  int GetShadowLightArrayIndex() const { return mShadowLightArrIdx; }
  const CColor& GetAmbientColor() const { return mAmbientColor; }
  uint GetFramesBetweenRecalculation() const { return mAreaUpdateFramePeriod; }
  int GetMaxAreaLights() const { return mMaxAreaLights; }
  void SetMaxAreaLights(int v) {
    mMaxAreaLights = v;
    mHasAreaLights = mMaxAreaLights > 0;
  }
  void SetMaxDynamicLights(int v) { mMaxDynamicLights = v; }
  void SetInArea(bool v) { mInArea = v; }
  void SetFindNearestDynamicLights(bool v) { mFindNearestDynamicLights = v; }
  void SetExcludeSpecialDynamicLights(bool v) { mExcludeSpecialDynamicLights = v; }

  void SetCastShadows(bool v) { mCastShadows = v; }
  void SetAmbientColor(const CColor& color) { mAmbientColor = color; }
  void SetFindShadowLight(bool v) { mFindShadowLight = v; }
  void SetFramesBetweenRecalculation(uint frames) { mAreaUpdateFramePeriod = frames; }
  void SetAmbienceGenerated(bool generated) { mAmbienceGenerated = generated; }
  void SetShadowDynamicRangeThreshold(float t) { mShadowDynamicRangeThreshold = t; }
  void SetWorldLightingLevel(float level) { mWorldLightingLevel = level; }
  // Guessed name; native registration reserves the final available slot.
  void AddExplicitLightId(const TUniqueId& id) {
    rstl::reserved_vector< TUniqueId, 4 >& ids = mExplicitLightIds;
    if (ids.size() + 1 < ids.capacity()) {
      ids.push_back(id);
    }
  }
  void SetNeedsRelight(bool v) { mDirty = v; }

private:
  rstl::reserved_vector< CLight, 4 > mAreaLights;
  rstl::reserved_vector< CLight, 4 > mDynamicLights;
  // Guessed name: explicit light objects considered before the manager's dynamic list.
  rstl::reserved_vector< TUniqueId, 4 > mExplicitLightIds;
  CColor mAmbientColor;
  // Guessed name.
  CColor mDynamicAmbientColor;
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
  bool mFindNearestDynamicLights : 1;
  // Guessed names.
  bool mDisableAmbientLights : 1;
  bool mExcludeSpecialDynamicLights : 1;
  int mShadowLightArrIdx;
  int mShadowLightIdx;
  uint mLastUpdateFrame;
  uint mAreaUpdateFramePeriod;
  CVector3f mLightingPositionOffset;
  short mMaxAreaLights;
  short mMaxDynamicLights;
  CVector3f mLastActorPos;
  float mActorPositionDeltaUpdateThreshold;
  float mShadowDynamicRangeThreshold;
  float mWorldLightingLevel;
  int mBrightLightIdx;
  uint mBrightLightLag;

  void UpdateBrightLight();
  void MultiplyLightingLevels(float level);
  void MoveAmbienceToLights(const CVector3f& color);
  void AddOverflowToLights(const CLight& light, const CVector3f& color, float mag);
  static void MergeOverflowLight(CLight& out, CVector3f& color, const CLight& in, float mag);
  // Guessed name: rejects a light entity when it is disabled for the selected light layer.
  bool IsLightExcluded(const CStateManager& mgr, TUniqueId id) const;

  static const uint kInvalidShadowLightIndex;
  static int sFrameSchedulerCount;
};
CHECK_SIZEOF(CActorLights, 0x2e4)

#endif // _CACTORLIGHTS
