#include "MetroidPrime/Enemies/CSwampBossStage2.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Factories/CScannableObjectInfo.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSwampBossStage2.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CShockWave.hpp"
#include "REL/REL_Setup.h"
#include "rstl/algorithm.hpp"

static EMaterialTypes PlatformFilterMaterial = kMT_Platform; // Guessed name
static EMaterialTypes PlatformMaterial = kMT_Platform;       // Guessed name
static EMaterialTypes SolidMaterial = kMT_Solid;             // Guessed name

struct SCollisionJoint {
  const char* name;
  float radius;
};

static const SCollisionJoint skCollisionJoints[10] = {
    {"L_backWing_lockon_SDK_LCTR", 3.f},
    {"L_frontWing_lockon_SDK_LCTR", 3.f},
    {"R_backWing_lockon_SDK_LCTR", 3.f},
    {"R_frontWing_lockon_SDK_LCTR", 3.f},
    {"spine_5_scale2", 5.f},
    {"L_mouth", 5.f},
    {"spine6_Lockon_LCTR", 3.f},
    {"spine_5", 4.f},
    {"spine_3", 3.f},
    {"spine_1", 3.f},
};

static const char* const skWingEffectNames[4] = {"WingTarget_LB", "WingTarget_LF", "WingTarget_RB",
                                                 "WingTarget_RF"};

static const char* const skWingLocatorNames[4] = {"L_backWing_1", "L_frontWing_1", "R_backWing_1",
                                                  "R_frontWing_1"};

CSwampBossStage2::CSwampBossStage2(const TUniqueId& uid, const rstl::string& name,
                                   CEntityInfo& info, const CTransform4f& xf,
                                   const CModelData& modelData, const CActorParameters& actorParams,
                                   const CPatternedInfo& patternedInfo,
                                   const SLdrSwampBossStage2Data& swampBossStage2Properties)
: CPatterned(kPAI_SwampBossStage2, uid, name, kFT_Zero, info, xf, modelData, patternedInfo,
             kMT_Flyer, kCT_One, kBT_AiMovedFlyer, actorParams)
, mProperties(swampBossStage2Properties)
, mCollisionManager()
, mMouthPosition(CVector3f::Zero())
, mSpine5Position(CVector3f::Zero())
, mSpine6Position(CVector3f::Zero())
, mWingLocators(4, CSegId::Null())
, mWingModels(4, rstl::optional_object< CModelData >())
, mWingHealth(4, swampBossStage2Properties.wingGrowthHealth)
, mWingHitTimers(4, 0.f)
, xc34_(3)
, xc38_()
, xc48_(kInvalidUniqueId)
, mLightFlyerHealth(swampBossStage2Properties.lightFlyer1.health)
, mDarkFlyerHealth(swampBossStage2Properties.darkFlyer1.health)
, xc54_()
, xc64_(kInvalidUniqueId)
, xc66_(kInvalidUniqueId)
, xc68_(CVector3f::Zero())
, xc74_24_(false)
, xc74_25_(false)
, xc74_26_(false)
, xc74_27_(false)
, xc74_28_(false)
, xc74_29_(false)
, xc74_30_(false)
, xc75_25_(false)
, xc75_27_(true)
, xc75_30_(false)
, xc75_31_(false)
, xc76_25_(false)
, xc76_26_(false)
, xc76_28_(false)
, xc76_29_(false)
, xc76_30_(false)
, xc76_31_(false)
, xc77_24_(false)
, xc77_28_(false)
, xc77_29_(false)
, xc77_30_(false)
, mAdditiveReactionAnim(0)
, xc7c_(0.f)
, xc80_(0.f)
, xc8c_(0)
, xc90_(0.f)
, xc94_(0)
, xc98_(0)
, mCurrentAttack(8)
, xca4_(kInvalidUniqueId)
, mSpitProjectile(swampBossStage2Properties.spitProjectile,
                  LdrToDamageInfo(swampBossStage2Properties.spitDamage))
, xcd0_()
, xce0_()
, xcf0_(kInvalidUniqueId)
, xcf4_(1)
, xcf8_(20.f)
, mSplashEffect(gpSimplePool->GetObj(SObjectTag('PART', swampBossStage2Properties.splash)))
, mWingDamageEffect(
      gpSimplePool->GetObj(SObjectTag('PART', swampBossStage2Properties.wingDamageEffect)))
, mBlowEffect(gpSimplePool->GetObj(SObjectTag('PART', swampBossStage2Properties.blowEffect)))
, xd20_(CVector3f::Zero())
, xd2c_(CVector3f::Zero())
, xd38_(0.f)
, xd3c_(kInvalidUniqueId)
, mSwoopDamage(LdrToDamageInfo(swampBossStage2Properties.swoopDamage))
, xd5c_(0.f)
, xd60_(CVector3f::Zero())
, xd6c_(500.f)
, xd74_(50.f)
, xd78_(0)
, xd7c_(0.f)
, xd84_(0)
, xd88_(0.f)
, xd8c_(0.f)
, xd94_()
, xdac_()
, xdbc_()
, xdcc_(0.2f)
, xdd0_(kInvalidUniqueId)
, xdd4_()
, xde4_()
, xdf4_(CVector3f::Zero())
, xe00_(0)
, mScanInfoLight(nullptr)
, mScanInfoDark(nullptr)
, xe0c_(xf.GetForward())
, xe18_(CVector3f::Zero())
, xe24_(0.f)
, xe28_(0.f)
, xe2c_(CDamageVulnerability::NormalVulnerabilty())
, xe5c_(CDamageVulnerability::NormalVulnerabilty())
, xe8c_()
, mSpitVisorEffect()
, mMissileRepeller(swampBossStage2Properties.unknown_0x9347820e.range,
                   swampBossStage2Properties.unknown_0x9347820e.turnRate,
                   swampBossStage2Properties.unknown_0x9347820e.soundEffect,
                   swampBossStage2Properties.unknown_0x9347820e.warpScale,
                   swampBossStage2Properties.unknown_0x9347820e.repelOffset)
, mBlowDamage(LdrToDamageInfo(swampBossStage2Properties.blowDamage))
, xf00_(kInvalidUniqueId)
, xf04_(0.f)
, mWaterSfxHandle()
, xf0c_(0.f)
, xf10_(0.f)
, xf14_(4, false)
, xf1c_(kInvalidUniqueId) {
  mSpitProjectile.Token().Lock();
  if (mProperties.spitVisorEffect != kInvalidAssetId) {
    mSpitVisorEffect = TLockedToken< CGenDescription >(
        gpSimplePool->GetObj(SObjectTag('PART', mProperties.spitVisorEffect)));
  }
  KnockBackController().EnableKnockBackPhysics(false);
  KnockBackController().EnableAllAnimReactions(false);
  SetupWingLocators();
  SetupAdditiveReaction();
  SetupScanInfos();
}

void CSwampBossStage2::SetupWingLocators() {
  CAnimData* animData = AnimationData();
  for (int i = 0; i < 4; ++i) {
    mWingLocators[i] = animData->GetLocatorSegId(rstl::string_l(skWingLocatorNames[i]));
  }

  const CVector3f scale = ModelData()->GetScale();
  if (mProperties.wingGrowthLF != kInvalidAssetId) {
    mWingModels[1] = CModelData(CStaticRes(mProperties.wingGrowthLF, scale));
  }
  if (mProperties.wingGrowthLB != kInvalidAssetId) {
    mWingModels[0] = CModelData(CStaticRes(mProperties.wingGrowthLB, scale));
  }
  if (mProperties.wingGrowthRF != kInvalidAssetId) {
    mWingModels[3] = CModelData(CStaticRes(mProperties.wingGrowthRF, scale));
  }
  if (mProperties.wingGrowthRB != kInvalidAssetId) {
    mWingModels[2] = CModelData(CStaticRes(mProperties.wingGrowthRB, scale));
  }
}

void CSwampBossStage2::SetupAdditiveReaction() {
  const CPASDatabase& db = AnimationData()->GetPASDatabase();
  const CPASAnimParmData parms(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(pas::kART_Five));
  const rstl::pair< float, int > best = db.FindBestAnimation(parms, -1);
  if (best.first > FLT_EPSILON) {
    mAdditiveReactionAnim = best.second;
  }
}

void CSwampBossStage2::SetupScanInfos() {
  if (mProperties.scanInfoLight != kInvalidAssetId) {
    mScanInfoLight = rs_new TCachedToken< CScannableObjectInfo >(
        gpSimplePool->GetObj(SObjectTag('SCAN', mProperties.scanInfoLight)), true);
  }
  if (mProperties.scanInfoDark != kInvalidAssetId) {
    mScanInfoDark = rs_new TCachedToken< CScannableObjectInfo >(
        gpSimplePool->GetObj(SObjectTag('SCAN', mProperties.scanInfoDark)), true);
  }
}

CCollisionActor* CSwampBossStage2::GetCollisionActor(CStateManager& mgr, uint index) {
  const TUniqueId id = mCollisionManager->GetCollisionDescFromIndex(index).GetCollisionActorId();
  return TCastToPtr< CCollisionActor >(mgr.ObjectById(id));
}

void CSwampBossStage2::SetupCollisionActors(CStateManager& mgr) {
  xe2c_ = *CPatterned::GetDamageVulnerability();
  xe5c_ = LdrToDamageVulnerability(mIngPossessionData.ingVulnerability);
  for (uint i = 0; i < mCollisionManager->GetNumCollisionActors(); ++i) {
    CCollisionActor* actor = GetCollisionActor(mgr, i);
    if (actor != nullptr) {
      actor->SetDamageVulnerability(xe2c_);
      *actor->HealthInfo() = *GetHealthInfo();
      actor->HealthInfo()->SetHP(xd6c_);
      CMaterialFilter filter = actor->GetMaterialFilter();
      filter.ExcludeList().Add(PlatformFilterMaterial);
      actor->SetMaterialFilter(filter);
      if (i < 4) {
        actor->AddMaterial(kMT_Orbit, kMT_SeekerTarget, mgr);
      }
    }
  }
}

