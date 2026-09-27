#include "MetroidPrime/Factories/CCharacterFactory.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimationDatabaseGame.hpp"
#include "MetroidPrime/CTransitionDatabaseGame.hpp"

#include "Kyoto/Animation/CAnimCharacterSet.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CPrimitive.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/Graphics/CModel.hpp"

inline CTransitionManager::CTransitionManager(const CAnimSysContext& context) : mContext(context) {}

rstl::auto_ptr< IObj > CCharacterFactory::CDummyFactory::Build(const SObjectTag& tag,
                                                               const CVParamTransfer& params) {
  const FourCC type = tag.GetType();
  const CVParamTransfer paramCopy(params);
  const CCharacterInfo& charInfo =
      *static_cast< const TObjOwnerParam< const CCharacterInfo* >& >(*paramCopy).GetData();
  switch (type) {
  case 0:
    return CFactoryFnReturn(
               rs_new CSkinnedModel(
                   gpSimplePool->GetObj(SObjectTag('CMDL', charInfo.GetModelId())),
                   gpSimplePool->GetObj(SObjectTag('CSKR', charInfo.GetSkinRulesId())),
                   gpSimplePool->GetObj(SObjectTag('CINF', charInfo.GetCharLayoutInfoId()))))
        .GetObjForTransfer();
  case 1:
    return CFactoryFnReturn(
               rs_new CSkinnedModel(
                   gpSimplePool->GetObj(SObjectTag('CMDL', charInfo.GetIceModelId())),
                   gpSimplePool->GetObj(SObjectTag('CSKR', charInfo.GetIceSkinRulesId())),
                   gpSimplePool->GetObj(SObjectTag('CINF', charInfo.GetCharLayoutInfoId()))))
        .GetObjForTransfer();
  }
  return rstl::auto_ptr< IObj >();
}

void CCharacterFactory::CDummyFactory::BuildAsync(const SObjectTag& tag,
                                                  const CVParamTransfer& params, IObj** out) {
  *out = Build(tag, params).release();
}

void CCharacterFactory::CDummyFactory::CancelBuild(const SObjectTag&) {}

CCharacterFactory::CCharacterFactory(CSimplePool& store,
                                     const TLockedToken< CAnimCharacterSet >& ancs, CAssetId selfId)
: mCharInfoDB(GetCharacterInfoDB(**ancs))
, mCharLayoutInfoDB(GetCharLayoutInfoDB(store, mCharInfoDB))
, mAdditiveInfo(ancs->GetAnimationSet().GetAdditiveAnimInfoList())
, mDefaultAdditiveInfo(ancs->GetAnimationSet().GetDefaultAdditiveAnimInfo())
, mSelfId(selfId)
, mCacheResPool(mDummyFactory)
, mAnimCharacterSet(ancs) {
  const CAnimationSet& animSet = ancs->GetAnimationSet();
  const CAnimationSet::AnimationList& animations = animSet.GetAnimations();
  const CAnimationSet::TransitionList& transitions = animSet.GetTransitions();
  const CAnimationSet::HalfTransitionList& halfTransitions = animSet.GetHalfTransitions();
  const rstl::rc_ptr< IMetaTrans > defaultTrans = animSet.GetDefaultTransition();
  const TToken< CAnimationDatabaseGame > animDB(rs_new CAnimationDatabaseGame(animations));
  const TToken< CTransitionDatabaseGame > transDB(
      rs_new CTransitionDatabaseGame(transitions, halfTransitions, defaultTrans));
  const rstl::ncrc_ptr< CRandom16 > random(rs_new CRandom16(2334));
  mSysContext = rstl::ncrc_ptr< CAnimSysContext >(
      rs_new CAnimSysContext(transDB, random, store, animSet.GetEventSets()));
  mAnimMgr = rs_new CAnimationManager(animDB, *mSysContext);
  mTransMgr = rs_new CTransitionManager(*mSysContext);

  rstl::vector< CPrimitive > primitives;
  animDB.NonConstCopy()->GetAllUniquePrimitives(primitives);
}

rstl::auto_ptr< CAnimData >
CCharacterFactory::CreateCharacter(int charIdx, bool loop,
                                   const TLockedToken< CCharacterFactory >& factory,
                                   int defaultAnim) const {
  const CCharacterInfo& charInfo = mCharInfoDB[charIdx];
  const SObjectTag modelTag(0, charInfo.GetModelId());
  TToken< CSkinnedModel > skinnedModel = mCacheResPool.GetObj(
      modelTag, CVParamTransfer(rs_new TObjOwnerParam< const CCharacterInfo* >(&charInfo)));

  const CAssetId iceModelId = charInfo.GetIceModelId();
  const CAssetId iceSkinId = charInfo.GetIceSkinRulesId();
  const SObjectTag iceTag(1, iceModelId);
  rstl::optional_object< TLockedToken< CSkinnedModelWithAvgNormals > > iceModel;
  if (iceModelId != kInvalidAssetId && iceSkinId != kInvalidAssetId && iceModelId != 0 &&
      iceSkinId != 0) {
    iceModel = TLockedToken< CSkinnedModelWithAvgNormals >(mCacheResPool.GetObj(
        iceTag, CVParamTransfer(rs_new TObjOwnerParam< const CCharacterInfo* >(&charInfo))));
  }

  const CAssetId spatialId = charInfo.GetSpatialPrimitiveId();
  const SObjectTag spatialTag('CSPP', spatialId);
  rstl::optional_object< TLockedToken< CSpatialPrimitive > > spatial;
  if (spatialId != kInvalidAssetId) {
    spatial = TLockedToken< CSpatialPrimitive >(gpSimplePool->GetObj(spatialTag));
  }

  CAnimData* animData = rs_new CAnimData(
      mSelfId, charInfo, defaultAnim, charIdx, loop, mCharLayoutInfoDB[charIdx], skinnedModel,
      iceModel, spatial, mSysContext, mAnimMgr, mTransMgr, factory, charInfo.GetAnimatedScale());
  return animData;
}

const CCharacterInfo& CCharacterFactory::GetCharInfo(int charIdx) const {
  return mCharInfoDB[charIdx];
}

rstl::vector< CCharacterInfo >
CCharacterFactory::GetCharacterInfoDB(const CAnimCharacterSet& ancs) {
  const rstl::vector< rstl::pair< int, CCharacterInfo > >& chars =
      ancs.GetCharacterSet().GetCharacterList();
  rstl::vector< rstl::pair< int, CCharacterInfo > >::const_iterator it = chars.begin();
  rstl::vector< rstl::pair< int, CCharacterInfo > >::const_iterator end = chars.end();
  rstl::vector< CCharacterInfo > result;
  result.reserve(chars.size());
  for (; it != end; ++it) {
    result.push_back_unsafe(it->second);
  }
  return result;
}

rstl::vector< TToken< CCharLayoutInfo > >
CCharacterFactory::GetCharLayoutInfoDB(CSimplePool& store,
                                       const rstl::vector< CCharacterInfo >& chars) {
  rstl::vector< TToken< CCharLayoutInfo > > result;
  const uint count = chars.size();
  result.reserve(count);
  for (uint i = 0; i < count; ++i) {
    TToken< CCharLayoutInfo > layout =
        store.GetObj(SObjectTag('CINF', chars[i].GetCharLayoutInfoId()));
    result.push_back_unsafe(layout);
  }
  return result;
}

CCharacterFactory::~CCharacterFactory() {}
