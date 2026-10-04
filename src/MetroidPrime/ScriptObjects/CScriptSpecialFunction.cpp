#include "MetroidPrime/ScriptObjects/CScriptSpecialFunction.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAxisAngle.hpp"
#include "MetroidPrime/CArchitectureMessage.hpp"
#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CCredits.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CMapWorld.hpp"
#include "MetroidPrime/CMapWorldInfo.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CGMMultiplayer.hpp"
#include "MetroidPrime/Player/CGMSinglePlayer.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CEnvironmentVariable.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CWorldTransManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpawnPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTriggerEllipsoid.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWorldTeleporter.hpp"
#include "MetroidPrime/TCastTo.hpp"

static const ERumbleFxId skRumbleFxList[6] = {
    kRFX_Twenty, kRFX_One, kRFX_TwentyOne, kRFX_TwentyTwo, kRFX_TwentyThree, kRFX_Zero,
};

// Guessed names
static int kCreditsMsgPriority = 11;
static int kCreditsDrawPriority = 1001;

CScriptSpecialFunction::CScriptSpecialFunction(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    ESpecialFunction function, const rstl::string& stringParm, float value1, float value2,
    float value3, float value4, int intParm1, int intParm2, const CVector3f& vectorParm,
    const CColor& colorParm, const CDamageInfo& damageInfo, CPlayerState::EItemType item,
    ushort sfx1, ushort sfx2, ushort sfx3)
: CActor(uid, name, info, 0, xf, CModelData(), CMaterialList(), CActorParameters::None(),
         kInvalidUniqueId)
, mFunction(function)
, mStringParm(stringParm)
, mValue1(value1)
, mValue2(value2)
, mValue3(value3)
, mValue4(value4)
, mIntParm1(intParm1)
, mIntParm2(intParm2)
, mVectorParm(vectorParm)
, mColorParm(colorParm)
, x194_(kInvalidUniqueId)
, mLastOriginatorPlayer(kInvalidUniqueId)
, mDamageInfo(damageInfo)
, mSpinnerPosition(0.f)
, mSpinnerInitialXf(CTransform4f::Identity())
, mShotSpinnerImpulse(0.f)
, mSfx1(sfx1)
, mSfx2(sfx2)
, mSfx3(sfx3)
, mSfxHandle()
, mPreviousSpinnerSpeed(0.f)
, mVolumeAverage(6, 0.f)
, mChaffTimer(0.f)
, mSilhouetteStrength(0.f)
, mTargetSilhouetteStrength(0.f)
, mItem(item)
, mSpinnerInitializedXf(false)
, mSpinnerCanMove(false)
, mSfx2Played(true)
, mSfx3Played(false)
, mInAreaDamage(false)
, mDoSave(false)
, mPlayerInArea(false)
, mFrustumEntered(false)
, mFrustumExited(false)
, mInFrustum(false) {
  if (mFunction == kSF_HUDTarget) {
    mTouchBounds = CAABox(CVector3f(-1.f, -1.f, -1.f), CVector3f(1.f, 1.f, 1.f));
  }
}

void CScriptSpecialFunction::AddFogVolumeToRenderer(const CStateManager& mgr) const {
  if (mInFrustum) {
    EnsureRendered(mgr);
  }
}

void CScriptSpecialFunction::AddSilhouetteToRenderer(const CStateManager& mgr) const {
  if (mInFrustum) {
    EnsureRendered(mgr);
  }
}

void CScriptSpecialFunction::AddToRenderer(const CStateManager& mgr) const {
  if (!GetActive()) {
    return;
  }

  switch (mFunction) {
  case kSF_FogVolume:
    AddFogVolumeToRenderer(mgr);
    break;
  case kSF_Silhouette:
    AddSilhouetteToRenderer(mgr);
    break;
  }
}

void CScriptSpecialFunction::PreRenderFogVolume(CStateManager& mgr) {
  CVector3f max = GetTranslation() + mVectorParm;
  max.SetZ(max.GetZ() + mValue1);
  SetInFrustum(mgr.GetFrustumPlanes().BoxInFrustumPlanes(CAABox(GetTranslation() - mVectorParm, max)));
}

void CScriptSpecialFunction::PreRenderViewFrustumTester(CStateManager& mgr) {
  SetInFrustum(mgr.GetFrustumPlanes().PointInFrustumPlanes(GetTranslation()));
}

void CScriptSpecialFunction::PreRenderPlayerFrustumTester(CStateManager& mgr) {
  if (mIntParm2 == mgr.GetCurrentRenderPlayerIndex()) {
    SetInFrustum(mgr.GetFrustumPlanes().PointInFrustumPlanes(GetTranslation()));
  }
}

void CScriptSpecialFunction::PreRenderSilhouette(CStateManager& mgr) {
  // TODO: copy the connected actor's bounds and test visibility.
}

void CScriptSpecialFunction::PreRenderBillboard(CStateManager& mgr) {
  if (mIntParm1 & 1) {
    mgr.RenderLastHUD(GetUniqueId());
  }
}

void CScriptSpecialFunction::PreRender(CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  switch (mFunction) {
  case kSF_FogVolume:
    PreRenderFogVolume(mgr);
    break;
  case kSF_ViewFrustumTester:
    PreRenderViewFrustumTester(mgr);
    break;
  case kSF_PlayerOffscreen:
    PreRenderPlayerFrustumTester(mgr);
    break;
  case kSF_Silhouette:
    PreRenderSilhouette(mgr);
    break;
  case kSF_VisorBlowout:
    PreRenderBillboard(mgr);
    break;
  }
}

void CScriptSpecialFunction::RenderFogVolume(const CStateManager& mgr) const {
  // TODO: draw the animated fog volume.
}

void CScriptSpecialFunction::RenderSilhouette(const CStateManager& mgr) const {
  // TODO: render the connected actor's silhouette.
}

void CScriptSpecialFunction::RenderBillboard() const {
  // TODO: draw the camera-facing textured effect.
}

void CScriptSpecialFunction::Render(const CStateManager& mgr) const {
  switch (mFunction) {
  case kSF_FogVolume:
    RenderFogVolume(mgr);
    break;
  case kSF_Silhouette:
    RenderSilhouette(mgr);
    break;
  case kSF_VisorBlowout:
    RenderBillboard();
  default:
    CActor::Render(mgr);
    break;
  }
}

void CScriptSpecialFunction::AcceptChaffTarget(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_XALD:
    AddMaterial(kMT_Target, mgr);
    break;
  }
}

void CScriptSpecialFunction::AcceptHUDFadeIn(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    mgr.GetPlayer(0)->SetHudDisable(mValue1, 0.f, 0.5f);
  }
}

void CScriptSpecialFunction::AcceptEscapeSequence(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action && mValue1 >= 0.f) {
    mgr.ResetEscapeSequenceTimer(mValue1);
  }
}

