#include "MetroidPrime/CActorModelParticles.hpp"

#include "Kyoto/Animation/CSkinRules.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CRainSplashGenerator.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "rstl/string.hpp"

static const char* const skParticleNames[] = {
    "Effect_OnFire",   "Effect_IceBreak", "Effect_Ash",       "Effect_FirePop",
    "Effect_Electric", "Effect_IcePop",   "Effect_Blackhole", "Effect_Imploder",
};

static bool IsMediumOrLarge(const CActor& actor) {
  if (const CPatterned* patterned = TCastToConstPtr< CPatterned >(&actor)) {
    return patterned->GetCreatureSize() != 0;
  }
  return TCastToConstPtr< CPlayer >(&actor) != nullptr;
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
, mOnFireGens(8,
              rstl::pair< rstl::auto_ptr< CElementGen >, uint >(rstl::auto_ptr< CElementGen >(), 0))
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
  bool active = false;
  CActor* actor = static_cast< CActor* >(mgr.ObjectById(mId));
  if (actor != nullptr && actor->HasModelData()) {
    mParticleOffsetScale = actor->GetModelData()->GetScale();
    mIceXf = actor->GetTransform();
    mAreaId = actor->GetCurrentAreaId();
  } else {
    mId = kInvalidUniqueId;
    mAshMaxParticles = 0;
    mAshQueuedParticles = 0;
    mIcePointIterator = -1;
    if (!mImplosionGen.null()) {
      mImplosionGen->SetParticleEmission(false);
      mImplosionMaxParticles = 0;
      mImplosionQueuedParticles = 0;
    }
    if (!mElectricGen.null()) {
      mElectricGen->SetParticleEmission(false);
    }
    if (mSfx) {
      CSfxManager::RemoveEmitter(mSfx);
      mSfx.Clear();
    }
    mRemTime -= dt;
    if (mRemTime <= 0.f) {
      return false;
    }
  }
  if (UpdateOnFire(dt, actor, mgr)) {
    active = true;
  }
  if (UpdateAshGen(dt, actor, mgr)) {
    active = true;
  }
  if (UpdateIce(dt, actor, mgr)) {
    active = true;
  }
  if (UpdateFirePop(dt, actor)) {
    active = true;
  }
  if (UpdateElectric(dt, actor, mgr)) {
    active = true;
  }
  if (UpdateRainSplash(dt, actor, mgr)) {
    active = true;
  }
  if (UpdateBurn(dt, actor, mgr)) {
    active = true;
  }
  if (UpdateImplosion(dt, actor, mgr)) {
    active = true;
  }
  if (UpdateIcePop(dt, actor)) {
    active = true;
  }
  return active;
}

bool CActorModelParticles::CItem::UpdateRainSplash(float dt, const CActor* actor,
                                                   CStateManager& mgr) {
  if (!mRainSplashGen.null()) {
    if (!mRainSplashGen->IsRaining()) {
      mRainSplashGen = rstl::auto_ptr< CRainSplashGenerator >();
    } else {
      mRainSplashGen->Update(dt, mgr);
      return true;
    }
  }
  return false;
}

bool CActorModelParticles::CItem::UpdateElectric(float dt, const CActor* actor,
                                                 CStateManager& mgr) {
  if (!mElectricGen.null()) {
    if (mElectricGen->IsSystemDeletable()) {
      mElectricGen = rstl::auto_ptr< CParticleElectric >();
    } else {
      if (actor != nullptr && actor->GetActive()) {
        mElectricGen->SetGlobalOrientation(actor->GetTransform().GetRotation());
        mElectricGen->SetGlobalTranslation(actor->GetTranslation());
      }
      if (actor == nullptr || actor->GetActive()) {
        mElectricGen->SetModulationColor(mElectricColor);
        mElectricGen->Update(dt);
        return true;
      }
    }
  } else if (mLockDeps & (1 << kST_Electric)) {
    if (mParent->mLoadedDeps & (1 << kST_Electric)) {
      CParticleElectric* gen = mParent->MakeElectricGen();
      gen->SetModulationColor(mElectricColor);
      mElectricGen = gen;
      mElectricPointIterator = 0;
      mElectricSeed = mgr.Random()->Next();
    }
    return true;
  }
  DontUseType(kST_Electric);
  return false;
}