void CSwampBossStage2::SetupCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > descriptions;
  descriptions.reserve(10);
  CAnimData* animData = AnimationData();
  for (int i = 0; i < 10; ++i) {
    const CSegId segId = animData->GetLocatorSegId(rstl::string_l(skCollisionJoints[i].name));
    CJointCollisionDescription description = CJointCollisionDescription::SphereCollision(
        segId, CVector3f::Zero(), skCollisionJoints[i].radius,
        rstl::string_l(skCollisionJoints[i].name), 1000.f);
    descriptions.push_back(description);
  }
  mCollisionManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), descriptions, false);
  SetupCollisionActors(mgr);
  AddMaterial(kMT_Unknown54, kMT_Scannable, mgr);
  SetMaterialFilter(CMaterialFilter(CMaterialList(), CMaterialList(0x00000000FFFFFFFF),
                                    CMaterialFilter::kFT_Never));
}

const CVector3f& CSwampBossStage2::GetXc64Position(const CStateManager& mgr) const {
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(xc64_))) {
    return actor->GetTranslation();
  }
  return CVector3f::Zero();
}

const CVector3f& CSwampBossStage2::GetXc48Position(const CStateManager& mgr) const {
  if (xc74_24_) {
    return GetXdf4();
  }
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(xc48_))) {
    return actor->GetTranslation();
  }
  return GetTranslation();
}

const CVector3f& CSwampBossStage2::GetXdf4() const { return xdf4_; }

CScannableObjectInfo* CSwampBossStage2::GetScannableObjectInfo() const {
  switch (xc94_) {
  case 0:
  case 2:
    if (mScanInfoLight.get() != nullptr) {
      return mScanInfoLight->GetObject();
    }
    break;
  case 1:
  case 3:
    if (mScanInfoDark.get() != nullptr) {
      return mScanInfoDark->GetObject();
    }
    break;
  }
  return CPatterned::GetScannableObjectInfo();
}

void CSwampBossStage2::BeginFight(CStateManager& mgr) {
  BodyController()->Activate(mgr, pas::kAS_Invalid);
  SetupCollisionManager(mgr);
  mCollisionManager->SetActive(mgr, true);
  SetWingsTargetable(mgr, false);
  const float health = GetHealthInfo()->GetInitialHP();
  mgr.SetBossParams(GetUniqueId(), health, gpStringTable->GetStringIndex("BossSwampBossStage2"));
}

void CSwampBossStage2::UpdateTargetability(CStateManager& mgr) {
  const CVector3f toPlayer = mgr.GetPlayer(0)->GetTranslation() - GetTranslation();
  const bool behind = CVector3f::Dot(toPlayer, GetTransform().GetForward()) < 0.f;
  SetWingsTargetable(mgr, behind);
  if (behind) {
    RemoveMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
  } else {
    AddMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
  }
}

CActor* CSwampBossStage2::FindFirstXc54ActorInFront(CStateManager& mgr) {
  CActor* result = nullptr;
  for (int i = 0; i < xc54_.size(); ++i) {
    if (CActor* actor = TCastToPtr< CActor >(const_cast< CEntity* >(mgr.GetObjectById(xc54_[i])))) {
      if (i == xc54_.size() - 1) {
        result = actor;
        break;
      }
      const CVector3f toActor = actor->GetTranslation() - GetXc64Position(mgr);
      const CVector3f& anchor = GetXc64Position(mgr);
      const CVector3f toOther = GetXc48Position(mgr) - anchor;
      if (CVector3f::Dot(toActor, toOther) < 0.f) {
        result = actor;
        break;
      }
    }
  }
  return result;
}

void CSwampBossStage2::MoveTowardsTarget(float dt) {
  const CVector3f toTarget = xe18_ - GetTranslation();
  const float step = dt * mProperties.hoverSpeed;
  if (xc76_30_) {
    float slowdown = toTarget.Magnitude();
    slowdown *= 0.125f;
    float maxSpeed = mProperties.hoverSpeed;
    if (slowdown < 1.f) {
      maxSpeed *= slowdown;
    }
    xe24_ = xe24_ > maxSpeed ? xe24_ - step : xe24_ + step;
    xe24_ = rstl::min_val(xe24_, mProperties.hoverSpeed);
  } else {
    xe24_ = rstl::max_val(0.f, xe24_ - step);
    if (xe24_ == 0.f) {
      xe0c_ = GetTransform().GetForward();
    }
  }
  xc76_30_ = false;
  if (xe24_ > 0.f && toTarget.IsMagnitudeSafe()) {
    xe0c_.Normalize();
    xe0c_ = CVector3f::Slerp(xe0c_, toTarget.AsNormalized(), CRelAngle::FromDegrees(360.f * dt));
    const CVector3f delta = xe0c_ * (xe24_ * dt);
    MoveToInOneFrameWR(GetTranslation() + delta, dt);
  }
}

void CSwampBossStage2::SetCollisionVulnerabilities(CStateManager& mgr, bool reflect) {
  for (int i = 4; i <= 9; ++i) {
    const CDamageVulnerability& vulnerability =
        reflect ? ((xc94_ & 1) ? xe5c_ : xe2c_) : CDamageVulnerability::ReflectVulnerabilty();
    GetCollisionActor(mgr, i)->SetDamageVulnerability(vulnerability);
  }
}

void CSwampBossStage2::CreateBubbleTelegraph(const CVector3f& position) {
  if (mProperties.bubbleTelegraphEffect != kInvalidAssetId) {
    xe8c_ = rstl::auto_ptr< CParticleGen >(rs_new CElementGen(
        gpSimplePool->GetObj(SObjectTag('PART', mProperties.bubbleTelegraphEffect)),
        CElementGen::kMOT_Normal, CElementGen::kOSF_One));
    xe8c_->SetParticleEmission(true);
    xe8c_->SetGlobalTranslation(CVector3f(position.GetX(), position.GetY(), xd38_));
  }
}

void CSwampBossStage2::AddAdditiveDirectionalReaction(CStateManager& mgr) {
  const CPASAnimParmData parms(pas::kAS_AdditiveDirectionalReaction, CPASAnimParm::FromReal32(90.f),
                               CPASAnimParm::FromEnum(0));
  const rstl::pair< float, int > best =
      AnimationData()->GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
  if (best.first > 0.f) {
    AnimationData()->AddAdditiveAnimation(best.second, 1.f, false, true);
  }
}

const SLdrSwampBossStage2Phase& CSwampBossStage2::GetCurrentPhase() const {
  switch (xc94_) {
  case 0:
    return mProperties.lightFlyer1;
  case 1:
    return mProperties.darkFlyer1;
  case 2:
    return mProperties.lightFlyer2;
  case 3:
    return mProperties.darkFlyer2;
  default:
    return mProperties.lightFlyer1;
  }
}

int CSwampBossStage2::GetPhaseAttack(const SLdrSwampBossStage2Phase& phase, int index) const {
  int attack = kAttack_None;
  switch (index) {
  case 0:
    attack = phase.firstAttack;
    break;
  case 1:
    attack = phase.secondAttack;
    break;
  case 2:
    attack = phase.thirdAttack;
    break;
  case 3:
    attack = phase.fourthAttack;
    break;
  case 4:
    attack = phase.fifthAttack;
    break;
  }
  return attack;
}

void CSwampBossStage2::SelectNextAttack(const SLdrSwampBossStage2Phase& phase) {
  mCurrentAttack = GetPhaseAttack(phase, xc98_);
  if (mCurrentAttack == kAttack_None) {
    xc98_ = 0;
    mCurrentAttack = GetPhaseAttack(phase, xc98_);
  }
  ++xc98_;
}

float CSwampBossStage2::SumXd94() const {
  float sum = 0.f;
  for (rstl::list< rstl::pair< float, float > >::const_iterator it = xd94_.begin();
       it != xd94_.end(); ++it) {
    sum += it->second;
  }
  return sum;
}

void CSwampBossStage2::UpdateXd94(float dt) {
  rstl::list< rstl::pair< float, float > >::iterator it = xd94_.begin();
  while (it != xd94_.end()) {
    it->first -= dt;
    if (it->first <= 0.f) {
      it = xd94_.erase(it);
    } else {
      ++it;
    }
  }
}

const CScriptGrapplePoint* CSwampBossStage2::GetOrbitGrapplePoint(const CStateManager& mgr) const {
  const TUniqueId target = mgr.GetPlayer(0)->GetOrbitTargetId();
  return TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(target));
}

void CSwampBossStage2::UpdateBossHealth() {
  const float initialHealth = GetHealthInfo()->GetInitialHP();
  float ratio = 0.f;
  switch (xc94_) {
  case 0:
  case 2:
    for (int i = 0; i < 4; ++i) {
      if (mWingHealth[i] > 0.f) {
        ratio += mWingHealth[i];
      }
    }
    ratio /= 4.f * mProperties.wingGrowthHealth;
    break;
  case 1:
  case 3:
    ratio = mDarkFlyerHealth / GetCurrentPhase().health;
    break;
  default:
    break;
  }
  const float fraction = 0.25f * (static_cast< float >(3 - xc94_) + ratio);
  HealthInfo()->SetHP(fraction * initialHealth);
}

void CSwampBossStage2::SpawnSplashShockWave(CStateManager& mgr, CVector3f position) {
  if (mProperties.splashShockWave.shockWaveEffect == kInvalidAssetId) {
    return;
  }
  CTransform4f xf = GetTransform();
  const float x = position.GetX();
  const float y = position.GetY();
  xf.SetTranslation(CVector3f(x, y, GetXc64Position(mgr).GetZ()));
  CShockWave* shockWave =
      rs_new CShockWave(mgr.AllocateUniqueId(), rstl::string_l("SwampBossStage2 ShockWave"),
                        CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true), xf,
                        GetUniqueId(), CShockWaveInfo(mProperties.splashShockWave),
                        mProperties.splashShockWaveMaxTime, mProperties.unknown_0xe57ca27c);
  if (shockWave != nullptr) {
    mgr.AddObject(shockWave);
  }
}

void CSwampBossStage2::SpawnBlowEffect(CStateManager& mgr) {
  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(xf00_));
  if (actor == nullptr) {
    return;
  }
  CTransform4f xf = GetTransform();
  CVector3f direction = actor->GetTranslation() - GetTranslation();
  direction.SetZ(0.f);
  direction.Normalize();
  xf.SetTranslation(actor->GetTranslation() - direction * 50.f);
  CExplosion* explosion = rs_new CExplosion(
      mBlowEffect, mgr.AllocateUniqueId(),
      CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true),
      rstl::string_l("Chykka wind effect"), xf, 0, CVector3f(3.f, 3.f, 3.f), CColor::White(), -1);
  if (explosion != nullptr) {
    mgr.AddObject(*explosion);
  }
}