void CScriptSpecialFunction::AcceptSpinner(CStateManager& mgr, const CScriptMsg& msg) {
  if (!mgr.IsMultiplayer()) {
    switch (msg.GetMessage()) {
    case kSM_Play:
      mSfx3Played = mSfx2Played = false;
      mSpinnerCanMove = true;
      mgr.GetPlayer(0)->SetAngularVelocityWR(CAxisAngle::Identity());
      if (mSfx2 != CSfxManager::kInternalInvalidSfxId) {
        CSfxManager::AddEmitter(mSfx2, GetTranslation(), GetCurrentAreaId().Value(), true, false,
                                CSfxManager::kMedPriority);
      }
      break;
    case kSM_Stop:
      mSfx2Played = false;
      mSpinnerCanMove = false;
      break;
    case kSM_Reset:
      mSpinnerPosition = 0.f;
      mPreviousSpinnerSpeed = 0.f;
      mSfx3Played = mSfx2Played = false;
    case kSM_Deactivate:
      DeleteEmitter(mSfxHandle);
      break;
    }
  }
}

void CScriptSpecialFunction::AcceptShotSpinner(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Increment:
    mShotSpinnerImpulse = rstl::max_val(0.f, rstl::min_val(mShotSpinnerImpulse + 1.f, 1.f));
    SendScriptMsgs(kSS_Play, mgr, kInvalidUniqueId, kSM_None);
    break;
  case kSM_SetToMax:
    mShotSpinnerImpulse = mValue3;
    SendScriptMsgs(kSS_Play, mgr, kInvalidUniqueId, kSM_None);
    break;
  case kSM_SetToZero:
    mShotSpinnerImpulse = -0.5f * mValue3;
    break;
  }
}

void CScriptSpecialFunction::AcceptMapStation(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_Play, kSM_Activate);
    bool foundTeleporter = false;
    for (rstl::vector< TUniqueId >::const_iterator it = ids.begin(); it != ids.end(); ++it) {
      if (const CScriptWorldTeleporter* teleporter =
              TCastToConstPtr< CScriptWorldTeleporter >(mgr.GetObjectById(*it))) {
        foundTeleporter = true;
        if (teleporter->GetWorldId() == mgr.World()->GetWorldAssetId()) {
          TAreaId areaId = mgr.World()->GetAreaId(teleporter->GetAreaId());
          if (static_cast< uint >(areaId.Value()) != -1u) {
            mgr.MapWorldInfo()->SetIsMapped(areaId, true);
          }
        }
      }
    }
    if (!foundTeleporter) {
      mgr.MapWorldInfo()->SetMapStationUsed(true);
    }
    CMapWorld* mapWorld = mgr.World()->GetMapWorld();
    mapWorld->RecalculateWorldSphere(*mgr.MapWorldInfo(), *mgr.World());
    mgr.EnterMapScreen();
  }
}

void CScriptSpecialFunction::AcceptMissileStation(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action && !mgr.IsMultiplayer()) {
    CPlayerState* playerState = mgr.PlayerState(0);
    playerState->ResetAndIncrPickUp(CPlayerState::kIT_Missile,
                                    playerState->GetItemCapacity(CPlayerState::kIT_Missile));
  }
}

void CScriptSpecialFunction::AcceptPowerBombStation(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action && !mgr.IsMultiplayer()) {
    CPlayerState* playerState = mgr.PlayerState(0);
    playerState->ResetAndIncrPickUp(CPlayerState::kIT_Powerbomb,
                                    playerState->GetItemCapacity(CPlayerState::kIT_Powerbomb));
  }
}

void CScriptSpecialFunction::AcceptSaveStation(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    if (!mgr.IsMultiplayer()) {
      const bool noCard = gpGameState->GetCardSerial() == 0;
      mgr.PlayerState(0)->IncrPickUp(CPlayerState::kIT_EnergyTanks, 1);
      if (noCard) {
        SendScriptMsgs(kSS_Closed, mgr, kInvalidUniqueId, kSM_None);
      } else if (!noCard) {
        mgr.EnterSaveGameScreen();
        mDoSave = true;
      }
    }
  } else if (msg.GetMessage() == kSM_Increment) {
    gpGameState->RecordCheckpoint();
  } else if (msg.GetMessage() == kSM_Decrement) {
    gpGameState->ClearCheckpoint();
  }
}

void CScriptSpecialFunction::AcceptEnergyTank(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mLastOriginatorPlayer))) {
      player->GetPlayerState()->IncrPickUp(CPlayerState::kIT_EnergyTanks, 1);
    }
  }
}

void CScriptSpecialFunction::AcceptRadialDamage(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Activate:
  case kSM_XCRT:
    if ((mIntParm1 & 1) == 0 || !GetActive()) {
      break;
    }
  case kSM_Action: {
    CDamageInfo info = mDamageInfo;
    info.SetRadius(mValue1);
    if ((mIntParm1 & 4) != 0) {
      mgr.ApplyDamage(GetUniqueId(), msg.GetOriginator(), kInvalidUniqueId, info,
                      CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59),
                                                          CMaterialList()),
                      CVector3f::Zero());
    } else {
      mgr.ApplyDamageToWorld(GetUniqueId(), *this, GetTranslation(), info,
                             CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59),
                                                                 CMaterialList()));
    }
    if ((mIntParm1 & 2) != 0) {
      mgr.DeleteObjectRequest(GetUniqueId());
    }
    break;
  }
  }
}

void CScriptSpecialFunction::AcceptBossEnergyBar(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Increment: {
    int stringIdx = static_cast< int >(mValue2);
    stringIdx += gpStringTable->GetStringIndex("Boss0");
    if (mStringParm.length() != 0) {
      int idx = gpStringTable->GetStringIndex(mStringParm.data());
      if (idx != -1) {
        stringIdx = idx;
      }
    }
    TUniqueId bossId = FindConnectedObject(mgr, kSS_Play, kSM_Activate);
    if (const CActor* boss = TCastToConstPtr< CActor >(mgr.GetObjectById(bossId))) {
      float maxEnergy = mValue1;
      if (maxEnergy == 0.f) {
        if (const CHealthInfo* healthInfo = boss->GetHealthInfo()) {
          maxEnergy = healthInfo->GetInitialHP();
        }
      }
      mgr.SetBossParams(bossId, maxEnergy, stringIdx);
    } else {

      mgr.SetBossParams(msg.GetUnk(), mValue1, stringIdx);
    }
    break;
  }
  case kSM_Decrement:
    mgr.SetBossParams(kInvalidUniqueId, 0.f, 0);
    break;
  }
}

void CScriptSpecialFunction::AcceptEndGame(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    gpGameState->GetGameMode().EndGame(mIntParm1, mgr);
  }
}

void CScriptSpecialFunction::AcceptCinematicSkip(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Increment:
    mgr.SetSkipCinematicSpecialFunction(GetUniqueId());
    break;
  case kSM_Decrement:
    mgr.SetSkipCinematicSpecialFunction(kInvalidUniqueId);
    break;
  case kSM_XDelete:
    if (mgr.GetSkipCinematicSpecialFunction() == GetUniqueId()) {
      mgr.SetSkipCinematicSpecialFunction(kInvalidUniqueId);
    }
    break;
  }
}