bool CActorModelParticles::CItem::UpdateIce(float dt, const CActor* actor, CStateManager& mgr) {
  if (mIcePointIterator != -1) {
    return true;
  }
  if (!mIceGens.empty()) {
    bool active = false;
    for (rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 4 >::iterator it = mIceGens.begin();
         it != mIceGens.end(); ++it) {
      CElementGen* gen = it->get();
      if (!gen->IsSystemDeletable()) {
        active = true;
      }
      gen->Update(dt);
    }
    if (!active) {
      mIceGens.clear();
    } else {
      return true;
    }
  } else if ((mLockDeps & (1 << kST_Ice)) && actor != nullptr) {
    if (mParent->mLoadedDeps & (1 << kST_Ice)) {
      mIcePointIterator = 0;
      mIceSeed = mgr.Random()->Next();
    }
    return true;
  }
  DontUseType(kST_Ice);
  return false;
}

bool CActorModelParticles::CItem::UpdateFirePop(float dt, const CActor* actor) {
  if (!mFirePopGen.null()) {
    if (mFirePopGen->IsSystemDeletable()) {
      mFirePopGen = rstl::auto_ptr< CElementGen >();
    } else {
      mFirePopGen->Update(dt);
      return true;
    }
  } else if ((mLockDeps & (1 << kST_FirePop)) && actor != nullptr) {
    if (mParent->mLoadedDeps & (1 << kST_FirePop)) {
      CElementGen* gen = mParent->MakeFirePopGen();
      gen->SetGlobalOrientation(actor->GetTransform());
      if (actor->HasModelData()) {
        gen->SetGlobalTranslation(
            actor->GetModelData()->GetBounds(actor->GetTransform()).GetCenterPoint());
      } else {
        gen->SetGlobalTranslation(actor->GetRenderBoundsCached().GetCenterPoint());
      }
      mFirePopGen = gen;
    }
    return true;
  }
  DontUseType(kST_FirePop);
  return false;
}

bool CActorModelParticles::CItem::UpdateIcePop(float dt, const CActor* actor) {
  if (!mIcePopGen.null()) {
    if (mIcePopGen->IsSystemDeletable()) {
      mIcePopGen = rstl::auto_ptr< CElementGen >();
    } else {
      mIcePopGen->Update(dt);
      return true;
    }
  } else if ((mLockDeps & (1 << kST_IcePop)) && actor != nullptr) {
    if (mParent->mLoadedDeps & (1 << kST_IcePop)) {
      CElementGen* gen = mParent->MakeIcePopGen();
      gen->SetGlobalOrientation(actor->GetTransform());
      if (actor->HasModelData()) {
        gen->SetGlobalTranslation(
            actor->GetModelData()->GetBounds(actor->GetTransform()).GetCenterPoint());
      } else {
        gen->SetGlobalTranslation(actor->GetRenderBoundsCached().GetCenterPoint());
      }
      mIcePopGen = gen;
    }
    return true;
  }
  DontUseType(kST_IcePop);
  return false;
}

bool CActorModelParticles::CItem::UpdateAshGen(float dt, const CActor* actor, CStateManager& mgr) {
  if (!mAshGen.null()) {
    if (mAshMaxParticles == 0 && mAshGen->IsSystemDeletable()) {
      mAshGen = rstl::auto_ptr< CElementGen >();
    } else {
      if (actor != nullptr) {
        mAshGen->SetGlobalOrientAndTrans(actor->GetTransform());
        if (mAshMaxParticles > 0) {
          mAshQueuedParticles = rstl::min_val(16, mAshMaxParticles);
          mAshMaxParticles -= mAshQueuedParticles;
        }
      }
      mAshGen->Update(dt);
      return true;
    }
  } else if ((mLockDeps & (1 << kST_Ash)) && actor != nullptr) {
    if (mParent->mLoadedDeps & (1 << kST_Ash)) {
      CElementGen* gen = mParent->MakeAshGen();
      mAshGen = gen;
      mAshPointIterator = 0;
      gen->SetGlobalOrientAndTrans(actor->GetTransform());
      float scale = IsMediumOrLarge(*actor) ? 1.f : 0.3f;
      mAshMaxParticles = static_cast< uint >(scale * gen->GetMaxParticles());
      mAshSeed = mgr.Random()->Next();
      if (mAshMaxParticles > 0) {
        mAshQueuedParticles = rstl::min_val(16, mAshMaxParticles);
        mAshMaxParticles -= mAshQueuedParticles;
      }
    }
    return true;
  }
  DontUseType(kST_Ash);
  return false;
}

