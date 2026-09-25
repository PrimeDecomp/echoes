#ifndef _CANIMDATA
#define _CANIMDATA

// TODO: check for Echoes

#include "Kyoto/Math/CAABox.hpp"
#include "rstl/optional_object.hpp"
#include "types.h"

#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CCharacterInfo.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "Kyoto/Animation/CAdditiveAnimPlayback.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CHierarchyPoseBuilder.hpp"
#include "MetroidPrime/CParticleDatabase.hpp"
#include "MetroidPrime/CPoseAsTransforms.hpp"

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/set.hpp"

class CAnimationManager;
class CAnimSysContext;
class CAnimTreeNode;
class CCharacterFactory;
class CCharLayoutInfo;
class CSkinnedModel;
class CSkinnedModelWithAvgNormals;
class CTransitionManager;
class CVertexMorphEffect;
class CModelFlags;
class CPrimitive;

class CAnimData {
public:
  ~CAnimData();

  enum EAnimDir {
    kAD_Forward,
    kAD_Backward,
  };

  void PreRender();
  void EnableLooping(bool v) {
    mLoop = v;
    mAnimating = true;
  }

  const TLockedToken< CSkinnedModel >& GetModelData() const { return mModelData; }

  void SetIsAnimating(bool v) { mAnimating = v; }
  void SetParticleEffectState(const rstl::string& name, const bool active, CStateManager& mgr);

  int GetCharacterIndex() const { return mCharIdx; }
  float GetAverageVelocity(int idx) const;