void CSwampBossStage2::SpawnWingDamageEffect(CStateManager& mgr, int index) {
  const CTransform4f xf = GetLctrTransform(rstl::string_l(skWingLocatorNames[index]));
  const float scale = ModelData()->GetScale().GetX();
  CExplosion* explosion =
      rs_new CExplosion(mWingDamageEffect, mgr.AllocateUniqueId(),
                        CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true),
                        rstl::string_l("Chykka wing damage effect"), xf, 0,
                        CVector3f(scale, scale, scale), CColor::White(), -1);
  if (explosion != nullptr) {
    mgr.AddObject(*explosion);
  }
}

void CSwampBossStage2::UpdateWaterEffects(CStateManager& mgr, const CVector3f& position,
                                          CVector3f& lastSplashPosition) {
  const float depth = xd38_ - position.GetZ();
  const bool hasLoopedSound = static_cast< bool >(mWaterSfxHandle);
  bool inShallowWater = false;
  if (depth > 0.f) {
    if (depth < 4.f) {
      if (!hasLoopedSound && !xc77_28_) {
        PlayCustomSound(position, GetTransform().GetForward(), mProperties.audioPlaybackParms,
                        false);
        mWaterSfxHandle = PlayCustomSound(position, GetTransform().GetForward(),
                                          mProperties.audioPlaybackParms_0x2b3c923a, true);
      }
      inShallowWater = true;
    }
    const float x = position.GetX();
    const float y = position.GetY();
    const float z = 0.01f + xd38_;
    if ((CVector3f(x, y, z) - lastSplashPosition).MagSquared() > 8.f) {
      CTransform4f xf = GetTransform();
      xf.SetTranslation(CVector3f(x, y, z));
      rstl::reserved_vector< TUniqueId, 1024 > nearList;
      bool colliding = false;
      const CAABox bounds =
          CAABox(CVector3f(x - 1.f, y - 1.f, z - 1.f), CVector3f(x + 1.f, y + 1.f, z + 1.f));
      mgr.BuildNearList(nearList, bounds,
                        CMaterialFilter::MakeInclude(CMaterialList(PlatformMaterial)), this);
      if (nearList.size() != 0) {
        const CCollidableAABox primitive = CCollidableAABox(bounds, CMaterialList(kMT_Solid));
        colliding = CGameCollision::DetectDynamicCollisionBoolean(
            primitive, CTransform4f::Identity(), nearList, mgr);
      }
      if (!colliding) {
        const float scale = rstl::min_val(0.2f * depth + 0.8f, 1.6f);
        CExplosion* explosion =
            rs_new CExplosion(mSplashEffect, mgr.AllocateUniqueId(),
                              CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true),
                              rstl::string_l("Chykka head/tail splash"), xf, 0,
                              CVector3f(scale, scale, scale), CColor::White(), -1);
        if (explosion != nullptr) {
          explosion->SetNextDrawNode(xd3c_);
          mgr.AddObject(*explosion);
        }
      }
      lastSplashPosition = CVector3f(x, y, z);
    }
  }
  if (hasLoopedSound && !inShallowWater) {
    CSfxManager::RemoveEmitter(mWaterSfxHandle);
    mWaterSfxHandle = CSfxHandle();
    if (!xc77_28_) {
      PlayCustomSound(position, GetTransform().GetForward(),
                      mProperties.audioPlaybackParms_0xc05d5c7a, false);
    }
  }
  if (xc77_28_ && mWaterSfxHandle) {
    CSfxManager::RemoveEmitter(mWaterSfxHandle);
    mWaterSfxHandle = CSfxHandle();
  }
}

void CSwampBossStage2::UpdateWaterLevel(CStateManager& mgr) {
  const CActor* actor = TCastToPtr< CActor >(const_cast< CEntity* >(mgr.GetObjectById(xc64_)));
  if (actor != nullptr) {
    xd38_ = actor->GetTranslation().GetZ();
    if (!xc54_.empty()) {
      const CActor* first =
          TCastToPtr< CActor >(const_cast< CEntity* >(mgr.GetObjectById(xc54_[0])));
      if (first != nullptr) {
        xcf8_ = (first->GetTranslation() - actor->GetTranslation()).Magnitude();
      }
    }
    const CActor* other = TCastToPtr< CActor >(const_cast< CEntity* >(mgr.GetObjectById(xc66_)));
    if (other != nullptr) {
      xd74_ = (other->GetTranslation() - actor->GetTranslation()).Magnitude();
    }
  }
  const CScriptWater* water =
      TCastToPtr< CScriptWater >(const_cast< CEntity* >(mgr.GetObjectById(xd3c_)));
  if (water != nullptr) {
    xd38_ = water->GetTriggerBoundsWR().GetMaxPoint().GetZ();
  }
}

void CSwampBossStage2::SelectXc38Target(CStateManager& mgr) {
  if (mCurrentAttack == kAttack_Swoop) {
    xc48_ = FindNearestXc54Actor(mgr)->FindConnectedObject(mgr, kSS_Arrived, kSM_Follow);
    return;
  }
  const CVector3f playerPosition = mgr.GetPlayer(0)->GetTranslation();
  float best = 0.f;
  for (int i = 0; i < xc38_.size(); ++i) {
    const CActor* actor = TCastToPtr< CActor >(const_cast< CEntity* >(mgr.GetObjectById(xc38_[i])));
    if (actor != nullptr) {
      const float distance = (playerPosition - actor->GetTranslation()).Magnitude();
      const float score = distance + 0.5f * xcf8_ * mgr.Random()->Float();
      if (score > best) {
        best = score;
        xc48_ = xc38_[i];
      }
    }
  }
}

CActor* CSwampBossStage2::FindNearestXc54Actor(CStateManager& mgr) const {
  const CVector3f playerPosition = mgr.GetPlayer(0)->GetTranslation();
  CActor* nearest = nullptr;
  float nearestDistance = FLT_MAX;
  for (int i = 0; i < xc54_.size(); ++i) {
    CActor* actor = TCastToPtr< CActor >(const_cast< CEntity* >(mgr.GetObjectById(xc54_[i])));
    if (actor != nullptr) {
      const CVector3f offset = playerPosition - actor->GetTranslation();
      const float distance = CVector3f::Dot(offset, offset);
      if (distance < nearestDistance) {
        nearestDistance = distance;
        nearest = actor;
      }
    }
  }
  return nearest;
}

CVector3f CSwampBossStage2::GetPlayerCenter(const CStateManager& mgr) const {
  const CPlayer& player = *mgr.GetPlayer(0);
  const CAABox& bounds = player.GetBaseBoundingBox();
  const float halfHeight = 0.5f * (bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ());
  return player.GetTranslation() + CVector3f::Up() * halfHeight;
}

CVector3f CSwampBossStage2::GetXcf0Position(const CStateManager& mgr) const {
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(xcf0_))) {
    return actor->GetTranslation();
  }
  return GetPlayerCenter(mgr);
}

void CSwampBossStage2::AdvanceXcf0(CStateManager& mgr) {
  if (const CEntity* entity = mgr.GetObjectById(xcf0_)) {
    xcf0_ = entity->FindConnectedObject(mgr, kSS_Arrived, kSM_Next);
  }
}

void CSwampBossStage2::LaunchSpit(CStateManager& mgr, const CInt32POINode& node) {
  const CTransform4f locator = GetLctrTransform(node.GetLocatorName());
  const CPlayer* player = mgr.GetPlayer(0);
  CVector3f target = player->GetTranslation();
  if (mCurrentAttack == kAttack_Barrage) {
    target = mSpitProjectile.PredictInterceptPos(locator.GetTranslation(), GetPlayerCenter(mgr),
                                                 *player, true, xdcc_);
    if (const CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(xc64_))) {
      const float minHeight = 1.f + actor->GetTranslation().GetZ();
      if (target.GetZ() < minHeight) {
        target.SetZ(minHeight);
      }
    }
    if (mgr.Random()->Float() < 0.8f) {
      CVector3f forward = GetTransform().GetForward();
      target += forward * (20.f * (mgr.Random()->Float() - 0.5f));
      const CVector3f right = GetTransform().GetRight();
      target += right * (20.f * (mgr.Random()->Float() - 0.5f));
    }
    ++xcf4_;
  } else {
    target = GetXcf0Position(mgr);
    AdvanceXcf0(mgr);
  }
  const CTransform4f xf = CTransform4f::LookAt(locator.GetTranslation(), target, CVector3f::Up());
  CEnergyProjectile* projectile = LaunchProjectile(
      xf, mgr, 0x40, 0x400, true,
      mSpitVisorEffect.valid()
          ? CImpactVisorEffect::ParticleEffect(mSpitVisorEffect, mProperties.sound_SpitVisor, false)
          : CImpactVisorEffect::None(),
      CVector3f(1.f, 1.f, 1.f));
  if (projectile != nullptr) {
    projectile->SetProjExtent(mProperties.spitProjectileRadius);
  }
}

void CSwampBossStage2::SendXca4Message(CStateManager& mgr) {
  CEntity* entity = mgr.ObjectById(xca4_);
  if (entity != nullptr) {
    entity->AcceptScriptMsg(mgr, CScriptMsg(GetUniqueId(), xca4_, kSM_Decrement));
  }
}

void CSwampBossStage2::ApplyChainPositions(CStateManager& mgr, const rstl::vector< TUniqueId >& ids,
                                           const rstl::vector< CVector3f >& positions) {
  for (int i = 0; i < ids.size(); ++i) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(ids[i]))) {
      actor->SetTranslation(positions[i]);
    }
  }
}

