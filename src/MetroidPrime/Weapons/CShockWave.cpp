#include "MetroidPrime/Weapons/CShockWave.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "rstl/math.hpp"

static EMaterialTypes DamageMaterial = kMT_Unknown59;
static EMaterialTypes ProjectileMaterial = kMT_Projectile;

CShockWave::CShockWave(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CTransform4f& xf, TUniqueId parent, const CShockWaveInfo& data,
                       float minActiveTime, float knockback)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(ProjectileMaterial),
         CActorParameters::None(), kInvalidUniqueId)
, mParentId(parent)
, mDamageInfo(data.GetDamageInfo())
, mElementGenDesc(gpSimplePool->GetObj(SObjectTag('PART', data.GetParticleDescId())))
, mElementGen(rs_new CElementGen(mElementGenDesc))
, mShockWaveInfo(data)
, mRadius(data.GetInitialRadius())
, mExpansionSpeed(data.GetInitialExpansionSpeed())
, mActiveTime(0.f)
, mMinActiveTime(minActiveTime)
, mKnockback(knockback)
, mTimeSinceHitPlayerInAir(0.f)
, mTimeSinceHitPlayer(0.f)
, mHitPlayerInAir(false)
, mHitPlayer(false)
, mElectricDesc(data.GetWeaponDescId() != kInvalidAssetId
                    ? rstl::optional_object< TToken< CElectricDescription > >(
                          gpSimplePool->GetObj(SObjectTag('ELSC', data.GetWeaponDescId())))
                    : rstl::optional_object_null())
, mLightId(kInvalidUniqueId) {
  mElementGen->SetParticleEmission(true);
  mElementGen->SetOrientation(GetTransform().GetRotation());
  mElementGen->SetGlobalTranslation(GetTranslation());
}

void CShockWave::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    mElementGen->Update(dt);
    mActiveTime += dt;
    mRadius += mExpansionSpeed * dt;
    mExpansionSpeed += dt * mShockWaveInfo.GetSpeedIncrease();
    mElementGen->SetExternalParam(0, mRadius);
    for (int i = 0; i < mElementGen->GetNumSpawnedParticleSystems(); ++i) {
      CParticleGen* gen = mElementGen->SpawnedParticleSystem(i);
      if (gen->Get4CharId() == 'PART') {
        static_cast< CElementGen* >(gen)->SetExternalParam(0, mRadius);
      }
    }

    if (mHitPlayerInAir) {
      mTimeSinceHitPlayerInAir += dt;
      mHitPlayerInAir = false;
    }
    if (mHitPlayer) {
      mTimeSinceHitPlayer += dt;
      mHitPlayer = false;
    }
  }

  if (mElementGen->IsSystemDeletable() && mMinActiveTime > 0.f && mActiveTime >= mMinActiveTime) {
    mgr.DeleteObjectRequest(GetUniqueId());
    return;
  }

  if (mLightId != kInvalidUniqueId) {
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId))) {
      if (GetActive()) {
        light->SetLight(mElementGen->GetLight());
      }
    }
  }
}

void CShockWave::Render(const CStateManager& mgr) const {
  CActor::Render(mgr);
  mElementGen->Render();
}

void CShockWave::AddToRenderer(const CStateManager& mgr) const {
  CActor::AddToRenderer(mgr);
  gpRender->AddParticleGen(*mElementGen);
}