void CScriptSpecialFunction::AcceptEnvFxDensity(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    mgr.EnvFxManager()->FadeDensity(mValue1, static_cast< int >(mValue2));
  }
}

void CScriptSpecialFunction::AcceptRumble(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    int rumbFxIdx = static_cast< int >(mValue2);
    mgr.IsMultiplayer();
    if (rumbFxIdx >= 0 && rumbFxIdx < static_cast< int >(sizeof(skRumbleFxList) / sizeof(ERumbleFxId))) {
      ERumbleFxId rumbFx = skRumbleFxList[rumbFxIdx];
      uint flags = mValue3;
      if ((flags & 1) != 0) {
        mgr.RumbleManager(0)->Rumble(mgr, rumbFx, 1.f, kRP_One);
      } else {
        CVector3f pos = GetTranslation();
        if ((flags & 2) != 0) {
          TUniqueId uid = msg.GetUnk();
          if (const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(uid))) {
            pos = act->GetTranslation();
          }
        }
        mgr.RumbleManager(0)->Rumble(mgr, pos, rumbFx, mValue1, kRP_One);
      }
    }
  }
}

void CScriptSpecialFunction::AcceptInventoryActivator(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    for (int i = 0; i < static_cast< uint >(mgr.GetNumPlayers()); ++i) {
      if (mgr.PlayerState(i)->HasPowerUp(mItem)) {
        SendScriptMsgs(kSS_Zero, mgr, kInvalidUniqueId, kSM_None);
        return;
      }
    }
  }
}

void CScriptSpecialFunction::AcceptAreaDamage(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Deactivate:
  case kSM_XDelete:
    if (!mgr.IsMultiplayer() && mInAreaDamage) {
      mInAreaDamage = false;
      mgr.GetPlayer(0)->PopSustainedDamage();
      mgr.SetIsFullThreat(false);
    }
    break;
  }
}

void CScriptSpecialFunction::AcceptDropBomb(CStateManager& mgr, const CScriptMsg& msg) {
  // Empty in the original.
}

void CScriptSpecialFunction::AcceptHint(CStateManager& mgr, const CScriptMsg& msg) {
  CHintOptions& hints = gpGameState->HintOptions();
  switch (msg.GetMessage()) {
  case kSM_Action:
    hints.ActivateContinueDelayHintTimer(mStringParm);
    break;
  case kSM_Increment:
    hints.ActivateImmediateHintTimer(mStringParm);
    break;
  case kSM_Decrement:
    hints.DelayHint(mStringParm);
    break;
  case kSM_Start:
    hints.SetInRezbitState(false);
    break;
  case kSM_Stop:
    hints.SetInRezbitState(true);
    break;
  }
}

void CScriptSpecialFunction::AcceptPlayerInArea(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Action:
  case kSM_SetToZero:
    if (!mgr.IsMultiplayer() && mgr.GetPlayer(0)->GetCurrentAreaId() == GetCurrentAreaId()) {
      SendScriptMsgs(kSS_Zero, mgr, kInvalidUniqueId, kSM_None);
    }
    break;
  }
}

void CScriptSpecialFunction::AcceptHUDTarget(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Increment:
    AddMaterial(kMT_Target, kMT_RadarObject, mgr);
    break;
  case kSM_Decrement:
    RemoveMaterial(kMT_Target, kMT_RadarObject, mgr);
    break;
  }
}

void CScriptSpecialFunction::AcceptFogFader(CStateManager& mgr, const CScriptMsg& msg) {
  float scale = 1.f;
  switch (msg.GetMessage()) {
  case kSM_Increment:
    scale = mValue1;
  case kSM_Decrement:
    for (int i = 0; i < static_cast< uint >(mgr.GetNumPlayers()); ++i) {
      mgr.CameraManager(i)->SetWaterFogScale(scale, mValue2);
    }
    break;
  }
}

void CScriptSpecialFunction::AcceptLogbook(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    mgr.EnterLogBookScreen();
  }
}

void CScriptSpecialFunction::AcceptEnding(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    CGameMode* mode = &gpGameState->GetGameMode();
    if (mode && mode->GetGameModeType() == 'SNGL') {
      int result = static_cast< CGMSinglePlayer* >(mode)->CalculateResult(mgr);
      bool send = false;
      switch (static_cast< int >(mValue1)) {
      case 0:
        send = result == CGMSinglePlayer::kRI_Under75Percent;
        break;
      case 1:
        send = result == CGMSinglePlayer::kRI_Under100Percent;
        break;
      case 2:
        send = result == CGMSinglePlayer::kRI_AtLeast100Percent;
        break;
      }
      if (send) {
        SendScriptMsgs(kSS_Zero, mgr, kInvalidUniqueId, kSM_None);
      }
    }
  }
}

void CScriptSpecialFunction::AcceptPlayerVelocity(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(FindConnectedObject(mgr, kSS_Play, kSM_Activate)));
    TUniqueId originator = msg.GetOriginator();
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(originator));
    if (player) {
      CVector3f dir = (actor->GetTranslation() - player->GetTranslation()).AsNormalized();
      player->SetVelocityWR(mValue1 * dir);
    }
  }
}

void CScriptSpecialFunction::AcceptDarkWorld(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_XALD:
    if (GetActive()) {
      if (mgr.GetIsDarkWorld()) {
        SendScriptMsgs(kSS_Zero, mgr, kInvalidUniqueId, kSM_None);
      } else {
        SendScriptMsgs(kSS_MaxReached, mgr, kInvalidUniqueId, kSM_None);
      }
    }
    break;
  }
}

void CScriptSpecialFunction::fn_80107a58(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    uint player = mIntParm1;
    if (player < mgr.GetNumPlayers()) {
      gpGameState->GetGameMode().RespawnPlayer(mgr, player);
    }
  }
}

void CScriptSpecialFunction::AcceptPlayerSpawnPoint(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    uint player = mIntParm1;
    if (player < mgr.GetNumPlayers()) {
      TUniqueId spawn = FindConnectedObject(mgr, kSS_Play, kSM_Activate);
      if (TCastToConstPtr< CScriptSpawnPoint >(mgr.GetObjectById(spawn))) {
        gpGameState->GetGameMode().SetSpawnPoint(mIntParm1, spawn);
      }
    }
  }
}

void CScriptSpecialFunction::fn_80107994(CStateManager& mgr, const CScriptMsg& msg) {
  // Empty in the original.
}

void CScriptSpecialFunction::AcceptSetItemCapacity(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    uint player = ResolvePlayerIndex(mIntParm1, msg.GetOriginator(), mgr);
    if (player < mgr.GetNumPlayers()) {
      mgr.PlayerState(player)->ReInitializePowerUp(mItem, mIntParm2);
      mgr.DisplayAlertAboutOutOfAmmo(*mgr.GetPlayer(player), mItem);
    }
  }
}