  const CBoolPOINode* GetBoolPOIList(int& count) const {
    count = mPassedBoolCount;
    return mBoolPOINodes.data();
  }
  const CInt32POINode* GetInt32POIList(int& count) const {
    count = mPassedIntCount;
    return mInt32POINodes.data();
  }
  const CParticlePOINode* GetParticlePOIList(int& count) const {
    count = mPassedParticleCount;
    return mParticlePOINodes.data();
  }
  const CSoundPOINode* GetSoundPOIList(int& count) const {
    count = mPassedSoundCount;
    return mSoundPOINodes.data();
  }
  CParticleDatabase& GetParticleDB() { return mParticleDB; }
  const CParticleDatabase& GetParticleDB() const { return mParticleDB; }
  // SetIsAnimating__9CAnimDataFb
  // SetAnimDir__9CAnimDataFQ29CAnimData8EAnimDir
  CAABox GetBoundingBox() const;
  // GetBoundingBox__9CAnimDataCFRC12CTransform4f
  // GetLocatorSegId__9CAnimDataCFRCQ24rstl66basic_string
  // ResetPOILists__9CAnimDataFv
  // GetAverageVelocity__9CAnimDataCFi
  // AdvanceParticles__9CAnimDataFRC12CTransform4ffRC9CVector3fR13CStateManager
  // PoseSkinnedModel__9CAnimDataCFRC13CSkinnedModelRC17CPoseAsTransformsRCQ24rstl37optional_object<18CVertexMorphEffect>PCf
  // DrawSkinnedModel__9CAnimDataCFRC13CSkinnedModelRC11CModelFlags
  // InitializeCache__9CAnimDataFv
  // FreeCache__9CAnimDataFv
  // SetInfraModel__9CAnimDataFRC21TLockedToken<6CModel>RC26TLockedToken<10CSkinRules>
  // SetXRayModel__9CAnimDataFRC21TLockedToken<6CModel>RC26TLockedToken<10CSkinRules>
  // AdvanceAnim__9CAnimDataFR13CCharAnimTimeR9CVector3fR11CQuaternion
  // AdvanceIgnoreParticles__9CAnimDataFfR9CRandom16b
  // Advance__9CAnimDataFfRC9CVector3fR13CStateManagerb
  // DoAdvance__9CAnimDataFfRbR9CRandom16b
  void SetAnimation(const CAnimPlaybackParms& parms, bool noTrans);
  void GetAnimationPrimitives(const CAnimPlaybackParms& parms,
                              rstl::set< CPrimitive >& primsOut) const;
  // PrimitiveSetToTokenVector__9CAnimDataFRCQ24rstl72set<10CPrimitive,Q24rstl18less<10CPrimitive>,Q24rstl17rmemory_allocator>RQ24rstl42vector<6CToken,Q24rstl17rmemory_allocator>b
  // BuildPose__9CAnimDataFv
  // PreRender__9CAnimDataFv
  // SetupRender__9CAnimDataCFRC13CSkinnedModelRCQ24rstl37optional_object<18CVertexMorphEffect>PCf
  // Render__9CAnimDataCFRC13CSkinnedModelRC11CModelFlagsRCQ24rstl37optional_object<18CVertexMorphEffect>PCf
  void Render(const CSkinnedModel&, const CModelFlags&,
              const rstl::optional_object< CVertexMorphEffect >&, const float*) const;
  // RenderAuxiliary__9CAnimDataCFRC14CFrustumPlanes
  // RecalcPoseBuilder__9CAnimDataCFPC13CCharAnimTime
  float GetAnimationDuration(int animIn) const;
  float GetAnimTimeRemaining(const rstl::string& name) const;
  // IsAnimTimeRemaining__9CAnimDataCFfRCQ24rstl66basic_string<c,Q24rstl14char_traits<c>,Q24rstl17rmemory_allocator>
  bool IsAnimTimeRemaining(float, const rstl::string&) const;
  // GetLocatorTransform__9CAnimDataCFRCQ24rstl66basic_string<c,Q24rstl14char_traits<c>,Q24rstl17rmemory_allocator>PC13CCharAnimTime
  // GetLocatorTransform__9CAnimDataCF6CSegIdPC13CCharAnimTime
  // CalcPlaybackAlignmentParms__9CAnimDataFRC18CAnimPlaybackParmsRCQ24rstl25ncrc_ptr<13CAnimTreeNode>
  // SetRandomPlaybackRate__9CAnimDataFR9CRandom16
  void SetPlaybackRate(float set);
  void MultiplyPlaybackRate(float scale);
  CCharAnimTime GetTimeOfUserEvent(EUserEventType type, const CCharAnimTime& time) const;
  // GetAdvancementDeltas__9CAnimDataCFRC13CCharAnimTimeRC13CCharAnimTime
  // Touch__9CAnimDataCFRC13CSkinnedModeli
  void InitializeEffects(CStateManager&, TAreaId, const CVector3f&);
  // SetPhase__9CAnimDataFf -> SetPhase__11IAnimReaderFf
  void SetPhase(float ph);
  void AddAdditiveAnimation(uint idx, float weight, bool active, bool fadeOut);
  void DelAdditiveAnimation(uint idx);
  bool IsAdditiveAnimation(uint idx) const;
  const rstl::rc_ptr< CAnimTreeNode >& GetAdditiveAnimationTree(uint idx) const;
  // GetAnimationTree__9CAnimDataCFv
  // AnimationTree__9CAnimDataFv
  // IsAdditiveAnimation__9CAnimDataCFUi
  bool IsAdditiveAnimationAdded(uint idx) const;
  // UpdateAdditiveAnims__9CAnimDataFf
  // AdvanceAdditiveAnims__9CAnimDataFf
  // AddAdditiveSegData__9CAnimDataCFRC10CSegIdListR16CSegStatementSet
  int GetEventResourceIdForAnimResourceId(int id) const;
  // GetAnimationManager__9CAnimDataFv
  // SetPoseValid__9CAnimDataFb

  float GetAdditiveAnimationWeight(uint idx);

  short GetCurrentAnimation() const { return mCurrentAnim; }
  const CCharacterInfo& GetCharacterInfo() const { return mCharInfo; }
  // GetCharLayoutInfo__9CAnimDataCFv
  // GetDeltaRotation__9CAnimDataCFv
  // GetDeltaOffset__9CAnimDataCFv
  // IsDeltaOffsetInUse__9CAnimDataCFv
  // GetAdvancementDeltas__19CAdvancementResultsCFv
  // SetDeltaRotation__9CAnimDataFRC11CQuaternionb
  // SetDeltaOffset__9CAnimDataFRC9CVector3fb
  // SetDeltaOffsetInUse__9CAnimDataFv
  // IsDeltaRotationInUse__9CAnimDataCFv
  // IsDeltaOffsetPrimed__9CAnimDataCFv
  // GetAnimDir__9CAnimDataCFv
  // GetIsLoop__9CAnimDataCFv
  // IsAnimating__9CAnimDataCFv
  // SetPoseBuilderValid__9CAnimDataFb
  // GetAnimationManager__9CAnimDataCFv
  // GetPoseValid__9CAnimDataCFv
  // GetPoseBuilderValid__9CAnimDataCFv
  // GetAnimSysContext__9CAnimDataCFv
  // CacheInt32PoiList__9CAnimDataFRC13CCharAnimTimeiRCQ24rstl25ncrc_ptr<13CAnimTreeNode>

