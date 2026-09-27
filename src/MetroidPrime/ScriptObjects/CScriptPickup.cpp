#include "MetroidPrime/ScriptObjects/CScriptPickup.hpp"

// #include "MetroidPrime/CAnimData.hpp"
// #include "MetroidPrime/CAnimPlaybackParms.hpp"
// #include "MetroidPrime/CArtifactDoll.hpp"
// #include "MetroidPrime/CExplosion.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/SEchoParameters.hpp"

// #include "MetroidPrime/Cameras/CCameraManager.hpp"
// #include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
// #include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"

#include "MetroidPrime/HUD/CHUDMemoParms.hpp"
// #include "MetroidPrime/HUD/CSamusHud.hpp"

#include "MetroidPrime/ScriptLoader/SLdrPickup.hpp"

#include "Kyoto/CEnvironmentVariable.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CStringTable.hpp"

#include "rstl/math.hpp"

static float skDrawInDistance = 30.f;

CScriptPickup::CScriptPickup(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, const CModelData& modelData,
                             const CActorParameters& aParams, const SEchoParameters& echo,
                             const CAABox& aabb, CPlayerState::EItemType itemType, int amount,
                             int capacityIncrease, int itemPercentageIncrease,
                             CAssetId pickupEffect, bool absoluteValue, bool unknown, bool autoSpin,
                             bool blinkOut, float lifeTime, float respawnTime, float fadeTime,
                             float activateDelay, float pickupEffectLifetime, float autoHomeRange,
                             float delayUntilHome, float homingSpeed, const CVector3f& orbitOffset)
: CActor(uid, name, info, 0, xf, modelData, CMaterialList(), aParams, kInvalidUniqueId)
, mItemType(itemType)
, mAmount(amount)
, mCapacity(capacityIncrease)
, mPercentageIncrease(itemPercentageIncrease)
, mLifeTime(lifeTime)
, mRespawnTime(respawnTime)
, x170(0.f)
, mFadeTime(fadeTime)
, mCurTime(0.0f)
, mPickupEffectLifetime(pickupEffectLifetime)
, mActivateDelay(activateDelay)
, mAutoHomeRange(autoHomeRange)
, mDelayUntilHome(delayUntilHome)
, mHomingSpeed(homingSpeed)
, mTransformZ(xf.GetTranslation().GetZ())
, mPickupParticleDesc()
, mTouchBounds(aabb)
, x1bc(0)
, x1c0(0)
, mOrbitOffset(orbitOffset)
, mUnknownProp(unknown)
, mGenerated(false)
, mInTractor(false)
, mAbsoluteValue(absoluteValue)
, mEnableTractorTest(false)
, mAutoSpin(autoSpin)
, mUnk2(false)
, mUnk3(false)
, mBlinkOut(blinkOut) {
  if (pickupEffect != kInvalidAssetId) {
    mPickupParticleDesc = gpSimplePool->GetObj(SObjectTag('PART', pickupEffect));
    mPickupParticleDesc->Lock();
  }

  if (HasAnimation()) {
    // AnimationData()->SetAnimation(CAnimPlaybackParms(0, -1, 1.f, true), false);
  }

  if (fadeTime) {
  //   SetModelFlags(CModelFlags::AlphaBlended(0.f).DepthCompareUpdate(true, false));
  }
}

CScriptPickup::~CScriptPickup() {}

void CScriptPickup::PreRenderAllViewports(CStateManager& mgr) {
  CActor::PreRenderAllViewports(mgr);
  if (mUnk2) {
    mUnk2 = false;
    x1bc = 0;
  } else {
    x1bc += 1;
  }
}

void CScriptPickup::PreRender(CStateManager& mgr) {
  CActor::PreRender(mgr);
  if (!GetPreRenderClipped()) {
    mUnk2 = true;
  }
}

bool CScriptPickup::IsVisible() const {
  if (mActivateDelay >= 0.0f) {
    return false;
  }
  return !(x170 > 0.0f);
}