void CScriptSpecialFunction::AcceptSetTimedItemAmount(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    uint player = ResolvePlayerIndex(mIntParm1, msg.GetOriginator(), mgr);
    if (player < mgr.GetNumPlayers()) {
      CPlayerState::EItemType item = mItem;
      CPlayerState* playerState = mgr.PlayerState(player);
      playerState->SetItemAmount(item, mIntParm2);
      playerState->SetTimeLeft(item, mValue1);
      mgr.DisplayAlertAboutOutOfAmmo(*mgr.GetPlayer(player), mItem);
    }
  }
}

void CScriptSpecialFunction::AcceptModifyItemAmount(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    uint player = ResolvePlayerIndex(mIntParm1, msg.GetOriginator(), mgr);
    if (player < mgr.GetNumPlayers()) {
      CPlayerState* playerState = mgr.PlayerState(player);
      if (mIntParm2 > 0) {
        playerState->IncrPickUp(mItem, mIntParm2);
      } else {
        playerState->DecrPickUp(mItem, -mIntParm2);
      }
      mgr.DisplayAlertAboutOutOfAmmo(*mgr.GetPlayer(player), mItem);
    }
  }
}

void CScriptSpecialFunction::AcceptModifyItemCapacity(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    uint player = ResolvePlayerIndex(mIntParm1, msg.GetOriginator(), mgr);
    if (player < mgr.GetNumPlayers()) {
      mgr.PlayerState(player)->AddPowerUp(mItem, mIntParm2);
      mgr.DisplayAlertAboutOutOfAmmo(*mgr.GetPlayer(player), mItem);
    }
  }
}

void CScriptSpecialFunction::AcceptModifyItem(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    uint player = ResolvePlayerIndex(mIntParm1, msg.GetOriginator(), mgr);
    if (player < mgr.GetNumPlayers()) {
      CPlayerState* playerState = mgr.PlayerState(player);
      playerState->AddPowerUp(mItem, mIntParm2);
      if (mIntParm2 > 0) {
        playerState->IncrPickUp(mItem, mIntParm2);
      } else {
        playerState->DecrPickUp(mItem, -mIntParm2);
      }
      mgr.DisplayAlertAboutOutOfAmmo(*mgr.GetPlayer(player), mItem);
    }
  }
}

void CScriptSpecialFunction::AcceptGiveTimedItem(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    uint player = ResolvePlayerIndex(mIntParm1, msg.GetOriginator(), mgr);
    if (player < mgr.GetNumPlayers()) {
      CPlayerState* playerState = mgr.PlayerState(player);
      playerState->ReInitializePowerUp(mItem, mIntParm2);
      playerState->SetItemAmount(mItem, mIntParm2);
      playerState->SetTimeLeft(mItem, mValue1);
      mgr.DisplayAlertAboutOutOfAmmo(*mgr.GetPlayer(player), mItem);
    }
  }
}

void CScriptSpecialFunction::AcceptLastDamager(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(msg.GetUnk()))) {
      if (const CHealthInfo* healthInfo = actor->GetHealthInfo()) {
        TUniqueId damager = healthInfo->GetDamageId1();
        if (damager != kInvalidUniqueId) {
          SendScriptMsgs(kSS_Zero, mgr, damager, kSM_None);
        }
      }
    }
  }
}

void CScriptSpecialFunction::fn_80107458(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Increment:
    mgr.SetUnkFlagA3(true);
    break;
  case kSM_Decrement:
    mgr.SetUnkFlagA3(false);
    break;
  }
}

void CScriptSpecialFunction::AcceptSilhouette(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Increment:
    mTargetSilhouetteStrength = 1.f;
    SetActive(true);
    break;
  case kSM_Decrement:
    mTargetSilhouetteStrength = 0.f;
    break;
  case kSM_Activate:
    mTargetSilhouetteStrength = 1.f;
    mSilhouetteStrength = 1.f;
    break;
  case kSM_Deactivate:
    mTargetSilhouetteStrength = 0.f;
    mSilhouetteStrength = 0.f;
    break;
  }
}

void CScriptSpecialFunction::AcceptPauseGame(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    mgr.EnterPauseScreen();
  }
}

void CScriptSpecialFunction::AcceptSkyboxLighting(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_XCRT:
    mIntParm1 = 0;
    break;
  case kSM_Increment:
    mIntParm1 = 1;
    break;
  case kSM_Decrement:
    mIntParm1 = 2;
    break;
  case kSM_SetToMax:
    mgr.World()->SetSkyboxLightingLevel(mValue4);
    mIntParm1 = 0;
    break;
  case kSM_SetToZero:
    mgr.World()->SetSkyboxLightingLevel(mValue3);
    mIntParm1 = 0;
    break;
  }
}

void CScriptSpecialFunction::AcceptAreaOcclusion(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_XCRT:
    mIntParm1 = 0;
    break;
  }
}

void CScriptSpecialFunction::AcceptMultiplayerEndConditions(CStateManager& mgr,
                                                            const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_XCRT:
    mIntParm1 = 0;
    mIntParm2 = 0;
    break;
  }
}

void CScriptSpecialFunction::AcceptViewFrustumTester(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Deactivate:
    if (GetActive()) {
      SetInFrustum(false);
      SendFrustumMessages(mgr);
    }
    break;
  }
}

void CScriptSpecialFunction::AcceptDamageActor(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Action: {
    CDamageInfo info = mDamageInfo;
    info.SetRadius(mValue1);
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_Connect, kSM_Attach);
    for (rstl::vector< TUniqueId >::const_iterator it = ids.begin(); it != ids.end(); ++it) {
      if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(*it))) {
        if (act->GetActive()) {
          mgr.ApplyDamage(msg.GetOriginator(), act->GetUniqueId(), msg.GetOriginator(), info,
                          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59),
                                                              CMaterialList()),
                          CVector3f::Zero());
        }
      }
    }
    break;
  }
  }
}

void CScriptSpecialFunction::AcceptRezbitState(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_XCRT:
    mValue2 = 0.f;
    break;
  case kSM_Action: {
    mValue2 = mValue1;
    uint player = mLastOriginatorPlayer == kInvalidUniqueId
                      ? 0
                      : mgr.MaskUIdNumPlayers(mLastOriginatorPlayer);
    mgr.GetPlayer(player)->SetRezbitState(CPlayer::kRS_Recovered);
    break;
  }
  case kSM_Deactivate: {
    uint player = mLastOriginatorPlayer == kInvalidUniqueId
                      ? 0
                      : mgr.MaskUIdNumPlayers(mLastOriginatorPlayer);
    if (mgr.GetPlayer(player)->GetRezbitState() == CPlayer::kRS_Recovered) {
      mgr.GetPlayer(player)->SetRezbitState(CPlayer::kRS_None);
    }
    break;
  }
  }
}

