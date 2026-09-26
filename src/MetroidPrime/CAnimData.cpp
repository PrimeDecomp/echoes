#include "MetroidPrime/CAnimData.hpp"

#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CAnimationManager.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#include "Kyoto/Animation/CTransitionManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CModelData.hpp"

typedef rstl::reserved_vector< rstl::pair< uint, CAdditiveAnimPlayback >, 8 > TAdditiveAnims;

rstl::reserved_vector< CBoolPOINode, 8 > CAnimData::mBoolPOINodes;
rstl::reserved_vector< CInt32POINode, 16 > CAnimData::mInt32POINodes;
rstl::reserved_vector< CParticlePOINode, 64 > CAnimData::mParticlePOINodes;
rstl::reserved_vector< CSoundPOINode, 48 > CAnimData::mSoundPOINodes;
static rstl::reserved_vector< CInt32POINode, 16 > sInt32TransientCache;
static CInt32POINode* sInt32TransientCacheData;
static int sPOICacheReferenceCount;

CAnimData::CAnimData(
    CAssetId selfId, const CCharacterInfo& charInfo, int defaultAnim, int charIdx, bool loop,
    const TLockedToken< CCharLayoutInfo >& layoutData, const TToken< CSkinnedModel >& modelData,
    const rstl::optional_object< TLockedToken< CSkinnedModelWithAvgNormals > >& iceModelData,
    const rstl::optional_object< TLockedToken< CSpatialPrimitive > >& spatialPrimitive,
    const rstl::ncrc_ptr< CAnimSysContext >& animCtx,
    const rstl::rc_ptr< CAnimationManager >& animMgr,
    const rstl::rc_ptr< CTransitionManager >& transMgr,
    const TLockedToken< CCharacterFactory >& charFactory, bool animatedScale)
: mCharFactory(charFactory)
, mCharInfo(charInfo)
, mLayoutData(layoutData)
, mModelData(modelData)
, mIceModelData(iceModelData)
, mSpatialPrimitive(spatialPrimitive)
, mXrayModel(nullptr)
, mInfraModel(nullptr)
, mAnimCtx(animCtx)
, mAnimMgr(animMgr)
, mAnimDir(kAD_Forward)
, mAabb(CAABox::MakeMaxInvertedBox())
, mParticleDB()
, mSelfId(selfId)
, mAlignPos(CVector3f::Zero())
, mAlignRot(CQuaternion::NoRotation())
, mAnimRoot()
, mTransMgr(transMgr)
, mSpeedScale(1.f)
, mCharIdx(charIdx)
, mCurrentAnim(defaultAnim)
, mPassedBoolCount(0)
, mPassedIntCount(0)
, mPassedParticleCount(0)
, mPassedSoundCount(0)
, mParticleLightIdx(0)
, x2a8_(8)
, mAnimating(false)
, mLoop(loop)
, mAligningPos(false)
, x2ac_27_(false)
, x2ac_28_(false)
, mAnimationJustStarted(false)
, mPoseBuilt(false)
, mAnimatedScale(animatedScale)
, mUniformScale(false)
, x2ad_25_(true)
, mPose(layoutData->GetBodyPartSegIds().GetCount(), animatedScale ? 1 : 0, 0)
, mPoseBuilder(CLayoutDescription(layoutData), animatedScale)
, mJointData()
, mPlaybackParms(-1, -1, 1.f, true)
, mAdditiveAnims()
, mCachedBoundsAnimId(-1)
, mCachedAnimBounds(CAABox::MakeMaxInvertedBox()) {
  if (sPOICacheReferenceCount == 0) {
    mBoolPOINodes.resize(8, CBoolPOINode(0xffffffff, kPT_EmptyBool, CCharAnimTime(0.f), -1, false,
                                         1.f, -1, 0, false));
    mInt32POINodes.resize(16, CInt32POINode(0xffffffff, kPT_EmptyInt32, CCharAnimTime(0.f), -1,
                                            false, 1.f, -1, 0, 0, rstl::string_l("root")));
    mParticlePOINodes.resize(64, CParticlePOINode(0xffffffff, kPT_Particle, CCharAnimTime(0.f), -1,
                                                  false, 1.f, -1, 0,
                                                  CParticleData(0, SObjectTag(0, 0), CSegId(0), 1.f,
                                                                CParticleData::kPM_Initial)));
    mSoundPOINodes.resize(48, CSoundPOINode(0xffffffff, kPT_Sound, CCharAnimTime(0.f), -1, false,
                                            1.f, -1, 0, 0, 0.f, 0.f, CSegId(0), 0, 0, 0.f));
  }
  ++sPOICacheReferenceCount;

  mAabb = mModelData->GetModel()->GetAABB();
  mParticleDB.CacheParticleDesc(charInfo.GetParticleResData());
  // TODO: Build mAnimRoot from the character-mapped defaultAnim with no special orders.
}

