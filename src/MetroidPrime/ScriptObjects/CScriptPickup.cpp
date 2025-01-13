#include "MetroidPrime/ScriptObjects/CScriptPickup.hpp"

// #include "MetroidPrime/CAnimData.hpp"
// #include "MetroidPrime/CAnimPlaybackParms.hpp"
// #include "MetroidPrime/CArtifactDoll.hpp"
// #include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CEchoParameters.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"

// #include "MetroidPrime/Cameras/CCameraManager.hpp"
// #include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
// #include "MetroidPrime/Player/CPlayer.hpp"
// #include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"

#include "MetroidPrime/HUD/CHUDMemoParms.hpp"
// #include "MetroidPrime/HUD/CSamusHud.hpp"

#include "MetroidPrime/ScriptLoader/Struct/SLdrPickup.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CStringTable.hpp"

#include "rstl/math.hpp"

static float skDrawInDistance = 30.f;

CScriptPickup::CScriptPickup(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, const CModelData& modelData,
                             const CActorParameters& aParams, const CEchoParameters& echo,
                             const CAABox& aabb, CPlayerState::EItemType itemType, int amount,
                             int capacityIncrease, int itemPercentageIncrease,
                             CAssetId pickupEffect, bool absoluteValue, bool unknown, bool autoSpin,
                             bool blinkOut, float lifeTime, float respawnTime, float fadeTime,
                             float activateDelay, float pickupEffectLifetime, float autoHomeRange,
                             float delayUntilHome, float homingSpeed, const CVector3f& orbitOffset)
: CActor(uid, name, info, 0, xf, modelData, CMaterialList(), aParams, kInvalidUniqueId)
, m_itemType(itemType)
, m_amount(amount)
, m_capacity(capacityIncrease)
, m_percentageIncrease(itemPercentageIncrease)
, m_lifeTime(lifeTime)
, m_respawnTime(respawnTime)
, x170(0.f)
, m_fadeTime(fadeTime)
, m_curTime(0.0f)
, m_pickupEffectLifetime(pickupEffectLifetime)
, m_activateDelay(activateDelay)
, m_autoHomeRange(autoHomeRange)
, m_delayUntilHome(delayUntilHome)
, m_homingSpeed(homingSpeed)
, m_transformZ(xf.GetTranslation().GetZ())
, m_pickupParticleDesc()
, m_touchBounds(aabb)
, x1bc(0)
, x1c0(0)
, m_orbitOffset(orbitOffset)
, m_unknownProp(unknown)
, m_generated(false)
, m_inTractor(false)
, m_absoluteValue(absoluteValue)
, m_enableTractorTest(false)
, m_autoSpin(autoSpin)
, m_unk2(false)
, m_unk3(false)
, m_blinkOut(blinkOut) {
  if (pickupEffect != kInvalidAssetId) {
    m_pickupParticleDesc = gpSimplePool->GetObj(SObjectTag('PART', pickupEffect));
    m_pickupParticleDesc->Lock();
  }

  if (HasAnimation()) {
    // AnimationData()->SetAnimation(CAnimPlaybackParms(0, -1, 1.f, true), false);
  }

  if (fadeTime) {
  //   SetModelFlags(CModelFlags::AlphaBlended(0.f).DepthCompareUpdate(true, false));
  }
}

CScriptPickup::~CScriptPickup() {}

bool CScriptPickup::IsVisible() const { return false; }

