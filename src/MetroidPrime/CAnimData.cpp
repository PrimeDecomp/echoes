#include "MetroidPrime/CAnimData.hpp"

#include "Kyoto/Animation/CAllFormatsAnimSource.hpp"
#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CAnimTreeBlend.hpp"
#include "Kyoto/Animation/CAnimationManager.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#include "Kyoto/Animation/CPrimitive.hpp"
#include "Kyoto/Animation/CSkinRules.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Animation/CTransitionManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Factories/CCharacterFactory.hpp"

#include "rstl/algorithm.hpp"

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
    const rstl::optional_object< TLockedToken< CSkinnedModel > >& iceModelData,
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
, mAnimationTreeLimit(8)
, mAnimating(false)
, mLoop(loop)
, mAligningPos(false)
, mAligningRot(false)
, mAlignPosPrimed(false)
, mAnimationJustStarted(false)
, mPoseBuilt(false)
, mAnimatedScale(animatedScale)
, mUniformScale(false)
, mUseFastSlerp(true)
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
  mAnimRoot = mAnimMgr->GetAnimationTree(mCharInfo.GetAnimationIndexList()[defaultAnim],
                                         CMetaAnimTreeBuildOrders::NoSpecialOrders());
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
  typedef rstl::vector< rstl::pair< uint, CAABox > > TAnimBounds;
  const TAnimBounds& bounds = mCharInfo.GetAnimBoundsById();
  if (bounds.size() > 0) {
    const CAnimTreeEffectiveContribution contribution =
        mAnimRoot->GetContributionOfHighestInfluence();
    const uint anim = contribution.GetAnimDatabaseIndex();
    if (anim != mCachedBoundsAnimId) {
      TAnimBounds::const_iterator found = rstl::find_by_key(bounds, anim);
      if (found == bounds.end()) {
        mCachedAnimBounds = mAabb;
      } else {
        mCachedAnimBounds = found->second;
      }
    }
    return mCachedAnimBounds;
  }
  return mAabb;
}

CAABox CAnimData::GetBoundingBox(const CTransform4f& xf) const {
  return GetBoundingBox().GetTransformedAABox(xf);
}

CAABox CAnimData::CalcBoundingBoxFromModelVerts() const {
  BuildPoseIfNecessary();
  CVector3f min(1000000.f, 1000000.f, 1000000.f);
  CVector3f max(-1000000.f, -1000000.f, -1000000.f);
  CSkinnedModelState state = mModelData->MakeDefaultStorage();
  mModelData->StoreCalculation(state, &mPose);

  const int count = mModelData->GetSkinRules()->GetNumPoints();
  for (int i = 0; i < count; ++i) {
    const CVector3f point = mModelData->GetSkinnedPosition(state.GetWorkspace(), i);
    if (point.GetX() > max.GetX()) {
      max.SetX(point.GetX());
    } else if (point.GetX() < min.GetX()) {
      min.SetX(point.GetX());
    }
    if (point.GetY() > max.GetY()) {
      max.SetY(point.GetY());
    } else if (point.GetY() < min.GetY()) {
      min.SetY(point.GetY());
    }
    if (point.GetZ() > max.GetZ()) {
      max.SetZ(point.GetZ());
    } else if (point.GetZ() < min.GetZ()) {
      min.SetZ(point.GetZ());
    }
  }
  return CAABox(min, max);
}

void CAnimData::ResetPOILists() {
  mPassedBoolCount = 0;
  mPassedIntCount = 0;
  mPassedParticleCount = 0;
  mPassedSoundCount = 0;
}

float CAnimData::GetAverageVelocity(int animIn) const {
  const uint animRes = mCharInfo.GetAnimationIndexList()[animIn];
  rstl::rc_ptr< IMetaAnim > anim = mAnimMgr->GetMetaAnimation(animRes);

  rstl::set< CPrimitive > primitiveSet;
  anim->GetUniquePrimitives(primitiveSet);

  float ret;
  float weightedVel = 0.f;
  float totalDur = 0.f;
  rstl::set< CPrimitive >::const_iterator it = primitiveSet.begin();
  rstl::set< CPrimitive >::const_iterator end = primitiveSet.end();
  while (it != end) {
    const SObjectTag animTag('ANIM', it->GetAnimResId());

    TLockedToken< CAllFormatsAnimSource > animData = mAnimCtx->GetSimplePool().GetObj(animTag);

    weightedVel += animData->GetAverageVelocity() * animData->GetAnimationDuration().GetSeconds();
    totalDur += animData->GetAnimationDuration().GetSeconds();
    ++it;
  }

  ret = 0.f;
  if (totalDur > 0.f) {
    ret = weightedVel / totalDur;
  }

  return ret;
}

