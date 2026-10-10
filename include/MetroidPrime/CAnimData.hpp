#ifndef _CANIMDATA
#define _CANIMDATA

#include "Kyoto/Math/CAABox.hpp"
#include "rstl/optional_object.hpp"
#include "types.h"

#include "Kyoto/Animation/CAdditiveAnimPlayback.hpp"
#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CCECharacterInfo.hpp"
#include "Kyoto/Animation/CHierarchyPoseBuilder.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CPoseAsTransforms_Linear.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CParticleDatabase.hpp"

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/set.hpp"

class CAnimationManager;
class CAnimSysContext;
class CAnimTreeNode;
class IMetaTrans;
class CCharacterFactory;
class CCharLayoutInfo;
class CJointData_LinearStorage;
class CRandom16;
class CPASAnimParmData;
class CParticleGenInfo;
class CModel;
class CSkinRules;
struct CAdvancementDeltas;
struct SAdvancementResults;
class CSkinnedModel;
class CTransitionManager;
class CModelFlags;
class CPrimitive;
class IMetaTrans;
class CSpatialPrimitive; // Guessed name: CSPP resource, inherited Ghidra annotation.

class CAnimData {
public:
  enum EAnimDir {
    kAD_Forward,
    kAD_Backward,
  };

  CAnimData(CAssetId selfId, const CCECharacterInfo& charInfo, int defaultAnim, int charIdx,
            bool loop, const TLockedToken< CCharLayoutInfo >& layoutData,
            const TToken< CSkinnedModel >& modelData,
            const rstl::optional_object< TLockedToken< CSkinnedModel > >& iceModelData,
            const rstl::optional_object< TLockedToken< CSpatialPrimitive > >& spatialPrimitive,
            const rstl::ncrc_ptr< CAnimSysContext >& animCtx,
            const rstl::rc_ptr< CAnimationManager >& animMgr,
            const rstl::rc_ptr< CTransitionManager >& transMgr,
            const TLockedToken< CCharacterFactory >& charFactory, bool animatedScale);
  ~CAnimData();

  CAABox GetBoundingBox() const;
  CAABox GetBoundingBox(const CTransform4f& xf) const;
  CAABox CalcBoundingBoxFromModelVerts() const;
  CSegId GetLocatorSegId(const rstl::string& name) const;
  CTransform4f GetLocatorTransform(const rstl::string& name, const CCharAnimTime* time) const;
  CTransform4f GetLocatorTransform(CSegId id, const CCharAnimTime* time) const;
  void ResetPOILists();
  float GetAverageVelocity(int anim) const;
  void AdvanceParticles(const CTransform4f& xf, float dt, const CVector3f& scale,
                        CStateManager* mgr);
  void DrawSkinnedModel(const CSkinnedModel& model, const CModelFlags& flags) const;
  void SetSkinnedModel(const TLockedToken< CSkinnedModel >& model);
  void SetXRayModel(const TLockedToken< CModel >& model, const TLockedToken< CSkinRules >& skin);
  void SetInfraModel(const TLockedToken< CModel >& model, const TLockedToken< CSkinRules >& skin);
  // Guessed names.
  void CollectAnimationTokens(rstl::vector< CToken >& tokensOut, bool lock) const;
  void CollectAnimationResources(rstl::vector< SObjectTag >& tagsOut) const;

  CAdvancementDeltas Advance(float dt, float particleDistance, const CVector3f& scale,
                             CStateManager* mgr, CRandom16& random, TAreaId areaId,
                             bool advanceTree);
  CAdvancementDeltas AdvanceIgnoreParticles(float dt, CRandom16& random, bool advanceTree);
  CAdvancementDeltas DoAdvance(float dt, bool& suspendEffects, CRandom16& random, bool advanceTree);
  void AdvanceAnim(CCharAnimTime& time, CVector3f& offset, CQuaternion& rotation);
  void SetAnimation(const CAnimPlaybackParms& parms, bool noTrans);
  void GetAnimationPrimitives(const CAnimPlaybackParms& parms,
                              rstl::set< CPrimitive >& primsOut) const;

