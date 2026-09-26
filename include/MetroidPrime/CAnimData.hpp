#ifndef _CANIMDATA
#define _CANIMDATA

#include "Kyoto/Math/CAABox.hpp"
#include "rstl/optional_object.hpp"
#include "types.h"

#include "Kyoto/Animation/CAdditiveAnimPlayback.hpp"
#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CCharacterInfo.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CPoseAsTransforms_Linear.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CHierarchyPoseBuilder.hpp"
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
class CSkinnedModelWithAvgNormals;
class CTransitionManager;
class CModelFlags;
class CPrimitive;
class CSpatialPrimitive; // Guessed name: CSPP resource, inherited Ghidra annotation.

class CAnimData {
public:
  enum EAnimDir {
    kAD_Forward,
    kAD_Backward,
  };

  CAnimData(
      CAssetId selfId, const CCharacterInfo& charInfo, int defaultAnim, int charIdx, bool loop,
      const TLockedToken< CCharLayoutInfo >& layoutData, const TToken< CSkinnedModel >& modelData,
      const rstl::optional_object< TLockedToken< CSkinnedModelWithAvgNormals > >& iceModelData,
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

  CAdvancementDeltas AdvanceIgnoreParticles(float dt, CRandom16& random, bool advanceTree);
  CAdvancementDeltas DoAdvance(float dt, bool& suspendEffects, CRandom16& random, bool advanceTree);
  void AdvanceAnim(CCharAnimTime& time, CVector3f& offset, CQuaternion& rotation);
  void SetAnimation(const CAnimPlaybackParms& parms, bool noTrans);
  void GetAnimationPrimitives(const CAnimPlaybackParms& parms,
                              rstl::set< CPrimitive >& primsOut) const;

  void BuildPoseIfNecessary() const;
  void BuildPose() const;
  void PreRender();
  void SetupRender() const;
  void Render(const CSkinnedModel& model, const CModelFlags& flags) const;
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

  void InitializeEffects(CStateManager& mgr, TAreaId areaId, const CVector3f& scale);
  void SetEffectComponentExternalParam(const rstl::string& name, int index, float value);
  void SetKeepJSPose(bool keep);
  void AddAnimatedScale();
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
  int GetCharacterIndex() const { return mCharIdx; }
  short GetCurrentAnimation() const { return mCurrentAnim; }
  float GetPlaybackRate() const { return mSpeedScale; }
  const CCharacterInfo& GetCharacterInfo() const { return mCharInfo; }
  const CPASDatabase& GetPASDatabase() const { return mCharInfo.GetPASDatabase(); }
  CParticleDatabase& GetParticleDB() { return mParticleDB; }
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

private:
  TLockedToken< CCharacterFactory > mCharFactory;
  CCharacterInfo mCharInfo;
  TLockedToken< CCharLayoutInfo > mLayoutData;
  TLockedToken< CSkinnedModel > mModelData;
  rstl::optional_object< TLockedToken< CSkinnedModelWithAvgNormals > > mIceModelData;
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
  int x2a8_;
  uchar mAnimating : 1;
  uchar mLoop : 1;
  uchar mAligningPos : 1;
  uchar x2ac_27_ : 1;
  uchar x2ac_28_ : 1;
  uchar mAnimationJustStarted : 1;
  mutable uchar mPoseBuilt : 1;
  uchar mAnimatedScale : 1;
  uchar mUniformScale : 1;
  uchar x2ad_25_ : 1;
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