bool CActorModelParticles::CItem::UpdateImplosion(float dt, const CActor* actor,
                                                  CStateManager& mgr) {
  if (!mImplosionGen.null()) {
    if (const CPatterned* patterned = TCastToConstPtr< CPatterned >(actor)) {
      mImplosionClipPlane = patterned->GetIngSnatchingPlane();
      const CTransform4f inverse = patterned->GetTransform().GetInverse();
      const CUnitVector3f normal(inverse.Rotate(mImplosionClipPlane.GetNormal()));
      const CVector3f point =
          inverse * (mImplosionClipPlane.GetConstant() * mImplosionClipPlane.GetNormal());
      mImplosionClipPlane = CPlane(point, normal);
    }
    if (mImplosionMaxParticles == 0 && mImplosionGen->IsSystemDeletable()) {
      mImplosionGen = rstl::auto_ptr< CElementGen >();
    } else {
      if (actor != nullptr) {
        mImplosionGen->SetGlobalOrientAndTrans(actor->GetTransform());
        if (mImplosionMaxParticles > 0) {
          mImplosionQueuedParticles = rstl::min_val(5, mImplosionMaxParticles);
          mImplosionMaxParticles -= mImplosionQueuedParticles;
        }
      }
      mImplosionGen->Update(dt);
      return true;
    }
  } else if ((mLockDeps & ((1 << kST_BlackHole) | (1 << kST_Imploder))) && actor != nullptr) {
    const bool blackHole = (mLockDeps & (1 << kST_BlackHole)) != 0;
    if (mParent->mLoadedDeps & (1 << (blackHole ? kST_BlackHole : kST_Imploder))) {
      CElementGen* gen = blackHole ? mParent->MakeBlackHoleGen() : mParent->MakeImploderGen();
      mImplosionGen = gen;
      mImplosionPointIterator = 0;
      const CTransform4f xf = actor->GetTransform();
      gen->SetGlobalOrientAndTrans(xf);
      CVector3f point = xf.GetInverse() * mImplosionPoint;
      if (!blackHole) {
        if (point.CanBeNormalized()) {
          point.Normalize();
        } else {
          point = CVector3f::Right();
        }
        point *= -1.f;
      }
      gen->SetExternalParam(0, point.GetX());
      gen->SetExternalParam(1, point.GetY());
      gen->SetExternalParam(2, point.GetZ());
      const float scale = IsMediumOrLarge(*actor) ? 1.f : 0.3f;
      mImplosionMaxParticles = static_cast< uint >(scale * gen->GetMaxParticles());
      mImplosionSeed = mgr.Random()->Next();
      if (mImplosionMaxParticles > 0) {
        mImplosionQueuedParticles = rstl::min_val(5, mImplosionMaxParticles);
        mImplosionMaxParticles -= mImplosionQueuedParticles;
      }
    }
    return true;
  }
  DontUseType(kST_BlackHole);
  DontUseType(kST_Imploder);
  return false;
}

bool CActorModelParticles::CItem::UpdateBurn(float dt, const CActor* actor, CStateManager& mgr) {
  if (actor == nullptr) {
    mAshy.Unlock();
  }
  return mAshy.HasLock();
}

