#include "MetroidPrime/CActorModelParticles.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CRainSplashGenerator.hpp"

#include "rstl/string.hpp"

static const char* const skParticleNames[] = {
    "Effect_OnFire",   "Effect_IceBreak", "Effect_Ash",       "Effect_FirePop",
    "Effect_Electric", "Effect_IcePop",   "Effect_Blackhole", "Effect_Imploder",
};

static bool IsMediumOrLarge(const CActor& actor) {
  // TODO: inspect the patterned creature size, treating players as large.
  return false;
}

CActorModelParticles::CSystem::CSystem(const char* name) : mRefCount(0), mLoaded(false) {
  TLockedToken< CDependencyGroup > group = gpSimplePool->GetObj(name);
  const rstl::vector< SObjectTag >& tags = group->GetObjectTagVector();
  mTokens.reserve(tags.size());
  for (rstl::vector< SObjectTag >::const_iterator it = tags.begin(); it != tags.end(); ++it) {
    mTokens.push_back(gpSimplePool->GetObj(*it));
  }
}

void CActorModelParticles::CSystem::AddRef() {
  if (++mRefCount == 1) {
    Lock();
  }
}

void CActorModelParticles::CSystem::DelRef() {
  if (--mRefCount <= 0) {
    Unlock();
  }
}

void CActorModelParticles::CSystem::Lock() {
  bool loading = false;
  for (rstl::vector< CToken >::iterator it = mTokens.begin(); it != mTokens.end(); ++it) {
    if (!it->HasLock()) {
      it->Lock();
      loading = true;
    } else if (!it->IsLoaded()) {
      loading = true;
    }
  }
  if (!loading) {
    mLoaded = true;
  }
}

void CActorModelParticles::CSystem::Unlock() {
  for (rstl::vector< CToken >::iterator it = mTokens.begin(); it != mTokens.end(); ++it) {
    it->Unlock();
  }
  mLoaded = false;
}

void CActorModelParticles::CSystem::Update() {
  if (mLoaded || mRefCount == 0) {
    return;
  }
  for (rstl::vector< CToken >::const_iterator it = mTokens.begin(); it != mTokens.end(); ++it) {
    if (!it->IsLoaded()) {
      return;
    }
  }
  mLoaded = true;
}

CActorModelParticles::CItem::CItem(const CEntity& ent, CActorModelParticles& parent)
: mId(ent.GetUniqueId())
, mAreaId(ent.GetCurrentAreaId())
, mOnFireGens(rstl::pair< rstl::auto_ptr< CElementGen >, uint >(rstl::auto_ptr< CElementGen >(), 0))
, mOnFireDelayTimer(0.f)
, mOnFire(false)
, mAshPointIterator(0)
, mAshMaxParticles(-1)
, mAshQueuedParticles(0)
, mAshSeed(99)
, mIcePointIterator(-1)
, mIceSeed(99)
, mElectricPointIterator(0)
, mElectricSeed(99)
, mElectricColor(CColor::White())
, mImplosionPointIterator(0)
, mImplosionMaxParticles(-1)
, mImplosionQueuedParticles(0)
, mImplosionSeed(99)
, mImplosionPoint(CVector3f::Zero())
, mImplosionClipPlane(0.f, CUnitVector3f(CVector3f::Up()))
, mAshy(parent.mAshy)
, mParticleOffsetScale(1.f, 1.f, 1.f)
, mIceXf(CTransform4f::Identity())
, mParent(&parent)
, mRemTime(10.f)
, mLockDeps(0) {}

CActorModelParticles::CItem::~CItem() {
  if (mSfx) {
    CSfxManager::RemoveEmitter(mSfx);
  }
  for (int i = 0; i < 8; ++i) {
    if (mLockDeps & (1 << i)) {
      mParent->DelTypeRef(static_cast< ESystemTypes >(i));
    }
  }
}

bool CActorModelParticles::CItem::Update(float dt, CStateManager& mgr) {
  // TODO: refresh actor/model state, retire orphaned systems, and update all nine effects.
  return false;
}

bool CActorModelParticles::CItem::UpdateRainSplash(float dt, const CActor* actor,
                                                   CStateManager& mgr) {
  // TODO: update active rain splashes and release the generator when rain stops.
  return false;
}

bool CActorModelParticles::CItem::UpdateElectric(float dt, const CActor* actor,
                                                 CStateManager& mgr) {
  // TODO: load/update electric particles, actor transforms, color and dependency lifetime.
  return false;
}

bool CActorModelParticles::CItem::UpdateIce(float dt, const CActor* actor, CStateManager& mgr) {
  // TODO: queue ice surface points and retire the four ice generators.
  return false;
}