void CSwampBossStage2::CollectChain(CStateManager& mgr, rstl::vector< TUniqueId >& ids,
                                    const CActor& actor, rstl::vector< CVector3f >& positions) {
  ++xc8c_;
  if (positions.size() >= positions.capacity()) {
    positions.reserve(positions.capacity() + 16);
  }
  positions.push_back_unsafe(actor.GetTranslation());
  if (ids.size() >= ids.capacity()) {
    ids.reserve(ids.capacity() + 16);
  }
  ids.push_back_unsafe(actor.GetUniqueId());
  const rstl::vector< TUniqueId > connected =
      actor.FindConnectedObjects(mgr, kSS_Arrived, kSM_Next);
  for (int i = 0; i < connected.size(); ++i) {
    if (rstl::find(ids.begin(), ids.end(), connected[i]) == ids.end()) {
      if (CActor* next = TCastToPtr< CActor >(mgr.ObjectById(connected[i]))) {
        CollectChain(mgr, ids, *next, positions);
      }
    }
  }
  --xc8c_;
}

void CSwampBossStage2::TransformChain(CStateManager& mgr, const rstl::vector< TUniqueId >& ids,
                                      const CVector3f& origin, const CTransform4f& xf) {
  for (int i = 0; i < ids.size(); ++i) {
    CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(ids[i]));
    if (actor != nullptr) {
      actor->SetTranslation(origin + xf * (actor->GetTranslation() - origin));
      const CVector3f& anchor = GetXc64Position(mgr);
      const float z = actor->GetTranslation().GetZ();
      const CVector3f offset = actor->GetTranslation() - anchor;
      if (offset.MagSquared() > xd74_ * xd74_) {
        CVector3f position = anchor + offset.AsNormalized() * xd74_;
        position.SetZ(z);
        actor->SetTranslation(position);
      }
    }
  }
}

void CSwampBossStage2::TransformChainRecursive(CStateManager& mgr, CActor& actor,
                                               const CVector3f& origin, const CTransform4f& xf) {
  ++xc8c_;
  ++xcf4_;
  actor.SetTranslation(origin + xf * (actor.GetTranslation() - origin));
  const rstl::vector< TUniqueId > connected =
      actor.FindConnectedObjects(mgr, kSS_Arrived, kSM_Next);
  for (int i = 0; i < connected.size(); ++i) {
    if (CActor* next = TCastToPtr< CActor >(mgr.ObjectById(connected[i]))) {
      TransformChainRecursive(mgr, *next, origin, xf);
    }
  }
  --xc8c_;
}

void CSwampBossStage2::CollectChainFrom(CStateManager& mgr, TUniqueId id,
                                        rstl::vector< TUniqueId >& ids,
                                        rstl::vector< CVector3f >& positions) {
  if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id))) {
    if (CActor* next = TCastToPtr< CActor >(
            mgr.ObjectById(actor->FindConnectedObject(mgr, kSS_Patrol, kSM_Follow)))) {
      CollectChain(mgr, ids, *next, positions);
    }
  }
}

void CSwampBossStage2::ActivateChain(CStateManager& mgr, TUniqueId id,
                                     const rstl::vector< TUniqueId >& ids,
                                     const rstl::vector< CVector3f >& positions,
                                     const CVector3f& target) {
  CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id));
  if (actor == nullptr) {
    return;
  }
  actor->AcceptScriptMsg(mgr, CScriptMsg(GetUniqueId(), id, kSM_Activate));
  actor->AcceptScriptMsg(mgr, CScriptMsg(GetUniqueId(), id, kSM_Decrement));
  CActor* start =
      TCastToPtr< CActor >(mgr.ObjectById(actor->FindConnectedObject(mgr, kSS_Patrol, kSM_Follow)));
  if (start == nullptr) {
    return;
  }
  ApplyChainPositions(mgr, ids, positions);
  CActor* next =
      TCastToPtr< CActor >(mgr.ObjectById(start->FindConnectedObject(mgr, kSS_Arrived, kSM_Next)));
  if (next == nullptr) {
    return;
  }
  const CVector3f origin = start->GetTranslation();
  const CVector3f& forward = GetTransform().GetForward();
  const float chainAngle =
      static_cast< float >(atan2(next->GetTranslation().GetY() - origin.GetY(),
                                 next->GetTranslation().GetX() - origin.GetX()));
  const float angle = static_cast< float >(atan2(forward.GetY(), forward.GetX())) - chainAngle;
  CTransform4f xf = CTransform4f::RotateZ(CRelAngle::FromRadians(angle));
  xf.SetTranslation(CVector3f(target.GetX() - origin.GetX(), target.GetY() - origin.GetY(), 0.f));
  TransformChain(mgr, ids, origin, xf);
  start->SetTranslation(target);
}

void CSwampBossStage2::PositionChainAtPlayer(CStateManager& mgr,
                                             const rstl::vector< TUniqueId >& ids,
                                             const CVector3f& gunPosition) {
  const int count = ids.size();
  if (count <= 0) {
    return;
  }
  xcf0_ = ids[mgr.Random()->Next() % count];
  CActor* head = TCastToPtr< CActor >(mgr.ObjectById(xcf0_));
  if (head == nullptr) {
    return;
  }
  CEntity* entity = head;
  do {
    entity = mgr.ObjectById(entity->FindConnectedObject(mgr, kSS_Arrived, kSM_Next));
  } while (entity != nullptr && !entity->GetActive());
  if (entity == nullptr) {
    return;
  }
  CActor* actor = TCastToPtr< CActor >(entity);
  CActor* anchor = TCastToPtr< CActor >(mgr.ObjectById(xc64_));
  if (actor != nullptr && anchor != nullptr) {
    const float forwardX = GetTransform().Get01();
    const float forwardY = GetTransform().Get11();
    const float actorAngle =
        static_cast< float >(atan2(actor->GetTransform().Get11(), actor->GetTransform().Get01()));
    const float angle = static_cast< float >(atan2(forwardY, forwardX)) - actorAngle;
    CVector3f target = mSpitProjectile.PredictInterceptPos(gunPosition, GetPlayerCenter(mgr),
                                                           *mgr.GetPlayer(0), true, xdcc_);
    const float minHeight = 1.f + anchor->GetTranslation().GetZ();
    if (target.GetZ() < minHeight) {
      target.SetZ(minHeight);
    }
    CTransform4f xf = CTransform4f::RotateZ(CRelAngle::FromRadians(angle));
    xf.SetTranslation(target - actor->GetTranslation());
    xcf4_ = 0;
    const CVector3f origin = actor->GetTranslation();
    TransformChainRecursive(mgr, *head, origin, xf);
    actor->SetTransform(GetTransform());
    actor->SetTranslation(target);
  }
}

void CSwampBossStage2::UpdateAdditiveReaction(float dt, CStateManager& mgr) {
  bool changed = false;
  if (xc80_ > 0.f) {
    if (xc7c_ < 0.85f) {
      xc7c_ = rstl::min_val(dt * xc80_ + xc7c_, 0.85f);
      changed = true;
    } else {
      xc80_ = 0.f;
    }
  } else if (xc80_ < 0.f) {
    if (xc7c_ > 0.f) {
      xc7c_ = rstl::max_val(0.f, dt * xc80_ + xc7c_);
      changed = true;
    } else {
      xc80_ = 0.f;
    }
  }
  float wobble = 0.f;
  if (xe28_ > 0.f) {
    const float phase = xe28_ / 0.3f;
    wobble = CMath::FastSinR(M_PIF / 180.f * (360.f * (phase * phase)));
    wobble = 0.15f * wobble;
    xe28_ = rstl::max_val(0.f, xe28_ - dt);
    changed = true;
  }
  if (changed) {
    AnimationData()->AddAdditiveAnimation(mAdditiveReactionAnim, xc7c_ + wobble, false, false);
    if (CCollisionActor* actor = GetCollisionActor(mgr, 4)) {
      actor->SetSphereRadius((2.f * xc7c_ + 5.f) * ModelData()->GetScale().GetY());
    }
  } else if (xc7c_ == 0.f) {
    AnimationData()->DelAdditiveAnimationImmediately(mAdditiveReactionAnim);
  }
}

void CSwampBossStage2::SetWingEffectState(CStateManager& mgr, bool active) {
  for (int i = 0; i < 4; ++i) {
    if (!active || mWingHealth[i] > 0.f) {
      AnimationData()->SetEffectState(rstl::string_l(skWingEffectNames[i]), active, mgr);
    }
  }
}

void CSwampBossStage2::SetWingsTargetable(CStateManager& mgr, bool targetable) {
  for (int i = 0; i < 4; ++i) {
    if (!targetable || mWingHealth[i] > 0.f) {
      SetWingTargetable(mgr, i, targetable);
    }
  }
}

void CSwampBossStage2::SetWingTargetable(CStateManager& mgr, int index, bool targetable) {
  GetCollisionActor(mgr, index)->SetActive(targetable);
}

CVector3f CSwampBossStage2::GetIngSnatchingPoint(float t) const {
  const CAABox bounds = GetModelData()->GetBounds();
  const float height = bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ();
  const CVector3f normal = GetIngSnatchingNormal(t);
  const float offset = (2.f + xc88_) * GetModelData()->GetScale().GetZ() - height * t;
  return mSpine6Position + normal * offset;
}

CVector3f CSwampBossStage2::GetIngSnatchingNormal(float t) const {
  const float scale = (1.f - t) * xc84_;
  CVector3f offset = GetTransform().GetRight() * scale;
  return (CVector3f::Down() + offset).AsNormalized();
}

CVector3f CSwampBossStage2::GetXc54SwoopDirection(CStateManager& mgr) const {
  const CVector3f anchor = GetXc64Position(mgr);
  int best = 0;
  float bestDot = FLT_MAX;
  for (int i = 0; i < xc54_.size(); ++i) {
    const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(xc54_[i]));
    const CVector3f toAnchor = anchor - actor->GetTranslation();
    const float dot = CVector3f::Dot(toAnchor, GetTransform().GetForward());
    if (dot < bestDot) {
      bestDot = dot;
      best = i;
    }
  }
  int index = mgr.Random()->Next() % 2;
  if (index == best) {
    ++index;
  }
  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(xc54_[index]));
  return (anchor - actor->GetTranslation()).AsNormalized();
}

void CSwampBossStage2::SetMoveTarget(const CVector3f& position, float dt) {
  xe18_ = position;
  xc76_30_ = true;
}

CVector3f CSwampBossStage2::GetAimDirection(const CStateManager& mgr) const {
  const CVector3f aim = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f);
  CVector3f direction = aim - GetTransform().GetTranslation();
  return direction.AsNormalized();
}

