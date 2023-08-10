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
                             const CTransform4f& xf, const CModelData& mData,
                             const CActorParameters& aParams, const CEchoParameters& echo,
                             const CAABox& aabb, CPlayerState::EItemType itemType, int amount,
                             int capacityIncrease, int itemPercentageIncrease,
                             CAssetId pickupEffect, bool absoluteValue, bool unknown, bool autoSpin,
                             bool blinkOut, float lifetime, float respawnTime, float fadeTime,
                             float activateDelay, float pickupEffectLifetime, float autoHomeRange,
                             float delayUntilHome, float homingSpeed, const CVector3f& orbitOffset)
: CActor(uid, name, info, 0, xf, mData, CMaterialList(), aParams, kInvalidUniqueId)
, itemType(itemType)
, amount(amount)
, capacity(capacity)
, percentage(percentage)
, lifeTime(lifeTime)
, respawnTime(respawnTime)
, x170(0.f)
, fadeTime(fadeTime)
, curTime(0.0f)
, timeLeftToSet(timeLeftToSet)
, activateDelay(activateDelay)
, autoHomeRange(autoHomeRange)
, delayUntilHome(delayUntilHome)
, homingSpeed(homingSpeed)
, transformZ(xf.GetTranslation().GetZ())
, pickupParticleDesc()
, touchBounds(aabb)
, x1bc(0)
, x1c0(0)
, orbitOffset(orbitOffset)
, unknownProp(unknown)
, generated(false)
, inTractor(false)
, absoluteValue(absoluteValue)
, enableTractorTest(false)
, autoSpin(autoSpin)
, unk2(false)
, unk3(false)
, blinkOut(blinkOut) {
  if (pickupEffect != kInvalidAssetId) {
    pickupParticleDesc = gpSimplePool->GetObj(SObjectTag('PART', pickupEffect));
    pickupParticleDesc->Lock();
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
  //   CAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
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
  //           TLockedToken< CGenDescription >(*x27c_pickupParticleDesc), mgr.AllocateUniqueId(),
  //           true, CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, kInvalidEditorId),
  //           rstl::string_l("Explosion - Pickup Effect"), GetTransform(), 0,
  //           CVector3f(1.f, 1.f, 1.f), CColor::White()));

  //     }
  //   }

  //   mgr.PlayerState()->InitializePowerUp(itemType, x260_capacity);
  //   mgr.PlayerState()->IncrPickUp(itemType, x25c_amount);
  //   mgr.FreeScriptObject(GetUniqueId());
  //   SendScriptMsgs(kSS_Arrived, mgr, kSM_None);

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

void CScriptPickup::AcceptScriptMsg(CStateManager& mgr, CScriptMsg& msg) {
  CActor::AcceptScriptMsg(mgr, msg);
}

void CScriptPickup::Render(const CStateManager& mgr) const { CActor::Render(mgr); }

void CScriptPickup::AddToRenderer(const CFrustumPlanes& a, const CStateManager& mgr) const {
  if (IsVisible()) {
    CActor::AddToRenderer(a, mgr);
  }
}

CPlayerState::EItemType CScriptPickup::GetItem() const { return itemType; }

void CScriptPickup::SetSpawned() { generated = true; }

CAABox LoadCAABox(CStateManager& mgr, const TAreaId& areaId, const CVector3f& collisionSize,
                  const CVector3f& collisionOffset);
CTransform4f LoadEditorTransform(const SLdrEditorProperties&);
const CEntityInfo& EntityInfoWithEditorProperties(const CEntityInfo&, const SLdrEditorProperties&);
CActorParameters LoadActorParameters(const SLdrActorParameters&);
CEchoParameters LoadEchoParameters(const SLdrEchoParameters&);

rstl::optional_object< CModelData > LoadModelData(const CVector3f&, CAssetId asset,
                                                  const SLdrAnimationParameters&, bool);

CScriptPickup* LoadPickup(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  SLdrPickup sldrPickup;

  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();
    switch (propertyId) {
    case 0x255a4580:
      LoadTypedefSLdrEditorProperties(sldrPickup.editor_properties, input);
      break;
    case 0x3a3e03ba:
      sldrPickup.collision_size = CVector3f(input);
      break;
    case 0x2e686c2a:
      sldrPickup.collision_offset = CVector3f(input);
      break;
    case 0xa02ef0c4:
      LoadTypedefSLdrPlayerItem(sldrPickup.item_to_give, input);
      break;
    case 0x28c71b54:
      sldrPickup.capacity_increase = input.ReadInt32();
      break;
    case 0x165ab069:
      sldrPickup.item_percentage_increase = input.ReadInt32();
      break;
    case 0x94af1445:
      sldrPickup.amount = input.ReadInt32();
      break;
    case 0xf7fbaaa5:
      sldrPickup.respawn_time = input.ReadFloat();
      break;
    case 0xc80fc827:
      sldrPickup.pickup_effect_lifetime = input.ReadFloat();
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
      LoadTypedefSLdrAnimationParameters(sldrPickup.animation_information, input);
      break;
    case 0x7e397fed:
      LoadTypedefSLdrActorParameters(sldrPickup.actor_information, input);
      break;
    case 0x192b0e70:
      LoadTypedefSLdrEchoParameters(sldrPickup.echo_information, input);
      break;
    case 0xe585f166:
      sldrPickup.activation_delay = input.ReadFloat();
      break;
    case 0xa9fe872a:
      sldrPickup.pickup_effect = input.ReadInt32();
      break;
    case 0xe10bcb96:
      sldrPickup.absolute_value = input.ReadBool();
      break;
    case 0xce33239f:
      sldrPickup.calculate_visibility = input.ReadBool();
      break;
    case 0x2de4a294:
      sldrPickup.unknown = input.ReadBool();
      break;
    case 0xa6ea280d:
      sldrPickup.auto_home_range = input.ReadFloat();
      break;
    case 0xc2b11cfd:
      sldrPickup.delay_until_home = input.ReadFloat();
      break;
    case 0x2db59fcf:
      sldrPickup.homing_speed = input.ReadFloat();
      break;
    case 0x961c0d17:
      sldrPickup.auto_spin = input.ReadBool();
      break;
    case 0xa755eb02:
      sldrPickup.blink_out = input.ReadBool();
      break;
    case 0x850115e4:
      sldrPickup.orbit_offset = CVector3f(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }

  rstl::optional_object< CModelData > modelData(
      LoadModelData(sldrPickup.editor_properties.transform.scale, sldrPickup.model,
                    sldrPickup.animation_information, true));
  if (!modelData) {
    return nullptr;
  }

  CAABox box =
      LoadCAABox(mgr, info.GetAreaId(), sldrPickup.collision_size, sldrPickup.collision_offset);
  if (sldrPickup.collision_size == CVector3f::Zero()) {
    box = modelData->GetBounds(CTransform4f(LoadEditorTransform(sldrPickup.editor_properties)));
  }
  return new CScriptPickup(
      mgr.AllocateUniqueId(), sldrPickup.editor_properties.name,
      EntityInfoWithEditorProperties(info, sldrPickup.editor_properties),
      LoadEditorTransform(sldrPickup.editor_properties), *modelData,
      LoadActorParameters(sldrPickup.actor_information),
      LoadEchoParameters(sldrPickup.echo_information), box,
      CPlayerState::EItemType(sldrPickup.item_to_give.value), sldrPickup.amount,
      sldrPickup.capacity_increase, sldrPickup.item_percentage_increase, sldrPickup.pickup_effect,
      sldrPickup.absolute_value, sldrPickup.unknown, sldrPickup.auto_spin, sldrPickup.blink_out,
      sldrPickup.lifetime, sldrPickup.respawn_time, sldrPickup.fadetime,
      sldrPickup.activation_delay, sldrPickup.pickup_effect_lifetime, sldrPickup.auto_home_range,
      sldrPickup.delay_until_home, sldrPickup.homing_speed, CVector3f(sldrPickup.orbit_offset));
}