void CScriptSpecialFunction::AcceptFogPlane(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Increment:
    mIntParm1 = 1;
    break;
  case kSM_Decrement:
    mIntParm1 = 0;
    break;
  }
}

void CScriptSpecialFunction::AcceptBillboard(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_XCRT:
    mIntParm1 = (mIntParm1 & ~2) | ((mIntParm2 != 0) << 1);
    mValue4 = 0.f;
    mIntParm2 = 0;
    break;
  case kSM_Increment:
    mIntParm2 = 1;
    break;
  case kSM_Decrement:
    mIntParm2 = 2;
    break;
  case kSM_SetToMax:
    mIntParm2 = 0;
    mValue4 = 1.f;
    break;
  case kSM_SetToZero:
    mIntParm2 = 0;
    mValue4 = 0.f;
    break;
  case kSM_Start:
    mIntParm1 = (mIntParm1 & ~1) | 1;
    break;
  case kSM_Stop:
    mIntParm1 = mIntParm1 & ~1;
    break;
  }
}

void CScriptSpecialFunction::AcceptAreaDocks(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Start:
    mgr.World()->Area(GetCurrentAreaId())->EnableDocks();
    break;
  case kSM_Stop:
    mgr.World()->Area(GetCurrentAreaId())->DisableDocks(mgr);
    break;
  }
}

void CScriptSpecialFunction::AcceptEnvironmentVariable(CStateManager& mgr, const CScriptMsg& msg) {
  CEnvironmentVariable* var;
  if (mFunction == kSF_SystemStateEnvVarController) {
    var = gpGameState->SystemOptions().FindEnvironmentVariable(mStringParm.data());
  } else {
    var = gpGameState->PersistentOptions().FindEnvironmentVariable(mStringParm.data());
  }
  if (GetActive()) {
    switch (msg.GetMessage()) {
    case kSM_Increment:
      var->Set(var->GetValue() + 1);
      break;
    case kSM_Decrement:
      var->Set(var->GetValue() - 1);
      break;
    case kSM_SetToZero:
      if (var->GetValue() == var->GetMaximum()) {
        SendScriptMsgs(kSS_Opened, mgr, kInvalidUniqueId, kSM_None);
      }
      if (var->GetValue() == var->GetMinimum()) {
        SendScriptMsgs(kSS_Closed, mgr, kInvalidUniqueId, kSM_None);
      }
      break;
    }
  }
}

void CScriptSpecialFunction::AcceptMultiplayerResult(CStateManager& mgr, const CScriptMsg& msg) {
  if (GetActive() && msg.GetMessage() == kSM_SetToZero) {
    CGameMode& mode = gpGameState->GetGameMode();
    if (mode.GetGameModeType() == 'DTHM' || mode.GetGameModeType() == 'COIN') {
      int musicIndex = static_cast< CGMMultiplayer& >(mode).GetMusicIndex();
      if (musicIndex >= 0 && musicIndex < GetConnectionList().size()) {
        const SConnection& conn = GetConnectionList()[musicIndex];
        if (conn.state == kSS_Zero) {
          CStateManager::TIdListResult search = mgr.GetIdListForScript(conn.objId);

          CStateManager::TIdList::const_iterator current = search.first;
          CStateManager::TIdList::const_iterator end = search.second;
          while (current != end) {
            mgr.SendScriptMsg(
                CScriptMsg(GetUniqueId(), msg.GetOriginator(), current->second, conn.msg, conn.state));
            ++current;
          }
        }
      }
    }
  }
}

void CScriptSpecialFunction::AcceptMapObjectVisibility(CStateManager& mgr, const CScriptMsg& msg) {
  if (GetActive()) {
    switch (msg.GetMessage()) {
    case kSM_Decrement:
      mgr.MapWorldInfo()->SetObjectUnmapped(mgr.GetEditorIdForUniqueId(GetUniqueId()), true);
      break;
    case kSM_Increment:
      mgr.MapWorldInfo()->SetObjectUnmapped(mgr.GetEditorIdForUniqueId(GetUniqueId()), false);
      break;
    }
  }
}

void CScriptSpecialFunction::AcceptStopRezbitState(CStateManager& mgr, const CScriptMsg& msg) {
  if (GetActive() && msg.GetMessage() == kSM_SetToZero) {
    mgr.GetPlayer(0)->StopRezbitState(mgr);
  }
}

void CScriptSpecialFunction::AcceptCredits(CStateManager& mgr, const CScriptMsg& msg) {
  if (GetActive() && msg.GetMessage() == kSM_Action) {
    gpGameState->WorldTransitionManager()->WaitForModelsAndTextures();
    CIOWin* credits = rs_new CCredits();
    CArchitectureQueue& queue = mgr.ArchQueue();
    queue.Push(
        MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, kCreditsMsgPriority, kCreditsDrawPriority, credits));
  }
}

