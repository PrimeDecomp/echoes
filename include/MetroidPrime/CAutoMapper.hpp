#ifndef _CAUTOMAPPER
#define _CAUTOMAPPER

#include "types.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CInGameGuiManagerCommon.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CFinalInput;
class CGuiFrame;
class CGuiTextPane;
class CGuiWidget;
class CMapUniverse;
class CMapWorld;
class CMapWorldInfo;
class CModel;
class CStateManager;
class CStringTable;
class CTexture;
class CUnitVector3f;
class IWorld;

class CAutoMapper {
public:
  enum ELoadPhase { kLP_LoadResources, kLP_LoadUniverse, kLP_Done };
  enum EAutoMapperState { kAMS_MiniMap, kAMS_MapScreen, kAMS_MapScreenUniverse };
  enum EZoomState { kZS_None, kZS_In, kZS_Out };

  // Guessed names
  enum EMapMode { kMM_Normal, kMM_Teleport };
  enum ETransitionState {
    kTS_Idle,
    kTS_LeaveViaUniverse,
    kTS_ReturnToPlayer,
    kTS_MiniMapReady,
    kTS_SwitchToLightWorld,
    kTS_SwitchToDarkWorld,
  };

  struct SAutoMapperRenderState {
    enum Ease { kE_None, kE_Linear, kE_Out, kE_In, kE_InOut };

    CVector2i mViewportSize;
    CQuaternion mCamOrientation;
    float mCamDist;
    float mCamAngle;
    CVector3f mAreaPoint;
    float mDrawDepth1;
    float mDrawDepth2;
    float mAlphaSurfaceVisited;
    float mAlphaOutlineVisited;
    float mAlphaSurfaceUnvisited;
    float mAlphaOutlineUnvisited;
    Ease mViewportEase;
    Ease mCamEase;
    Ease mPointEase;
    Ease mDepth1Ease;
    Ease mDepth2Ease;
    Ease mAlphaEase;

    SAutoMapperRenderState(const CVector2i& viewportSize, const CQuaternion& camOrientation,
                           float camDist, float camAngle, const CVector3f& areaPoint,
                           float drawDepth1, float drawDepth2, float alphaSurfaceVisited,
                           float alphaOutlineVisited, float alphaSurfaceUnvisited,
                           float alphaOutlineUnvisited);
    SAutoMapperRenderState(const SAutoMapperRenderState& other);
    void ResetInterpolation();
    static void InterpolateWithClamp(const SAutoMapperRenderState& a, SAutoMapperRenderState& out,
                                     const SAutoMapperRenderState& b, float t);
  };

  struct SAutoMapperHintStep {
    enum Type {
      kHST_PanToArea,
      kHST_PanToWorld,
      kHST_SwitchToUniverse,
      kHST_SwitchToWorld,
      kHST_ShowBeacon,
      kHST_ZoomIn,
      kHST_ZoomOut,
      kHST_SwitchLightDarkWorld, // Guessed name
      kHST_LeaveMapScreen,       // Guessed name
    };

    union Data {
      CAssetId mWorldId;
      int mAreaId;
      float mFloat;

      Data(int value) : mAreaId(value) {}
      Data(float value) : mFloat(value) {}
    };

    Type mType;
    Data mData;
    bool mProcessing;

    SAutoMapperHintStep(Type type, int data) : mType(type), mData(data), mProcessing(false) {}
    SAutoMapperHintStep(Type type, const float& data)
    : mType(type), mData(data), mProcessing(false) {}
  };

  struct SAutoMapperHintLocation {
    int mShowBeacon;
    float mBeaconAlpha;
    CAssetId mWorldId;
    TAreaId mAreaId;

    SAutoMapperHintLocation(uint showBeacon, float beaconAlpha, CAssetId worldId, int areaId);
  };

  CAutoMapper(const CStateManager& mgr, int playerIndex);
  virtual ~CAutoMapper();

  bool CheckLoadComplete();
  bool CanLeaveMapScreen(const CStateManager& mgr) const;
  void SetCurWorldAssetId(int worldId);
  void UnmuteAllLoopedSounds();
  void ProcessControllerInput(const CFinalInput& input, CStateManager& mgr);
  void Update(float dt, CStateManager& mgr);
  void Draw(const CStateManager& mgr, const CTransform4f& xf, float alpha) const;
  void OnNewInGameGuiState(EInGameGuiState state, CStateManager& mgr);
  void BeginMapperStateTransition(EAutoMapperState state, CStateManager& mgr);
  void CompleteMapperStateTransition(CStateManager& mgr);
  void ResetInterpolationTimer(float duration);
  bool IsInMapperState(EAutoMapperState state) const;
  bool IsInMapperStateTransition() const;
  bool IsRenderStateInterpolating() const;