  const CCharLayoutInfo* GetCharLayoutInfo() const { return *mLayoutData; }
  // Guessed name; the optional CSPP collision primitive of the character.
  const rstl::optional_object< TLockedToken< CSpatialPrimitive > >& GetSpatialPrimitive() const {
    return mSpatialPrimitive;
  }
  CPoseAsTransforms_Linear& Pose() { return mPose; }             // Guessed name.
  const CPoseAsTransforms_Linear& Pose() const { return mPose; } // Guessed name.
  void SetPoseBuilt(bool built) { mPoseBuilt = built; }          // Guessed name.
  CHierarchyPoseBuilder& PoseBuilder() const { return mPoseBuilder; }
  const CHierarchyPoseBuilder& GetPoseBuilder() const { return mPoseBuilder; }

  void BuildPoseIfNecessary() const;
  void BuildPose() const;
  void PreRender();
  void SetupRender() const;
  void Render(const CSkinnedModel& model, const CModelFlags& flags) const;
  void RenderAuxiliary(const CFrustumPlanes& planes) const;
  void RecalcPoseBuilder(const CCharAnimTime* time) const;
  float GetAnimationDuration(int anim) const;
  float GetAnimTimeRemaining(const rstl::string& name) const;
  bool IsAnimTimeRemaining(float tolerance, const rstl::string& name) const;
  void CalcPlaybackAlignmentParms(const CAnimPlaybackParms& parms,
                                  const rstl::ncrc_ptr< CAnimTreeNode >& tree);
  void SetRandomPlaybackRate(CRandom16& random);
  void SetPlaybackRate(float rate);
  void MultiplyPlaybackRate(float scale);
  CCharAnimTime GetTimeOfUserEvent(EUserEventType type, const CCharAnimTime& time) const;
  CCharAnimTime GetTimeOfUserEventForAnimation(int anim, EUserEventType type) const;
  CCharAnimTime GetTimeOfUserEvent(EUserEventType type, const CCharAnimTime& time,
                                   const rstl::ncrc_ptr< CAnimTreeNode >& tree) const;
  // Guessed names.
  int CountUserEventsForAnimation(int anim, EUserEventType type) const;
  static void Touch(const CSkinnedModel& model, int shaderIdx);
  static void Touch(const CSkinnedModel& model); // Guessed name; touches every material set.
  int CountUserEvents(EUserEventType type, const CCharAnimTime& time,
                      const rstl::ncrc_ptr< CAnimTreeNode >& tree) const;

  void InitializeEffects(CStateManager& mgr, TAreaId areaId, const CVector3f& scale);
  CParticleGenInfo* GetFirstParticleEffect(const rstl::string& name);
  void SetEffectState(const rstl::string& name, bool active, CStateManager& mgr);
  void SetEffectComponentExternalParam(const rstl::string& name, int index, float value);
  void SetKeepJSPose(bool keep);
  void AddAnimatedScale();
  bool HasAnimatedScale() const { return mAnimatedScale; } // Guessed name
  void SetModelScale(const CVector3f& scale); // Guessed name.
  void SetAnimationTreeLimit(int limit);      // Guessed name.
  void SetPhase(float phase);
  void AddAdditiveAnimation(uint idx, float weight, bool active, bool fadeOut);
  void DelAdditiveAnimation(uint idx);
  void DelAdditiveAnimationImmediately(uint idx);
  bool IsAdditiveAnimation(uint idx) const;
  bool IsAdditiveAnimationActive(uint idx) const;
  float GetAdditiveAnimationWeight(uint idx);
  rstl::rc_ptr< CAnimTreeNode > GetAdditiveAnimationTree(uint idx) const;
  const rstl::ncrc_ptr< CAnimTreeNode >& GetAnimationTree() const;
  CAdvancementDeltas UpdateAdditiveAnims(float dt);
  CAdvancementDeltas AdvanceAdditiveAnims(float dt);
  static SAdvancementResults AdvanceAdditiveAnim(rstl::rc_ptr< CAnimTreeNode >& tree,
                                                 CCharAnimTime time);
  void AddAdditiveSegData(CJointData_LinearStorage& data) const;

  rstl::rc_ptr< CAnimationManager > GetAnimationManager();
  rstl::rc_ptr< CAnimationManager > GetAnimationManager() const;
  rstl::ncrc_ptr< CAnimSysContext > GetAnimSysContext() const;

  // Guessed name.
  int FindBestAnimation(const CPASAnimParmData& parms) const;