bool CActorModelParticles::CItem::UpdateOnFire(float dt, CActor* actor, CStateManager& mgr) {
  bool sfxActive = false;
  bool effectActive = false;
  mOnFireDelayTimer -= dt;
  if (mOnFireDelayTimer < 0.f) {
    mOnFireDelayTimer = 0.f;
  }
  if (mLockDeps & (1 << kST_OnFire)) {
    if (mParent->mLoadedDeps & (1 << kST_OnFire)) {
      if (mOnFire && actor != nullptr) {
        bool create = true;
        if (!mAshGen.null() || mAshy.HasLock()) {
          create = false;
        } else if (!IsMediumOrLarge(*actor)) {
          uchar count = 0;
          for (int i = 0; i < 8; ++i) {
            if (!mOnFireGens[i].first.null()) {
              ++count;
            }
          }
          if (count >= 4) {
            create = false;
          }
        }
        if (create) {
          for (int i = 0; i < 8; ++i) {
            rstl::pair< rstl::auto_ptr< CElementGen >, uint >& pair = mOnFireGens[i];
            if (pair.first.null()) {
              pair.second = mgr.Random()->Next();
              pair.first = mParent->MakeOnFireGen();
              mOnFireDelayTimer = 0.3f;
              break;
            }
          }
        }
        if (!mSfx) {
          short sfx = IsMediumOrLarge(*actor) ? 0x1ce1 : 0x1ce2;
          if (mgr.IsMultiplayer()) {
            sfx = IsMediumOrLarge(*actor) ? 0x2689 : 0x268a;
          } else if (TCastToPtr< CPlayer >(actor)) {
            sfx = 0x9b;
          }
          mSfx = CSfxManager::AddEmitter(sfx, actor->GetTranslation(),
                                         actor->GetCurrentAreaId().Value(), true, true);
        }
        mOnFire = false;
      }
      for (int i = 0; i < 8; ++i) {
        if (!mOnFireGens[i].first.null()) {
          CElementGen* const gen = mOnFireGens[i].first.get();
          if (gen->IsSystemDeletable()) {
            mOnFireGens[i].first = rstl::auto_ptr< CElementGen >();
          } else {
            if (actor != nullptr) {
              gen->SetGlobalTranslation(actor->GetTranslation());
            }
            gen->Update(dt);
            effectActive = true;
            sfxActive = true;
          }
        }
      }
    } else {
      effectActive = true;
    }
  }
  if (mSfx) {
    if (sfxActive) {
      CSfxManager::UpdateEmitter(mSfx, mIceXf.GetTranslation(), CVector3f::Zero(), 0x7f);
    } else {
      CSfxManager::RemoveEmitter(mSfx);
      mSfx.Clear();
    }
  }
  if (!effectActive) {
    DontUseType(kST_OnFire);
  }
  return effectActive;
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
  UpdateSystemTypes();
  rstl::list< CItem >::iterator it = mItems.begin();
  while (it != mItems.end()) {
    if (!it->Update(dt, mgr)) {
      if (CActor* actor = static_cast< CActor* >(mgr.ObjectById(it->mId))) {
        actor->SetPointGeneratorParticles(false);
      }
      it = mItems.erase(it);
    } else {
      ++it;
    }
  }
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
  rstl::list< CItem >::iterator it = FindOrCreateSystem(actor);
  it->UseType(kST_Ash);
}

void CActorModelParticles::StartImplosion(CActor& actor, const CVector3f& point, bool blackHole) {
  rstl::list< CItem >::iterator it = FindOrCreateSystem(actor);
  it->mImplosionPoint = point;
  it->UseType(blackHole ? kST_BlackHole : kST_Imploder);
}

void CActorModelParticles::StopImplosion(CActor& actor) {
  if (actor.GetPointGeneratorParticles()) {
    rstl::list< CItem >::iterator it = FindSystem(actor.GetUniqueId());
    if (it != mItems.end()) {
      if (!it->mImplosionGen.null()) {
        it->mImplosionGen->SetParticleEmission(false);
      }
      it->mImplosionMaxParticles = 0;
      it->mImplosionQueuedParticles = 0;
    }
  }
}

void CActorModelParticles::DoFirePop(CActor& actor) {
  rstl::list< CItem >::iterator it = FindOrCreateSystem(actor);
  it->UseType(kST_FirePop);
}