  float GetMapRotationX() const { return mRenderState0.mCamAngle; }
  TAreaId GetFocusAreaIndex() const { return mCurAreaId; }
  float GetTimeIntoInterpolation() const { return mInterpTime; }
  EAutoMapperState GetCurrentState() const { return mState; }
  EAutoMapperState GetNextState() const { return mNextState; }
  bool IsInPlayerControlState() const {
    return IsInMapperState(kAMS_MapScreen) || IsInMapperState(kAMS_MapScreenUniverse);
  }
  bool IsFullyInMiniMapState() const { return IsInMapperState(kAMS_MiniMap); }
  bool IsFullyOutOfMiniMapState() const {
    return mState != kAMS_MiniMap && mNextState != kAMS_MiniMap;
  }
  float GetInterp() const { return mInterpDur > 0.f ? mInterpTime / mInterpDur : 0.f; }

private:
  void SetCurAreaId(int areaId); // Guessed name
  void UpdateTempleKeys(const CStateManager& mgr);
  void SetupMiniMapWorld(CStateManager& mgr);
  void LeaveMapScreen(CStateManager& mgr);
  bool TryLeaveMapScreen(CStateManager& mgr);         // Guessed name
  bool SwitchLightDarkWorld();                        // Guessed name
  bool CanSwitchLightDarkWorld() const;               // Guessed name
  int FindTeleportArea(const CMapWorld& world) const; // Guessed name
  void SetupTeleportNavigation();                     // Guessed name
  void SetupHintNavigation();
  void UpdateHintNavigation(float dt, CStateManager& mgr);
  bool NotHintNavigating(const CStateManager& mgr) const;
  bool CanLeaveMapScreenInternal(const CStateManager& mgr) const;
  bool HasCurrentMapUniverseWorld(const CStateManager& mgr) const;
  bool CheckDummyWorldLoad(CStateManager& mgr);
  void LeaveMapScreenState();
  void SetShouldPanningSoundBePlaying(bool shouldBePlaying);
  void SetShouldZoomingSoundBePlaying(bool shouldBePlaying);
  void SetShouldRotatingSoundBePlaying(bool shouldBePlaying);
  void TransformRenderStatesWorldToUniverse();
  void TransformRenderStatesUniverseToWorld();
  void TransformRenderStateWorldToUniverse(SAutoMapperRenderState& state);
  static float GetMapAreaMiniMapDrawDepth();
  static CVector2i GetMiniMapViewportSize();
  static CVector2i GetMapScreenViewportSize();
  float GetMapAreaMaxDrawDepth(const CStateManager& mgr, int areaId) const;
  float GetMapAreaMiniMapDrawAlphaSurfaceVisited(const CStateManager& mgr) const;
  float GetMapAreaMiniMapDrawAlphaOutlineVisited(const CStateManager& mgr) const;
  float GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(const CStateManager& mgr) const;
  float GetMapAreaMiniMapDrawAlphaOutlineUnvisited(const CStateManager& mgr) const;
  float GetDesiredMiniMapCameraDistance(const CStateManager& mgr) const;
  float GetBaseMapScreenCameraMoveSpeed() const;
  float GetClampedMapScreenCameraDistance(float value) const;
  float GetFinalMapScreenCameraMoveSpeed() const;
  CQuaternion GetMiniMapCameraOrientation(const CStateManager& mgr) const;
  CVector3f GetAreaPointOfInterest(const CStateManager& mgr, int areaId) const;
  SAutoMapperRenderState BuildMiniMapWorldRenderState(const CStateManager& mgr,
                                                      const CQuaternion& rot, int areaId) const;
  SAutoMapperRenderState BuildMapScreenWorldRenderState(const CStateManager& mgr,
                                                        const CQuaternion& rot, int areaId,
                                                        bool doingHint) const;
  SAutoMapperRenderState BuildMapScreenUniverseRenderState(const CStateManager& mgr,
                                                           const CQuaternion& rot,
                                                           int areaId) const;
  CAssetId GetAreaHintDescriptionString(CAssetId areaId);

  template < class T >
  void SetResLockState(T& list, bool lock);