void CScriptSpecialFunction::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  EScriptObjectMessage message = msg.GetMessage();
  switch (mFunction) {
  case kSF_ViewFrustumTester:
    AcceptViewFrustumTester(mgr, msg);
    break;
  }

  CActor::AcceptScriptMsg(mgr, msg);

  switch (mFunction) {
  case kSF_ChaffTarget:
    AcceptChaffTarget(mgr, msg);
    break;
  case kSF_Silhouette:
    AcceptSilhouette(mgr, msg);
    break;
  }

  if (!GetActive() && message != kSM_XCRT && message != kSM_XALD) {
    return;
  }

  if (message == kSM_Activate || message == kSM_Increment || message == kSM_Action) {
    TUniqueId originator = msg.GetOriginator();
    if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(originator))) {
      mLastOriginatorPlayer = originator;
    }
  }

  void (CScriptSpecialFunction::*func)(CStateManager&, const CScriptMsg&) = nullptr;
  switch (mFunction) {
  case kSF_WorldSwapper:
    func = &CScriptSpecialFunction::AcceptDarkWorld;
    break;
  case kSF_DisableHud:
    func = &CScriptSpecialFunction::AcceptHUDFadeIn;
    break;
  case kSF_EscapeSequence:
    func = &CScriptSpecialFunction::AcceptEscapeSequence;
    break;
  case kSF_SpinnerController:
    func = &CScriptSpecialFunction::AcceptSpinner;
    break;
  case kSF_ShotSpinnerController:
    func = &CScriptSpecialFunction::AcceptShotSpinner;
    break;
  case kSF_MapStation:
    func = &CScriptSpecialFunction::AcceptMapStation;
    break;
  case kSF_MissileStation:
    func = &CScriptSpecialFunction::AcceptMissileStation;
    break;
  case kSF_PowerBombStation:
    func = &CScriptSpecialFunction::AcceptPowerBombStation;
    break;
  case kSF_SaveStation:
    func = &CScriptSpecialFunction::AcceptSaveStation;
    break;
  case kSF_RechargeStation:
    func = &CScriptSpecialFunction::AcceptEnergyTank;
    break;
  case kSF_RadialDamage:
    func = &CScriptSpecialFunction::AcceptRadialDamage;
    break;
  case kSF_BossEnergyBar:
    func = &CScriptSpecialFunction::AcceptBossEnergyBar;
    break;
  case kSF_EndGame:
    func = &CScriptSpecialFunction::AcceptEndGame;
    break;
  case kSF_CinematicSkip:
  case kSF_CinematicSkipSignal:
    func = &CScriptSpecialFunction::AcceptCinematicSkip;
    break;
  case kSF_EnvFxDensityController:
    func = &CScriptSpecialFunction::AcceptEnvFxDensity;
    break;
  case kSF_RumbleEffect:
    func = &CScriptSpecialFunction::AcceptRumble;
    break;
  case kSF_InventoryActivator:
    func = &CScriptSpecialFunction::AcceptInventoryActivator;
    break;
  case kSF_AreaDamage:
    func = &CScriptSpecialFunction::AcceptAreaDamage;
    break;
  case kSF_DropBomb:
    func = &CScriptSpecialFunction::AcceptDropBomb;
    break;
  case kSF_HintController:
    func = &CScriptSpecialFunction::AcceptHint;
    break;
  case kSF_PlayerInAreaRelay:
    func = &CScriptSpecialFunction::AcceptPlayerInArea;
    break;
  case kSF_HUDTarget:
    func = &CScriptSpecialFunction::AcceptHUDTarget;
    break;
  case kSF_UnderwaterFog:
    func = &CScriptSpecialFunction::AcceptFogFader;
    break;
  case kSF_EnterLogbookScreen:
    func = &CScriptSpecialFunction::AcceptLogbook;
    break;
  case kSF_EndingActivator:
    func = &CScriptSpecialFunction::AcceptEnding;
    break;
  case kSF_LaunchPlayer:
    func = &CScriptSpecialFunction::AcceptPlayerVelocity;
    break;
  case kSF_Function37:
    func = &CScriptSpecialFunction::fn_80107a58;
    break;
  case kSF_Function38:
    func = &CScriptSpecialFunction::AcceptPlayerSpawnPoint;
    break;
  case kSF_Function39:
    func = &CScriptSpecialFunction::fn_80107994;
    break;
  case kSF_SetInventoryCapacity:
    func = &CScriptSpecialFunction::AcceptSetItemCapacity;
    break;
  case kSF_SetInventoryAmount:
    func = &CScriptSpecialFunction::AcceptSetTimedItemAmount;
    break;
  case kSF_ModifyInventoryAmount:
    func = &CScriptSpecialFunction::AcceptModifyItemAmount;
    break;
  case kSF_ModifyInventoryCapacity:
    func = &CScriptSpecialFunction::AcceptModifyItemCapacity;
    break;
  case kSF_ModifyInventoryAmountAndCapacity:
    func = &CScriptSpecialFunction::AcceptModifyItem;
    break;
  case kSF_SetInventoryAmountAndCapacity:
    func = &CScriptSpecialFunction::AcceptGiveTimedItem;
    break;
  case kSF_Function48:
    func = &CScriptSpecialFunction::AcceptLastDamager;
    break;
  case kSF_DemoTimeoutResetController:
    func = &CScriptSpecialFunction::fn_80107458;
    break;
  case kSF_SunGeneratorTeleporter:
    func = &CScriptSpecialFunction::AcceptPauseGame;
    break;
  case kSF_SkyLighting:
    func = &CScriptSpecialFunction::AcceptSkyboxLighting;
    break;
  case kSF_OcclusionRelay:
    func = &CScriptSpecialFunction::AcceptAreaOcclusion;
    break;
  case kSF_MultiplayerCountdown:
    func = &CScriptSpecialFunction::AcceptMultiplayerEndConditions;
    break;
  case kSF_DamageActor:
    func = &CScriptSpecialFunction::AcceptDamageActor;
    break;
  case kSF_Function59:
    func = &CScriptSpecialFunction::AcceptRezbitState;
    break;
  case kSF_ExtraRenderClipPlane:
    func = &CScriptSpecialFunction::AcceptFogPlane;
    break;
  case kSF_VisorBlowout:
    func = &CScriptSpecialFunction::AcceptBillboard;
    break;
  case kSF_AreaAutoLoadController:
    func = &CScriptSpecialFunction::AcceptAreaDocks;
    break;
  case kSF_SystemStateEnvVarController:
  case kSF_GameStateEnvVarController:
    func = &CScriptSpecialFunction::AcceptEnvironmentVariable;
    break;
  case kSF_MultiplayerMusic:
    func = &CScriptSpecialFunction::AcceptMultiplayerResult;
    break;
  case kSF_UnmappableObject:
    func = &CScriptSpecialFunction::AcceptMapObjectVisibility;
    break;
  case kSF_RemoveRezbitVirus:
    func = &CScriptSpecialFunction::AcceptStopRezbitState;
    break;
  case kSF_CompletionScreen:
    func = &CScriptSpecialFunction::AcceptCredits;
    break;
  case kSF_ObjectFollowObject:
  case kSF_Billboard:
  case kSF_WeaponSwitch:
  case kSF_PlayerOffscreen:
    break;
  }

  if (func) {
    (this->*func)(mgr, msg);
  }
}