void CScriptPickup::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  // if (delayTimer >= 0.f) {
  //   CActor::Stop();
  //   x278_delayTimer -= dt;
  //   return;
  // }

  // x270_curTime += dt;
  // if (x28c_25_inTractor && (x26c_lifeTime - x270_curTime) < 2.f) {
  //   x270_curTime = rstl::max_val(x26c_lifeTime - 2.f - FLT_EPSILON, x270_curTime - 2.f * dt);
  // }

  // CModelFlags drawFlags = CModelFlags::Normal();

  // if (x268_fadeInTime) {
  //   if (x270_curTime < x268_fadeInTime) {
  //     drawFlags =
  //         CModelFlags::AlphaBlended(x270_curTime / x268_fadeInTime).DepthCompareUpdate(true,
  //         false);
  //   } else {
  //     x268_fadeInTime = 0.f;
  //   }
  // } else if (x26c_lifeTime) {
  //   float alpha = 1.f;
  //   if (x26c_lifeTime < 2.f) {
  //     alpha = 1.f - (x26c_lifeTime / x270_curTime);
  //   } else if ((x26c_lifeTime - x270_curTime) < 2.f) {
  //     alpha = (x26c_lifeTime - x270_curTime) / 2.f;
  //   }

  //   drawFlags = CModelFlags::AlphaBlended(alpha).DepthCompareUpdate(true, false);
  // }

  // SetModelFlags(drawFlags);

  // if (HasAnimation()) {
  //   CAdvancementDeltas deltas = UpdateAnimation(dt, m_gr, true);
  //   MoveToOR(deltas.GetOffsetDelta(), dt);
  //   RotateToOR(deltas.GetOrientationDelta(), dt);
  // }

  // if (x28c_25_inTractor) {
  //   CVector3f velocity =
  //       mgr.GetPlayer()->GetTranslation() + (CVector3f::Up() * 2.f) - GetTranslation();
  //   x274_tractorTime += dt;
  //   float halfTractorTime = rstl::min_val(x274_tractorTime, 2.f) * 0.5f;
  //   velocity = velocity.AsNormalized() * (halfTractorTime * 20.f);
  //   if (x28c_26_enableTractorTest && mgr.GetPlayer()->GetPlayerGun()->GetChargeBeamFactor() <
  //                                        CPlayerGun::GetTractorBeamFactor()) {
  //     x28c_26_enableTractorTest = false;
  //     x28c_25_inTractor = false;
  //     velocity = CVector3f::Zero();
  //   }
  //   SetVelocityWR(velocity);
  // } else if (x28c_24_generated) {
  //   if (mgr.GetPlayer()->GetPlayerGun()->GetChargeBeamFactor() >
  //       CPlayerGun::GetTractorBeamFactor()) {
  //     const CFirstPersonCamera* camera = mgr.CameraManager()->FirstPersonCamera();
  //     CVector3f posDelta = GetTranslation() - camera->GetTranslation();
  //     CVector3f cameraFront = camera->GetTransform().GetColumn(kDY);
  //     float dot = CVector3f::Dot(cameraFront, posDelta.AsNormalized());
  //     float fovCos = cosine(CAbsAngle::FromDegrees(gpTweakGame->GetFirstPersonFOV()));
  //     if (dot > fovCos && posDelta.MagSquared() < skDrawInDistance * skDrawInDistance) {
  //       x28c_25_inTractor = true;
  //       x28c_26_enableTractorTest = true;
  //       x274_tractorTime = 0.f;
  //     }
  //   }
  // }

  // if (x26c_lifeTime && x270_curTime > x26c_lifeTime) {
  //   mgr.FreeScriptObject(GetUniqueId());
  // }
}