CAnimData::~CAnimData() {
  if (--sPOICacheReferenceCount == 0) {
    mBoolPOINodes.clear();
    mInt32POINodes.clear();
    mParticlePOINodes.clear();
    mSoundPOINodes.clear();
  }
}

CAABox CAnimData::GetBoundingBox() const {
  // TODO: Select the uint-keyed bounds using the best unblended animation child.
  return mAabb;
}

CAABox CAnimData::GetBoundingBox(const CTransform4f& xf) const {
  return GetBoundingBox().GetTransformedAABox(xf);
}

CAABox CAnimData::CalcBoundingBoxFromModelVerts() const {
  // TODO: Accumulate the model vertices after applying the reference pose.
  return mAabb;
}

void CAnimData::ResetPOILists() {
  mPassedBoolCount = 0;
  mPassedIntCount = 0;
  mPassedParticleCount = 0;
  mPassedSoundCount = 0;
}

float CAnimData::GetAverageVelocity(int anim) const {
  // TODO: Weight primitive velocities by their animation durations.
  return 0.f;
}

// Guessed name.
void CAnimData::CollectAnimationResources(rstl::vector< SObjectTag >& tagsOut) const {
  // TODO: Collect unique ANIM resource tags from every character animation's primitives.
}

// Guessed name.
void CAnimData::CollectAnimationTokens(rstl::vector< CToken >& tokensOut, bool lock) const {
  // TODO: Fetch the collected animation resources from the simple pool and optionally lock.
}

void CAnimData::AdvanceParticles(const CTransform4f& xf, float dt, const CVector3f& scale,
                                 CStateManager* mgr) {
  mParticleDB.Update(dt, *this, **mLayoutData, xf, scale, mgr);
}

void CAnimData::DrawSkinnedModel(const CSkinnedModel& model, const CModelFlags& flags) const {
  // TODO: Set lighting/debug render state and draw with the linear pose.
}

void CAnimData::InitializeCache() {
  sInt32TransientCache.clear();
  sInt32TransientCache.resize(16, CInt32POINode(0xffffffff, kPT_EmptyInt32, CCharAnimTime(0.f), -1,
                                                false, 1.f, -1, 0, 0, rstl::string_l("root")));
  sInt32TransientCacheData = sInt32TransientCache.data();
}

void CAnimData::FreeCache() {
  sInt32TransientCache.clear();
  sInt32TransientCacheData = nullptr;
}

void CAnimData::SetSkinnedModel(const TLockedToken< CSkinnedModel >& model) {
  mModelData = model;
  mAabb = mModelData->GetModel()->GetAABB();
}

void CAnimData::SetXRayModel(const TLockedToken< CModel >& model,
                             const TLockedToken< CSkinRules >& skin) {
  mXrayModel =
      rstl::rc_ptr< CSkinnedModel >(rs_new CSkinnedModel(model, skin, mModelData->GetLayoutInfo()));
}

void CAnimData::SetInfraModel(const TLockedToken< CModel >& model,
                              const TLockedToken< CSkinRules >& skin) {
  mInfraModel =
      rstl::rc_ptr< CSkinnedModel >(rs_new CSkinnedModel(model, skin, mModelData->GetLayoutInfo()));
}

void CAnimData::AdvanceAnim(CCharAnimTime& time, CVector3f& offset, CQuaternion& rotation) {
  // TODO: Advance/simplify the root and apply the resulting position and rotation deltas.
}

CAdvancementDeltas CAnimData::AdvanceIgnoreParticles(float dt, CRandom16& random,
                                                     bool advanceTree) {
  bool suspendEffects = false;
  return DoAdvance(dt, suspendEffects, random, advanceTree);
}

CAdvancementDeltas CAnimData::Advance(float dt, float minParticleWeight, const CVector3f& scale,
                                      CStateManager* mgr, CRandom16& random, TAreaId areaId,
                                      bool advanceTree) {
  // TODO: Advance, suspend effects when requested, and emit eligible particle POIs.
  return CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation());
}