void CActorModelParticles::StartElectric(CActor& actor) {
  rstl::list< CItem >::iterator it = FindOrCreateSystem(actor);
  if (it->mElectricGen.get() == nullptr) {
    it->UseType(kST_Electric);
  } else {
    CParticleElectric* gen = it->mElectricGen.get();
    if (!gen->GetParticleEmission()) {
      gen->SetParticleEmission(true);
    }
  }
}

void CActorModelParticles::StopElectric(CActor& actor) {
  if (actor.GetPointGeneratorParticles()) {
    rstl::list< CItem >::iterator it = FindSystem(actor.GetUniqueId());
    if (it != mItems.end() && !it->mElectricGen.null()) {
      it->mElectricGen->SetParticleEmission(false);
    }
  }
}

void CActorModelParticles::LightDudeOnFire(CActor& actor) {
  rstl::list< CItem >::iterator it = FindOrCreateSystem(actor);
  it->UseType(kST_OnFire);
  if (it->mOnFireDelayTimer <= 0.f) {
    it->mOnFire = true;
  }
}

void CActorModelParticles::StopFire(CActor& actor) {
  if (actor.GetPointGeneratorParticles()) {
    rstl::list< CItem >::iterator it = FindSystem(actor.GetUniqueId());
    if (it != mItems.end()) {
      for (int i = 0; i < 8; ++i) {
        CElementGen* gen = it->mOnFireGens[i].first.get();
        if (gen != nullptr) {
          gen->SetParticleEmission(false);
        }
      }
    }
  }
}

void CActorModelParticles::StartRainSplashes(CActor& actor, CStateManager& mgr, int maxSplashes,
                                             int genRate, float minZ) {
  rstl::list< CItem >::iterator it = FindOrCreateSystem(actor);
  if (it->mRainSplashGen.null() && actor.HasModelData()) {
    it->mRainSplashGen = rs_new CRainSplashGenerator(actor.GetModelData()->GetScale(), maxSplashes,
                                                     genRate, minZ, 0.1875f);
  }
}

void CActorModelParticles::StopRainSplashes(CActor& actor) {
  rstl::list< CItem >::iterator it = FindOrCreateSystem(actor);
  if (!it->mRainSplashGen.null()) {
    it->mRainSplashGen = rstl::auto_ptr< CRainSplashGenerator >();
  }
}

void CActorModelParticles::PointGenerator(const CSkinnedModel& model,
                                          const SSkinningWorkspace& workspace, void* context) {
  static_cast< CItem* >(context)->GeneratePoints(model, workspace,
                                                 model.GetSkinRules()->GetNumPoints());
}

static int GetNextBestPt(int start, const CSkinnedModel& model, const SSkinningWorkspace& workspace,
                         int count, CRandom16& random) {
  int best = start;
  const CVector3f& startVec = model.GetSkinnedPosition(workspace, start);
  float maxDistance = 0.f;
  for (int i = 0; i < 10; ++i) {
    const int index = random.Range(0, count - 1);
    const CVector3f& point = model.GetSkinnedPosition(workspace, index);
    const CVector3f& delta = startVec - point;
    const float distance = delta.MagSquared();
    if (distance > maxDistance) {
      best = index;
      maxDistance = distance;
    }
  }
  return best;
}