  void EnableLooping(bool loop) {
    mLoop = loop;
    mAnimating = true;
  }
  bool GetIsLoop() const { return mLoop; }
  void SetIsAnimating(bool animating) { mAnimating = animating; }
  bool IsAnimating() const { return mAnimating; }
  void SetAnimDir(EAnimDir dir) { mAnimDir = dir; }
  EAnimDir GetAnimDir() const { return mAnimDir; }
  const TLockedToken< CSkinnedModel >& GetModelData() const { return mModelData; }
  const rstl::optional_object< TLockedToken< CSkinnedModel > >& GetIceModel() const {
    return mIceModelData;
  }
  CSkinnedModel* GetXRayModel() const { return mXrayModel.GetPtr(); }
  CSkinnedModel* GetInfraModel() const { return mInfraModel.GetPtr(); }
  int GetCharacterIndex() const { return mCharIdx; }
  short GetCurrentAnimation() const { return mCurrentAnim; }
  float GetPlaybackRate() const { return mSpeedScale; }
  const CCECharacterInfo& GetCharacterInfo() const { return mCharInfo; }
  const CPASDatabase& GetPASDatabase() const { return mCharInfo.GetPASDatabase(); }
  CParticleDatabase& GetParticleDB() { return mParticleDB; }
  // Guessed names; gun turrets drive a joint rotation directly and rebuild the pose.
  CJointData_LinearStorage& JointData() const { return *mJointData; }
  void BuildPose(const CJointData_LinearStorage& data) const {
    mPose.BuildPose(**mLayoutData, data);
  }
  const CParticleDatabase& GetParticleDB() const { return mParticleDB; }
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

  static void InitializeCache();
  static void FreeCache();

  // Guessed name. Returns the transition that would be used to start the requested animation.
  rstl::rc_ptr< IMetaTrans > BuildMetaTransition(const CAnimPlaybackParms& parms) const;

private:
  // Guessed names.
  rstl::ncrc_ptr< CAnimTreeNode > BuildAnimationTree(const CAnimPlaybackParms& parms) const;

  TLockedToken< CCharacterFactory > mCharFactory;
  CCECharacterInfo mCharInfo;
  TLockedToken< CCharLayoutInfo > mLayoutData;
  TLockedToken< CSkinnedModel > mModelData;
  rstl::optional_object< TLockedToken< CSkinnedModel > > mIceModelData;
  rstl::optional_object< TLockedToken< CSpatialPrimitive > > mSpatialPrimitive;
  rstl::rc_ptr< CSkinnedModel > mXrayModel;
  rstl::rc_ptr< CSkinnedModel > mInfraModel;
  rstl::ncrc_ptr< CAnimSysContext > mAnimCtx;
  rstl::rc_ptr< CAnimationManager > mAnimMgr;
  EAnimDir mAnimDir;
  CAABox mAabb;
  CParticleDatabase mParticleDB;
  CAssetId mSelfId;
  CVector3f mAlignPos;
  CQuaternion mAlignRot;
  rstl::ncrc_ptr< CAnimTreeNode > mAnimRoot;
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
  int mAnimationTreeLimit;
  uchar mAnimating : 1;
  uchar mLoop : 1;
  uchar mAligningPos : 1;
  uchar mAligningRot : 1;
  uchar mAlignPosPrimed : 1;
  uchar mAnimationJustStarted : 1;
  mutable uchar mPoseBuilt : 1;
  uchar mAnimatedScale : 1;
  bool mUniformScale : 1;
  bool mUseFastSlerp : 1;
  mutable CPoseAsTransforms_Linear mPose;
  mutable CHierarchyPoseBuilder mPoseBuilder;
  mutable rstl::auto_ptr< CJointData_LinearStorage > mJointData;
  CAnimPlaybackParms mPlaybackParms;
  rstl::reserved_vector< rstl::pair< uint, CAdditiveAnimPlayback >, 8 > mAdditiveAnims;
  mutable uint mCachedBoundsAnimId;
  mutable CAABox mCachedAnimBounds;

  static rstl::reserved_vector< CBoolPOINode, 8 > mBoolPOINodes;
  static rstl::reserved_vector< CInt32POINode, 16 > mInt32POINodes;
  static rstl::reserved_vector< CParticlePOINode, 64 > mParticlePOINodes;
  static rstl::reserved_vector< CSoundPOINode, 48 > mSoundPOINodes;
};
CHECK_SIZEOF(CAnimData, 0x5b8)

#endif // _CANIMDATA