CAdvancementDeltas CAnimData::DoAdvance(float dt, bool& suspendEffects, CRandom16& random,
                                        bool advanceTree) {
  // TODO: Advance the animation tree, process POIs and combine additive deltas.
  return CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation());
}

// Guessed name.
rstl::ncrc_ptr< CAnimTreeNode >
CAnimData::BuildAnimationTree(const CAnimPlaybackParms& parms) const {
  // TODO: Build the requested animation or a blend of the two requested animations.
  return rstl::ncrc_ptr< CAnimTreeNode >();
}

// Guessed name.
rstl::ncrc_ptr< CAnimTreeNode >
CAnimData::BuildTransitionTree(const CAnimPlaybackParms& parms) const {
  // TODO: Build a transition from the current root to BuildAnimationTree(parms).
  return rstl::ncrc_ptr< CAnimTreeNode >();
}

void CAnimData::SetAnimation(const CAnimPlaybackParms& parms, bool noTrans) {
  // TODO: Construct the new tree/transition, reset POIs and set playback alignment.
}

void CAnimData::GetAnimationPrimitives(const CAnimPlaybackParms& parms,
                                       rstl::set< CPrimitive >& primsOut) const {
  // TODO: Collect unique primitives from the requested animation(s).
}

void CAnimData::BuildPoseIfNecessary() const {
  if (!mPoseBuilt) {
    RecalcPoseBuilder(nullptr);
    mPoseBuilt = true;
  }
}

void CAnimData::BuildPose() const { BuildPoseIfNecessary(); }

void CAnimData::PreRender() { BuildPoseIfNecessary(); }

void CAnimData::SetupRender() const { BuildPoseIfNecessary(); }

void CAnimData::Render(const CSkinnedModel& model, const CModelFlags& flags) const {
  SetupRender();
  DrawSkinnedModel(model, flags);
}

void CAnimData::RecalcPoseBuilder(const CCharAnimTime* time) const {
  // TODO: Sample the root into joint storage, add additive segments and build the linear pose.
  // The inherited IAnimReader virtual interface must be recovered before dispatching here.
}

rstl::ncrc_ptr< CAnimSysContext > CAnimData::GetAnimSysContext() const { return mAnimCtx; }

float CAnimData::GetAnimationDuration(int anim) const {
  // TODO: Query the selected animation tree's steady-state duration.
  return 0.f;
}

float CAnimData::GetAnimTimeRemaining(const rstl::string& name) const {
  // TODO: Query the root's remaining time using the recovered animation-tree interface.
  return 0.f;
}

bool CAnimData::IsAnimTimeRemaining(float tolerance, const rstl::string& name) const {
  // TODO: Recover the zero-time and tolerance tests against the root's remaining time.
  return false;
}

CTransform4f CAnimData::GetLocatorTransform(const rstl::string& name,
                                            const CCharAnimTime* time) const {
  return GetLocatorTransform(mLayoutData->GetSegIdFromString(name), time);
}

CTransform4f CAnimData::GetLocatorTransform(CSegId id, const CCharAnimTime* time) const {
  if (id == CSegId::Invalid()) {
    return CTransform4f::Identity();
  }
  if (time != nullptr || !mPoseBuilt) {
    RecalcPoseBuilder(time);
    mPoseBuilt = time == nullptr;
  }
  return CTransform4f(mPose.GetRotation(id), mPose.GetOffset(id));
}

void CAnimData::CalcPlaybackAlignmentParms(const CAnimPlaybackParms& parms,
                                           const rstl::ncrc_ptr< CAnimTreeNode >& tree) {
  // TODO: Recover alignment events and the locator-relative position/rotation adjustments.
}

void CAnimData::SetRandomPlaybackRate(CRandom16& random) {
  // TODO: Read the random-rate POI and choose the signed playback-rate variation.
}

void CAnimData::SetPlaybackRate(float rate) { mSpeedScale = rate; }

void CAnimData::MultiplyPlaybackRate(float scale) { mSpeedScale *= scale; }

CCharAnimTime CAnimData::GetTimeOfUserEventForAnimation(int anim, EUserEventType type) const {
  // TODO: Build the selected animation tree and query its user-event time.
  return CCharAnimTime::Infinity();
}

CCharAnimTime CAnimData::GetTimeOfUserEvent(EUserEventType type, const CCharAnimTime& time) const {
  return GetTimeOfUserEvent(type, time, mAnimRoot);
}