void CShockWave::Touch(CActor& actor, CStateManager& mgr) {
  if (mActiveTime >= mMinActiveTime && mMinActiveTime > 0.f) {
    return;
  }

  bool isParent = actor.GetUniqueId() == mParentId;
  if (const CCollisionActor* collisionActor =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(actor.GetUniqueId()))) {
    isParent = collisionActor->GetOwnerId() == mParentId;
  }
  if (isParent) {
    return;
  }

  const float maxDistance = mRadius * mRadius;
  CVector3f distance = actor.GetTranslation() - GetTranslation();
  const float minDistance =
      maxDistance * mShockWaveInfo.GetWidthPercent() * mShockWaveInfo.GetWidthPercent();
  CDamageInfo damageInfo = mDamageInfo;
  const float knockbackScale = rstl::max_val(1.f - mKnockback * mActiveTime, 0.f);
  const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(actor.GetUniqueId()));
  const bool isPlayerInAir = player && player->GetPlayerMovementState() != NPlayer::kMS_OnGround;
  distance.SetZ(0.f);
  const float distanceSquared = distance.MagSquared();
  if (distanceSquared >= minDistance && distanceSquared <= maxDistance) {
    damageInfo.SetKnockBackPower(knockbackScale * mDamageInfo.GetKnockBackPower());
    bool canDamage = true;
    if (player && (mTimeSinceHitPlayerInAir >= 0.1333f || mTimeSinceHitPlayer >= 0.2666f)) {
      canDamage = false;
    }

    if (canDamage) {
      if (!WasAlreadyDamaged(actor.GetUniqueId())) {
        mgr.ApplyDamage(
            GetUniqueId(), actor.GetUniqueId(), GetUniqueId(), damageInfo,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(DamageMaterial), CMaterialList()),
            CVector3f::Zero());
        if (player && mElectricDesc) {
          const int playerIndex = mgr.MaskUIdNumPlayers(player->GetUniqueId());
          mgr.AddObject(rs_new CHUDBillboardEffect(
              rstl::optional_object_null(), mElectricDesc, mgr.AllocateUniqueId(), true,
              rstl::string_l("VisorElectricFx"),
              CHUDBillboardEffect::GetNearClipDistance(mgr, playerIndex),
              CHUDBillboardEffect::GetScaleForPOV(mgr), playerIndex, CColor::White(),
              CVector3f::One(), CVector3f::Zero(), false));
          CSfxManager::SfxStart(mShockWaveInfo.GetElectrocuteSfx(), 127,
                                player->GetSoundPan(CPlayer::kMSP_4));
        }
        mHitIds.push_back(actor.GetUniqueId());
      } else {
        damageInfo.SetDamage(0.f);
        mgr.ApplyDamage(
            GetUniqueId(), actor.GetUniqueId(), GetUniqueId(), damageInfo,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(DamageMaterial), CMaterialList()),
            CVector3f::Zero());
      }
      if (isPlayerInAir) {
        mHitPlayerInAir = true;
      }
      if (player) {
        mHitPlayer = true;
      }
    }
  }
}

rstl::optional_object< CAABox > CShockWave::GetTouchBounds() const {
  if (mRadius > 0.f) {
    CAABox bounds(CVector3f(-mRadius, -mRadius, 0.f),
                  CVector3f(mRadius, mRadius, mShockWaveInfo.GetHeight()));
    return bounds.GetTransformedAABox(GetTransform());
  }
  return rstl::optional_object_null();
}

bool CShockWave::WasAlreadyDamaged(TUniqueId uid) const {
  for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = mHitIds.begin();
       it != mHitIds.end(); ++it) {
    if (*it == uid) {
      return true;
    }
  }
  return false;
}

void CShockWave::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  const TUniqueId sender = msg.GetUnk();
  switch (message) {
  case kSM_XCRT:
    if (mElementGen->SystemHasLight()) {
      mLightId = mgr.AllocateUniqueId();
      const int partId = mShockWaveInfo.GetParticleDescId();
      mgr.AddObject(rs_new CGameLight(mLightId, GetCurrentAreaId(), GetActive(), rstl::string_l(""),
                                      GetTransform(), GetUniqueId(), mElementGen->GetLight(),
                                      partId, 1, 0.f));
    }
    break;
  case kSM_XDelete:
    if (mLightId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mLightId);
      mLightId = kInvalidUniqueId;
    }
    break;
  default:
    break;
  }

  CActor::AcceptScriptMsg(mgr, msg);
  mgr.SendScriptMsg(mLightId, sender, message, kInvalidUniqueId);
}

CShockWave::~CShockWave() {}