void CScriptSpecialFunction::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  void (CScriptSpecialFunction::*func)(float, CStateManager&) = nullptr;
  switch (mFunction) {
  case kSF_PlayerFollowLocator:
    func = &CScriptSpecialFunction::ThinkPlayerFollowLocator;
    break;
  case kSF_SpinnerController:
    ThinkSpinnerController(dt, mgr, kSCM_Spinner);
    break;
  case kSF_ShotSpinnerController:
    ThinkSpinnerController(dt, mgr, kSCM_ShotSpinner);
    break;
  case kSF_ObjectFollowLocator:
    func = &CScriptSpecialFunction::ThinkObjectFollowLocator;
    break;
  case kSF_ObjectFollowObject:
    func = &CScriptSpecialFunction::ThinkObjectFollowObject;
    break;
  case kSF_ChaffTarget:
    func = &CScriptSpecialFunction::ThinkChaffTarget;
    break;
  case kSF_ViewFrustumTester:
  case kSF_FogVolume:
    func = &CScriptSpecialFunction::ThinkViewFrustumTester;
    break;
  case kSF_SaveStation:
    ThinkSaveStation(dt, mgr);
    break;
  case kSF_RainSimulator:
    ThinkRainSimulator(dt, mgr);
    break;
  case kSF_AreaDamage:
    func = &CScriptSpecialFunction::ThinkAreaDamage;
    break;
  case kSF_ScaleActor:
    func = &CScriptSpecialFunction::ThinkActorScale;
    break;
  case kSF_PlayerInAreaRelay:
    func = &CScriptSpecialFunction::ThinkPlayerInArea;
    break;
  case kSF_PlayerOffscreen:
    func = &CScriptSpecialFunction::ThinkPlayerFrustumTester;
    break;
  case kSF_Function46:
    func = &CScriptSpecialFunction::ThinkPlayerItemRelay;
    break;
  case kSF_SunPlacement:
    func = &CScriptSpecialFunction::ThinkPlayerOffset;
    break;
  case kSF_TransparencyWipe:
    func = &CScriptSpecialFunction::ThinkConnectedEffectPlane;
    break;
  case kSF_Silhouette:
    func = &CScriptSpecialFunction::ThinkSilhouette;
    break;
  case kSF_SunGeneratorTeleporter:
    func = &CScriptSpecialFunction::ThinkMapTeleport;
    break;
  case kSF_SkyLighting:
    func = &CScriptSpecialFunction::ThinkSkyboxLighting;
    break;
  case kSF_OcclusionRelay:
    func = &CScriptSpecialFunction::ThinkAreaOcclusion;
    break;
  case kSF_MultiplayerCountdown:
    func = &CScriptSpecialFunction::ThinkMultiplayerEndConditions;
    break;
  case kSF_ScaleSZ:
    func = &CScriptSpecialFunction::ThinkTriggerScale;
    break;
  case kSF_ObjectFollowJoint:
    func = &CScriptSpecialFunction::ThinkObjectFollowJoint;
    break;
  case kSF_Function59:
    func = &CScriptSpecialFunction::ThinkRezbitState;
    break;
  case kSF_VisorBlowout:
    func = &CScriptSpecialFunction::ThinkBillboard;
    break;
  case kSF_IntroBossRingController:
  case kSF_ExtraRenderClipPlane:
    break;
  }

  if (func) {
    (this->*func)(dt, mgr);
  }
}

void CScriptSpecialFunction::ThinkBillboard(float dt, CStateManager& mgr) {
  switch (mIntParm2) {
  case 1:
    if (!CMath::IsEpsilon(mValue2, 0.f, 0.00001f)) {
      float alpha = mValue4 + dt / mValue2;
      if (alpha >= 1.f) {
        mValue4 = 1.f;
        mIntParm2 = 0;
      } else {
        mValue4 = alpha;
      }
    } else {
      mValue4 = 1.f;
      mIntParm2 = 0;
    }
    break;
  case 2:
    if (!CMath::IsEpsilon(mValue3, 0.f, 0.00001f)) {
      float alpha = mValue4 - dt / mValue3;
      if (alpha <= 0.f) {
        mValue4 = 0.f;
        mIntParm2 = 0;
      } else {
        mValue4 = alpha;
      }
    } else {
      mValue4 = 0.f;
      mIntParm2 = 0;
    }
    break;
  }
}

void CScriptSpecialFunction::ThinkSaveStation(float dt, CStateManager& mgr) {
  if (mDoSave && !mgr.GetWantsToEnterSaveGameScreen()) {
    mDoSave = false;
    if (mgr.GetInSaveUI()) {
      SendScriptMsgs(kSS_MaxReached, mgr, kInvalidUniqueId, kSM_None);
    } else {
      SendScriptMsgs(kSS_Zero, mgr, kInvalidUniqueId, kSM_None);
    }
  }
}

void CScriptSpecialFunction::ThinkPlayerFollowLocator(float dt, CStateManager& mgr) {
  // TODO: move the originating player to the connected actor's locator.
}

void CScriptSpecialFunction::ThinkSpinnerController(float dt, CStateManager& mgr,
                                                    ESpinnerControllerMode mode) {
  // TODO: update spinner progress, connected actors and its sound emitter.
}

void CScriptSpecialFunction::ThinkObjectFollowLocator(float dt, CStateManager& mgr) {
  // TODO: move connected actors to the source actor's locator.
}

void CScriptSpecialFunction::ThinkObjectFollowObject(float dt, CStateManager& mgr) {
  // TODO: copy the active source actor's transform to its connected target.
}

void CScriptSpecialFunction::ThinkChaffTarget(float dt, CStateManager& mgr) {
  // TODO: inspect nearby projectiles and update each player's HUD interference.
}

void CScriptSpecialFunction::ThinkRainSimulator(float dt, CStateManager& mgr) {
  if (static_cast< float >(static_cast< uint >(mgr.GetUpdateFrameIdx()) % 3600) / 3600.f < 0.5f) {
    SendScriptMsgs(kSS_MaxReached, mgr, kInvalidUniqueId, kSM_None);
  } else {
    SendScriptMsgs(kSS_Zero, mgr, kInvalidUniqueId, kSM_None);
  }
}

void CScriptSpecialFunction::ThinkAreaDamage(float dt, CStateManager& mgr) {
  // TODO: track single-player area damage and apply the frame's damage.
}

void CScriptSpecialFunction::ThinkActorScale(float dt, CStateManager& mgr) {
  // TODO: scale connected actors toward the configured limit.
}

void CScriptSpecialFunction::ThinkPlayerInArea(float dt, CStateManager& mgr) {
  if (mgr.GetPlayer(0)->GetCurrentAreaId() == GetCurrentAreaId()) {
    if (!mPlayerInArea) {
      mPlayerInArea = true;
      SendScriptMsgs(kSS_Entered, mgr, kInvalidUniqueId, kSM_None);
    }
  } else if (mPlayerInArea) {
    mPlayerInArea = false;
    SendScriptMsgs(kSS_Exited, mgr, kInvalidUniqueId, kSM_None);
  }
}

void CScriptSpecialFunction::ThinkViewFrustumTester(float dt, CStateManager& mgr) {
  SendFrustumMessages(mgr);
}

void CScriptSpecialFunction::ThinkPlayerFrustumTester(float dt, CStateManager& mgr) {
  uint player = mIntParm1;
  if (player < mgr.GetNumPlayers()) {
    SetTranslation(mgr.GetPlayer(player)->GetTranslation());
  }
  SendFrustumMessages(mgr);
}

void CScriptSpecialFunction::ThinkPlayerItemRelay(float dt, CStateManager& mgr) {
  for (int i = 0; i < static_cast< uint >(mgr.GetNumPlayers()); ++i) {
    int amount = mgr.PlayerState(i)->GetItemAmount(mItem, true);
    if (amount != 0 && amount <= static_cast< uint >(mgr.GetNumPlayers())) {
      SendScriptMsgs(kSS_Play, mgr, mgr.GetPlayer(i)->GetUniqueId(), kSM_None);
      SendScriptMsgs(kSS_Zero, mgr, mgr.GetPlayer(amount - 1)->GetUniqueId(), kSM_None);
    }
  }
}

void CScriptSpecialFunction::ThinkPlayerOffset(float dt, CStateManager& mgr) {
  // TODO: position connected actors at the configured angular offset from the player.
}

void CScriptSpecialFunction::ThinkConnectedEffectPlane(float dt, CStateManager& mgr) {
  // TODO: update the connected effect's plane from its source actor.
}