CCharAnimTime CAnimData::GetTimeOfUserEvent(EUserEventType type, const CCharAnimTime& time,
                                            const rstl::ncrc_ptr< CAnimTreeNode >& tree) const {
  // TODO: Search the supplied tree's int POIs and reset the transient cache afterward.
  return CCharAnimTime::Infinity();
}

// Guessed name.
int CAnimData::CountUserEvents(EUserEventType type, const CCharAnimTime& time,
                               const rstl::ncrc_ptr< CAnimTreeNode >& tree) const {
  // TODO: Count matching int POIs and reset the transient cache afterward.
  return 0;
}

rstl::rc_ptr< CAnimationManager > CAnimData::GetAnimationManager() const { return mAnimMgr; }

// Guessed name.
int CAnimData::CountUserEventsForAnimation(int anim, EUserEventType type) const {
  // TODO: Build the selected animation and count events over its duration.
  return 0;
}

void CAnimData::InitializeEffects(CStateManager& mgr, TAreaId areaId, const CVector3f& scale) {
  const CCharacterInfo::TEffectList& effects = mCharInfo.GetEffects();
  for (uint i = 0; i < effects.size(); ++i) {
    const rstl::vector< CEffectComponent >& components = effects[i].second;
    for (uint j = 0; j < components.size(); ++j) {
      const CEffectComponent& component = components[j];
      mParticleDB.CacheParticleDesc(component.GetParticleTag());
      const CParticleData data(0, component.GetParticleTag(), component.GetSegmentId(),
                               component.GetScale(), component.GetParentedMode());
      mParticleDB.AddParticleEffect(component.GetComponentNameHash(), component.GetFlags(), data,
                                    scale, &mgr, areaId, true, mParticleLightIdx);
      mParticleDB.SetParticleEffectState(component.GetComponentNameHash(), false, &mgr);
    }
  }
}

CParticleGenInfo* CAnimData::GetFirstParticleEffect(const rstl::string& name) {
  const CCharacterInfo::TEffectList& effects = mCharInfo.GetEffects();
  for (uint i = 0; i < effects.size(); ++i) {
    if (effects[i].first == name) {
      const rstl::vector< CEffectComponent >& components = effects[i].second;
      return components.empty()
                 ? nullptr
                 : mParticleDB.GetParticleEffect(components[0].GetComponentNameHash());
    }
  }
  return nullptr;
}

void CAnimData::SetEffectState(const rstl::string& name, bool active, CStateManager& mgr) {
  const CCharacterInfo::TEffectList& effects = mCharInfo.GetEffects();
  for (uint i = 0; i < effects.size(); ++i) {
    if (effects[i].first == name) {
      const rstl::vector< CEffectComponent >& components = effects[i].second;
      for (uint j = 0; j < components.size(); ++j) {
        mParticleDB.SetParticleEffectState(components[j].GetComponentNameHash(), active, &mgr);
      }
      return;
    }
  }
}

void CAnimData::SetEffectComponentExternalParam(const rstl::string& name, int index, float value) {
  const CCharacterInfo::TEffectList& effects = mCharInfo.GetEffects();
  for (uint i = 0; i < effects.size(); ++i) {
    if (effects[i].first == name) {
      const rstl::vector< CEffectComponent >& components = effects[i].second;
      if (!components.empty()) {
        mParticleDB.SetParticleExternalParam(components[0].GetComponentNameHash(), index, value);
      }
      return;
    }
  }
}

void CAnimData::SetPhase(float phase) {
  // TODO: Forward the phase to the root's virtual interface once its slots are recovered.
}

void CAnimData::SetKeepJSPose(bool keep) {
  if (!keep) {
    mJointData = rstl::auto_ptr< CJointData_LinearStorage >();
  } else if (mJointData.null()) {
    mJointData = rstl::auto_ptr< CJointData_LinearStorage >(rs_new CJointData_LinearStorage(
        mLayoutData->GetBodyPartSegIds().GetCount(), CJointData_LinearStorage::kAF_Heap));
    mJointData->SetZeroRotation();
    mJointData->ResetScales();
    mJointData->SetReferenceOffsets(**mLayoutData);
  }
}

// Guessed name.
void CAnimData::SetAnimationTreeLimit(int limit) { x2a8_ = limit; }