bool CActorModelParticles::CItem::UpdateFirePop(float dt, const CActor* actor) {
  // TODO: create the fire-pop effect at the actor/model bounds and update its lifetime.
  return false;
}

bool CActorModelParticles::CItem::UpdateIcePop(float dt, const CActor* actor) {
  // TODO: create the ice-pop effect at the actor/model bounds and update its lifetime.
  return false;
}

bool CActorModelParticles::CItem::UpdateAshGen(float dt, const CActor* actor, CStateManager& mgr) {
  // TODO: create ash particles and schedule up to sixteen model points per update.
  return false;
}

bool CActorModelParticles::CItem::UpdateImplosion(float dt, const CActor* actor,
                                                  CStateManager& mgr) {
  // TODO: select black-hole/imploder resources, transform the clip plane and queue five points.
  return false;
}

bool CActorModelParticles::CItem::UpdateBurn(float dt, const CActor* actor, CStateManager& mgr) {
  if (actor == nullptr) {
    mAshy.Unlock();
  }
  return mAshy.HasLock();
}

bool CActorModelParticles::CItem::UpdateOnFire(float dt, CActor* actor, CStateManager& mgr) {
  // TODO: manage the eight surface fire generators and the player/multiplayer sound variants.
  return false;
}

void CActorModelParticles::CItem::UseType(ESystemTypes type) {
  const uchar mask = 1 << type;
  if (!(mLockDeps & mask)) {
    mParent->AddTypeRef(type);
    mLockDeps |= mask;
  }
}

void CActorModelParticles::CItem::DontUseType(ESystemTypes type) {
  const uchar mask = 1 << type;
  if (mLockDeps & mask) {
    mParent->DelTypeRef(type);
    mLockDeps &= ~mask;
  }
}

CActorModelParticles::CActorModelParticles()
: mOnFire(gpSimplePool->GetObj(skParticleNames[kST_OnFire]))
, mAsh(gpSimplePool->GetObj(skParticleNames[kST_Ash]))
, mIceBreak(gpSimplePool->GetObj(skParticleNames[kST_Ice]))
, mFirePop(gpSimplePool->GetObj(skParticleNames[kST_FirePop]))
, mIcePop(gpSimplePool->GetObj(skParticleNames[kST_IcePop]))
, mBlackHole(gpSimplePool->GetObj(skParticleNames[kST_BlackHole]))
, mImploder(gpSimplePool->GetObj(skParticleNames[kST_Imploder]))
, mElectric(gpSimplePool->GetObj(skParticleNames[kST_Electric]))
, mAshy(gpSimplePool->GetObj("TXTR_Ashy")) {
  InitializeSystemTypes();
}

void CActorModelParticles::Update(float dt, CStateManager& mgr) {
  // TODO: update dependency loads and erase finished items, clearing the actor's hook flag.
}

CElementGen* CActorModelParticles::MakeAshGen() { return rs_new CElementGen(mAsh); }

CElementGen* CActorModelParticles::MakeFirePopGen() { return rs_new CElementGen(mFirePop); }

CElementGen* CActorModelParticles::MakeIcePopGen() { return rs_new CElementGen(mIcePop); }

CElementGen* CActorModelParticles::MakeBlackHoleGen() { return rs_new CElementGen(mBlackHole); }

CElementGen* CActorModelParticles::MakeImploderGen() { return rs_new CElementGen(mImploder); }

CParticleElectric* CActorModelParticles::MakeElectricGen() {
  return rs_new CParticleElectric(mElectric);
}

CElementGen* CActorModelParticles::MakeOnFireGen() { return rs_new CElementGen(mOnFire); }

void CActorModelParticles::StartAsh(CActor& actor) {
  // TODO: find/create the actor item and acquire the ash dependency.
}

void CActorModelParticles::StartImplosion(CActor& actor, const CVector3f& point, bool blackHole) {
  // TODO: store the effect point and acquire the black-hole or imploder dependency.
}

void CActorModelParticles::StopImplosion(CActor& actor) {
  // TODO: stop emission and clear the remaining/queued implosion particle counts.
}

void CActorModelParticles::DoFirePop(CActor& actor) {
  // TODO: find/create the actor item and acquire the fire-pop dependency.
}

void CActorModelParticles::StartElectric(CActor& actor) {
  // TODO: acquire the electric dependency or resume the existing generator's emission.
}

void CActorModelParticles::StopElectric(CActor& actor) {
  // TODO: find the existing actor item and stop electric emission.
}

void CActorModelParticles::LightDudeOnFire(CActor& actor) {
  // TODO: acquire the fire dependency and request ignition when its delay has elapsed.
}

void CActorModelParticles::StopFire(CActor& actor) {
  // TODO: stop emission from each existing surface fire generator.
}