void CScriptPickup::Think(float dt, CStateManager& mgr) {
  CActor::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }

  // if (mDelayTimer >= 0.f) {
  //   // CActor::Stop();
  //   mDelayTimer -= dt;
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
  if (!GetActive() || !IsVisible()) {
    return;
  }
  if (CPlayer* player = TCastToPtr< CPlayer >(act)) {
    int playerIndex = mgr.MaskUIdNumPlayers(player->GetUniqueId());
    CPlayerState* playerState = mgr.PlayerState(playerIndex);
    if (!playerState->IsPlayerAlive())
      return;

    CPlayerState::EItemType itemType = mItemType;

    if (mPickupParticleDesc) {
      // mgr.AddObject(rs_new CExplosion(
      //     TLockedToken< CGenDescription >(*mPickupParticleDesc), mgr.AllocateUniqueId(),
      //     true, CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, kInvalidEditorId),
      //     rstl::string_l("Explosion - Pickup Effect"), GetTransform(), 0,
      //     CVector3f(1.f, 1.f, 1.f), CColor::White())
      //   );
    }

    int previousAmount = playerState->GetItemAmount(itemType);
    playerState->AddPowerUp(CPlayerState::kIT_ItemPercentage, mPercentageIncrease);
    playerState->IncrPickUp(CPlayerState::kIT_ItemPercentage, mPercentageIncrease);
    if (!mAbsoluteValue) {
      playerState->AddPowerUp(itemType, mAmount);
      playerState->IncrPickUp(itemType, mAmount);
    } else {
      playerState->ReInitializePowerUp(itemType, mCapacity);
      playerState->SetItemAmount(itemType, mAmount);
    }
    playerState->SetTimeLeft(itemType, mPickupEffectLifetime);
    SendScriptMsgs(kSS_Active, mgr, player->GetUniqueId(), kSM_None);
    if (mRespawnTime == 0.0f) {
      mEnableTractorTest = true;
      mgr.DeleteObjectRequest(GetUniqueId());
    } else {
      SetModelFlags(CModelFlags::AlphaBlended(0.f).DepthCompareUpdate(true, false));
      x170 = mRespawnTime;
      mCurTime = 0.f;
      mFadeTime = 0.25f;
    }

    if (previousAmount < playerState->GetItemAmount(itemType)) {
      ShowAllKeysCollectedAlert(mgr, playerState, itemType);
    }

    if (mPercentageIncrease > 0) {
      int total = playerState->GetTotalPickupCount();
      int colRate = playerState->CalculateItemCollectionRate();
      if (colRate == total) {
        CAssetId id =
            gpResourceFactory
                ->GetResourceIdByName("STRG_AllPickupsFound_2")
                ->id;
              
        mgr.QueueMessage(mgr.GetHUDMessageFrameCount() + 1, id, 0.f);
        gpGameState->PersistentOptions().FindEnvironmentVariable("AllPickupsFound")->Set(1);
      }
    }

    if (!mgr.fn_80036F10() && itemType == CPlayerState::kIT_Powerbomb && mCapacity == 0) {
      CPersistentOptions& opts = gpGameState->PersistentOptions();
      if (opts.FindEnvironmentVariable("PowerbombPickupMessages")->Get() == 0) {
        opts.FindEnvironmentVariable("PowerbombPickupMessages")->Set(1);
        CSamusHud::DisplayHudMemo(
          rstl::wstring_l(gpStringTable->GetString("FirstPowerBombPickup")),
          CHUDMemoParms(5.f, true, false, false, 1 << playerIndex, true)
        );
      }
    }
    switch (itemType) {
      case CPlayerState::kIT_SwitchVisorCombat:
        playerState->StartTransitionToVisor(CPlayerState::kPV_Combat);
        break;
      case CPlayerState::kIT_SwitchVisorScan:
        playerState->StartTransitionToVisor(CPlayerState::kPV_Scan);
        break;
      case CPlayerState::kIT_SwitchVisorDark:
        playerState->StartTransitionToVisor(CPlayerState::kPV_Dark);
        break;
      case CPlayerState::kIT_SwitchVisorEcho:
        playerState->StartTransitionToVisor(CPlayerState::kPV_Echo);
        break;
    }
  }
}

rstl::optional_object< CAABox > CScriptPickup::GetTouchBounds() const {
  const CVector3f& off = GetTranslation();
  return CAABox(mTouchBounds.GetMinPoint() + off, mTouchBounds.GetMaxPoint() + off);
}

void CScriptPickup::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CActor::AcceptScriptMsg(mgr, msg);
}

void CScriptPickup::AddToRenderer(const CStateManager& mgr) const {
  if (IsVisible()) {
    CActor::AddToRenderer(mgr);
  }
}

CPlayerState::EItemType CScriptPickup::GetItem() const { return mItemType; }

void CScriptPickup::SetSpawned() { mGenerated = true; }

CAABox LoadCAABox(CStateManager& mgr, const TAreaId& areaId, const CVector3f& collisionSize,
                  const CVector3f& collisionOffset);
CTransform4f LoadEditorTransform(const SLdrEditorProperties&);
CActorParameters LoadActorParameters(const SLdrActorParameters&);
SEchoParameters LoadEchoParameters(const SLdrEchoParameters&);

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
      sldrPickup.canHomeByDefault = input.ReadBool();
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
      sldrPickup.absoluteValue, sldrPickup.canHomeByDefault, sldrPickup.autoSpin,
      sldrPickup.blinkOut,
      sldrPickup.lifetime, sldrPickup.respawnTime, sldrPickup.fadetime,
      sldrPickup.activationDelay, sldrPickup.pickupEffectLifetime, sldrPickup.autoHomeRange,
      sldrPickup.delayUntilHome, sldrPickup.homingSpeed, CVector3f(sldrPickup.orbitOffset));
}