  // GetIceModel__9CAnimDataCFv
  const CPASDatabase& GetPASDatabase() const { return mCharInfo.GetPASDatabase(); }
  // EnableLooping__9CAnimDataFb
  // GetSkinnedModel__9CAnimDataCFv
  // GetXRayModel__9CAnimDataCFv
  // GetInfraModel__9CAnimDataCFv
  // GetPose__9CAnimDataCFv
  // PoseBuilder__9CAnimDataCFv
  // GetPlaybackRate__9CAnimDataCFv
  // Pose__9CAnimDataFv
  // GetPoseBuilder__9CAnimDataCFv

  // CacheSoundPoiList__9CAnimDataFRCQ24rstl25ncrc_ptr<13CAnimTreeNode>RC13CCharAnimTimei
  // CacheParticlePoiList__9CAnimDataFRCQ24rstl25ncrc_ptr<13CAnimTreeNode>RC13CCharAnimTimei
  // CacheBoolPoiList__9CAnimDataFRCQ24rstl25ncrc_ptr<13CAnimTreeNode>RC13CCharAnimTimei
  // CacheInt32PoiList__9CAnimDataFRCQ24rstl25ncrc_ptr<13CAnimTreeNode>RC13CCharAnimTimei

  static void InitializeCache();
  static void FreeCache();

private:
  TLockedToken< CCharacterFactory > mCharFactory;
  CCharacterInfo mCharInfo;
  TLockedToken< CCharLayoutInfo > mLayoutData;
  TLockedToken< CSkinnedModel > mModelData;
  rstl::optional_object< TLockedToken< CSkinnedModelWithAvgNormals > > mIceModelData;
  rstl::rc_ptr< CSkinnedModel > mXrayModel;
  rstl::rc_ptr< CSkinnedModel > mInfraModel;
  rstl::rc_ptr< CAnimSysContext > mAnimCtx;
  rstl::rc_ptr< CAnimationManager > mAnimMgr;
  EAnimDir mAnimDir;
  CAABox mAabb;
  CParticleDatabase mParticleDB; // TODO: should be 0x178
  CAssetId mSelfId;
  CVector3f mAlignPos;
  CQuaternion mAlignRot;
  rstl::rc_ptr< CAnimTreeNode > mAnimRoot;
  rstl::rc_ptr< CTransitionManager > mTransMgr;
  float mSpeedScale;
  int mCharIdx;
  short mCurrentAnim;
  short mPadding;
  int mPassedBoolCount;
  int mPassedIntCount;
  int mPassedParticleCount;
  int mPassedSoundCount;
  int mParticleLightIdx;
  uchar mAnimating : 1;
  uchar mLoop : 1;
  uchar mAligningPos : 1;
  uchar x220_27_ : 1;
  uchar x220_28_ : 1;
  uchar mAnimationJustStarted : 1;
  uchar mPoseBuilt : 1;
  uchar mPoseCached : 1;
  CPoseAsTransforms mPose;
  CHierarchyPoseBuilder mPoseBuilder;
  CAnimPlaybackParms mPlaybackParms;
  rstl::reserved_vector< rstl::pair< int, CAdditiveAnimPlayback >, 8 > mAdditiveAnims;

  static rstl::reserved_vector< CBoolPOINode, 8 > mBoolPOINodes;
  static rstl::reserved_vector< CInt32POINode, 16 > mInt32POINodes;
  static rstl::reserved_vector< CParticlePOINode, 20 > mParticlePOINodes;
  static rstl::reserved_vector< CSoundPOINode, 20 > mSoundPOINodes;
  // in cpp -> rstl::reserved_vector< CInt32POINode, 16 > sInt32TransientCache;
};
// CHECK_SIZEOF(CAnimData, 0x434 + 0x144)

#endif // _CANIMDATA