CVector3f CSwampBossStage2::GetCollisionActorPosition(CStateManager& mgr, int index) {
  if (const CCollisionActor* actor = GetCollisionActor(mgr, index)) {
    return actor->GetTranslation();
  }
  return CVector3f::Zero();
}

bool CSwampBossStage2::StateOver(CStateManager& mgr, const CTriggerData& data) const {
  return mAnimationState.IsOver();
}

bool CSwampBossStage2::AllGrowthsDead(CStateManager& mgr, const CTriggerData& data) const {
  for (int i = 0; i < 4; ++i) {
    if (mWingHealth[i] > 0.f) {
      return false;
    }
  }
  return true;
}

bool CSwampBossStage2::OvipositorDead(CStateManager& mgr, const CTriggerData& data) const {
  return mDarkFlyerHealth <= 0.f;
}

bool CSwampBossStage2::ZeroHealth(CStateManager& mgr, const CTriggerData& data) const {
  return xc94_ >= mProperties.unknown_0x96ce7897 * 2;
}

bool CSwampBossStage2::PlayerInWater(CStateManager& mgr, const CTriggerData& data) const {
  return xc75_24_ && xc75_25_;
}

bool CSwampBossStage2::ShouldSpit(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  if (xc75_24_) {
    if (mCurrentAttack == kAttack_Spit0 || mCurrentAttack == kAttack_Spit1 ||
        mCurrentAttack == kAttack_Spit2) {
      result = true;
    }
  }
  return result;
}

bool CSwampBossStage2::ShouldBarrage(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  if (xc75_24_ && mCurrentAttack == kAttack_Barrage) {
    result = true;
  }
  return result;
}

bool CSwampBossStage2::ShouldTaunt(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  if (xc75_24_ && mCurrentAttack == kAttack_Taunt) {
    result = true;
  }
  return result;
}

bool CSwampBossStage2::ShouldSwoop(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  if (xc75_24_ && mCurrentAttack == kAttack_Swoop) {
    result = true;
  }
  return result;
}

bool CSwampBossStage2::SwoopOver(CStateManager& mgr, const CTriggerData& data) const {
  return xc76_26_;
}

bool CSwampBossStage2::CircleDamage(CStateManager& mgr, const CTriggerData& data) const {
  return xc75_31_;
}

bool CSwampBossStage2::CircleOver(CStateManager& mgr, const CTriggerData& data) const {
  bool locomotion = false;
  if (GetBodyController()->GetCurrentStateId() == pas::kAS_Locomotion) {
    locomotion = true;
  }
  bool result = false;
  if (locomotion || xc76_28_) {
    result = true;
  }
  return result;
}

bool CSwampBossStage2::ShouldBlow(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  if (xc75_24_ && mCurrentAttack == kAttack_Blow) {
    result = true;
  }
  return result;
}

bool CSwampBossStage2::IsLevel(CStateManager& mgr, const CTriggerData& data) const {
  const bool locomotion = GetBodyController()->GetCurrentStateId() == pas::kAS_Locomotion;
  const CVector3f toPlatform = GetXc48Position(mgr) - GetTranslation();
  const float heightDifference = static_cast< float >(fabs(toPlatform.GetZ()));
  const float distance = toPlatform.Magnitude();
  const CVector2f targetDirection = (xd60_ - GetTranslation()).ToVec2f().AsNormalized();
  const CVector2f facing = GetTransform().GetForward().ToVec2f();
  const float alignment = CVector2f::Dot(targetDirection, facing);
  static const float skMinAlignment = CMath::FastCosR(0.034906585f);
  bool level = false;
  if (locomotion) {
    if (xe24_ < 0.2f && heightDifference < 1.f && distance < 2.f && alignment > skMinAlignment) {
      level = true;
    }
  }
  return level;
}

bool CSwampBossStage2::SwitchedPlatform(CStateManager& mgr, const CTriggerData& data) const {
  const CActor* platform = TCastToConstPtr< CActor >(mgr.GetObjectById(xf00_));
  return platform != FindNearestXc54Actor(mgr);
}

bool CSwampBossStage2::Stunned(CStateManager& mgr, const CTriggerData& data) const {
  return mLightFlyerHealth <= 0.f;
}

bool CSwampBossStage2::StunTimeOut(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  if (mStateMachine->GetTime() > mProperties.stunTime && xd80_ == 0.f) {
    result = true;
  }
  return result;
}

bool CSwampBossStage2::OneGrowthDead(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  if (xc74_26_ && xd80_ > 0.3f) {
    result = true;
  }
  return result;
}

bool CSwampBossStage2::ShouldDropFlyer(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  if (xc75_24_ && mCurrentAttack == kAttack_DropFlyer) {
    result = true;
  }
  return result;
}

void CSwampBossStage2::Start(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mgr.GetPlayer(0)->OverrideRadarRadius(mProperties.radarRange, mProperties.unknown_0xfe97e835);
  }
}

void CSwampBossStage2::ResetAttackPhase(CStateManager& mgr, float dt) { xc98_ = 0; }

void CSwampBossStage2::DropFlyer(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    xd78_ = GetCurrentPhase().unknown_0x2e7e55f2;
    xf0c_ = mProperties.unknown_0x8fe0bf01;
    break;
  case kStateMsg_Update:
    BodyController()->FaceDirection(GetXc64Position(mgr) - GetTranslation(), dt);
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_LoopAttack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_One, true));
    } else if (xd78_ <= 0) {
      BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSwampBossStage2::Flinch(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Step)) {
      BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Backward, pas::kStep_Dodge));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    SetCollisionVulnerabilities(mgr, true);
    break;
  }
}

void CSwampBossStage2::Recover(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Step)) {
      BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Forward, pas::kStep_Dodge));
    } else if (mAnimationState.IsOver()) {
      SendScriptMsgs(kSS_InternalState11, mgr);
      SetCollisionVulnerabilities(mgr, true);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSwampBossStage2::Circle(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    xc75_27_ = false;
    xc76_28_ = false;
    break;
  case kStateMsg_Update:
    if ((xc94_ == 0 && xd88_ > mProperties.unknown_0xf55924da) ||
        (xc94_ == 2 && xd88_ > mProperties.unknown_0x83bc1de7)) {
      if (xc76_26_) {
        xc75_31_ = true;
        xc76_28_ = true;
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    if (xc76_28_) {
      const pas::EGenerateType type = xc76_29_ ? pas::kGType_Three : pas::kGType_Four;
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(type, -1));
    }
    xc75_27_ = true;
    xc76_26_ = false;
    break;
  }
}

void CSwampBossStage2::Level(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CActor* actor = FindFirstXc54ActorInFront(mgr);
    xf00_ = actor->GetUniqueId();
    xd60_ = actor->GetTranslation();
    break;
  }
  case kStateMsg_Update:
    BodyController()->FaceDirection(xd60_ - GetTranslation(), dt);
    SetMoveTarget(GetXc48Position(mgr), dt);
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CSwampBossStage2::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Taunt)) {
      BodyController()->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Two));
    }
    SetMoveTarget(GetXc48Position(mgr), dt);
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSwampBossStage2::SpitBarrage(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    xcf4_ = 0;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_LoopAttack)) {
      const pas::ELoopAttackType type = xc74_24_ ? pas::kLAT_Two : pas::kLAT_Zero;
      BodyController()->CommandMgr().DeliverCmd(CBCLoopAttackCmd(type, true));
    } else if (xcf4_ == GetCurrentPhase().unknown_0x2b0bfd51) {
      BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    }
    BodyController()->CommandMgr().SetTargetVector(GetAimDirection(mgr));
    SetMoveTarget(GetXc48Position(mgr), dt);
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSwampBossStage2::SpitWater(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    xcf0_ = kInvalidUniqueId;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_ProjectileAttack)) {
      const CPlayer* player = mgr.GetPlayer(0);
      const pas::ESeverity severity =
          mCurrentAttack == kAttack_Spit0
              ? pas::kS_Zero
              : (mCurrentAttack == kAttack_Spit1 ? pas::kS_One : pas::kS_Two);
      BodyController()->CommandMgr().DeliverCmd(
          CBCProjectileAttackCmd(severity, player->GetTranslation(), false));
    }
    BodyController()->CommandMgr().SetTargetVector(GetAimDirection(mgr));
    SetMoveTarget(GetXc48Position(mgr), dt);
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSwampBossStage2::DeathSequence(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mgr.GetPlayer(0)->ClearRadarRadiusOverride();
    mgr.SetBossParams(kInvalidUniqueId, 0.f, 0);
    SendScriptMsgs(kSS_Dead, mgr);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Fall)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCKnockDownCmd(CVector3f::Forward(), pas::kS_One, false));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSwampBossStage2::Reel(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    xc68_ = GetXc54SwoopDirection(mgr);
    SetWingsTargetable(mgr, true);
    SetCollisionVulnerabilities(mgr, false);
    xc74_26_ = false;
    xc74_25_ = true;
    xf10_ = 0.f;
    BodyController()->SetTurnSpeed(BodyController()->GetTurnSpeed() * 0.125f);
    SendScriptMsgs(kSS_InternalState10, mgr);
    mMissileRepeller.SetActive(false);
    xc77_29_ = false;
    for (int i = 0; i < 4; ++i) {
      xf14_[i] = false;
    }
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_LoopReaction)) {
      BodyController()->CommandMgr().DeliverCmd(CBCLoopHitReactionCmd(pas::kRT_Three));
    }
    BodyController()->FaceDirection(xc68_, dt);
    SetMoveTarget(GetXc64Position(mgr), dt);
    UpdateTargetability(mgr);
    if (!xc77_29_) {
      if (mStateMachine->GetTime() > 1.f) {
        SetWingEffectState(mgr, true);
        xc77_29_ = true;
      }
    }
    break;
  case kStateMsg_Deactivate:
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    xc90_ = 0.5f;
    mLightFlyerHealth = GetCurrentPhase().health;
    BodyController()->SetTurnSpeed(8.f * BodyController()->GetTurnSpeed());
    xc98_ = 0;
    AddMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
    mMissileRepeller.SetActive(true);
    SetWingEffectState(mgr, false);
    break;
  }
}