void CActorModelParticles::StartRainSplashes(CActor& actor, CStateManager& mgr, int maxSplashes,
                                             int genRate, float minZ) {
  // TODO: create the rain generator using model scale and the fixed splash alpha.
}

void CActorModelParticles::StopRainSplashes(CActor& actor) {
  // TODO: find/create the actor item and release its rain generator.
}

void CActorModelParticles::PointGenerator(const CSkinnedModel& model,
                                          const SSkinningWorkspace& workspace, void* context) {
  // TODO: forward the model's vertex count to the item's GeneratePoints method.
}

static int GetNextBestPt(int start, const CSkinnedModel& model, const SSkinningWorkspace& workspace,
                         int count, CRandom16& random) {
  // TODO: sample ten skinned vertices and select the point farthest from the starting vertex.
  return start;
}

void CActorModelParticles::CItem::GeneratePoints(const CSkinnedModel& model,
                                                 const SSkinningWorkspace& workspace, int count) {
  // TODO: sample skinned positions/normals for fire, ash, implosion, ice, electric and rain
  // effects.
}

void CActorModelParticles::SetupHook(TUniqueId uid) const {
  // TODO: register PointGenerator with the shared skinned-model callback interface.
}

rstl::list< CActorModelParticles::CItem >::iterator
CActorModelParticles::FindOrCreateSystem(CActor& actor) {
  // TODO: honor/set the actor's point-generator flag and insert a new item when absent.
  return mItems.end();
}

rstl::list< CActorModelParticles::CItem >::const_iterator
CActorModelParticles::FindSystem(TUniqueId uid) const {
  for (rstl::list< CItem >::const_iterator it = mItems.begin(); it != mItems.end(); ++it) {
    if (it->mId == uid) {
      return it;
    }
  }
  return mItems.end();
}

rstl::list< CActorModelParticles::CItem >::iterator
CActorModelParticles::FindSystem(TUniqueId uid) {
  for (rstl::list< CItem >::iterator it = mItems.begin(); it != mItems.end(); ++it) {
    if (it->mId == uid) {
      return it;
    }
  }
  return mItems.end();
}

void CActorModelParticles::AddStragglersToRenderer(const CStateManager& mgr) const {
  // TODO: submit particles from visible areas without Prime's thermal-visor branches.
}

void CActorModelParticles::Render(const CStateManager& mgr, const CActor& actor) const {
  // TODO: render the visible actor's generators and restore the model matrix.
}

CElementGen* CActorModelParticles::MakeIceGen() { return rs_new CElementGen(mIceBreak); }

void CActorModelParticles::InitializeSystemTypes() {
  for (int i = 0; i < 8; ++i) {
    const rstl::string name = rstl::string_l(skParticleNames[i]) + rstl::string_l("_DGRP");
    mDgrps.push_back(CSystem(name.data()));
  }
}

void CActorModelParticles::AddTypeRef(ESystemTypes type) {
  mDgrps[type].AddRef();
  const uchar mask = 1 << type;
  if (!(mLoadedDeps & mask)) {
    mLoadingDeps |= mask;
  }
}

void CActorModelParticles::DelTypeRef(ESystemTypes type) {
  CSystem& system = mDgrps[type];
  system.DelRef();
  if (system.mRefCount == 0) {
    const uchar mask = ~(1 << type);
    mLoadingDeps &= mask;
    mLoadedDeps &= mask;
    mJustLoadedDeps &= mask;
  }
}

void CActorModelParticles::UpdateSystemTypes() {
  if (mLoadingDeps == 0) {
    return;
  }
  mJustLoadedDeps = 0;
  for (int i = 0; i < 8; ++i) {
    const uchar mask = 1 << i;
    if (mLoadingDeps & mask) {
      CSystem& system = mDgrps[i];
      system.Update();
      if (system.mLoaded) {
        mJustLoadedDeps |= mask;
        mLoadingDeps &= ~mask;
      }
    }
  }
  mLoadedDeps |= mJustLoadedDeps;
}

void CActorModelParticles::StartBurnDeath(CActor& actor, CStateManager& mgr) {
  // TODO: choose the single/multiplayer burn sound and lock the actor item's ash texture.
}

void CActorModelParticles::StopBurnDeath(CActor& actor) {
  rstl::list< CItem >::iterator it = FindSystem(actor.GetUniqueId());
  if (it != mItems.end()) {
    it->mAshy.Unlock();
  }
}

CTexture* CActorModelParticles::GetAshyTexture(const CActor& actor) const {
  rstl::list< CItem >::const_iterator it = FindSystem(actor.GetUniqueId());
  if (it != mItems.end()) {
    CToken& token = const_cast< CToken& >(it->mAshy);
    if (token.HasLock() && token.IsLoaded()) {
      return *TToken< CTexture >(token);
    }
  }
  return nullptr;
}