void CScriptPickup::Touch(CActor& act, CStateManager& mgr) {
  // if (GetActive() && !(x278_delayTimer >= 0) && TCastToPtr< CPlayer >(act)) {
  //   CPlayerState::EItemType itemType = x258_itemType;
  //   if (itemType >= CPlayerState::kIT_Truth && itemType <= CPlayerState::kIT_Newborn) {
  //     CAssetId id = CArtifactDoll::GetArtifactHeadScanFromItemType(itemType);
  //     if (id != kInvalidAssetId) {
  //       mgr.PlayerState()->SetScanTime(id, 0.5f);
  //     }
  //   }

  //   if (x27c_pickupParticleDesc) {
  //     if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_Thermal) {
  //       mgr.AddObject(new CExplosion(
  //           TLockedToken< CGenDescription >(*x27c_pickupParticleDesc), m_gr.AllocateUniqueId(),
  //           true, CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, kInvalidEditorId),
  //           rstl::string_l("Explosion - Pickup Effect"), GetTransform(), 0,
  //           CVector3f(1.f, 1.f, 1.f), CColor::White()));

  //     }
  //   }

  //   mgr.PlayerState()->InitializePowerUp(itemType, x260_capacity);
  //   mgr.PlayerState()->IncrPickUp(itemType, x25c_amount);
  //   mgr.FreeScriptObject(GetUniqueId());
  //   SendScriptMsgs(kSS_Arrived, m_gr, kSM_None);

  //   if (x260_capacity > 0) {
  //     const CPlayerState* playerState = mgr.GetPlayerState();
  //     int total = playerState->GetTotalPickupCount();
  //     int colRate = playerState->CalculateItemCollectionRate();
  //     if (colRate == total) {
  //       CSystemOptions& opts = gpGameState->SystemOptions();
  //       CAssetId id =
  //           gpResourceFactory
  //               ->GetResourceIdByName(opts.GetAllItemsCollected() ? "STRG_AllPickupsFound_2"
  //                                                                 : "STRG_AllPickupsFound_1")
  //               ->id;
  //       mgr.QueueMessage(mgr.GetHUDMessageFrameCount() + 1, id, 0.f);
  //       opts.SetAllItemsCollected(true);
  //     }
  //   }

  //   if (itemType == CPlayerState::kIT_PowerBombs) {
  //     CSystemOptions& opts = gpGameState->SystemOptions();
  //     if (opts.GetShowPowerBombAmmoMessage()) {
  //       opts.IncrementPowerBombAmmoCount();
  //       CSamusHud::DisplayHudMemo(rstl::wstring_l(gpStringTable->GetString(109)),
  //                                 CHUDMemoParms(5.f, true, false, false));
  //     }
  //   }
  // }
}

rstl::optional_object< CAABox > CScriptPickup::GetTouchBounds() const {
  // return CActor::GetBoundingBox();
}

void CScriptPickup::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CActor::AcceptScriptMsg(mgr, msg);
}

void CScriptPickup::Render(const CStateManager& mgr) const { CActor::Render(mgr); }

void CScriptPickup::AddToRenderer(const CFrustumPlanes& a, const CStateManager& mgr) const {
  if (IsVisible()) {
    CActor::AddToRenderer(mgr);
  }
}

CPlayerState::EItemType CScriptPickup::GetItem() const { return m_itemType; }

void CScriptPickup::SetSpawned() { m_generated = true; }

CAABox LoadCAABox(CStateManager& mgr, const TAreaId& areaId, const CVector3f& collisionSize,
                  const CVector3f& collisionOffset);
CTransform4f LoadEditorTransform(const SLdrEditorProperties&);
CActorParameters LoadActorParameters(const SLdrActorParameters&);
CEchoParameters LoadEchoParameters(const SLdrEchoParameters&);

rstl::optional_object< CModelData > LoadModelData(const CVector3f&, CAssetId asset,
                                                  const SLdrAnimationParameters&, bool);