void CSwampBossStage2::Blow(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    xc77_25_ = false;
    xc77_26_ = false;
    xc77_27_ = false;
    break;
  case kStateMsg_Update: {
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_MeleeAttack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_One));
    }
    if (xc77_26_) {
      CPlayer* player = mgr.GetPlayer(0);
      CVector3f delta = player->GetAimPosition(mgr, 0.f) - mMouthPosition;
      const float forwardDot = CVector3f::Dot(delta, GetTransform().GetForward());
      const float rightDot = CVector3f::Dot(delta, GetTransform().GetRight());
      if (forwardDot > 0.f &&
          static_cast< float >(fabs(rightDot)) < 18.f * GetModelData()->GetScale().GetX() &&
          xd8c_ == 0.f) {
        if (!xc77_27_) {
          player->PushSustainedDamage();
          xc77_27_ = true;
          player->DisableControls(mgr, 0xe2, GetUniqueId(), 1.f, CGameHint::kBHT_None);
        }
        const float push =
            (xc77_26_ ? mProperties.blowPush : mProperties.blowTelegraphPush) * player->GetMass();
        player->ApplyImpulseWR(GetTransform().GetForward() * push, CAxisAngle::Identity());
      }
      if (xc77_27_) {
        mgr.ApplyDamage(
            GetUniqueId(), player->GetUniqueId(), GetUniqueId(), CDamageInfo(mBlowDamage, dt),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
            CVector3f::Zero());
        player->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
      }
    }
    if (xc77_27_ && !xc77_26_) {
      mgr.GetPlayer(0)->PopSustainedDamage();
      xc77_27_ = false;
    }
    if (xc77_25_) {
      SpawnBlowEffect(mgr);
      xc77_25_ = false;
    }
    break;
  }
  case kStateMsg_Deactivate:
    if (xc77_27_) {
      mgr.GetPlayer(0)->PopSustainedDamage();
      xc77_27_ = false;
    }
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSwampBossStage2::Swoop(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    xc74_28_ = false;
    xc74_29_ = false;
    xc74_30_ = false;
    xd5c_ = 0.f;
    xc76_26_ = false;
    xc76_27_ = true;
    xc76_29_ = CVector3f::Dot(mgr.GetPlayer(0)->GetTranslation() - GetTranslation(),
                              GetTransform().GetRight()) > 0.f;
    break;
  case kStateMsg_Update: {
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_MeleeAttack)) {
      const pas::ESeverity severity = xc76_29_ ? pas::kS_Zero : pas::kS_Two;
      BodyController()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(severity));
    }
    if (GetTranslation().GetZ() < xd38_ + 5.f) {
      CVector3f position = GetTranslation();
      position.SetZ(xd38_ + 5.f);
      SetTranslation(position);
    }
    CPlayer* player = mgr.GetPlayer(0);
    const CVector3f delta = player->GetAimPosition(mgr, 0.f) - mMouthPosition;
    const float forwardDot = CVector3f::Dot(delta, GetTransform().GetForward());
    const float rightDot = CVector3f::Dot(delta, GetTransform().GetRight());
    if (xc74_28_ && !xc74_29_ && !xc74_30_ && xd8c_ == 0.f && forwardDot > 0.f &&
        forwardDot < 30.f &&
        static_cast< float >(fabs(rightDot)) < 18.f * GetModelData()->GetScale().GetX()) {
      player->PushSustainedDamage();
      player->DisableControls(mgr, 0xe2, GetUniqueId(), mProperties.swoopDamageTime,
                              CGameHint::kBHT_None);
      xd5c_ = mProperties.swoopDamageTime;
      xc74_29_ = true;
    }
    if (xd5c_ > 0.f) {
      if (player->GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
        const CVector3f push =
            rightDot > 0.f ? GetTransform().GetRight() : -GetTransform().GetRight();
        const float force = mProperties.swoopPush * player->GetMass();
        const CVector3f impulse = (GetTransform().GetForward() + push * 0.5f) * force;
        player->ApplyImpulseWR(impulse, CAxisAngle::Identity());
        mgr.ApplyDamage(
            GetUniqueId(), player->GetUniqueId(), GetUniqueId(), CDamageInfo(mSwoopDamage, dt),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
            CVector3f::Zero());
      } else {
        CDamageInfo damage = mSwoopDamage;
        damage.SetDamage(0.5f * damage.GetDamage());
        mgr.ApplyDamage(
            GetUniqueId(), player->GetUniqueId(), GetUniqueId(), CDamageInfo(damage, dt),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
            CVector3f::Zero());
      }
      player->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
      xd5c_ -= dt;
      if (xd5c_ <= 0.f) {
        player->PopSustainedDamage();
        xc74_29_ = false;
        xc74_30_ = true;
      }
    }
    break;
  }
  case kStateMsg_Deactivate:
    xc74_28_ = false;
    if (xc74_29_) {
      mgr.GetPlayer(0)->PopSustainedDamage();
      xc74_29_ = false;
    }
    break;
  }
}

void CSwampBossStage2::SelectXdf4Target(CStateManager& mgr) {
  CActor* source = nullptr;
  CPlayer* player = mgr.GetPlayer(0);
  CActor* nearest = nullptr;
  switch (mCurrentAttack) {
  case kAttack_DropFlyer: {
    const CVector3f& anchor = GetXc64Position(mgr);
    const CVector3f side = CVector3f::Cross(player->GetTranslation() - anchor, CVector3f::Up());
    const CVector3f offset = GetTranslation() - anchor;
    const float playerSide = CVector3f::Dot(side, offset);
    nearest = FindNearestXc54Actor(mgr);
    for (int i = 0; i < xc54_.size(); ++i) {
      if (xc54_[i] != nearest->GetUniqueId()) {
        source = TCastToPtr< CActor >(const_cast< CEntity* >(mgr.GetObjectById(xc54_[i])));
        const CVector3f toSource = source->GetTranslation() - anchor;
        if (playerSide * CVector3f::Dot(side, toSource) < 0.f) {
          break;
        }
      }
    }
    break;
  }
  case kAttack_Barrage:
  case kAttack_Taunt:
  default:
    source = FindNearestXc54Actor(mgr);
    break;
  }
  const rstl::vector< TUniqueId > targets =
      source->FindConnectedObjects(mgr, kSS_Arrived, kSM_Next);
  float best = 0.f;
  for (int i = 0; i < targets.size(); ++i) {
    CActor* actor = TCastToPtr< CActor >(const_cast< CEntity* >(mgr.GetObjectById(targets[i])));
    if (actor != nullptr) {
      if (mCurrentAttack == kAttack_DropFlyer) {
        const float separation = (actor->GetTranslation() - nearest->GetTranslation()).Magnitude();
        if (separation < mProperties.unknown_0x9e116385) {
          continue;
        }
      }
      const float distance = (player->GetTranslation() - actor->GetTranslation()).Magnitude();
      const float score = distance + 0.5f * xcf8_ * mgr.Random()->Float();
      if (score > best) {
        best = score;
        xdf4_ = actor->GetTranslation();
      }
    }
  }
}

void CSwampBossStage2::BecomeDarkFlyer(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    xc74_27_ = false;
    xc75_28_ = mgr.Random()->Float() > 0.5f;
    xc75_29_ = false;
    BodyController()->SetLocomotionType(pas::kLT_Crouch);
    ++xc94_;
    xc98_ = 0;
    xc77_24_ = false;
    mDarkFlyerHealth = GetCurrentPhase().health;
    xf10_ = 0.f;
    xc77_28_ = true;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_KnockBack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCKnockBackCmd(CVector3f::Forward(), pas::kS_One));
    }
    if (xc74_27_) {
      if (!xc75_29_) {
        const CRelAngle angle = CRelAngle::FromDegrees(xc75_28_ ? -70.f : 70.f);
        const CVector3f direction = CTransform4f::RotateZ(angle) * GetAimDirection(mgr);
        BodyController()->FaceDirection(direction, dt);
      } else {
        BodyController()->FaceDirection(GetAimDirection(mgr), dt);
      }
    }
    xc84_ = 0.2f * (mgr.Random()->Float() - 0.5f);
    xc88_ = 0.2f * (mgr.Random()->Float() - 0.5f);
    UpdateWaterEffects(mgr, mMouthPosition, xd2c_);
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    xc77_28_ = false;
    break;
  }
}

void CSwampBossStage2::BecomeLightFlyer(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    for (int i = 0; i < 4; ++i) {
      mWingHealth[i] = mProperties.wingGrowthHealth;
    }
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    ++xc94_;
    xc98_ = 0;
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    xd78_ = 20;
    xc76_31_ = true;
    if (xc94_ == 4) {
      SendScriptMsgs(kSS_Dead, mgr);
    } else {
      SendScriptMsgs(kSS_InternalState13, mgr);
    }
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_KnockBack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCKnockBackCmd(CVector3f::Forward(), pas::kS_Two));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mLightFlyerHealth = GetCurrentPhase().health;
    xc76_31_ = false;
    break;
  }
}