// Guessed name.
void CAnimData::CollectAnimationResources(rstl::vector< SObjectTag >& tagsOut) const {
  rstl::set< SObjectTag > tags;
  const rstl::vector< uint >& animations = mCharInfo.GetAnimationIndexList();
  for (rstl::vector< uint >::const_iterator anim = animations.begin(); anim != animations.end();
       ++anim) {
    rstl::rc_ptr< IMetaAnim > metaAnim = mAnimMgr->GetMetaAnimation(*anim);
    rstl::set< CPrimitive > primitives;
    metaAnim->GetUniquePrimitives(primitives);
    for (rstl::set< CPrimitive >::const_iterator primitive = primitives.begin();
         primitive != primitives.end(); ++primitive) {
      tags.insert(SObjectTag('ANIM', primitive->GetAnimResId()));
    }
  }

  tagsOut.reserve(tagsOut.size() + tags.size());
  tagsOut.insert(tagsOut.end(), tags.begin(), tags.end());
}

// Guessed name.
void CAnimData::CollectAnimationTokens(rstl::vector< CToken >& tokensOut, bool lock) const {
  rstl::vector< SObjectTag > tags;
  CollectAnimationResources(tags);
  if (tags.empty()) {
    return;
  }

  tokensOut.reserve(tokensOut.size() + tags.size());
  for (int i = 0; i < tags.size(); ++i) {
    CToken token = gpSimplePool->GetObj(tags[i]);
    if (lock) {
      token.Lock();
    }
    tokensOut.push_back(token);
  }
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
  const float dt = time.GetSeconds();
  SAdvancementResults results(CCharAnimTime(0.f),
                              CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simplified;

  if (mAnimDir == kAD_Forward) {
    results = mAnimRoot->VAdvanceView(time);
    simplified = mAnimRoot->Simplified();
  }

  if (simplified.valid()) {
    mAnimRoot = Cast(simplified.data());
  }

  if (mAlignPosPrimed || mAligningRot) {
    const int count = mPassedIntCount;
    const CInt32POINode* node = mInt32POINodes.data();
    if (count > 0) {
      for (int i = 0; i < count; ++i, ++node) {
        if (node->GetPoiType() == kPT_UserEvent) {
          switch (node->GetValue()) {
          case kUE_AlignTargetPosStart:
            mAligningPos = true;
            break;
          case kUE_AlignTargetPos:
            mAlignPos = CVector3f::Zero();
            mAlignPosPrimed = false;
            mAligningPos = false;
            break;
          case kUE_AlignTargetRot:
            mAlignRot = CQuaternion::NoRotation();
            mAligningRot = false;
            break;
          }
        }
      }
    }
  }
  const CAdvancementDeltas deltas = results.mDeltas;
  const CVector3f& deltaPos = deltas.GetOffsetDelta();
  const CQuaternion& deltaRot = deltas.GetOrientationDelta();

  offset += deltaPos;
  if (mAligningPos) {
    offset += mAlignPos * dt;
  }

  CQuaternion alignRot = deltaRot * mAlignRot;
  rotation *= alignRot;
  mAlignPos = alignRot.BuildInverted().Transform(mAlignPos);

  time = results.mRemTime;
}

CAdvancementDeltas CAnimData::AdvanceIgnoreParticles(float dt, CRandom16& random,
                                                     bool advanceTree) {
  bool suspendEffects;
  return DoAdvance(dt, suspendEffects, random, advanceTree);
}

CAdvancementDeltas CAnimData::Advance(float dt, float particleDistance, const CVector3f& scale,
                                      CStateManager* mgr, CRandom16& random, TAreaId areaId,
                                      bool advanceTree) {
  bool suspendParticles;
  CAdvancementDeltas deltas = DoAdvance(dt, suspendParticles, random, advanceTree);
  if (suspendParticles) {
    mParticleDB.SuspendAllActiveEffects(mgr);
  }

  const int count = mPassedParticleCount;
  for (int i = 0; i < count; ++i) {
    const CParticlePOINode& node = mParticlePOINodes[i];
    const int charIdx = node.GetCharacterIndex();
    if (charIdx == -1 || charIdx == mCharIdx) {
      if (node.GetMaximumDistance() > particleDistance ||
          (mgr != nullptr && !mgr->IsMultiplayer() &&
           mgr->GetCameraManager(0)->IsInCinematicCamera())) {
        mParticleDB.AddParticleEffect(node.GetNameHash(), node.GetFlags(), node.GetParticleData(),
                                      scale, mgr, areaId, false, mParticleLightIdx);
      }
    }
  }
  return deltas;
}

CAdvancementDeltas CAnimData::DoAdvance(float dt, bool& suspendParticles, CRandom16& random,
                                        bool advanceTree) {
  suspendParticles = false;

  CVector3f offset(0.f, 0.f, 0.f);
  CQuaternion rotation(CQuaternion::NoRotation());

  const float scaledDt = dt * mSpeedScale;
  CVector3f additiveOffset(0.f, 0.f, 0.f);
  CQuaternion additiveRotation(CQuaternion::NoRotation());

  ResetPOILists();

  if (mAdditiveAnims.size() > 0) {
    const CAdvancementDeltas additiveDeltas = UpdateAdditiveAnims(scaledDt);
    additiveOffset = additiveDeltas.GetOffsetDelta();
    additiveRotation = additiveDeltas.GetOrientationDelta();
    mPoseBuilt = false;
  }

  const bool animating = IsAnimating() == true;
  if (!animating) {
    suspendParticles = true;
    return CAdvancementDeltas(offset, rotation);
  }

  if (mAnimationJustStarted) {
    mAnimationJustStarted = false;
    suspendParticles = true;
  }

  if (advanceTree) {
    SetRandomPlaybackRate(random);

    CCharAnimTime time(scaledDt);

    if (mLoop) {
      while (time.GreaterThanZero() && !close_enough(time.GetSeconds(), 0.f)) {
        mPassedIntCount +=
            mAnimRoot->GetInt32POIList(time, mInt32POINodes.data(), 16, mPassedIntCount, 0);
        mPassedBoolCount +=
            mAnimRoot->GetBoolPOIList(time, mBoolPOINodes.data(), 8, mPassedBoolCount, 0);
        mPassedParticleCount += mAnimRoot->GetParticlePOIList(time, mParticlePOINodes.data(), 64,
                                                              mPassedParticleCount, 0);
        mPassedSoundCount +=
            mAnimRoot->GetSoundPOIList(time, mSoundPOINodes.data(), 48, mPassedSoundCount, 0);

        AdvanceAnim(time, offset, rotation);
      }
    } else {
      CCharAnimTime remTime = mAnimRoot->VGetTimeRemaining();

      while (!close_enough(remTime.GetSeconds(), 0.f) && !close_enough(time.GetSeconds(), 0.f)) {
        mPassedIntCount +=
            mAnimRoot->GetInt32POIList(time, mInt32POINodes.data(), 16, mPassedIntCount, 0);
        mPassedBoolCount +=
            mAnimRoot->GetBoolPOIList(time, mBoolPOINodes.data(), 8, mPassedBoolCount, 0);
        mPassedParticleCount += mAnimRoot->GetParticlePOIList(time, mParticlePOINodes.data(), 64,
                                                              mPassedParticleCount, 0);
        mPassedSoundCount +=
            mAnimRoot->GetSoundPOIList(time, mSoundPOINodes.data(), 48, mPassedSoundCount, 0);

        AdvanceAnim(time, offset, rotation);

        remTime = mAnimRoot->VGetTimeRemaining();
        time = CCharAnimTime(
            rstl::max_val(0.f, rstl::min_val(time.GetSeconds(), remTime.GetSeconds())));

        if (close_enough(remTime.GetSeconds(), 0.f)) {
          mAnimating = false;
          mAlignPos = CVector3f::Zero();
          mAlignPosPrimed = false;
          mAligningPos = false;
        }
      }
    }

    mPoseBuilt = false;
  }

  return CAdvancementDeltas(offset + additiveOffset, rotation * additiveRotation);
}

// Guessed name.
rstl::ncrc_ptr< CAnimTreeNode >
CAnimData::BuildAnimationTree(const CAnimPlaybackParms& parms) const {
  const int animB = parms.GetSecondAnimationId();
  const float blendFactor = parms.GetBlendFactor();
  const uint animResA = mCharInfo.GetAnimationIndexList()[parms.GetAnimationId()];
  if (animB != -1) {
    const uint animResB = mCharInfo.GetAnimationIndexList()[animB];
    const rstl::ncrc_ptr< CAnimTreeNode > treeA(GetAnimationManager()->GetAnimationTree(
        animResA, CMetaAnimTreeBuildOrders::NoSpecialOrders()));
    const rstl::ncrc_ptr< CAnimTreeNode > treeB(GetAnimationManager()->GetAnimationTree(
        animResB, CMetaAnimTreeBuildOrders::NoSpecialOrders()));
    return rstl::ncrc_ptr< CAnimTreeNode >(
        rs_new CAnimTreeBlend(false, treeA, treeB, blendFactor,
                              CAnimTreeBlend::CreatePrimitiveName(treeA, treeB, blendFactor)));
  }
  return GetAnimationManager()->GetAnimationTree(animResA,
                                                 CMetaAnimTreeBuildOrders::NoSpecialOrders());
}

// Guessed name.
rstl::rc_ptr< IMetaTrans > CAnimData::BuildMetaTransition(const CAnimPlaybackParms& parms) const {
  const rstl::ncrc_ptr< CAnimTreeNode > tree(BuildAnimationTree(parms));
  return mTransMgr->GetMetaTrans(mAnimRoot, tree);
}

void CAnimData::SetAnimation(const CAnimPlaybackParms& parms, bool noTrans) {
  const uint children = mAnimRoot->VGetNumChildren();
  if (parms.GetAnimationId() == mPlaybackParms.GetAnimationId() ||
      (parms.GetSecondAnimationId() == mPlaybackParms.GetSecondAnimationId() &&
       parms.GetSecondAnimationId() != -1) ||
      (parms.GetBlendFactor() == mPlaybackParms.GetBlendFactor() &&
       parms.GetBlendFactor() != 1.f)) {
    if (mAnimationJustStarted) {
      return;
    }
  }
  if (children < mAnimationTreeLimit) {
    ResetPOILists();
    mSpeedScale = 1.f;
    mPlaybackParms.SetAnimationId(parms.GetAnimationId());
    mPlaybackParms.SetSecondAnimationId(parms.GetSecondAnimationId());
    mPlaybackParms.SetBlendFactor(parms.GetBlendFactor());
    const bool animating = parms.GetIsPlayAnimation();
    mCurrentAnim = parms.GetAnimationId();
    const rstl::ncrc_ptr< CAnimTreeNode > tree(BuildAnimationTree(parms));
    if (!noTrans) {
      mAnimRoot = mTransMgr->GetTransitionTree(mAnimRoot, tree);
    } else {
      mAnimRoot = tree;
    }
    mAnimating = animating;
    CalcPlaybackAlignmentParms(parms, tree);
    ResetPOILists();
    mAnimationJustStarted = true;
  }
}

void CAnimData::GetAnimationPrimitives(const CAnimPlaybackParms& parms,
                                       rstl::set< CPrimitive >& primsOut) const {
  const int animB = parms.GetSecondAnimationId();

  const uint animResA = mCharInfo.GetAnimationIndexList()[parms.GetAnimationId()];
  GetAnimationManager()->GetMetaAnimation(animResA)->GetUniquePrimitives(primsOut);

  if (animB != -1) {
    const uint animResB = mCharInfo.GetAnimationIndexList()[animB];
    GetAnimationManager()->GetMetaAnimation(animResB)->GetUniquePrimitives(primsOut);
  }
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
  CAnimMathUtils::sUseFastSlerp = mUseFastSlerp;
  const CCharLayoutInfo& layout = **mLayoutData;
  rstl::optional_object< CJointData_LinearStorage > temporary;
  CJointData_LinearStorage* data = mJointData.get();
  if (data == nullptr) {
    data = new (temporary.prepare_emplace()) CJointData_LinearStorage(
        layout.GetBodyPartSegIds().GetCount(), CJointData_LinearStorage::kAF_Pool);
  } else {
    data->ResetFlags();
  }
  if (mAnimatedScale) {
    data->SetHasScales(true);
  }
  if (time == nullptr) {
    mAnimRoot->VGetSegData(layout, *data);
  } else {
    mAnimRoot->VGetSegData(layout, *data, *time);
  }
  AddAdditiveSegData(*data);
  mPose.BuildPose(layout, *data);
}

rstl::ncrc_ptr< CAnimSysContext > CAnimData::GetAnimSysContext() const { return mAnimCtx; }

float CAnimData::GetAnimationDuration(int animIn) const {
  const uint animRes = mCharInfo.GetAnimationIndexList()[animIn];
  rstl::rc_ptr< IMetaAnim > anim = GetAnimationManager()->GetMetaAnimation(animRes);

  rstl::set< CPrimitive > primitiveSet;
  anim->GetUniquePrimitives(primitiveSet);

  float duration = 0.f;
  rstl::set< CPrimitive >::const_iterator it = primitiveSet.begin();
  rstl::set< CPrimitive >::const_iterator end = primitiveSet.end();
  while (it != end) {
    const SObjectTag animTag('ANIM', it->GetAnimResId());

    TLockedToken< CAllFormatsAnimSource > animData =
        GetAnimSysContext()->GetSimplePool().GetObj(animTag);

    duration += animData->GetAnimationDuration().GetSeconds();
    ++it;
  }

  if (anim->GetType() == kMAT_Random) {
    duration /= primitiveSet.size();
  }

  return duration;
}

float CAnimData::GetAnimTimeRemaining(const rstl::string&) const {
  float remTime = mAnimRoot->VGetTimeRemaining().GetSeconds();
  if (mSpeedScale > 0.f) {
    remTime /= mSpeedScale;
  }
  return remTime;
}

bool CAnimData::IsAnimTimeRemaining(float rem, const rstl::string&) const {
  if (mAnimRoot.GetPtr() != 0) {
    const float remTime = mAnimRoot->VGetTimeRemaining().GetSeconds();
    return !close_enough(remTime, 0.f, rem);
  }

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

CMatrix3f CMatrix3f::Inverse() const {
  const float detScale = 1.f / Determinant();
  return CMatrix3f((m11 * m22 - m12 * m21) * detScale, (-(m01 * m22 - m02 * m21)) * detScale,
                   (m01 * m12 - m02 * m11) * detScale, (-(m10 * m22 - m12 * m20)) * detScale,
                   (m00 * m22 - m02 * m20) * detScale, (-(m00 * m12 - m02 * m10)) * detScale,
                   (m10 * m21 - m11 * m20) * detScale, (-(m00 * m21 - m01 * m20)) * detScale,
                   (m00 * m11 - m01 * m10) * detScale);
}

void CAnimData::CalcPlaybackAlignmentParms(const CAnimPlaybackParms& parms,
                                           const rstl::ncrc_ptr< CAnimTreeNode >& node) {
  const CQuaternion* deltaOrient = parms.GetDeltaOrient();
  const CTransform4f* objectXf = parms.GetObjectXform();

  CQuaternion alignRot = CQuaternion::NoRotation();
  mAlignRot = alignRot;
  mAligningRot = false;

  if (deltaOrient != nullptr && objectXf != nullptr) {
    ResetPOILists();
    mPassedIntCount += node->GetInt32POIList(CCharAnimTime::Infinity(), mInt32POINodes.data(), 16,
                                             mPassedIntCount, 64);

    const int count = mPassedIntCount;
    if (count > 0) {
      for (int i = 0; i < count; ++i) {
        const CInt32POINode* poi = &mInt32POINodes[i];
        if (poi->GetPoiType() == kPT_UserEvent && poi->GetValue() == kUE_AlignTargetRot) {
          const CCharAnimTime& poiTime = poi->GetTime();
          const SAdvancementResults adv =
              node->VGetAdvancementResults(poiTime, CCharAnimTime::ZeroFlat());
          const CMatrix3f invObjRot = objectXf->BuildMatrix3f().Inverse();
          const CQuaternion targetRot = (*deltaOrient) * CQuaternion::FromMatrix(invObjRot);
          const CQuaternion fullRot = targetRot * adv.mDeltas.GetOrientationDelta().BuildInverted();

          alignRot = CQuaternion::Slerp(CQuaternion::NoRotation(), fullRot,
                                        1.f / (60.f * poiTime.GetSeconds()));
          mAlignRot = alignRot;
          mAligningRot = true;
          break;
        }
      }
    }
  }

  if (!mAligningRot) {
    const CVector3f* targetPos = parms.GetTargetPos();
    bool foundStart = false;
    bool foundAlign = false;
    CVector3f startPos = CVector3f::Zero();
    CVector3f alignPos = CVector3f::Zero();
    CCharAnimTime startTime = CCharAnimTime::ZeroPlus();
    CCharAnimTime alignTime = CCharAnimTime::ZeroPlus();

    if (targetPos != nullptr && objectXf != nullptr) {
      ResetPOILists();
      mPassedIntCount += node->GetInt32POIList(CCharAnimTime::Infinity(), mInt32POINodes.data(), 16,
                                               mPassedIntCount, 64);

      const int count = mPassedIntCount;
      if (count > 0) {
        for (int i = 0; i < count; ++i) {
          const CInt32POINode* poi = &mInt32POINodes[i];
          if (poi->GetPoiType() == kPT_UserEvent) {
            const rstl::string& locator = poi->GetLocatorName();
            if (poi->GetValue() == kUE_AlignTargetPosStart) {
              startTime = poi->GetTime();
              foundStart = true;

              const SAdvancementResults adv =
                  node->VGetAdvancementResults(startTime, CCharAnimTime::ZeroFlat());
              startPos = adv.mDeltas.GetOffsetDelta();

              if (parms.GetIsUseLocator()) {
                const CTransform4f xf = GetLocatorTransform(locator, &startTime);
                startPos += xf.GetTranslation();
              }

              if (foundAlign) {
                break;
              }
            } else if (poi->GetValue() == kUE_AlignTargetPos) {
              alignTime = poi->GetTime();
              foundAlign = true;

              const SAdvancementResults adv =
                  node->VGetAdvancementResults(alignTime, CCharAnimTime::ZeroFlat());
              alignPos = adv.mDeltas.GetOffsetDelta();

              if (parms.GetIsUseLocator()) {
                const CTransform4f xf = GetLocatorTransform(locator, &alignTime);
                alignPos += xf.GetTranslation();
              }

              if (foundStart) {
                break;
              }
            }
          }
        }

        if (foundStart && foundAlign) {
          const CVector3f* const objectScale = parms.GetObjectScale();

          const CVector3f scaleStart = CVector3f::ByElementMultiply(*objectScale, startPos);
          const CVector3f scaleAlign = CVector3f::ByElementMultiply(*objectScale, alignPos);
          const CVector3f delta =
              objectXf->GetInverse() * (*targetPos) - scaleStart - (scaleAlign - scaleStart);
          CVector3f normalized = delta;
          normalized[kDX] /= (*objectScale)[kDX];
          normalized[kDY] /= (*objectScale)[kDY];
          normalized[kDZ] /= (*objectScale)[kDZ];

          const float timeScale = 1.f / (alignTime.GetSeconds() - startTime.GetSeconds());
          normalized *= timeScale;
          mAlignPos = normalized;
          mAlignPosPrimed = true;
          mAligningPos = false;
        } else {
          mAlignPos = CVector3f::Zero();
          mAlignPosPrimed = false;
          mAligningPos = false;
        }
      }
    } else {
      mAlignPos = CVector3f::Zero();
      mAlignPosPrimed = false;
      mAligningPos = false;
    }
  } else {
    const CVector3f* targetPos = parms.GetTargetPos();
    bool foundStart = false;
    bool foundAlign = false;
    CVector3f startPos = CVector3f::Zero();
    CCharAnimTime startTime = CCharAnimTime::ZeroPlus();
    CCharAnimTime alignTime = CCharAnimTime::ZeroPlus();

    if (targetPos != nullptr && objectXf != nullptr) {
      ResetPOILists();
      mPassedIntCount += node->GetInt32POIList(CCharAnimTime::Infinity(), mInt32POINodes.data(), 16,
                                               mPassedIntCount, 64);

      const int count = mPassedIntCount;
      if (count > 0) {
        for (int i = 0; i < count; ++i) {
          const CInt32POINode* poi = &mInt32POINodes[i];
          if (poi->GetPoiType() == kPT_UserEvent) {
            if (poi->GetValue() == kUE_AlignTargetPosStart) {
              startTime = poi->GetTime();
              foundStart = true;
              if (foundAlign) {
                break;
              }
            } else if (poi->GetValue() == kUE_AlignTargetPos) {
              alignTime = poi->GetTime();
              foundAlign = true;
              if (foundStart) {
                break;
              }
            }
          }
        }

        if (foundStart && foundAlign) {
          alignRot = CQuaternion::NoRotation();
          mAlignRot = alignRot;
          mAligningRot = true;

          foundStart = false;
          CCharAnimTime time = CCharAnimTime::ZeroFlat();
          CVector3f alignPos = CVector3f::Zero();
          const CCharAnimTime frameDt(1.f / 60.f);
          CQuaternion curRot = CQuaternion::NoRotation();

          while (time < alignTime) {
            const SAdvancementResults adv = node->VGetAdvancementResults(frameDt, time);
            alignPos += curRot.BuildTransform() * adv.mDeltas.GetOffsetDelta();
            curRot *= adv.mDeltas.GetOrientationDelta() * alignRot;

            if (!foundStart && time >= startTime) {
              foundStart = true;
              startPos = alignPos;
            }

            time += frameDt;
          }

          const CVector3f* const objectScale = parms.GetObjectScale();

          const CVector3f scaleStart = CVector3f::ByElementMultiply(*objectScale, startPos);
          const CVector3f scaleAlign = CVector3f::ByElementMultiply(*objectScale, alignPos);
          const CVector3f delta =
              objectXf->GetInverse() * (*targetPos) - scaleStart - (scaleAlign - scaleStart);
          CVector3f normalized = delta;
          normalized[kDX] /= (*objectScale)[kDX];
          normalized[kDY] /= (*objectScale)[kDY];
          normalized[kDZ] /= (*objectScale)[kDZ];

          const float timeScale = 1.f / (alignTime.GetSeconds() - startTime.GetSeconds());
          normalized *= timeScale;
          mAlignPos = normalized;
          mAlignPosPrimed = true;
          mAligningPos = false;
        } else {
          mAlignPos = CVector3f::Zero();
          mAlignPosPrimed = false;
          mAligningPos = false;
        }
      }
    } else {
      mAlignPos = CVector3f::Zero();
      mAlignPosPrimed = false;
      mAligningPos = false;
    }
  }
}

void CAnimData::SetRandomPlaybackRate(CRandom16& random) {
  for (int i = 0; i < mPassedIntCount; ++i) {
    const CInt32POINode& poi = mInt32POINodes[i];
    if (poi.GetPoiType() == kPT_RandRate) {
      const float scale = static_cast< float >(random.Next() % poi.GetValue()) / 100.f;
      if ((random.Next() % 100) < 50) {
        mSpeedScale = 1.f + scale;
      } else {
        mSpeedScale = 1.f - scale;
      }
      break;
    }
  }
}

void CAnimData::SetPlaybackRate(float rate) { mSpeedScale = rate; }

void CAnimData::MultiplyPlaybackRate(float scale) { mSpeedScale *= scale; }

CCharAnimTime CAnimData::GetTimeOfUserEventForAnimation(int anim, EUserEventType type) const {
  const uint animRes = mCharInfo.GetAnimationIndexList()[anim];
  const rstl::ncrc_ptr< CAnimTreeNode > tree(GetAnimationManager()->GetAnimationTree(
      animRes, CMetaAnimTreeBuildOrders::NoSpecialOrders()));
  return GetTimeOfUserEvent(type, CCharAnimTime(GetAnimationDuration(anim)), tree);
}

CCharAnimTime CAnimData::GetTimeOfUserEvent(EUserEventType type, const CCharAnimTime& time) const {
  return GetTimeOfUserEvent(type, time, mAnimRoot);
}

CCharAnimTime CAnimData::GetTimeOfUserEvent(EUserEventType type, const CCharAnimTime& time,
                                            const rstl::ncrc_ptr< CAnimTreeNode >& tree) const {
  const int count = tree->GetInt32POIList(time, sInt32TransientCacheData, 16, 0, 64);
  for (int i = 0; i < count; ++i) {
    CInt32POINode& poi = sInt32TransientCacheData[i];
    if (poi.GetPoiType() == kPT_UserEvent) {
      const int value = poi.GetValue();
      if (value == static_cast< int >(type)) {
        CCharAnimTime ret = poi.GetTime();
        for (int j = i; j < count; ++j) {
          sInt32TransientCacheData[j] =
              CInt32POINode(0xffffffff, kPT_EmptyInt32, CCharAnimTime(0.f), -1, false, 1.f, -1, 0,
                            0, rstl::string_l("root"));
        }
        return ret;
      }
    }
    sInt32TransientCacheData[i] = CInt32POINode(0xffffffff, kPT_EmptyInt32, CCharAnimTime(0.f), -1,
                                                false, 1.f, -1, 0, 0, rstl::string_l("root"));
  }
  return CCharAnimTime::Infinity();
}

// Guessed name.
int CAnimData::CountUserEvents(EUserEventType type, const CCharAnimTime& time,
                               const rstl::ncrc_ptr< CAnimTreeNode >& tree) const {
  const int count = tree->GetInt32POIList(time, sInt32TransientCacheData, 16, 0, 64);
  int matches = 0;
  for (int i = 0; i < count; ++i) {
    const CInt32POINode& poi = sInt32TransientCacheData[i];
    if (poi.GetPoiType() == kPT_UserEvent && poi.GetValue() == static_cast< int >(type)) {
      ++matches;
    }
    sInt32TransientCacheData[i] = CInt32POINode(0xffffffff, kPT_EmptyInt32, CCharAnimTime(0.f), -1,
                                                false, 1.f, -1, 0, 0, rstl::string_l("root"));
  }
  return matches;
}

rstl::rc_ptr< CAnimationManager > CAnimData::GetAnimationManager() const { return mAnimMgr; }

// Guessed name.
int CAnimData::CountUserEventsForAnimation(int anim, EUserEventType type) const {
  const uint animRes = mCharInfo.GetAnimationIndexList()[anim];
  const rstl::ncrc_ptr< CAnimTreeNode > tree(GetAnimationManager()->GetAnimationTree(
      animRes, CMetaAnimTreeBuildOrders::NoSpecialOrders()));
  return CountUserEvents(type, CCharAnimTime(GetAnimationDuration(anim)), tree);
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

void CAnimData::SetPhase(float phase) { mAnimRoot->VSetPhase(phase); }

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
void CAnimData::SetAnimationTreeLimit(int limit) { mAnimationTreeLimit = limit; }

rstl::rc_ptr< CAnimationManager > CAnimData::GetAnimationManager() { return mAnimMgr; }

void CAnimData::AddAdditiveAnimation(uint idx, float weight, bool active, bool fadeOut) {
  const uint animIdx = mCharInfo.GetAnimationIndexList()[idx];
  rstl::pair< uint, CAdditiveAnimPlayback >* end = mAdditiveAnims.end();
  rstl::pair< uint, CAdditiveAnimPlayback >* search = mAdditiveAnims.begin();

  while (search != end) {
    if (animIdx == search->first) {
      break;
    }
    ++search;
  }

  if (search != end) {
    search->second.SetLoop(active);
    CAdditiveAnimPlayback& playback = search->second;
    playback.SetWeight(weight);
    playback.SetFadeOutWhenAnimOver(!playback.IsLoop() && fadeOut);
  } else {
    rstl::ncrc_ptr< CAnimTreeNode > animTree(GetAnimationManager()->GetAnimationTree(
        animIdx, CMetaAnimTreeBuildOrders::NoSpecialOrders()));

    typedef rstl::vector< rstl::pair< uint, CAdditiveAnimationInfo > > TAdditiveInfoList;
    const TAdditiveInfoList& infoList = mCharFactory->GetAdditiveAnimInfoList();

    AUTO(finder, rstl::default_pair_sorter_finder< TAdditiveInfoList >());
    TAdditiveInfoList::const_iterator infoSearch =
        rstl::binary_find(infoList.begin(), infoList.end(), animIdx, finder);

    const CAdditiveAnimationInfo& infoRef = infoSearch != infoList.end()
                                                ? infoSearch->second
                                                : mCharFactory->GetDefaultAdditiveAnimInfo();
    const CAdditiveAnimationInfo info(infoRef);

    mAdditiveAnims.push_back(rstl::pair< uint, CAdditiveAnimPlayback >(
        animIdx, CAdditiveAnimPlayback(animTree, weight, active, info, fadeOut)));
  }
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
  const uint animIdx = mCharInfo.GetAnimationIndexList()[idx];

  typedef rstl::vector< rstl::pair< uint, CAdditiveAnimationInfo > > TAdditiveInfoList;
  const TAdditiveInfoList& infoList = mCharFactory->GetAdditiveAnimInfoList();

  AUTO(finder, rstl::default_pair_sorter_finder< TAdditiveInfoList >());
  TAdditiveInfoList::const_iterator found =
      rstl::binary_find(infoList.begin(), infoList.end(), animIdx, finder);
  return found != infoList.end();
}

SAdvancementResults CAnimData::AdvanceAdditiveAnim(rstl::rc_ptr< CAnimTreeNode >& anim,
                                                   CCharAnimTime time) {
  SAdvancementResults ret = anim->VAdvanceView(time);

  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simplified = anim->Simplified();
  if (simplified.valid()) {
    anim = Cast(simplified.data());
  }

  return ret;
}

CAdvancementDeltas CAnimData::UpdateAdditiveAnims(float dt) {
  rstl::pair< uint, CAdditiveAnimPlayback >* it = mAdditiveAnims.begin();
  rstl::pair< uint, CAdditiveAnimPlayback >* const begin = mAdditiveAnims.begin();

  while (it != begin + mAdditiveAnims.size()) {
    CAdditiveAnimPlayback& playback = it->second;
    playback.Update(dt);

    const CCharAnimTime remTime = playback.AnimationTree()->VGetTimeRemaining();
    const CAdditiveAnimPlayback::EPlaybackPhase phase = playback.GetFadingMode();
    if (close_enough(remTime.GetSeconds(), 0.f) && playback.IsFadeOutWhenAnimOver() &&
        phase != CAdditiveAnimPlayback::kPP_FadedOut &&
        phase != CAdditiveAnimPlayback::kPP_FadingOut) {
      playback.FadeOut();
    }

    if (phase == CAdditiveAnimPlayback::kPP_FadedOut) {
      it = mAdditiveAnims.erase(it);
    } else {
      ++it;
    }
  }

  return AdvanceAdditiveAnims(dt);
}

CAdvancementDeltas CAnimData::AdvanceAdditiveAnims(float dt) {
  CQuaternion rotDelta(CQuaternion::NoRotation());
  float posDeltaX = 0.f;
  float posDeltaY = 0.f;
  float posDeltaZ = 0.f;

  const uint count = mAdditiveAnims.size();
  for (uint i = 0; i < count; ++i) {
    CAdditiveAnimPlayback& playback = mAdditiveAnims[i].second;
    rstl::rc_ptr< CAnimTreeNode >& anim = playback.AnimationTree();

    CCharAnimTime time(dt);

    if (playback.IsLoop()) {
      while (time.GreaterThanZero() && !close_enough(time.GetSeconds(), 0.f)) {
        mPassedIntCount +=
            anim->GetInt32POIList(time, mInt32POINodes.data(), 16, mPassedIntCount, 0);
        mPassedBoolCount +=
            anim->GetBoolPOIList(time, mBoolPOINodes.data(), 8, mPassedBoolCount, 0);
        mPassedParticleCount +=
            anim->GetParticlePOIList(time, mParticlePOINodes.data(), 64, mPassedParticleCount, 0);
        mPassedSoundCount +=
            anim->GetSoundPOIList(time, mSoundPOINodes.data(), 48, mPassedSoundCount, 0);

        const SAdvancementResults advResult = AdvanceAdditiveAnim(anim, time);
        const CAdvancementDeltas deltas = advResult.mDeltas;
        const CQuaternion thisRot = deltas.GetOrientationDelta();

        posDeltaX += deltas.GetOffsetDelta().GetX();
        posDeltaY += deltas.GetOffsetDelta().GetY();
        posDeltaZ += deltas.GetOffsetDelta().GetZ();
        rotDelta = rotDelta * thisRot;
        time = advResult.GetRemainder();
      }
    } else {
      CCharAnimTime remTime = anim->VGetTimeRemaining();

      while (!close_enough(remTime.GetSeconds(), 0.f) && !close_enough(time.GetSeconds(), 0.f)) {
        mPassedIntCount +=
            anim->GetInt32POIList(time, mInt32POINodes.data(), 16, mPassedIntCount, 0);
        mPassedBoolCount +=
            anim->GetBoolPOIList(time, mBoolPOINodes.data(), 8, mPassedBoolCount, 0);
        mPassedParticleCount +=
            anim->GetParticlePOIList(time, mParticlePOINodes.data(), 64, mPassedParticleCount, 0);
        mPassedSoundCount +=
            anim->GetSoundPOIList(time, mSoundPOINodes.data(), 48, mPassedSoundCount, 0);

        const SAdvancementResults advResult = AdvanceAdditiveAnim(anim, time);
        const CAdvancementDeltas deltas = advResult.mDeltas;
        const CQuaternion thisRot = deltas.GetOrientationDelta();

        posDeltaX += deltas.GetOffsetDelta().GetX();
        posDeltaY += deltas.GetOffsetDelta().GetY();
        posDeltaZ += deltas.GetOffsetDelta().GetZ();
        rotDelta = rotDelta * thisRot;
        time = advResult.GetRemainder();

        remTime = anim->VGetTimeRemaining();
        time = CCharAnimTime(rstl::min_val(time.GetSeconds(), remTime.GetSeconds()));
      }
    }
  }

  return CAdvancementDeltas(CVector3f(posDeltaX, posDeltaY, posDeltaZ), rotDelta);
}

void CAnimData::AddAdditiveSegData(CJointData_LinearStorage& data) const {
  const uint count = mAdditiveAnims.size();
  const CCharLayoutInfo& layout = **mLayoutData;
  for (uint i = 0; i < count; ++i) {
    const CAdditiveAnimPlayback& playback = mAdditiveAnims[i].second;
    const float weight = playback.GetWeight();
    if (!close_enough(weight, 0.f)) {
      CJointData_LinearStorage additiveData(layout.GetNumSegments(),
                                            CJointData_LinearStorage::kAF_Pool);
      additiveData.SetUseZeroOffsets(true);
      if (data.HasScales()) {
        additiveData.SetHasScales(true);
      }
      playback.GetAnimationTree()->VGetSegData(layout, additiveData);
      data.Add(additiveData, weight);
    }
  }
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