void CScriptSpecialFunction::ThinkSilhouette(float dt, CStateManager& mgr) {
  if (mTargetSilhouetteStrength > mSilhouetteStrength) {
    mSilhouetteStrength += dt / mValue2;
    mSilhouetteStrength = mSilhouetteStrength < mTargetSilhouetteStrength ? mSilhouetteStrength : mTargetSilhouetteStrength;
  } else if (mTargetSilhouetteStrength < mSilhouetteStrength) {
    mSilhouetteStrength -= dt / mValue2;
    mSilhouetteStrength = mTargetSilhouetteStrength < mSilhouetteStrength ? mSilhouetteStrength : mTargetSilhouetteStrength;
  }
}

void CScriptSpecialFunction::ThinkMapTeleport(float dt, CStateManager& mgr) {
  CAssetId worldId = mgr.GetMapTeleportWorldId();
  if (worldId != kInvalidAssetId) {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_Play, kSM_Activate);
    for (rstl::vector< TUniqueId >::const_iterator it = ids.begin(); it != ids.end(); ++it) {
      if (CScriptWorldTeleporter* teleporter =
              TCastToPtr< CScriptWorldTeleporter >(mgr.ObjectById(*it))) {
        bool active = teleporter->GetWorldId() == worldId;
        mgr.SendScriptMsg(teleporter, GetUniqueId(),
                          active ? kSM_Activate : kSM_Deactivate,
                          kInvalidUniqueId);
      }
    }
    SendScriptMsgs(mgr.World()->GetWorldAssetId() == worldId ? kSS_Zero : kSS_MaxReached, mgr,
                   kInvalidUniqueId, kSM_None);
    mgr.SetMapTeleportWorldId(kInvalidAssetId);
  }
}

void CScriptSpecialFunction::ThinkSkyboxLighting(float dt, CStateManager& mgr) {
  CWorld* world = mgr.World();
  float level = world->GetSkyboxLightingLevel();
  bool changed = true;
  switch (mIntParm1) {
  case 1:
    level += dt * mValue2;
    break;
  case 2:
    level -= dt * mValue1;
    break;
  default:
    changed = false;
    break;
  }
  if (changed) {
    world->SetSkyboxLightingLevel(CMath::Clamp(mValue3, level, mValue4));
  }
}

void CScriptSpecialFunction::ThinkAreaOcclusion(float dt, CStateManager& mgr) {
  int state = mgr.World()->Area(GetCurrentAreaId())->GetOcclusionState();
  if (state != mIntParm1) {
    if (state == CGameArea::kOS_Occluded) {
      SendScriptMsgs(kSS_InternalState00, mgr, kInvalidUniqueId, kSM_None);
    } else if (state == CGameArea::kOS_Visible) {
      SendScriptMsgs(kSS_InternalState01, mgr, kInvalidUniqueId, kSM_None);
    }
    mIntParm1 = state;
  }
}

void CScriptSpecialFunction::ThinkMultiplayerEndConditions(float dt, CStateManager& mgr) {
  // TODO: send the multiplayer time/score threshold messages.
}

void CScriptSpecialFunction::ThinkTriggerScale(float dt, CStateManager& mgr) {
  if (CScriptTriggerEllipsoid* trigger = TCastToPtr< CScriptTriggerEllipsoid >(
          mgr.ObjectById(FindConnectedObject(mgr, kSS_Connect, kSM_Attach)))) {
    float d = dt * mValue1;
    trigger->SetScale(trigger->GetScale() + CVector3f(d, d, d));
  }
}

void CScriptSpecialFunction::ThinkObjectFollowJoint(float dt, CStateManager& mgr) {
  // TODO: include the source joint's bind rotation when following a locator.
}

void CScriptSpecialFunction::ThinkRezbitState(float dt, CStateManager& mgr) {
  uint player = mLastOriginatorPlayer == kInvalidUniqueId
                    ? 0
                    : mgr.MaskUIdNumPlayers(mLastOriginatorPlayer);
  if (mgr.GetPlayer(player)->GetRezbitState() == CPlayer::kRS_Recovered) {
    mValue2 -= dt;
    if (mValue2 <= 0.f) {
      mgr.GetPlayer(player)->SetRezbitState(CPlayer::kRS_None);
    }
  }
}

void CScriptSpecialFunction::AddOrUpdateEmitter(float pitch, float maxDist, float falloff,
                                                CSfxHandle& handle, ushort id, CVector3f position,
                                                uchar volume) {
  // TODO: create or update the spinner emitter with Echoes sound parameters.
}

void CScriptSpecialFunction::DeleteEmitter(CSfxHandle& handle) {
  if (handle) {
    CSfxManager::RemoveEmitter(handle);
    handle.Clear();
  }
}

void CScriptSpecialFunction::SkipCinematic(CStateManager& mgr) {
  SendScriptMsgs(kSS_Zero, mgr, kInvalidUniqueId, kSM_None);
  mgr.SetSkipCinematicSpecialFunction(kInvalidUniqueId);
}

void CScriptSpecialFunction::SetInFrustum(bool inFrustum) {
  if (mInFrustum == inFrustum) {
    return;
  }

  if (inFrustum) {
    if (mFrustumExited) {
      mFrustumExited = false;
    } else {
      mFrustumEntered = true;
    }
  } else if (mFrustumEntered) {
    mFrustumEntered = false;
  } else {
    mFrustumExited = true;
  }
  mInFrustum = inFrustum;
}

int CScriptSpecialFunction::ResolvePlayerIndex(int playerIndex, const TUniqueId& originator,
                                               CStateManager& mgr) {
  if (playerIndex == -1 && TCastToConstPtr< CPlayer >(mgr.GetObjectById(originator))) {
    return mgr.MaskUIdNumPlayers(originator);
  }
  return playerIndex;
}

void CScriptSpecialFunction::OnItemDepleted(CStateManager& mgr, int playerIndex,
                                            CPlayerState::EItemType item) {
  if (mFunction == kSF_ItemDepletion && item == mItem &&
      mgr.GetPlayerState(playerIndex)->GetItemAmount(item, true) == 0) {
    SendScriptMsgs(kSS_Zero, mgr, mgr.GetPlayer(playerIndex)->GetUniqueId(), kSM_None);
  }
}

void CScriptSpecialFunction::SendFrustumMessages(CStateManager& mgr) {
  if (mFrustumEntered) {
    mFrustumEntered = false;
    SendScriptMsgs(kSS_Entered, mgr, kInvalidUniqueId, kSM_None);
  }
  if (mFrustumExited) {
    mFrustumExited = false;
    SendScriptMsgs(kSS_Exited, mgr, kInvalidUniqueId, kSM_None);
  }
}

void CScriptSpecialFunction::PreRenderAllViewports(CStateManager& mgr) {
  // TODO: submit the per-viewport fog settings for the selected function.
}

CScriptSpecialFunction::~CScriptSpecialFunction() {}