void CSwampBossStage2::DarkLurk(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    const SLdrSwampBossStage2Phase& phase = GetCurrentPhase();
    xca0_ = phase.minTimeBetweenAttacks +
            mgr.Random()->Float() * (phase.maxTimeBetweenAttacks - phase.minTimeBetweenAttacks);
    xc75_24_ = false;
    if (mgr.Random()->Float() < phase.tauntChance) {
      mCurrentAttack = kAttack_Taunt;
    } else {
      SelectNextAttack(phase);
    }
    xd90_ = GetCurrentPhase().unknown_0x29e6ead6 +
            mgr.Random()->Float() *
                (GetCurrentPhase().unknown_0x1753225e - GetCurrentPhase().unknown_0x29e6ead6);
    SelectXdf4Target(mgr);
    break;
  }
  case kStateMsg_Update:
    if (mCurrentAttack == kAttack_DropFlyer) {
      BodyController()->FaceDirection(GetXc64Position(mgr) - GetTranslation(), dt);
    } else {
      BodyController()->FaceDirection(GetAimDirection(mgr), dt);
    }
    SetMoveTarget(GetXc48Position(mgr), dt);
    if (BodyController()->GetCurrentStateId() == pas::kAS_Locomotion) {
      xd90_ -= dt;
      if (xd90_ <= 0.f) {
        SelectXdf4Target(mgr);
        xd90_ = GetCurrentPhase().unknown_0x29e6ead6 +
                mgr.Random()->Float() *
                    (GetCurrentPhase().unknown_0x1753225e - GetCurrentPhase().unknown_0x29e6ead6);
      }
    }
    if (mStateMachine->GetTime() > xca0_ &&
        BodyController()->GetCurrentStateId() == pas::kAS_Locomotion && xf0c_ <= 0.f) {
      xc75_24_ = true;
      const float distance = (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).Magnitude();
      if (mCurrentAttack == kAttack_Barrage && distance < mProperties.unknown_0x5a844633) {
        mCurrentAttack = kAttack_DropFlyer;
      }
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CSwampBossStage2::Lurk(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    const SLdrSwampBossStage2Phase& phase = GetCurrentPhase();
    xca0_ = phase.minTimeBetweenAttacks +
            mgr.Random()->Float() * (phase.maxTimeBetweenAttacks - phase.minTimeBetweenAttacks);
    xc75_24_ = false;
    if (xc75_31_) {
      if (xc94_ == 0 || xc76_24_) {
        mCurrentAttack = kAttack_Barrage;
        xc75_24_ = true;
      } else {
        mCurrentAttack = kAttack_Swoop;
        xc76_24_ = true;
      }
      xc75_31_ = false;
    } else if (xc94_ > 0 && xd84_ < 3 && !xc74_24_ && mCurrentAttack == kAttack_Swoop &&
               !xc74_30_ && !xc75_30_) {
      xc75_24_ = true;
      xc75_30_ = true;
    } else if (mgr.Random()->Float() < phase.tauntChance) {
      mCurrentAttack = kAttack_Taunt;
    } else {
      SelectNextAttack(phase);
      xc75_30_ = false;
      xc76_24_ = false;
    }
    if (mCurrentAttack == kAttack_Swoop) {
      ++xd84_;
    } else {
      xd84_ = 0;
    }
    xc74_31_ = false;
    if (mgr.Random()->Float() < phase.dashChance) {
      xc74_31_ = true;
    }
    xd90_ = GetCurrentPhase().unknown_0x29e6ead6 +
            mgr.Random()->Float() *
                (GetCurrentPhase().unknown_0x1753225e - GetCurrentPhase().unknown_0x29e6ead6);
    SetCollisionVulnerabilities(mgr, true);
    SelectXc38Target(mgr);
    break;
  }
  case kStateMsg_Update:
    BodyController()->FaceDirection(GetAimDirection(mgr), dt);
    if (xc74_31_) {
      xc74_31_ = false;
      BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Up, pas::kStep_Dodge));
    }
    if (BodyController()->GetCurrentStateId() == pas::kAS_Locomotion) {
      SetMoveTarget(GetXc48Position(mgr), dt);
      xd90_ -= dt;
      if (xd90_ <= 0.f) {
        BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Up, pas::kStep_Dodge));
        xd90_ = GetCurrentPhase().unknown_0x29e6ead6 +
                mgr.Random()->Float() *
                    (GetCurrentPhase().unknown_0x1753225e - GetCurrentPhase().unknown_0x29e6ead6);
      }
    }
    if (mStateMachine->GetTime() > xca0_ &&
        BodyController()->GetCurrentStateId() == pas::kAS_Locomotion) {
      xc75_24_ = true;
    }
    if (!xc74_24_ && mCurrentAttack != kAttack_Swoop &&
        SumXd94() > mProperties.unknown_0x7fc50ac2) {
      mCurrentAttack = kAttack_Swoop;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CSwampBossStage2::HandleCollisionActorDamage(CStateManager& mgr, TUniqueId actorId) {
  CCollisionActor* collisionActor = TCastToPtr< CCollisionActor >(mgr.ObjectById(actorId));
  if (collisionActor == nullptr) {
    return;
  }
  const TUniqueId touched = collisionActor->GetLastTouchedObject();
  if (xf1c_ != kInvalidUniqueId && touched == xf1c_) {
    return;
  }
  xf1c_ = touched;
  int index = -1;
  for (uint i = 0; i < mCollisionManager->GetNumCollisionActors(); ++i) {
    if (mCollisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId() == actorId) {
      index = i;
      break;
    }
  }
  CHealthInfo* health = collisionActor->HealthInfo();
  const float damage = xd6c_ - health->GetHP();
  bool hit = false;
  bool wingFlinch = false;
  if (index >= 0 && index <= 3) {
    const CWeapon* weapon =
        TCastToPtr< CWeapon >(const_cast< CEntity* >(mgr.GetObjectById(touched)));
    if (weapon != nullptr) {
      if (CVector3f::Dot(weapon->GetTransform().GetForward(), GetTransform().GetForward()) > 0.f &&
          xc74_25_) {
        hit = true;
        if (mWingHealth[index] > 0.f) {
          mWingHealth[index] -= damage;
          mWingHitTimers[index] = 1.f;
          wingFlinch = true;
          if (damage >= mProperties.breakStunDamage) {
            xf14_[index] = true;
            int brokenCount = 0;
            for (int i = 0; i < 4; ++i) {
              if (xf14_[i]) {
                ++brokenCount;
              }
            }
            if (brokenCount > 1) {
              xc74_26_ = true;
              wingFlinch = false;
            }
          }
          if (damage >= mProperties.stunnedFlinchSoundDamageThreshold) {
            xc77_30_ = true;
          }
          if (mWingHealth[index] <= 0.f) {
            xc74_26_ = true;
            SpawnWingDamageEffect(mgr, index);
            SetWingTargetable(mgr, index, false);
            wingFlinch = false;
            PlayCustomSound(collisionActor->GetTranslation(), CVector3f::Up(),
                            mProperties.audioPlaybackParms_0xbe3d39aa, false);
          } else if (xc77_30_) {
            PlayCustomSound(collisionActor->GetTranslation(), CVector3f::Up(),
                            mProperties.audioPlaybackParms_0x692fa63c, false);
          }
        }
      }
    }
  } else if (xc74_24_) {
    if (index == 4) {
      hit = true;
      mDarkFlyerHealth -= damage;
    }
  } else if (!xc74_25_) {
    mLightFlyerHealth -= damage;
    if (!xc75_27_) {
      mLightFlyerHealth = rstl::max_val(3.f, mLightFlyerHealth);
    }
    hit = true;
    if (xc76_26_) {
      xd88_ += damage;
    }
    xd94_.push_back(rstl::pair< float, float >(mProperties.unknown_0x13448a4a, damage));
  }
  if (hit) {
    if (xc74_24_ || xc74_25_) {
      mDamageColor = skDamageColor;
    } else {
      mDamageColor = skHitsWithoutDamageColor;
    }
    if (!wingFlinch) {
      TakeDamage(GetTransform().GetForward(), damage);
    }
    xf10_ = rstl::max_val(xf10_, rstl::min_val(damage / 20.f, 1.f));
  }
  health->SetHP(xd6c_);
}

void CSwampBossStage2::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const bool wasActive = GetActive();
  const EScriptObjectMessage message = msg.GetMessage();
  CPatterned::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Create:
    if (GetActive()) {
      BeginFight(mgr);
    }
    break;
  case kSM_AreaLoaded:
    xc38_ = FindConnectedObjects(mgr, kSS_InternalState0, kSM_None);
    xc54_ = FindConnectedObjects(mgr, kSS_InternalState5, kSM_None);
    xc64_ = FindConnectedObject(mgr, kSS_InternalState1, kSM_None);
    xc66_ = FindConnectedObject(mgr, kSS_InternalState6, kSM_None);
    xcd0_ = FindConnectedObjects(mgr, kSS_InternalState3, kSM_None);
    xce0_ = FindConnectedObjects(mgr, kSS_InternalState4, kSM_None);
    xd3c_ = FindConnectedObject(mgr, kSS_InternalState12, kSM_None);
    xca4_ = FindConnectedObject(mgr, kSS_InternalState2, kSM_None);
    CollectChainFrom(mgr, xca4_, xdac_, xdbc_);
    xdd0_ = FindConnectedObject(mgr, kSS_InternalState7, kSM_None);
    CollectChainFrom(mgr, xdd0_, xdd4_, xde4_);
    UpdateWaterLevel(mgr);
    break;
  case kSM_Delete:
    if (mWaterSfxHandle) {
      CSfxManager::RemoveEmitter(mWaterSfxHandle);
      mWaterSfxHandle = CSfxHandle();
    }
    mCollisionManager->Destroy(mgr);
    if (xe8c_.get() != nullptr) {
      xe8c_->SetParticleEmission(false);
      xe8c_ = rstl::auto_ptr< CParticleGen >();
    }
    break;
  case kSM_Activate:
    if (!wasActive) {
      BeginFight(mgr);
    }
    break;
  case kSM_Deactivate:
    mCollisionManager->SetActive(mgr, false);
    break;
  case kSM_AIUpdateDisabled:
    if (mCollisionManager.get() != nullptr) {
      mCollisionManager->SetPhysicsActive(mgr, false);
    }
    break;
  case kSM_Damage:
    HandleCollisionActorDamage(mgr, msg.GetSenderId());
    break;
  case kSM_InternalMessage0:
    xc75_25_ = true;
    break;
  default:
    break;
  }
}

void CSwampBossStage2::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }
  mCollisionManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  mMouthPosition = GetCollisionActorPosition(mgr, 5);
  mSpine5Position = GetCollisionActorPosition(mgr, 4);
  mSpine6Position = GetCollisionActorPosition(mgr, 6);
  UpdateAdditiveReaction(dt, mgr);
  MoveTowardsTarget(dt);
  if (xc90_ > 0.f) {
    xc90_ -= dt;
    if (xc90_ <= 0.f) {
      xc74_25_ = false;
      SetWingsTargetable(mgr, false);
    }
  }
  UpdateWaterEffects(mgr, mSpine6Position, xd20_);
  if (xc75_25_ && mgr.GetPlayer(0)->GetRidingPlatform() != kInvalidUniqueId) {
    xc75_25_ = false;
  }
  UpdateXd94(dt);
  if (xc74_24_) {
    xd7c_ = rstl::min_val(xd7c_ + dt, 1.f);
  } else {
    xd7c_ = rstl::max_val(0.f, xd7c_ - dt);
  }
  if (xc74_26_) {
    xd80_ += dt;
  } else {
    xd80_ = 0.f;
  }
  if (GetOrbitGrapplePoint(mgr) != nullptr) {
    xd8c_ += dt;
  } else {
    xd8c_ = 0.f;
  }
  UpdateBossHealth();
  for (int i = 0; i < 4; ++i) {
    mWingHitTimers[i] = rstl::max_val(0.f, mWingHitTimers[i] - dt);
  }
  xdcc_ = dt;
  if (xe8c_.get() != nullptr) {
    xe8c_->Update(dt);
  }
  mMissileRepeller.Update(mgr, *this, dt);
  if (xf0c_ > 0.f) {
    xf0c_ -= dt;
  }
  if (xf10_ > 0.f) {
    BodyController()->CommandMgr().DeliverCmd(CBCAdditiveFlinchCmd(xf10_));
    xf10_ = 0.f;
  }
}

