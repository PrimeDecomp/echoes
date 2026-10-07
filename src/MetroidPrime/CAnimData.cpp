#include "MetroidPrime/CAnimData.hpp"

#include "Kyoto/Animation/CAllFormatsAnimSource.hpp"
#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimTreeBlend.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CAnimationManager.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#include "Kyoto/Animation/CPrimitive.hpp"
#include "Kyoto/Animation/CSkinRules.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Animation/CTransitionManager.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "rstl/algorithm.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Factories/CCharacterFactory.hpp"

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
  const rstl::vector< rstl::pair< uint, CAABox > >& bounds = mCharInfo.GetAnimBoundsById();
  if (bounds.size() > 0) {
    const CAnimTreeEffectiveContribution contrib = mAnimRoot->GetContributionOfHighestInfluence();
    const uint animId = contrib.GetAnimDatabaseIndex();
    if (animId != mCachedBoundsAnimId) {
      rstl::vector< rstl::pair< uint, CAABox > >::const_iterator it =
          rstl::find_by_key(bounds, contrib.GetAnimDatabaseIndex());
      if (it == bounds.end()) {
        mCachedAnimBounds = mAabb;
      } else {
        mCachedAnimBounds = it->second;
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
  float minX = 1000000.f;
  float maxX = -1000000.f;
  float minY = minX;
  float maxY = maxX;
  float minZ = minX;
  float maxZ = maxX;
  CSkinnedModelState state(mModelData->MakeDefaultStorage());
  const CSkinnedModel* model = *mModelData;
  model->StoreCalculation(state, &mPose);
  const int count = model->GetSkinRules()->GetNumPoints();
  for (int i = 0; i < count; ++i) {
    const CVector3f pos = mModelData->GetSkinnedPosition(state.GetWorkspace(), i);
    if (pos.GetX() > maxX) {
      maxX = pos.GetX();
    } else if (pos.GetX() < minX) {
      minX = pos.GetX();
    }
    if (pos.GetY() > maxY) {
      maxY = pos.GetY();
    } else if (pos.GetY() < minY) {
      minY = pos.GetY();
    }
    if (pos.GetZ() > maxZ) {
      maxZ = pos.GetZ();
    } else if (pos.GetZ() < minZ) {
      minZ = pos.GetZ();
    }
  }
  return CAABox(minX, minY, minZ, maxX, maxY, maxZ);
}

void CAnimData::ResetPOILists() {
  mPassedBoolCount = 0;
  mPassedIntCount = 0;
  mPassedParticleCount = 0;
  mPassedSoundCount = 0;
}

static inline CCharAnimTime GetSourceDuration(const CAllFormatsAnimSource& source) {
  switch (source.GetType()) {
  case 0:
    return source.AsCAnimSource().GetAnimationDuration();
  case 2:
    return source.AsCFBStreamedCompression().GetAnimationDuration();
  default:
    return source.AsCAnimSource().GetAnimationDuration();
  }
}

static inline float GetSourceAverageVelocity(const CAllFormatsAnimSource& source) {
  switch (source.GetType()) {
  case 0:
    return source.AsCAnimSource().GetAverageVelocity();
  case 2:
    return source.AsCFBStreamedCompression().GetAverageVelocity();
  default:
    return source.AsCAnimSource().GetAverageVelocity();
  }
}

float CAnimData::GetAverageVelocity(int anim) const {
  const rstl::rc_ptr< IMetaAnim > metaAnim =
      mAnimMgr->GetMetaAnimation(mCharInfo.GetAnimationIndexList()[anim]);
  rstl::set< CPrimitive > primitives;
  metaAnim->GetUniquePrimitives(primitives);
  float durationAccum = 0.f;
  float velocityAccum = 0.f;
  rstl::set< CPrimitive >::const_iterator begin = primitives.begin();
  rstl::set< CPrimitive >::const_iterator end = primitives.end();
  rstl::set< CPrimitive >::const_iterator it = begin;
  while (it != end) {
    const SObjectTag tag('ANIM', it->GetAnimResId());
    const TLockedToken< CAllFormatsAnimSource > source =
        mAnimCtx->GetSimplePool().GetObj(tag);
    const CAllFormatsAnimSource* animSource = *source;
    velocityAccum +=
        GetSourceAverageVelocity(*animSource) * GetSourceDuration(*animSource).GetSeconds();
    durationAccum += GetSourceDuration(**source).GetSeconds();
    ++it;
  }
  float result = 0.f;
  if (durationAccum > 0.f) {
    result = velocityAccum / durationAccum;
  }
  return result;
}

// Guessed name.
void CAnimData::CollectAnimationResources(rstl::vector< SObjectTag >& tagsOut) const {
  rstl::set< SObjectTag > tags;
  rstl::vector< uint >::const_iterator it = mCharInfo.GetAnimationIndexList().begin();
  for (; it != mCharInfo.GetAnimationIndexList().end(); ++it) {
    const rstl::rc_ptr< IMetaAnim > anim = mAnimMgr->GetMetaAnimation(*it);
    rstl::set< CPrimitive > primitives;
    anim->GetUniquePrimitives(primitives);
    rstl::set< CPrimitive >::const_iterator begin = primitives.begin();
    rstl::set< CPrimitive >::const_iterator end = primitives.end();
    rstl::set< CPrimitive >::const_iterator prim = begin;
    for (; prim != primitives.end(); ++prim) {
      tags.insert(SObjectTag('ANIM', prim->GetAnimResId()));
    }
  }
  tagsOut.reserve(tagsOut.size() + tags.size());
  tagsOut.insert(tagsOut.end(), tags.begin(), tags.end());
}

// Guessed name.
void CAnimData::CollectAnimationTokens(rstl::vector< CToken >& tokensOut, bool lock) const {
  rstl::vector< SObjectTag > tags;
  CollectAnimationResources(tags);
  if (tags.size() == 0) {
    return;
  }
  tokensOut.reserve(tags.size() + tokensOut.size());
  for (int i = 0; i < tags.size(); ++i) {
    CToken token = gpSimplePool->GetObj(tags[i]);
    if (lock) {
      token.Lock();
    }
    tokensOut.push_back_unsafe(token);
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
  const uint animA = mCharInfo.GetAnimationIndexList()[parms.GetAnimationId()];
  const float blendWeight = parms.GetBlendFactor();
  if (animB != -1) {
    const uint animBIdx = mCharInfo.GetAnimationIndexList()[animB];
    const rstl::ncrc_ptr< CAnimTreeNode > treeA =
        GetAnimationManager()->GetAnimationTree(animA, CMetaAnimTreeBuildOrders::NoSpecialOrders());
    const rstl::ncrc_ptr< CAnimTreeNode > treeB = GetAnimationManager()->GetAnimationTree(
        animBIdx, CMetaAnimTreeBuildOrders::NoSpecialOrders());
    return rstl::ncrc_ptr< CAnimTreeNode >(
        rs_new CAnimTreeBlend(false, treeA, treeB, blendWeight,
                              CAnimTreeBlend::CreatePrimitiveName(treeA, treeB, blendWeight)));
  }
  return GetAnimationManager()->GetAnimationTree(animA, CMetaAnimTreeBuildOrders::NoSpecialOrders());
}

// Guessed name.
rstl::rc_ptr< IMetaTrans > CAnimData::BuildMetaTransition(const CAnimPlaybackParms& parms) const {
  const rstl::ncrc_ptr< CAnimTreeNode > tree(BuildAnimationTree(parms));
  return mTransMgr->GetMetaTrans(mAnimRoot, tree);
}

void CAnimData::SetAnimation(const CAnimPlaybackParms& parms, bool noTrans) {
  const uint numChildren = mAnimRoot->VGetNumChildren();
  if (parms.GetAnimationId() == mPlaybackParms.GetAnimationId() ||
      (parms.GetSecondAnimationId() == mPlaybackParms.GetSecondAnimationId() &&
       parms.GetSecondAnimationId() != -1) ||
      (parms.GetBlendFactor() == mPlaybackParms.GetBlendFactor() &&
       parms.GetBlendFactor() != 1.f)) {
    if (mAnimationJustStarted) {
      return;
    }
  }
  if (numChildren >= mAnimationTreeLimit) {
    return;
  }
  ResetPOILists();
  mSpeedScale = 1.f;
  mPlaybackParms.SetAnimationId(parms.GetAnimationId());
  mPlaybackParms.SetSecondAnimationId(parms.GetSecondAnimationId());
  mPlaybackParms.SetBlendFactor(parms.GetBlendFactor());
  mCurrentAnim = parms.GetAnimationId();
  const bool animating = parms.GetIsPlayAnimation();
  const rstl::ncrc_ptr< CAnimTreeNode > tree = BuildAnimationTree(parms);
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

void CAnimData::GetAnimationPrimitives(const CAnimPlaybackParms& parms,
                                       rstl::set< CPrimitive >& primsOut) const {
  const uint animA = mCharInfo.GetAnimationIndexList()[parms.GetAnimationId()];
  const int animB = parms.GetSecondAnimationId();
  GetAnimationManager()->GetMetaAnimation(animA)->GetUniquePrimitives(primsOut);
  if (animB != -1) {
    const uint animBIdx = mCharInfo.GetAnimationIndexList()[animB];
    GetAnimationManager()->GetMetaAnimation(animBIdx)->GetUniquePrimitives(primsOut);
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

void CAnimData::RenderAuxiliary(const CFrustumPlanes& planes) const {
  mParticleDB.AddToRendererClipped(planes);
}

void CAnimData::RecalcPoseBuilder(const CCharAnimTime* time) const {
  CAnimMathUtils::sUseFastSlerp = mUseFastSlerp;
  const CCharLayoutInfo& layout = **mLayoutData;
  rstl::optional_object< CJointData_LinearStorage > localStorage;
  CJointData_LinearStorage* storage = mJointData.get();
  if (storage == nullptr) {
    storage = new (localStorage.prepare_emplace()) CJointData_LinearStorage(
        layout.GetBodyPartSegIds().GetCount(), CJointData_LinearStorage::kAF_Pool);
  } else {
    storage->ResetFlags();
  }
  if (mAnimatedScale) {
    storage->SetHasScales(true);
  }
  if (time == nullptr) {
    mAnimRoot->VGetSegData(layout, *storage);
  } else {
    mAnimRoot->VGetSegData(layout, *storage, *time);
  }
  AddAdditiveSegData(*storage);
  mPose.BuildPose(layout, *storage);
}

rstl::ncrc_ptr< CAnimSysContext > CAnimData::GetAnimSysContext() const { return mAnimCtx; }

float CAnimData::GetAnimationDuration(int anim) const {
  const uint animIdx = mCharInfo.GetAnimationIndexList()[anim];
  const rstl::rc_ptr< IMetaAnim > metaAnim = GetAnimationManager()->GetMetaAnimation(animIdx);
  rstl::set< CPrimitive > primitives;
  metaAnim->GetUniquePrimitives(primitives);
  float duration = 0.f;
  rstl::set< CPrimitive >::const_iterator begin = primitives.begin();
  rstl::set< CPrimitive >::const_iterator end = primitives.end();
  rstl::set< CPrimitive >::const_iterator it = begin;
  while (it != end) {
    const SObjectTag tag('ANIM', it->GetAnimResId());
    const TLockedToken< CAllFormatsAnimSource > source =
        GetAnimSysContext()->GetSimplePool().GetObj(tag);
    duration += GetSourceDuration(**source).GetSeconds();
    ++it;
  }
  if (metaAnim->GetType() == kMAT_Random) {
    duration /= primitives.size();
  }
  return duration;
}

float CAnimData::GetAnimTimeRemaining(const rstl::string& name) const {
  float remaining = mAnimRoot->VGetTimeRemaining().GetSeconds();
  if (mSpeedScale > 0.f) {
    remaining /= mSpeedScale;
  }
  return remaining;
}

bool CAnimData::IsAnimTimeRemaining(float tolerance, const rstl::string& name) const {
  if (mAnimRoot.GetPtr() != nullptr) {
    return !close_enough(mAnimRoot->VGetTimeRemaining().GetSeconds(), 0.f, tolerance);
  }
  return false;
}

CSegId CAnimData::GetLocatorSegId(const rstl::string& name) const {
  return mLayoutData->GetSegIdFromString(name);
}

CTransform4f CAnimData::GetLocatorTransform(const rstl::string& name,
                                            const CCharAnimTime* time) const {
  CSegId id = mLayoutData->GetSegIdFromString(name);
  return GetLocatorTransform(id, time);
}

CTransform4f CAnimData::GetLocatorTransform(CSegId id, const CCharAnimTime* time) const {
  if (id != CSegId::Invalid()) {
    if (time != nullptr || !mPoseBuilt) {
      RecalcPoseBuilder(time);
      mPoseBuilt = time == nullptr;
    }
    return CTransform4f(mPose.GetRotation(id), mPose.GetOffset(id));
  }
  return CTransform4f::Identity();
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
      const int range = poi.GetValue();
      const float rate = (random.Next() % range) / 100.f;
      if ((random.Next() % 100) < 50) {
        mSpeedScale = 1.f + rate;
      } else {
        mSpeedScale = 1.f - rate;
      }
      break;
    }
  }
}

void CAnimData::SetPlaybackRate(float rate) { mSpeedScale = rate; }

void CAnimData::MultiplyPlaybackRate(float scale) { mSpeedScale *= scale; }

CCharAnimTime CAnimData::GetTimeOfUserEventForAnimation(int anim, EUserEventType type) const {
  const uint animIdx = mCharInfo.GetAnimationIndexList()[anim];
  const rstl::ncrc_ptr< CAnimTreeNode > tree =
      GetAnimationManager()->GetAnimationTree(animIdx, CMetaAnimTreeBuildOrders::NoSpecialOrders());
  return GetTimeOfUserEvent(type, CCharAnimTime(GetAnimationDuration(anim)), tree);
}

CCharAnimTime CAnimData::GetTimeOfUserEvent(EUserEventType type, const CCharAnimTime& time) const {
  return GetTimeOfUserEvent(type, time, mAnimRoot);
}

CCharAnimTime CAnimData::GetTimeOfUserEvent(EUserEventType type, const CCharAnimTime& time,
                                            const rstl::ncrc_ptr< CAnimTreeNode >& tree) const {
  const int count = tree->GetInt32POIList(time, sInt32TransientCacheData, 16, 0, 64);
  for (int i = 0; i < count; ++i) {
    const CInt32POINode& node = sInt32TransientCacheData[i];
    if (node.GetPoiType() == kPT_UserEvent && node.GetValue() == type) {
      const CCharAnimTime result = node.GetTime();
      for (; i < count; ++i) {
        sInt32TransientCacheData[i] =
            CInt32POINode(0xffffffff, kPT_EmptyInt32, CCharAnimTime(0.f), -1, false, 1.f, -1, 0, 0,
                          rstl::string_l("root"));
      }
      return result;
    }
    sInt32TransientCacheData[i] = CInt32POINode(0xffffffff, kPT_EmptyInt32, CCharAnimTime(0.f),
                                                -1, false, 1.f, -1, 0, 0, rstl::string_l("root"));
  }
  return CCharAnimTime::Infinity();
}

// Guessed name.
int CAnimData::CountUserEvents(EUserEventType type, const CCharAnimTime& time,
                               const rstl::ncrc_ptr< CAnimTreeNode >& tree) const {
  const int count = tree->GetInt32POIList(time, sInt32TransientCacheData, 16, 0, 64);
  int result = 0;
  for (int i = 0; i < count; ++i) {
    if (sInt32TransientCacheData[i].GetPoiType() == kPT_UserEvent &&
        type == sInt32TransientCacheData[i].GetValue()) {
      ++result;
    }
    sInt32TransientCacheData[i] = CInt32POINode(0xffffffff, kPT_EmptyInt32, CCharAnimTime(0.f), -1, false, 1.f, -1, 0, 0,
                         rstl::string_l("root"));
  }
  return result;
}

rstl::rc_ptr< CAnimationManager > CAnimData::GetAnimationManager() const { return mAnimMgr; }

// Guessed name.
int CAnimData::CountUserEventsForAnimation(int anim, EUserEventType type) const {
  const uint animIdx = mCharInfo.GetAnimationIndexList()[anim];
  const rstl::ncrc_ptr< CAnimTreeNode > tree =
      GetAnimationManager()->GetAnimationTree(animIdx, CMetaAnimTreeBuildOrders::NoSpecialOrders());
  return CountUserEvents(type, CCharAnimTime(GetAnimationDuration(anim)), tree);
}

void CAnimData::Touch(const CSkinnedModel& model, int shaderIdx) {
  model.GetModel()->Touch(shaderIdx);
}

void CAnimData::Touch(const CSkinnedModel& model) {
  const CModel& cmodel = **model.GetModel();
  const int shaderCount = cmodel.GetNumMaterialSets();
  for (int shader = 0; shader < shaderCount; ++shader) {
    cmodel.Touch(shader);
  }
}

void CAnimData::InitializeEffects(CStateManager& mgr, TAreaId areaId, const CVector3f& scale) {
  const uint effectCount = mCharInfo.GetEffects().size();
  for (uint i = 0; i < effectCount; ++i) {
    const rstl::pair< rstl::string, rstl::vector< CEffectComponent > > effect =
        mCharInfo.GetEffects()[i];
    const uint componentCount = effect.second.size();
    for (uint j = 0; j < componentCount; ++j) {
      const CEffectComponent& component = effect.second[j];
      mParticleDB.CacheParticleDesc(component.GetParticleTag());
      mParticleDB.AddParticleEffect(
          component.GetComponentNameHash(), component.GetFlags(),
          CParticleData(0, component.GetParticleTag(), component.GetSegmentId(),
                        component.GetScale(), component.GetParentedMode()),
          scale, &mgr, areaId, true, mParticleLightIdx);
      mParticleDB.SetParticleEffectState(component.GetComponentNameHash(), false, &mgr);
    }
  }
}

CParticleGenInfo* CAnimData::GetFirstParticleEffect(const rstl::string& name) {
  const CCharacterInfo::TEffectList& effects = mCharInfo.GetEffects();
  CCharacterInfo::TEffectList::const_iterator it = rstl::find_by_key(effects, name);
  if (it != effects.end() && !it->second.empty()) {
    return mParticleDB.GetParticleEffect(it->second[0].GetComponentNameHash());
  }
  return nullptr;
}

void CAnimData::SetEffectState(const rstl::string& name, bool active, CStateManager& mgr) {
  const CCharacterInfo::TEffectList effects = mCharInfo.GetEffects();
  CCharacterInfo::TEffectList::const_iterator it = rstl::find_by_key(effects, name);
  if (it != effects.end()) {
    rstl::vector< CEffectComponent >::const_iterator end = it->second.end();
    rstl::vector< CEffectComponent >::const_iterator comp = it->second.begin();
    for (; comp != end; ++comp) {
      mParticleDB.SetParticleEffectState(comp->GetComponentNameHash(), active, &mgr);
    }
  }
}

void CAnimData::SetEffectComponentExternalParam(const rstl::string& name, int index, float value) {
  const CCharacterInfo::TEffectList effects = mCharInfo.GetEffects();
  CCharacterInfo::TEffectList::const_iterator it = rstl::find_by_key(effects, name);
  if (it != effects.end()) {
    rstl::vector< CEffectComponent >::const_iterator comp = it->second.begin();
    if (comp != it->second.end()) {
      mParticleDB.SetParticleExternalParam(comp->GetComponentNameHash(), index, value);
    }
  }
}

void CAnimData::SetPhase(float phase) { mAnimRoot->VSetPhase(phase); }

void CAnimData::SetKeepJSPose(bool keep) {
  if (keep) {
    if (mJointData.null()) {
      mJointData = rstl::auto_ptr< CJointData_LinearStorage >(rs_new CJointData_LinearStorage(
          mLayoutData->GetBodyPartSegIds().GetCount(), CJointData_LinearStorage::kAF_Heap));
      mJointData->SetZeroRotation();
      mJointData->ResetScales();
      mJointData->SetReferenceOffsets(**mLayoutData);
    }
  } else {
    mJointData = rstl::auto_ptr< CJointData_LinearStorage >();
  }
}

// Guessed name.
void CAnimData::SetAnimationTreeLimit(int limit) { mAnimationTreeLimit = limit; }

rstl::rc_ptr< CAnimationManager > CAnimData::GetAnimationManager() { return mAnimMgr; }

void CAnimData::AddAdditiveAnimation(uint idx, float weight, bool active, bool fadeOut) {
  const uint anim = mCharInfo.GetAnimationIndexList()[idx];
  TAdditiveAnims::iterator it = mAdditiveAnims.begin();
  for (; it != mAdditiveAnims.end(); ++it) {
    if (anim == it->first) {
      break;
    }
  }
  if (it != mAdditiveAnims.end()) {
    it->second.SetLoop(active);
    CAdditiveAnimPlayback& playback = it->second;
    playback.SetWeight(weight);
    playback.SetFadeOutWhenAnimOver(!playback.IsLoop() && fadeOut);
  } else {
    const rstl::ncrc_ptr< CAnimTreeNode > node =
        GetAnimationManager()->GetAnimationTree(anim, CMetaAnimTreeBuildOrders::NoSpecialOrders());
    const rstl::vector< rstl::pair< uint, CAdditiveAnimationInfo > >& infos =
        mCharFactory->GetAdditiveAnimInfoList();
    rstl::vector< rstl::pair< uint, CAdditiveAnimationInfo > >::const_iterator infoIt =
        rstl::binary_find(infos.begin(), infos.end(), anim,
                          rstl::pair_sorter_finder< rstl::pair< uint, CAdditiveAnimationInfo >,
                                                    rstl::less< uint > >(rstl::less< uint >()));
    const CAdditiveAnimationInfo info =
        infoIt != infos.end() ? infoIt->second : mCharFactory->GetDefaultAdditiveAnimInfo();
    const CAdditiveAnimPlayback playback(node, weight, active, info, fadeOut);
    mAdditiveAnims.push_back(rstl::pair< uint, CAdditiveAnimPlayback >(anim, playback));
  }
}

void CAnimData::DelAdditiveAnimation(uint idx) {
  const uint anim = mCharInfo.GetAnimationIndexList()[idx];
  TAdditiveAnims::iterator it = mAdditiveAnims.begin();
  for (; it != mAdditiveAnims.end(); ++it) {
    if (anim == it->first) {
      break;
    }
  }
  if (it != mAdditiveAnims.end()) {
    CAdditiveAnimPlayback& playback = it->second;
    const CAdditiveAnimPlayback::EPlaybackPhase phase = playback.GetFadingMode();
    if (phase != CAdditiveAnimPlayback::kPP_FadingOut &&
        phase != CAdditiveAnimPlayback::kPP_FadedOut) {
      playback.FadeOut();
    }
  }
}

void CAnimData::DelAdditiveAnimationImmediately(uint idx) {
  const uint anim = mCharInfo.GetAnimationIndexList()[idx];
  for (TAdditiveAnims::iterator it = mAdditiveAnims.begin(); it != mAdditiveAnims.end(); ++it) {
    if (anim == it->first) {
      mAdditiveAnims.erase(it);
      return;
    }
  }
}

float CAnimData::GetAdditiveAnimationWeight(uint idx) {
  const uint anim = mCharInfo.GetAnimationIndexList()[idx];
  for (TAdditiveAnims::const_iterator it = mAdditiveAnims.begin(); it != mAdditiveAnims.end();
       ++it) {
    if (anim == it->first) {
      return it->second.GetWeight();
    }
  }
  return 0.f;
}

bool CAnimData::IsAdditiveAnimationActive(uint idx) const {
  const uint anim = mCharInfo.GetAnimationIndexList()[idx];
  for (TAdditiveAnims::const_iterator it = mAdditiveAnims.begin(); it != mAdditiveAnims.end();
       ++it) {
    if (anim == it->first) {
      return true;
    }
  }
  return false;
}

rstl::rc_ptr< CAnimTreeNode > CAnimData::GetAdditiveAnimationTree(uint idx) const {
  const uint anim = mCharInfo.GetAnimationIndexList()[idx];
  TAdditiveAnims::const_iterator it = mAdditiveAnims.begin();
  for (; it != mAdditiveAnims.end(); ++it) {
    if (anim == it->first) {
      break;
    }
  }
  if (it == mAdditiveAnims.end()) {
    return rstl::rc_ptr< CAnimTreeNode >(nullptr);
  }
  return it->second.GetAnimationTree();
}

const rstl::ncrc_ptr< CAnimTreeNode >& CAnimData::GetAnimationTree() const { return mAnimRoot; }

bool CAnimData::IsAdditiveAnimation(uint idx) const {
  const uint anim = mCharInfo.GetAnimationIndexList()[idx];
  const rstl::vector< rstl::pair< uint, CAdditiveAnimationInfo > >& infos =
      mCharFactory->GetAdditiveAnimInfoList();
  rstl::vector< rstl::pair< uint, CAdditiveAnimationInfo > >::const_iterator it =
      rstl::binary_find(infos.begin(), infos.end(), anim,
                        rstl::pair_sorter_finder< rstl::pair< uint, CAdditiveAnimationInfo >,
                                                  rstl::less< uint > >(rstl::less< uint >()));
  return it != infos.end();
}

SAdvancementResults CAnimData::AdvanceAdditiveAnim(rstl::rc_ptr< CAnimTreeNode >& tree,
                                                   CCharAnimTime time) {
  const SAdvancementResults results = tree->VAdvanceView(time);
  const rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simplified =
      tree->Simplified();
  if (simplified.valid()) {
    tree = Cast(simplified.data());
  }
  return results;
}

CAdvancementDeltas CAnimData::UpdateAdditiveAnims(float dt) {
  TAdditiveAnims::iterator it = mAdditiveAnims.begin();
  while (it != mAdditiveAnims.end()) {
    CAdditiveAnimPlayback& playback = it->second;
    playback.Update(dt);
    const CCharAnimTime remaining = playback.GetAnimationTree()->VGetTimeRemaining();
    const CAdditiveAnimPlayback::EPlaybackPhase phase = playback.GetFadingMode();
    if (close_enough(remaining.GetSeconds(), 0.f) && playback.IsFadeOutWhenAnimOver() &&
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
  CVector3f posDelta(0.f, 0.f, 0.f);
  CQuaternion rotDelta = CQuaternion::NoRotation();
  const uint count = mAdditiveAnims.size();
  for (uint i = 0; i < count; ++i) {
    CAdditiveAnimPlayback& playback = mAdditiveAnims[i].second;
    rstl::ncrc_ptr< CAnimTreeNode >& anim = playback.AnimationTree();
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
        const SAdvancementResults results = AdvanceAdditiveAnim(anim, time);
        const CAdvancementDeltas deltas = results.mDeltas;
        posDelta += deltas.GetOffsetDelta();
        const CQuaternion rot = deltas.GetOrientationDelta();
        rotDelta = rotDelta * rot;
        time = results.mRemTime;
      }
    } else {
      CCharAnimTime remaining = anim->VGetTimeRemaining();
      while (!close_enough(remaining.GetSeconds(), 0.f) && !close_enough(time.GetSeconds(), 0.f)) {
        mPassedIntCount +=
            anim->GetInt32POIList(time, mInt32POINodes.data(), 16, mPassedIntCount, 0);
        mPassedBoolCount +=
            anim->GetBoolPOIList(time, mBoolPOINodes.data(), 8, mPassedBoolCount, 0);
        mPassedParticleCount +=
            anim->GetParticlePOIList(time, mParticlePOINodes.data(), 64, mPassedParticleCount, 0);
        mPassedSoundCount +=
            anim->GetSoundPOIList(time, mSoundPOINodes.data(), 48, mPassedSoundCount, 0);
        const SAdvancementResults results = AdvanceAdditiveAnim(anim, time);
        const CAdvancementDeltas deltas = results.mDeltas;
        posDelta += deltas.GetOffsetDelta();
        const CQuaternion rot = deltas.GetOrientationDelta();
        rotDelta = rotDelta * rot;
        time = results.mRemTime;
        remaining = anim->VGetTimeRemaining();
        time = CCharAnimTime(rstl::min_val(time.GetSeconds(), remaining.GetSeconds()));
      }
    }
  }
  return CAdvancementDeltas(posDelta, rotDelta);
}

void CAnimData::AddAdditiveSegData(CJointData_LinearStorage& data) const {
  const uint count = mAdditiveAnims.size();
  const CCharLayoutInfo& layout = **mLayoutData;
  for (uint i = 0; i < count; ++i) {
    const CAdditiveAnimPlayback& playback = mAdditiveAnims[i].second;
    const float weight = playback.GetWeight();
    if (!close_enough(weight, 0.f)) {
      CJointData_LinearStorage additive(layout.GetNumSegments(), CJointData_LinearStorage::kAF_Pool);
      additive.SetUseZeroOffsets(true);
      if (data.HasScales()) {
        additive.SetHasScales(true);
      }
      playback.GetAnimationTree()->VGetSegData(layout, additive);
      data.Add(additive, weight);
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