  int mPlayerIndex;
  ELoadPhase mLoadPhase;
  TCachedToken< CMapUniverse > mMapu;
  rstl::vector< rstl::auto_ptr< IWorld > > mDummyWorlds;
  rstl::vector< rstl::auto_ptr< IWorld > > mWorldsPendingUnload; // Guessed name
  IWorld* mWorld;
  rstl::single_ptr< TCachedToken< CGuiFrame > > mFrmeMapScreen;
  CGuiFrame* mFrmeInitialized;
  rstl::single_ptr< TCachedToken< CGuiFrame > > mFrmeMapScreenBackground;
  CGuiFrame* mFrmeBackgroundInitialized;
  TCachedToken< CModel > mMiniMapSamus;
  TCachedToken< CTexture > mHintBeacon;
  TCachedToken< CModel > mCompassModel;
  TCachedToken< CModel > mCompassShellModel;
  rstl::reserved_vector< CToken, 7 > mMapIcons;
  CAssetId mAreaHintDescId;
  rstl::optional_object< TCachedToken< CStringTable > > mAreaHintDesc;
  CAssetId mMapAreaStringId;
  rstl::optional_object< TCachedToken< CStringTable > > mMapAreaString;
  int mWorldIdx;
  TAreaId mCurAreaId;
  TAreaId mOtherAreaId;
  SAutoMapperRenderState mRenderState0;
  SAutoMapperRenderState mRenderState1;
  SAutoMapperRenderState mRenderState2;
  EAutoMapperState mState;
  EAutoMapperState mNextState;
  float mInterpDur;
  float mInterpTime;
  EMapMode mMapMode; // Guessed name
  CSfxHandle mPanningSfx;
  CSfxHandle mRotatingSfx;
  CSfxHandle mZoomingSfx;
  float mFlashTimer;
  float mPlayerFlashPulse;
  rstl::list< SAutoMapperHintStep > mHintSteps;
  rstl::list< SAutoMapperHintLocation > mHintLocations;
  rstl::reserved_vector< CToken, 9 > mLstick;
  rstl::reserved_vector< CToken, 9 > mCstick;
  rstl::reserved_vector< CToken, 2 > mLtrigger;
  rstl::reserved_vector< CToken, 2 > mRtrigger;
  rstl::reserved_vector< CToken, 2 > mAbutton;
  uint mLStickPos;
  uint mRStickPos;
  uint mLTriggerPos;
  uint mRTriggerPos;
  uint mAButtonPos;
  CGuiTextPane* mTextpaneLabel;
  CGuiTextPane* mTextpaneAreaname;
  CGuiTextPane* mTextpaneHint;
  CGuiTextPane* mTextpaneInstructions;
  CGuiTextPane* mTextpaneInstructions1;
  CGuiTextPane* mTextpaneInstructions2;
  CGuiTextPane* mTextpaneLeft;
  CGuiTextPane* mTextpaneRight;
  CGuiTextPane* mTextpaneYicon;
  CGuiTextPane* mTextpaneMapLegend;
  CGuiTextPane* mTextpaneMapLegend1;
  CGuiTextPane* mTextpaneXicon;
  CGuiTextPane* mTextpaneRight3;
  CGuiTextPane* mTextpaneXicon1;
  CGuiTextPane* mTextpaneKeylegend;
  CGuiTextPane* mTextpaneTeleporter;
  CGuiWidget* mBasewidgetLeftPane;
  CGuiWidget* mBasewidgetYButtonPane;
  CGuiWidget* mBasewidgetBottomPane;
  CGuiWidget* mBasewidgetHintgroup;
  rstl::vector< CGuiWidget* > mBackgroundHexagons; // Guessed name
  float mBackgroundAnimationPhase;                 // Guessed name
  float mLeftPanePos;
  float mYButtonPanePos;
  float mBottomPanePos;
  float mDarkWorldBlend; // Guessed name
  EZoomState mZoomState;
  ETransitionState mTransitionState; // Guessed name
  bool mLoadingDummyWorld : 1;
};
NESTED_CHECK_SIZEOF(CAutoMapper, SAutoMapperRenderState, 0x5c)
NESTED_CHECK_SIZEOF(CAutoMapper, SAutoMapperHintStep, 0xc)
NESTED_CHECK_SIZEOF(CAutoMapper, SAutoMapperHintLocation, 0x10)
CHECK_SIZEOF(CAutoMapper, 0x3c0)

#endif // _CAUTOMAPPER