void CSwampBossStage2::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                       EUserEventType type, float dt) {
  bool handled = true;
  switch (type) {
  case kUE_FadeIn:
    xc74_24_ = true;
    SetIngPossessed(true, 15.f, mgr);
    xc80_ = 0.2f;
    break;
  case kUE_FadeOut:
    xc74_24_ = false;
    SetIngPossessed(false, 20.f, mgr);
    xc80_ = -0.5f;
    break;
  case kUE_GenerateEnd:
    xc74_27_ = true;
    if (!xc77_24_) {
      CreateBubbleTelegraph(GetTranslation());
      xc77_24_ = true;
    }
    SetTranslation(CVector3f(GetTranslation().GetX(), GetTranslation().GetY(), xd38_ + 0.01f));
    break;
  case kUE_BreakLockOn:
    mgr.GetPlayer(0)->SetOrbitRequestForTarget(GetUniqueId(), CPlayer::kOR_ActivateOrbitSource,
                                               mgr);
    break;
  case kUE_EventStart: {
    const CVector3f position = GetLctrTransform(node.GetLocatorName()).GetTranslation();
    if (xc76_31_) {
      ActivateChain(mgr, xdd0_, xdd4_, xde4_, position);
    } else {
      ActivateChain(mgr, xca4_, xdac_, xdbc_, position);
    }
    break;
  }
  case kUE_EndAction:
    xc75_29_ = true;
    SetCollisionVulnerabilities(mgr, true);
    break;
  case kUE_EggLay:
    if (xd78_ > 0) {
      const TUniqueId id = xc76_31_ ? xdd0_ : xca4_;
      if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id))) {
        for (int i = 0; i < GetCurrentPhase().unknown_0xef3efec0; ++i) {
          actor->AcceptScriptMsg(mgr, CScriptMsg(GetUniqueId(), id, kSM_InternalMessage0));
          if (--xd78_ <= 0) {
            break;
          }
        }
        if (xe28_ == 0.f) {
          xe28_ = 0.3f;
        }
      }
    }
    break;
  case kUE_Projectile:
    LaunchSpit(mgr, node);
    break;
  case kUE_DamageOn:
    xc74_28_ = true;
    xc75_27_ = false;
    break;
  case kUE_DamageOff:
    xc74_28_ = false;
    break;
  case kUE_EffectOn:
    if (!xc76_26_) {
      xc76_26_ = true;
      xd88_ = 0.f;
      xe00_ = 0;
    } else {
      const CActor* nearest = FindNearestXc54Actor(mgr);
      const CVector3f toNearest = nearest->GetTranslation() - GetXc64Position(mgr);
      const CVector3f toSelf = GetTranslation() - GetXc64Position(mgr);
      if (CVector3f::Dot(toNearest, toSelf) < 0.f) {
        xc76_28_ = true;
      }
      ++xe00_;
    }
    break;
  case kUE_EffectOff:
    xc76_26_ = false;
    break;
  case kUE_Unknown35: {
    SendScriptMsgs(kSS_InternalState8, mgr);
    RemoveMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
    const CVector3f position = GetLctrTransform(node.GetLocatorName()).GetTranslation();
    SpawnSplashShockWave(mgr, position);
    break;
  }
  case kUE_Unknown40:
    if (!xc77_24_) {
      CreateBubbleTelegraph(GetTranslation());
      xc77_24_ = true;
    }
    break;
  case kUE_Unknown36:
    SendScriptMsgs(kSS_InternalState9, mgr);
    AddMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
    break;
  case kUE_Unknown37:
    SelectXc38Target(mgr);
    PositionChainAtPlayer(mgr, xcd0_, mMouthPosition);
    break;
  case kUE_Unknown38:
    SelectXc38Target(mgr);
    PositionChainAtPlayer(mgr, xce0_, mMouthPosition);
    break;
  case kUE_Unknown39:
    xc76_27_ = false;
    break;
  case kUE_Unknown41:
    xc77_25_ = true;
    break;
  case kUE_Unknown42:
    xc77_26_ = true;
    break;
  case kUE_Unknown43:
    xc77_26_ = false;
    break;
  case kUE_Unknown45:
    if (xc74_25_) {
      if (xc74_26_) {
        PlayCustomSound(GetLctrTransform(node.GetLocatorName()).GetTranslation(),
                        GetTransform().GetForward(), mProperties.stunnedReelSound, false);
      } else {
        if (xc77_30_) {
          PlayCustomSound(GetLctrTransform(node.GetLocatorName()).GetTranslation(),
                          GetTransform().GetForward(), mProperties.stunnedFlinchSound, false);
        }
        xc77_30_ = false;
      }
    } else if (mgr.Random()->Float() < mProperties.flinchSoundChance) {
      PlayCustomSound(GetLctrTransform(node.GetLocatorName()).GetTranslation(),
                      GetTransform().GetForward(), mProperties.flinchSound, false);
    }
    break;
  default:
    handled = false;
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CSwampBossStage2::OnScanStateChange(EScanState state, CStateManager& mgr) {
  CPatterned::OnScanStateChange(state, mgr);
}

void CSwampBossStage2::PreRender(CStateManager& mgr) { CPatterned::PreRender(mgr); }

void CSwampBossStage2::AddToRenderer(const CStateManager& mgr) const {
  if (!GetPreRenderClipped()) {
    CPatterned::AddToRenderer(mgr);
    if (IsBeingSnatched() == true) {
      RenderIngSnatchingTransition(mgr);
    } else {
      CPhysicsActor::Render(mgr);
    }
    for (int i = 0; i < 4; ++i) {
      rstl::optional_object< CModelData > model = mWingModels[i];
      if (mWingHealth[i] > 0.f && model.valid()) {
        const CTransform4f xf = GetLctrTransform(mWingLocators[i]);
        const float hitTimer = mWingHitTimers[i];
        if (hitTimer > 0.f) {
          model->Render(mgr, xf, GetActorLights(),
                        CModelFlags(CModelFlags::kT_Two,
                                    CColor::Lerp(CColor::Black(), mDamageColor, hitTimer)));
        } else {
          model->Render(mgr, xf, GetActorLights(), GetModelFlags());
        }
      }
    }
    if (xe8c_.get() != nullptr) {
      gpRender->AddParticleGen(*xe8c_);
    }
  }
}

void CSwampBossStage2::Render(const CStateManager& mgr) const {
  uint mask = 0;
  uint target = 0;
  if (mDrawParticles) {
    mgr.GetCharacterRenderMaskAndTarget(mask, target);
  }
  RenderSystemsToBeDrawnFirst(mgr, mask, target);
  RenderSystemsToBeDrawnLast(mgr, mask, target);
  mMissileRepeller.Render(mgr, *this);
}

CVector3f CSwampBossStage2::GetAimPosition(const CStateManager& mgr, float dt) const {
  return mSpine5Position * xd7c_ + mMouthPosition * (1.f - xd7c_);
}

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"StateOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::StateOver)},
    {"AllGrowthsDead",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::AllGrowthsDead)},
    {"OvipositorDead",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::OvipositorDead)},
    {"ZeroHealth",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::ZeroHealth)},
    {"PlayerInWater",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::PlayerInWater)},
    {"ShouldSpit",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::ShouldSpit)},
    {"ShouldBarrage",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::ShouldBarrage)},
    {"ShouldTaunt",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::ShouldTaunt)},
    {"ShouldSwoop",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::ShouldSwoop)},
    {"SwoopOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::SwoopOver)},
    {"CircleDamage",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::CircleDamage)},
    {"CircleOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::CircleOver)},
    {"ShouldBlow",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::ShouldBlow)},
    {"IsLevel", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::IsLevel)},
    {"SwitchedPlatform",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::SwitchedPlatform)},
    {"Stunned", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::Stunned)},
    {"StunTimeOut",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::StunTimeOut)},
    {"OneGrowthDead",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::OneGrowthDead)},
    {"ShouldDropFlyer",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage2::ShouldDropFlyer)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Start", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::Start)},
    {"BecomeLightFlyer",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::BecomeLightFlyer)},
    {"BecomeDarkFlyer",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::BecomeDarkFlyer)},
    {"DeathSequence",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::DeathSequence)},
    {"Lurk", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::Lurk)},
    {"DarkLurk", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::DarkLurk)},
    {"SpitWater", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::SpitWater)},
    {"SpitBarrage",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::SpitBarrage)},
    {"Taunt", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::Taunt)},
    {"Level", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::Level)},
    {"Swoop", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::Swoop)},
    {"Blow", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::Blow)},
    {"Circle", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::Circle)},
    {"Reel", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::Reel)},
    {"Recover", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::Recover)},
    {"Flinch", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::Flinch)},
    {"DropFlyer", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage2::DropFlyer)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"ResetAttackPhase",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSwampBossStage2::ResetAttackPhase)},
};

void CSwampBossStage2::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  stateMachine->SetCodeFunctions(skCodeFuncs, ARRAY_SIZE(skCodeFuncs));
}

CEntity* LoadSwampBossStage2(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSwampBossStage2 sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSwampBossStage2.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CSwampBossStage2(
      TUniqueId(mgr.AllocateUniqueId()), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToActorParameters(sldrThis.actorInformation),
      LdrToPatternedInfo(sldrThis.patterned, &sldrThis.ingPossessionData),
      sldrThis.swampBossStage2Properties);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SSwampBossStage2_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadSwampBossStage2;
  SetSSwampBossStage2_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSSwampBossStage2_FuncPtrs(nullptr); }
#endif