void CActorModelParticles::CItem::GeneratePoints(const CSkinnedModel& model,
                                                 const SSkinningWorkspace& workspace, int count) {
  if (count == 0) {
    return;
  }
  for (int i = 0; i < 8; ++i) {
    CElementGen* const gen = mOnFireGens[i].first.get();
    if (gen != nullptr) {
      CRandom16 random(mOnFireGens[i].second);
      const float randomValue = random.Float();
      const int index = randomValue * (count - 1);
      gen->SetTranslation(CVector3f::ByElementMultiply(mParticleOffsetScale,
                                                       model.GetSkinnedPosition(workspace, index)));
    }
  }
  if (mAshQueuedParticles > 0) {
    CRandom16 random(mAshSeed);
    int previousIndex = mAshPointIterator;
    while (mAshQueuedParticles-- > 0) {
      const int index = GetNextBestPt(previousIndex, model, workspace, count, random);
      mAshGen->SetTranslation(CVector3f::ByElementMultiply(
          mParticleOffsetScale, model.GetSkinnedPosition(workspace, index)));
      CVector3f normal = model.GetSkinnedNormal(workspace, index);
      normal.SetZ(0.f);
      if (normal.CanBeNormalized()) {
        normal.Normalize();
        const CVector3f& right = CVector3f::Cross(normal, CVector3f::Up());
        CElementGen* gen = mAshGen.get();
        gen->SetOrientation(
            CTransform4f::FromColumns(right, normal, CVector3f::Up(), CVector3f::Zero()));
      }
      mAshGen->ForceParticleCreation(1);
      previousIndex = index;
    }
    mAshSeed = random.GetSeed();
    mAshPointIterator = previousIndex;
  }
  if (mImplosionQueuedParticles > 0) {
    CRandom16 random(mImplosionSeed);
    int previousIndex = mImplosionPointIterator;
    while (mImplosionQueuedParticles-- > 0) {
      const int index = GetNextBestPt(previousIndex, model, workspace, count, random);
      const CVector3f point = CVector3f::ByElementMultiply(
          mParticleOffsetScale, model.GetSkinnedPosition(workspace, index));
      if (mImplosionClipPlane.IsFacing(point)) {
        mImplosionGen->SetTranslation(point);
        CVector3f normal = model.GetSkinnedNormal(workspace, index);
        normal.SetZ(0.f);
        if (normal.CanBeNormalized()) {
          normal.Normalize();
          const CVector3f& right = CVector3f::Cross(normal, CVector3f::Up());
          CElementGen* gen = mImplosionGen.get();
          gen->SetOrientation(
              CTransform4f::FromColumns(right, normal, CVector3f::Up(), CVector3f::Zero()));
        }
        mImplosionGen->ForceParticleCreation(1);
        previousIndex = index;
      }
    }
    mImplosionSeed = random.GetSeed();
    mImplosionPointIterator = previousIndex;
  }
  if (mIcePointIterator != -1) {
    CRandom16 random(mIceSeed);
    CElementGen* gen = mParent->MakeIceGen();
    gen->SetGlobalOrientAndTrans(mIceXf);
    const int index = GetNextBestPt(mIcePointIterator, model, workspace, count, random);
    gen->SetTranslation(CVector3f::ByElementMultiply(mParticleOffsetScale,
                                                     model.GetSkinnedPosition(workspace, index)));
    const CUnitVector3f normal(model.GetSkinnedNormal(workspace, index));
    gen->SetOrientation(CTransform4f::MakeRotationsBasedOnY(normal));
    mIceGens.push_back(gen);
    if (mIceGens.size() == 4) {
      mIcePointIterator = -1;
    } else {
      mIcePointIterator = index;
    }
  }
  if (!mElectricGen.null() && mElectricGen->GetParticleEmission()) {
    CRandom16 random(mElectricSeed);
    const int initialIndex = random.Range(0, count - 1);
    mElectricGen->SetOverrideIPos(CVector3f::ByElementMultiply(
        mParticleOffsetScale, model.GetSkinnedPosition(workspace, initialIndex)));
    const int index = random.Range(0, count - 1);
    mElectricGen->SetOverrideFPos(CVector3f::ByElementMultiply(
        mParticleOffsetScale, model.GetSkinnedPosition(workspace, index)));
    mElectricGen->ForceParticleCreation(1);
    mElectricSeed = random.GetSeed();
    mElectricPointIterator = index;
  }
  if (!mRainSplashGen.null()) {
    mRainSplashGen->GeneratePoints(model, workspace);
  }
}

void CActorModelParticles::SetupHook(TUniqueId uid) const {
  rstl::list< CItem >::const_iterator it = FindSystem(uid);
  if (it != mItems.end()) {
    CSkinnedModel::SetPointGeneratorFunc(const_cast< CItem* >(&*it), PointGenerator);
  }
}