rstl::rc_ptr< CAnimationManager > CAnimData::GetAnimationManager() { return mAnimMgr; }

void CAnimData::AddAdditiveAnimation(uint idx, float weight, bool active, bool fadeOut) {
  // TODO: Create or update the character-mapped additive animation and its fade parameters.
}

void CAnimData::DelAdditiveAnimation(uint idx) {
  const uint anim = mCharInfo.GetAnimationIndexList()[idx];
  for (TAdditiveAnims::iterator it = mAdditiveAnims.begin(); it != mAdditiveAnims.end(); ++it) {
    if (it->first == anim) {
      const CAdditiveAnimPlayback::EPlaybackPhase phase = it->second.GetFadingMode();
      if (phase != CAdditiveAnimPlayback::kPP_FadingOut &&
          phase != CAdditiveAnimPlayback::kPP_FadedOut) {
        it->second.FadeOut();
      }
      return;
    }
  }
}

void CAnimData::DelAdditiveAnimationImmediately(uint idx) {
  const uint anim = mCharInfo.GetAnimationIndexList()[idx];
  for (TAdditiveAnims::iterator it = mAdditiveAnims.begin(); it != mAdditiveAnims.end(); ++it) {
    if (it->first == anim) {
      mAdditiveAnims.erase(it);
      return;
    }
  }
}

float CAnimData::GetAdditiveAnimationWeight(uint idx) {
  const uint anim = mCharInfo.GetAnimationIndexList()[idx];
  for (TAdditiveAnims::const_iterator it = mAdditiveAnims.begin(); it != mAdditiveAnims.end();
       ++it) {
    if (it->first == anim) {
      return it->second.GetWeight();
    }
  }
  return 0.f;
}

bool CAnimData::IsAdditiveAnimationActive(uint idx) const {
  const uint anim = mCharInfo.GetAnimationIndexList()[idx];
  for (TAdditiveAnims::const_iterator it = mAdditiveAnims.begin(); it != mAdditiveAnims.end();
       ++it) {
    if (it->first == anim) {
      return true;
    }
  }
  return false;
}

rstl::rc_ptr< CAnimTreeNode > CAnimData::GetAdditiveAnimationTree(uint idx) const {
  const uint anim = mCharInfo.GetAnimationIndexList()[idx];
  for (TAdditiveAnims::const_iterator it = mAdditiveAnims.begin(); it != mAdditiveAnims.end();
       ++it) {
    if (it->first == anim) {
      return it->second.GetAnimationTree();
    }
  }
  return rstl::rc_ptr< CAnimTreeNode >(nullptr);
}

const rstl::ncrc_ptr< CAnimTreeNode >& CAnimData::GetAnimationTree() const { return mAnimRoot; }

bool CAnimData::IsAdditiveAnimation(uint idx) const {
  // TODO: Search the animation database's additive-animation information.
  return false;
}

SAdvancementResults CAnimData::AdvanceAdditiveAnim(rstl::rc_ptr< CAnimTreeNode >& tree,
                                                   CCharAnimTime time) {
  // TODO: Advance and simplify the additive tree, preserving its unconsumed time.
  SAdvancementResults result;
  result.mRemTime = time;
  result.mDeltas.mPosDelta = CVector3f::Zero();
  result.mDeltas.mRotDelta = CQuaternion::NoRotation();
  return result;
}

CAdvancementDeltas CAnimData::UpdateAdditiveAnims(float dt) {
  // TODO: Update fades, remove finished entries and combine their weighted deltas.
  return CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation());
}

CAdvancementDeltas CAnimData::AdvanceAdditiveAnims(float dt) {
  // TODO: Advance active additive trees and accumulate their motion.
  return CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation());
}

void CAnimData::AddAdditiveSegData(CJointData_LinearStorage& data) const {
  // TODO: Accumulate weighted additive rotations, translations and scales into joint storage.
}

// Guessed name.
int CAnimData::FindBestAnimation(const CPASAnimParmData& parms) const {
  return GetPASDatabase().FindBestAnimation(parms, -1).second;
}

// Guessed name.
void CAnimData::SetModelScale(const CVector3f& scale) {
  mUniformScale =
      close_enough(scale.GetX(), scale.GetY()) && close_enough(scale.GetX(), scale.GetZ());
  mPose.SetUniformScale(mUniformScale);
}

void CAnimData::AddAnimatedScale() {
  mAnimatedScale = true;
  mPose.AllocateScale();
}