CScriptPickup* LoadPickup(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrPickup sldrPickup;

  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();
    switch (propertyId) {
    case 0x255a4580:
      LoadTypedefSLdrEditorProperties(sldrPickup.editorProperties, input);
      break;
    case 0x3a3e03ba:
      sldrPickup.collisionSize = CVector3f(input);
      break;
    case 0x2e686c2a:
      sldrPickup.collisionOffset = CVector3f(input);
      break;
    case 0xa02ef0c4:
      LoadTypedefSLdrPlayerItem(sldrPickup.itemToGive, input);
      break;
    case 0x28c71b54:
      sldrPickup.capacityIncrease = input.ReadInt32();
      break;
    case 0x165ab069:
      sldrPickup.itemPercentageIncrease = input.ReadInt32();
      break;
    case 0x94af1445:
      sldrPickup.amount = input.ReadInt32();
      break;
    case 0xf7fbaaa5:
      sldrPickup.respawnTime = input.ReadFloat();
      break;
    case 0xc80fc827:
      sldrPickup.pickupEffectLifetime = input.ReadFloat();
      break;
    case 0x32dc67f6:
      sldrPickup.lifetime = input.ReadFloat();
      break;
    case 0x56e3ceef:
      sldrPickup.fadetime = input.ReadFloat();
      break;
    case 0xc27ffa8f:
      sldrPickup.model = input.ReadInt32();
      break;
    case 0xe25fb08c:
      LoadTypedefSLdrAnimationParameters(sldrPickup.animationInformation, input);
      break;
    case 0x7e397fed:
      LoadTypedefSLdrActorParameters(sldrPickup.actorInformation, input);
      break;
    case 0x192b0e70:
      LoadTypedefSLdrEchoParameters(sldrPickup.echoInformation, input);
      break;
    case 0xe585f166:
      sldrPickup.activationDelay = input.ReadFloat();
      break;
    case 0xa9fe872a:
      sldrPickup.pickupEffect = input.ReadInt32();
      break;
    case 0xe10bcb96:
      sldrPickup.absoluteValue = input.ReadBool();
      break;
    case 0xce33239f:
      sldrPickup.calculateVisibility = input.ReadBool();
      break;
    case 0x2de4a294:
      sldrPickup.unknown = input.ReadBool();
      break;
    case 0xa6ea280d:
      sldrPickup.autoHomeRange = input.ReadFloat();
      break;
    case 0xc2b11cfd:
      sldrPickup.delayUntilHome = input.ReadFloat();
      break;
    case 0x2db59fcf:
      sldrPickup.homingSpeed = input.ReadFloat();
      break;
    case 0x961c0d17:
      sldrPickup.autoSpin = input.ReadBool();
      break;
    case 0xa755eb02:
      sldrPickup.blinkOut = input.ReadBool();
      break;
    case 0x850115e4:
      sldrPickup.orbitOffset = CVector3f(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }

  rstl::optional_object< CModelData > modelData(
      LoadModelData(sldrPickup.editorProperties.transform.scale, sldrPickup.model,
                    sldrPickup.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  CAABox box =
      LoadCAABox(mgr, info.GetAreaId(), sldrPickup.collisionSize, sldrPickup.collisionOffset);
  if (sldrPickup.collisionSize == CVector3f::Zero()) {
    box = modelData->GetBounds(CTransform4f(LoadEditorTransform(sldrPickup.editorProperties)));
  }
  return new CScriptPickup(
      mgr.AllocateUniqueId(), sldrPickup.editorProperties.name,
      LdrToEntityInfo(info, sldrPickup.editorProperties),
      LoadEditorTransform(sldrPickup.editorProperties), *modelData,
      LoadActorParameters(sldrPickup.actorInformation),
      LoadEchoParameters(sldrPickup.echoInformation), box,
      CPlayerState::EItemType(sldrPickup.itemToGive.value), sldrPickup.amount,
      sldrPickup.capacityIncrease, sldrPickup.itemPercentageIncrease, sldrPickup.pickupEffect,
      sldrPickup.absoluteValue, sldrPickup.unknown, sldrPickup.autoSpin, sldrPickup.blinkOut,
      sldrPickup.lifetime, sldrPickup.respawnTime, sldrPickup.fadetime,
      sldrPickup.activationDelay, sldrPickup.pickupEffectLifetime, sldrPickup.autoHomeRange,
      sldrPickup.delayUntilHome, sldrPickup.homingSpeed, CVector3f(sldrPickup.orbitOffset));
}