rstl::list< CActorModelParticles::CItem >::iterator
CActorModelParticles::FindOrCreateSystem(CActor& actor) {
  const TUniqueId uid = actor.GetUniqueId();
  if (actor.GetPointGeneratorParticles()) {
    for (rstl::list< CItem >::iterator it = mItems.begin(); it != mItems.end(); ++it) {
      if (it->mId == uid) {
        return it;
      }
    }
  }
  actor.SetPointGeneratorParticles(true);
  return mItems.insert(mItems.begin(), CItem(actor, *this));
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
  for (rstl::list< CItem >::const_iterator it = mItems.begin(); it != mItems.end(); ++it) {
    const CItem& item = *it;
    if (item.mAreaId != kInvalidAreaId) {
      const CGameArea& area = mgr.GetWorld()->GetAreaAlways(item.mAreaId);
      if (!area.IsLoaded() || area.GetOcclusionState() == CGameArea::kOS_Occluded) {
        continue;
      }
    }
    for (int i = 0; i < 8; ++i) {
      if (!item.mOnFireGens[i].first.null()) {
        gpRender->AddParticleGen(*item.mOnFireGens[i].first);
      }
    }
    if (!item.mAshGen.null()) {
      gpRender->AddParticleGen(*item.mAshGen);
    }
    if (!item.mFirePopGen.null()) {
      gpRender->AddParticleGen(*item.mFirePopGen);
    }
    if (!item.mElectricGen.null()) {
      gpRender->AddParticleGen(*item.mElectricGen);
    }
    if (!item.mImplosionGen.null()) {
      gpRender->AddParticleGen(*item.mImplosionGen);
    }
    for (rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 4 >::const_iterator gen =
             item.mIceGens.begin();
         gen != item.mIceGens.end(); ++gen) {
      gpRender->AddParticleGen(**gen);
    }
    if (!item.mIcePopGen.null()) {
      gpRender->AddParticleGen(*item.mIcePopGen);
    }
  }
}

void CActorModelParticles::Render(const CStateManager& mgr, const CActor& actor) const {
  const CTransform4f modelMatrix = CGraphics::GetModelMatrix();
  const TUniqueId uid = actor.GetUniqueId();
  rstl::list< CItem >::const_iterator it = FindSystem(uid);
  if (it == mItems.end()) {
    return;
  }
  const CItem& item = *it;
  if (item.mAreaId != kInvalidAreaId) {
    const CGameArea& area = mgr.GetWorld()->GetAreaAlways(item.mAreaId);
    if (!area.IsLoaded() || area.GetOcclusionState() == CGameArea::kOS_Occluded) {
      return;
    }
  }
  for (int i = 0; i < 8; ++i) {
    if (!item.mOnFireGens[i].first.null()) {
      item.mOnFireGens[i].first->Render();
    }
  }
  if (!item.mAshGen.null()) {
    item.mAshGen->Render();
  }
  if (!item.mFirePopGen.null()) {
    item.mFirePopGen->Render();
  }
  if (!item.mElectricGen.null()) {
    item.mElectricGen->Render();
  }
  if (!item.mImplosionGen.null()) {
    item.mImplosionGen->Render();
  }
  for (rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 4 >::const_iterator gen =
           item.mIceGens.begin();
       gen != item.mIceGens.end(); ++gen) {
    (*gen)->Render();
  }
  if (!item.mRainSplashGen.null() && actor.HasModelData()) {
    item.mRainSplashGen->Draw(actor.GetTransform());
  }
  if (!item.mIcePopGen.null()) {
    item.mIcePopGen->Render();
  }
  CGraphics::SetModelMatrix(modelMatrix);
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
  rstl::list< CItem >::iterator it = FindOrCreateSystem(actor);
  ushort sfx = IsMediumOrLarge(actor) ? 0x1d61 : 0x1d62;
  if (mgr.IsMultiplayer()) {
    if (CPlayer* player = TCastToPtr< CPlayer >(&actor)) {
      sfx = player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed ? 0x2581 : 0x2582;
    } else {
      sfx = IsMediumOrLarge(actor) ? 0x2581 : 0x2582;
    }
  }
  CSfxManager::AddEmitter(sfx, actor.GetTranslation(), actor.GetCurrentAreaId().Value(), true,
                          false);
  it->mAshy.Lock();
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
