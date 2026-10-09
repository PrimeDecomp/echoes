#include "MetroidPrime/ScriptObjects/CScriptSpecialFunction.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CArchitectureMessage.hpp"
#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/CAxisAngle.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CCredits.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CMapWorld.hpp"
#include "MetroidPrime/CMapWorldInfo.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CEnvironmentVariable.hpp"
#include "MetroidPrime/Player/CGMMultiplayer.hpp"
#include "MetroidPrime/Player/CGMSinglePlayer.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CWorldTransManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageActor.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEnvFxDensityController.hpp"
#include "MetroidPrime/ScriptLoader/SLdrFogVolume.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRadialDamage.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRumbleEffect.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSilhouette.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSpecialFunction.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSpinner.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActorRotate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpawnPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTriggerEllipsoid.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWorldTeleporter.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

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
  const CAABox box(GetTranslation() - mVectorParm, max);
  SetInFrustum(mgr.GetFrustumPlanes().BoxInFrustumPlanes(box));
}

void CScriptSpecialFunction::PreRenderViewFrustumTester(CStateManager& mgr) {
  SetInFrustum(mgr.GetFrustumPlanes().PointInFrustumPlanes(GetTranslation()));
}

void CScriptSpecialFunction::PreRenderPlayerFrustumTester(CStateManager& mgr) {
  if (static_cast< uint >(mIntParm2) == mgr.GetCurrentRenderPlayerIndex()) {
    SetInFrustum(mgr.GetFrustumPlanes().PointInFrustumPlanes(GetTranslation()));
  }
}

void CScriptSpecialFunction::PreRenderSilhouette(CStateManager& mgr) {
  SetInFrustum(false);
  if (mSilhouetteStrength <= 0.f) {
    return;
  }
  CActor* act =
      TCastToPtr< CActor >(mgr.ObjectById(FindConnectedObject(mgr, kSS_Connect, kSM_Attach)));
  if (!act || !act->GetActive()) {
    return;
  }
  SetOtherBounds(act->GetOtherBounds());
  SetRenderBounds(act->GetOtherBounds());
  SetInFrustum(mgr.GetFrustumPlanes().BoxInFrustumPlanes(act->GetOtherBounds()));
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
  if (GetActive()) {
    const float z =
        mgr.IntegrateVisorFog(mValue1 * CMath::FastSinR(CGraphics::GetSecondsMod900() * mValue2));
    if (z > 0.f) {
      const CVector3f pos = GetTranslation();
      CVector3f min(pos - mVectorParm);
      CVector3f max(pos + mVectorParm);
      max[kDZ] += z;
      CAABox box(min, max);
      CTransform4f modelMtx = CTransform4f::Translate(box.GetCenterPoint()) *
                              CTransform4f::Scale((box.GetMaxPoint() - box.GetMinPoint()) * 0.5f);

      CAABox renderbox(CVector3f(-1.f, -1.f, -1.f), CVector3f(1.f, 1.f, 1.f));

      gpRender->SetModelMatrix(modelMtx);
      gpRender->RenderFogVolume(mColorParm, renderbox, nullptr, nullptr);
    }
  }
}

void CScriptSpecialFunction::RenderSilhouette(const CStateManager& mgr) const {
  const CActor* act = TCastToConstPtr< CActor >(
      mgr.GetObjectById(FindConnectedObject(mgr, kSS_Connect, kSM_Attach)));
  if (!act || !act->GetActive() || !act->GetDrawEnabled()) {
    return;
  }

  CCubeRenderer* const renderer = gpRender;
  renderer->AllocatePhazonSuitMaskTexture();
  renderer->CopyScreenTex(3, true, CGraphics::GetDolphinSpareBuffer(), GX_TF_RGB565, false);
  CGX::SetDstAlpha(true, 0xff);
  gpRender->SetModelMatrix(act->GetTransform());
  act->Render(mgr);
  renderer->RenderSilhouette(
      mValue1,
      CColor(mSilhouetteStrength * mColorParm.GetRed(), mSilhouetteStrength * mColorParm.GetGreen(),
             mSilhouetteStrength * mColorParm.GetBlue(), mColorParm.GetAlpha()),
      rstl::optional_object< TCachedToken< CTexture > >(), 0.f, 0.f, 0.f, CColor::White());
}

void CScriptSpecialFunction::RenderBillboard() const {
  CCubeRenderer::That()->GetSphereRamp().Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  const CTransform4f& view = CGraphics::GetViewMatrix();
  const CVector3f pos =
      (mIntParm1 & 2) ? view.GetTranslation() + view.GetForward() * 10.f : GetTranslation();
  float alpha = mValue4;
  if (!(mIntParm1 & 2)) {
    const CVector3f delta = pos - view.GetTranslation();
    if (!delta.CanBeNormalized()) {
      return;
    }
    const float dot = CVector3f::Dot(delta.AsNormalized(), view.GetForward());
    if (dot < 0.f) {
      return;
    }
    alpha *= dot;
  }

  const float scale = alpha * mValue1;
  const CVector3f right = view.GetRight() * scale;
  const CVector3f up = view.GetUp() * scale;
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  CGraphics::SetDepthWriteMode(false, kE_Always, false);
  CGraphics::DisableAllLights();
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_Or, kAF_Always, 0);
  const CColor color(alpha, alpha, alpha, alpha);
  CGraphics::StreamColor(color);
  CGraphics::StreamBegin(kP_TriangleFan);
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(pos - right + up);
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(pos - right - up);
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(pos + right - up);
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(pos + right + up);
  CGraphics::StreamEnd();
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
  case kSM_AreaLoaded:
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
    SendScriptMsgs(kSS_Play, mgr);
    break;
  case kSM_SetToMax:
    mShotSpinnerImpulse = mValue3;
    SendScriptMsgs(kSS_Play, mgr);
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
        SendScriptMsgs(kSS_Closed, mgr);
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
  case kSM_Create:
    if ((mIntParm1 & 1) == 0 || !GetActive()) {
      break;
    }
  case kSM_Action: {
    CDamageInfo info = mDamageInfo;
    info.SetRadius(mValue1);
    if ((mIntParm1 & 4) != 0) {
      mgr.ApplyDamage(
          GetUniqueId(), msg.GetOriginator(), kInvalidUniqueId, info,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
    } else {
      mgr.ApplyDamageToWorld(
          GetUniqueId(), *this, GetTranslation(), info,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()));
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

      mgr.SetBossParams(msg.GetSenderId(), mValue1, stringIdx);
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
    static_cast< const CGameState* >(gpGameState)->GetGameMode().EndGame(mIntParm1, mgr);
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
  case kSM_Delete:
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
    if (rumbFxIdx >= 0 &&
        rumbFxIdx < static_cast< int >(sizeof(skRumbleFxList) / sizeof(ERumbleFxId))) {
      ERumbleFxId rumbFx = skRumbleFxList[rumbFxIdx];
      uint flags = mValue3;
      if ((flags & 1) != 0) {
        mgr.RumbleManager(0)->Rumble(mgr, rumbFx, 1.f, kRP_One);
      } else {
        CVector3f pos = GetTranslation();
        if ((flags & 2) != 0) {
          if (const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(msg.GetSenderId()))) {
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
        SendScriptMsgs(kSS_Zero, mgr);
        return;
      }
    }
  }
}

void CScriptSpecialFunction::AcceptAreaDamage(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Deactivate:
  case kSM_Delete:
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
      SendScriptMsgs(kSS_Zero, mgr);
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
        SendScriptMsgs(kSS_Zero, mgr);
      }
    }
  }
}

void CScriptSpecialFunction::AcceptPlayerVelocity(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    CActor* actor =
        TCastToPtr< CActor >(mgr.ObjectById(FindConnectedObject(mgr, kSS_Play, kSM_Activate)));
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(msg.GetOriginator()));
    if (player) {
      CVector3f dir = (actor->GetTranslation() - player->GetTranslation()).AsNormalized();
      player->SetVelocityWR(mValue1 * dir);
    }
  }
}

void CScriptSpecialFunction::AcceptDarkWorld(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_AreaLoaded:
    if (GetActive()) {
      if (mgr.GetIsDarkWorld()) {
        SendScriptMsgs(kSS_Zero, mgr);
      } else {
        SendScriptMsgs(kSS_MaxReached, mgr);
      }
    }
    break;
  }
}

void CScriptSpecialFunction::fn_80107a58(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    uint player = mIntParm1;
    if (player < mgr.GetNumPlayers()) {
      static_cast< const CGameState* >(gpGameState)->GetGameMode().RespawnPlayer(mgr, player);
    }
  }
}

void CScriptSpecialFunction::AcceptPlayerSpawnPoint(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    uint player = mIntParm1;
    if (player < mgr.GetNumPlayers()) {
      TUniqueId spawn = FindConnectedObject(mgr, kSS_Play, kSM_Activate);
      if (TCastToConstPtr< CScriptSpawnPoint >(mgr.GetObjectById(spawn))) {
        static_cast< const CGameState* >(gpGameState)
            ->GetGameMode()
            .SetSpawnPoint(mIntParm1, spawn);
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
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(msg.GetSenderId()))) {
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
  case kSM_Create:
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
  case kSM_Create:
    mIntParm1 = 0;
    break;
  }
}

void CScriptSpecialFunction::AcceptMultiplayerEndConditions(CStateManager& mgr,
                                                            const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Create:
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
          mgr.ApplyDamage(
              msg.GetOriginator(), act->GetUniqueId(), msg.GetOriginator(), info,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
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
  case kSM_Create:
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
  case kSM_Create:
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
    var = gpGameState->SystemOptions().EnvVars().FindEnvironmentVariable(mStringParm.data());
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
        SendScriptMsgs(kSS_Open, mgr);
      }
      if (var->GetValue() == var->GetMinimum()) {
        SendScriptMsgs(kSS_Closed, mgr);
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
            mgr.SendScriptMsg(CScriptMsg(GetUniqueId(), current->second, conn.msg,
                                         msg.GetOriginator(), conn.state));
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
    queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, kCreditsMsgPriority,
                                          kCreditsDrawPriority, credits));
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

  if (!GetActive() && message != kSM_Create && message != kSM_AreaLoaded) {
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
      SendScriptMsgs(kSS_MaxReached, mgr);
    } else {
      SendScriptMsgs(kSS_Zero, mgr);
    }
  }
}

void CScriptSpecialFunction::ThinkPlayerFollowLocator(float dt, CStateManager& mgr) {
  if (const CActor* act = TCastToConstPtr< CActor >(
          mgr.GetObjectById(FindConnectedObject(mgr, kSS_Play, kSM_Activate)))) {
    CTransform4f xf = act->HasAnimation()
                          ? act->GetTransform() * act->GetScaledLocatorTransform(mStringParm)
                          : act->GetTransform();
    if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mLastOriginatorPlayer))) {
      CTransform4f playerXf =
          CTransform4f::Translate(CVector3f(0.f, 0.f, -player->GetMorphBall()->GetBallRadius())) *
          xf;
      player->SetTransform(playerXf);
      player->SetVelocityWR(CVector3f::Zero());
      player->SetAngularVelocityWR(CAxisAngle::Identity());
      player->ClearForcesAndTorques();
    }
  }
}

void CScriptSpecialFunction::ThinkSpinnerController(float dt, CStateManager& mgr,
                                                    ESpinnerControllerMode mode) {
  ushort sfx1 = static_cast< ushort >(mSfx1);
  ushort sfx3 = static_cast< ushort >(mSfx3);
  const float value1 = mValue1;
  const float value2 = mValue2;
  const float value4 = mValue4;
  if (mgr.IsMultiplayer()) {
    return;
  }
  if (!mSpinnerCanMove && mSfx2Played) {
    return;
  }

  const bool allowWrap = (mIntParm1 & 1) != 0;
  const bool noBackward = (mIntParm1 & 2) != 0;
  const bool splineControl = (mIntParm1 & 4) != 0;
  if (mSfx3Played && !allowWrap) {
    return;
  }

  rstl::vector< TUniqueId > ids;
  {
    rstl::vector< TUniqueId > platforms(FindConnectedObjects(mgr, kSS_Play, kSM_Activate));
    rstl::vector< TUniqueId > rotators(FindConnectedObjects(mgr, kSS_Connect, kSM_Attach));
    ids.reserve(platforms.size() + rotators.size());
    ids.insert(ids.begin(), platforms.begin(), platforms.end());
    ids.insert(ids.end(), rotators.begin(), rotators.end());
  }

  const float decay = 0.1f * dt * value2;
  const float previous = mSpinnerPosition;

  if (mode == kSCM_Spinner) {
    if (mSpinnerCanMove) {
      CPlayer* player = mgr.GetPlayer(0);
      const CPlayer::EPlayerMorphBallState morphState =
          player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
              ? player->GetMorphballTransitionState()
              : CPlayer::kMS_Unmorphed;
      bool isMorphed = morphState == CPlayer::kMS_Morphed;
      const CVector3f angVel = player->GetAngularVelocityOR().GetVector();
      float mag = angVel.CanBeNormalized() ? angVel.Magnitude() : 0.f;
      const float spinImpulse = isMorphed ? 0.025f * mag : 0.f;
      if (spinImpulse > mPreviousSpinnerSpeed) {
        SendScriptMsgs(kSS_Play, mgr);
      }

      mPreviousSpinnerSpeed = spinImpulse;
      mSpinnerPosition += 0.01f * spinImpulse * value1;

      if (!noBackward) {
        mSpinnerPosition -= decay;
      }
    } else if (!noBackward) {
      mSpinnerPosition = previous - 2.f * dt;
    }
  } else if (mode == kSCM_ShotSpinner) {
    mSpinnerPosition = (0.01f * mShotSpinnerImpulse) * value1 + previous;

    if (!noBackward) {
      mSpinnerPosition -= decay;

      if (CMath::AbsF(mShotSpinnerImpulse) < dt) {
        mShotSpinnerImpulse = 0.f;
      } else {
        mShotSpinnerImpulse = -(dt * CMath::Sign(mShotSpinnerImpulse) - mShotSpinnerImpulse);
      }
    }
  }

  if (allowWrap) {
    mSpinnerPosition = fmod(mSpinnerPosition, 1.0);
    if (mSpinnerPosition < 0.f) {
      mSpinnerPosition += 1.f;
    }
  } else {
    mSpinnerPosition = rstl::min_val(1.f, rstl::max_val(0.f, mSpinnerPosition));
  }

  bool noSfxPlayed = true;
  const float movementDelta = mSpinnerPosition - previous;
  if (close_enough(mSpinnerPosition, 1.f)) {
    mSpinnerPosition = 1.f;
    if (!mSfx3Played) {
      if (sfx3 != CSfxManager::kInternalInvalidSfxId) {
        CSfxManager::AddEmitter(sfx3, GetTranslation(), GetCurrentAreaId().Value(), true, false);
      }

      mSfx3Played = true;
    }

    SendScriptMsgs(kSS_MaxReached, mgr);
    noSfxPlayed = false;
  } else {
    mSfx3Played = false;
  }

  if (close_enough(mSpinnerPosition, 0.f)) {
    mSpinnerPosition = 0.f;
    if (!mSfx2Played) {
      mSfx2Played = true;
    }

    SendScriptMsgs(kSS_Zero, mgr);
    noSfxPlayed = false;
  } else {
    mSfx2Played = false;
  }

  rstl::optional_object< float > previousAverage = mVolumeAverage.GetAverage();

  if (noSfxPlayed) {
    if (sfx1 != CSfxManager::kInternalInvalidSfxId) {
      bool movingForward = movementDelta >= 0.f;
      if (noSfxPlayed) {
        mVolumeAverage.AddValue(movingForward ? uchar(100) : uchar(0x7f));
      } else {
        mVolumeAverage.AddValue(0.f);
      }
      const float& volume = mVolumeAverage.GetAverage().data();
      const float pitch = movingForward ? value4 : 1.f;
      AddOrUpdateEmitter(pitch, 200.f, 1.f, mSfxHandle, sfx1, GetTranslation(),
                         static_cast< uchar >(volume));
    }
  } else {
    DeleteEmitter(mSfxHandle);
  }

  for (rstl::vector< TUniqueId >::const_iterator it = ids.begin(); it != ids.end(); ++it) {
    CScriptPlatform* plat = TCastToPtr< CScriptPlatform >(mgr.ObjectById(*it));
    CScriptActorRotate* rot = TCastToPtr< CScriptActorRotate >(mgr.ObjectById(*it));
    if (plat) {
      plat->SetControlledAnimation(true);
      if (!mSpinnerInitializedXf) {
        mSpinnerInitialXf = plat->GetTransform();
        mSpinnerInitializedXf = true;
      }

      if (splineControl) {
        plat->SetMotionTime(mSpinnerPosition * plat->GetMotionDuration(), mgr);
      } else {
        const float dur = mSpinnerPosition * plat->GetAnimationData()->GetAnimationDuration(
                                                 plat->GetAnimationData()->GetCurrentAnimation());
        plat->AnimationData()->SetPhase(0.f);
        plat->AnimationData()->SetPlaybackRate(1.f);
        CAdvancementDeltas deltas = plat->UpdateAnimation(dur, mgr, true);
        plat->SetTransform(mSpinnerInitialXf *
                           deltas.GetOrientationDelta().BuildTransform4f(deltas.GetOffsetDelta()));
      }
    }

    if (rot) {
      rot->SetExternalTime();
      if (!rot->IsPlaying()) {
        rot->UpdateActors(false, mgr);
      }
      rot->SetCurrentTime(mSpinnerPosition * rot->GetDuration(), mgr);
      if (mSfx3Played || (mSfx2Played && !mSpinnerCanMove)) {
        rot->UpdateActorRotations(dt, mgr);
        rot->StopRotation();
      }
    }
  }
}

void CScriptSpecialFunction::ThinkObjectFollowLocator(float dt, CStateManager& mgr) {
  rstl::vector< TUniqueId > followers;
  TUniqueId followedAct = kInvalidUniqueId;

  int count = 0;
  for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
       conn != GetConnectionList().end(); ++conn) {
    if (conn->state == kSS_Play && conn->msg == kSM_Deactivate) {
      ++count;
    }
  }
  followers.reserve(count);

  for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
       conn != GetConnectionList().end(); ++conn) {
    if (conn->state != kSS_Play || (conn->msg != kSM_Activate && conn->msg != kSM_Deactivate)) {
      continue;
    }

    const CStateManager::TIdListResult ids = mgr.GetIdListForScript(conn->objId);
    if (ids.first == ids.second) {
      continue;
    }
    for (CStateManager::TIdList::const_iterator id = ids.first; id != ids.second; ++id) {
      TUniqueId uid = id->second;
      if (const CActor* const act = TCastToConstPtr< CActor >(mgr.GetObjectById(uid))) {
        if (conn->msg == kSM_Activate && act->HasAnimation()) {
          if (!act->GetActive()) {
            return;
          }
          followedAct = uid;
        } else if (conn->msg == kSM_Deactivate) {
          followers.push_back_unsafe(uid);
        }
      }
    }
  }

  const CActor* const followed = TCastToConstPtr< CActor >(mgr.GetObjectById(followedAct));
  if (followedAct == kInvalidUniqueId || !followed) {
    return;
  }

  for (rstl::vector< TUniqueId >::iterator it = followers.begin(); it != followers.end(); ++it) {
    CActor* follower = TCastToPtr< CActor >(mgr.ObjectById(*it));
    if (followed && follower) {
      CTransform4f xf = followed->GetTransform() * followed->GetScaledLocatorTransform(mStringParm);
      follower->SetTransform(xf);
    }
  }
}

void CScriptSpecialFunction::ThinkObjectFollowObject(float dt, CStateManager& mgr) {
  TUniqueId followerAct = kInvalidUniqueId;
  TUniqueId followedAct = kInvalidUniqueId;
  for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
       conn != GetConnectionList().end(); ++conn) {
    if (conn->state != kSS_Play || (conn->msg != kSM_Activate && conn->msg != kSM_Deactivate)) {
      continue;
    }

    const CStateManager::TIdListResult it = mgr.GetIdListForScript(conn->objId);
    if (!(it.first == it.second)) {
      TUniqueId uid = it.first->second;
      if (const CActor* const act = TCastToConstPtr< CActor >(mgr.GetObjectById(uid))) {
        if (conn->msg == kSM_Activate) {
          if (act->GetActive()) {
            followedAct = uid;
          }
        } else if (conn->msg == kSM_Deactivate) {
          followerAct = uid;
        }
      }
    }
  }

  const CActor* followed = TCastToConstPtr< CActor >(mgr.GetObjectById(followedAct));
  CActor* follower = TCastToPtr< CActor >(mgr.ObjectById(followerAct));
  if (follower && followed) {
    follower->SetTransform(followed->GetTransform());
  }
}

void CScriptSpecialFunction::ThinkChaffTarget(float dt, CStateManager& mgr) {
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  const CVector3f offset(5.f, 5.f, 5.f);
  const CAABox box(GetTranslation() - offset, GetTranslation() + offset);
  mgr.BuildNearList(nearList, box, CMaterialFilter::MakeInclude(CMaterialList(kMT_Projectile)),
                    nullptr);

  for (int i = 0; i < nearList.size(); ++i) {
    if (CEnergyProjectile* proj = TCastToPtr< CEnergyProjectile >(mgr.ObjectById(nearList[i]))) {
      if (proj->GetHomingTargetId() == GetUniqueId()) {
        proj->SetExplodePending(true);
        for (int p = 0; p < static_cast< uint >(mgr.GetNumPlayers()); ++p) {
          if (mgr.GetPlayer(p)->GetCurrentAreaId() == GetCurrentAreaId()) {
            mgr.Player(p)->SetHudDisable(mValue2);
            mChaffTimer = mValue1;

            CCameraFilterPass& filter = mgr.CameraFilterPass(p, 7);
            filter.SetFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen, 0.f,
                             CColor(1.f, 1.f, 1.f, 1.f), kInvalidAssetId);
            filter.DisableFilter(0.1f);
          }
        }
      }
    }
  }

  bool addedInterference = false;
  mChaffTimer = rstl::max_val(mChaffTimer - dt, 0.f);
  for (int p = 0; p < static_cast< uint >(mgr.GetNumPlayers()); ++p) {
    CPlayer* player = mgr.Player(p);
    if (mChaffTimer && player->GetCurrentAreaId() == GetCurrentAreaId()) {
      addedInterference = true;
      float intfMag = mValue3 * (0.5f + ((0.5f * mChaffTimer) / mValue1));
      if (mChaffTimer < 1.f) {
        intfMag *= mChaffTimer;
      }
      player->GetPlayerState()->StaticInterference().AddSource(GetUniqueId(), intfMag, 0.5f);
    }
    if (addedInterference) {
      player->AddOrbitDisableSource(mgr, GetUniqueId());
    } else {
      player->RemoveOrbitDisableSource(GetUniqueId());
    }
  }
}

void CScriptSpecialFunction::ThinkRainSimulator(float dt, CStateManager& mgr) {
  if (static_cast< float >(static_cast< uint >(mgr.GetUpdateFrameIdx()) % 3600) / 3600.f < 0.5f) {
    SendScriptMsgs(kSS_MaxReached, mgr);
  } else {
    SendScriptMsgs(kSS_Zero, mgr);
  }
}

void CScriptSpecialFunction::ThinkAreaDamage(float dt, CStateManager& mgr) {
  if (mgr.IsMultiplayer()) {
    return;
  }
  CPlayer* player = mgr.GetPlayer(0);
  bool inArea = player->GetCurrentAreaId() == GetCurrentAreaId();
  bool immune = mgr.PlayerState(0)->GetCurrentSuitRaw() > CPlayerState::kPS_Varia;
  if (mInAreaDamage) {
    if (!inArea || immune) {
      mInAreaDamage = false;
      player->PopSustainedDamage();
      SendScriptMsgs(kSS_Exited, mgr);
      mgr.SetIsFullThreat(false);
      return;
    }
  } else if (!inArea || immune) {
    return;
  } else {
    mInAreaDamage = true;
    player->PushSustainedDamage();
    SendScriptMsgs(kSS_Entered, mgr);
    mgr.SetIsFullThreat(true);
  }

  CDamageInfo dInfo(CWeaponMode(kWT_Heat), mValue1 * dt, 0.f, 0.f, true);
  mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), dInfo,
                  CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                  CVector3f::Zero());
}

void CScriptSpecialFunction::ThinkActorScale(float dt, CStateManager& mgr) {
  const float deltaScale = dt * mValue1;
  const float f2 = mValue2;

  for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
       conn != GetConnectionList().end(); ++conn) {
    if (conn->state != kSS_Play || conn->msg != kSM_Activate) {
      continue;
    }

    TUniqueId uid = mgr.GetIdForScript(conn->objId);
    if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(uid))) {
      if (act->HasModelData()) {
        CVector3f scale = act->GetModelData()->GetScale();
        if (deltaScale > 0.f) {
          scale.SetX(rstl::min_val(f2, deltaScale + scale.GetX()));
          scale.SetY(rstl::min_val(f2, deltaScale + scale.GetY()));
          scale.SetZ(rstl::min_val(f2, deltaScale + scale.GetZ()));
        } else {
          scale.SetX(rstl::max_val(f2, deltaScale + scale.GetX()));
          scale.SetY(rstl::max_val(f2, deltaScale + scale.GetY()));
          scale.SetZ(rstl::max_val(f2, deltaScale + scale.GetZ()));
        }
        act->ModelData()->SetScale(scale);
      }
    }
  }
}

void CScriptSpecialFunction::ThinkPlayerInArea(float dt, CStateManager& mgr) {
  if (mgr.GetPlayer(0)->GetCurrentAreaId() == GetCurrentAreaId()) {
    if (!mPlayerInArea) {
      mPlayerInArea = true;
      SendScriptMsgs(kSS_Entered, mgr);
    }
  } else if (mPlayerInArea) {
    mPlayerInArea = false;
    SendScriptMsgs(kSS_Exited, mgr);
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
  CPlayer* player = mgr.GetPlayer(0);
  CVector3f pos = player->GetTransform().GetTranslation();
  CTransform4f rotation = CTransform4f::RotateZ(CRelAngle::FromDegrees(-mValue1)) *
                          CTransform4f::RotateX(CRelAngle::FromDegrees(mValue2));
  CTransform4f xf = CTransform4f::Translate(pos + rotation * (mValue3 * CVector3f::Forward()));

  for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
       conn != GetConnectionList().end(); ++conn) {
    if (conn->state != kSS_Connect || conn->msg != kSM_Activate) {
      continue;
    }

    const CStateManager::TIdListResult it = mgr.GetIdListForScript(conn->objId);
    if (!(it.first == it.second)) {
      TUniqueId uid = it.first->second;
      if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(uid))) {
        act->SetTransform(xf);
      }
    }
  }
}

void CScriptSpecialFunction::ThinkConnectedEffectPlane(float dt, CStateManager& mgr) {
  if (CActor* act =
          TCastToPtr< CActor >(mgr.ObjectById(FindConnectedObject(mgr, kSS_Play, kSM_Activate)))) {
    const CTransform4f& xf = act->GetTransform();
    const float x = -1.f * xf.Get02();
    const float y = -1.f * xf.Get12();
    const float z = -1.f * xf.Get22();
    CPlane plane(act->GetTranslation(), CUnitVector3f(x, y, z));
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_Play, kSM_Deactivate);
    for (int i = 0; i < ids.size(); ++i) {
      if (CScriptActor* scriptActor = TCastToPtr< CScriptActor >(mgr.ObjectById(ids[i]))) {
        scriptActor->SetPortalPlane(plane);
      }
    }
  }
}

void CScriptSpecialFunction::ThinkSilhouette(float dt, CStateManager& mgr) {
  if (mTargetSilhouetteStrength > mSilhouetteStrength) {
    mSilhouetteStrength += dt / mValue2;
    const float target = mTargetSilhouetteStrength;
    mSilhouetteStrength = mSilhouetteStrength < target ? mSilhouetteStrength : target;
  } else if (mTargetSilhouetteStrength < mSilhouetteStrength) {
    mSilhouetteStrength -= dt / mValue2;
    mSilhouetteStrength = mTargetSilhouetteStrength < mSilhouetteStrength
                              ? mSilhouetteStrength
                              : mTargetSilhouetteStrength;
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
        mgr.SendScriptMsg(teleporter, GetUniqueId(), active ? kSM_Activate : kSM_Deactivate);
      }
    }
    SendScriptMsgs(mgr.World()->GetWorldAssetId() == worldId ? kSS_Zero : kSS_MaxReached, mgr);
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
      SendScriptMsgs(kSS_InternalState0, mgr);
    } else if (state == CGameArea::kOS_Visible) {
      SendScriptMsgs(kSS_InternalState1, mgr);
    }
    mIntParm1 = state;
  }
}

void CScriptSpecialFunction::ThinkMultiplayerEndConditions(float dt, CStateManager& mgr) {
  if (mIntParm1 == 0 && gpGameState->GetGameMode().GetMatchTimeLimit() > 0.f) {
    float elapsed = gpGameState->GetGameMode().GetElapsedTime();
    if (gpGameState->GetGameMode().GetMatchTimeLimit() - elapsed <= 61.f) {
      mIntParm1 = 1;
      SendScriptMsgs(kSS_MaxReached, mgr);
    }
  }
  if (mIntParm2 == 0) {
    for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
      if (gpGameState->GetGameMode().GetGameModeType() == 'DTHM' &&
          gpGameState->GetGameMode().IsNearScoreLimit(mgr, i)) {
        mIntParm2 = 1;
        SendScriptMsgs(kSS_Arrived, mgr);
      }
    }
  }
}

void CScriptSpecialFunction::ThinkTriggerScale(float dt, CStateManager& mgr) {
  if (CScriptTriggerEllipsoid* trigger = TCastToPtr< CScriptTriggerEllipsoid >(
          mgr.ObjectById(FindConnectedObject(mgr, kSS_Connect, kSM_Attach)))) {
    float d = dt * mValue1;
    trigger->SetScale(trigger->GetScale() + CVector3f(d, d, d));
  }
}

void CScriptSpecialFunction::ThinkObjectFollowJoint(float dt, CStateManager& mgr) {
  TUniqueId followerAct = kInvalidUniqueId;
  TUniqueId followedAct = kInvalidUniqueId;
  for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
       conn != GetConnectionList().end(); ++conn) {
    if (conn->state != kSS_Play || (conn->msg != kSM_Activate && conn->msg != kSM_Deactivate)) {
      continue;
    }

    const CStateManager::TIdListResult it = mgr.GetIdListForScript(conn->objId);
    if (!(it.first == it.second)) {
      TUniqueId uid = it.first->second;
      if (const CActor* const act = TCastToConstPtr< CActor >(mgr.GetObjectById(uid))) {
        if (conn->msg == kSM_Activate && act->HasAnimation()) {
          if (!act->GetActive()) {
            return;
          }
          followedAct = uid;
        } else if (conn->msg == kSM_Deactivate) {
          followerAct = uid;
        }
      }
    }
  }

  if (followerAct != kInvalidUniqueId && followedAct != kInvalidUniqueId) {
    const CActor* const followed = TCastToConstPtr< CActor >(mgr.GetObjectById(followedAct));
    CActor* const follower = TCastToPtr< CActor >(mgr.ObjectById(followerAct));
    if (followed && follower) {
      const CCharLayoutInfo* layout =
          followed->GetModelData()->GetAnimationData()->GetCharLayoutInfo();
      CSegId id = layout->GetSegIdFromString(mStringParm);
      CTransform4f xf = followed->GetTransform() * followed->GetScaledLocatorTransform(id) *
                        layout->GetLinearRotations()[id.val()].BuildTransform4f();
      follower->SetTransform(xf);
    }
  }
}

void CScriptSpecialFunction::ThinkRezbitState(float dt, CStateManager& mgr) {
  uint player =
      mLastOriginatorPlayer == kInvalidUniqueId ? 0 : mgr.MaskUIdNumPlayers(mLastOriginatorPlayer);
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
  if (!handle) {
    CAudioSys::C3DEmitterParmData data(maxDist, falloff, 1, 0x7f, 0x14);
    data.mPos = position;
    data.mSfxId = id;
    handle = CSfxManager::AddEmitter(data, GetCurrentAreaId().Value(), true, true);
  } else {
    CSfxManager::UpdateEmitter(handle, position, CVector3f::Zero(), volume);
    CSfxManager::PitchBend(handle, static_cast< short >(8192.f * pitch + 8192.f));
  }
}

void CScriptSpecialFunction::DeleteEmitter(CSfxHandle& handle) {
  if (handle) {
    CSfxManager::RemoveEmitter(handle);
    handle.Clear();
  }
}

void CScriptSpecialFunction::SkipCinematic(CStateManager& mgr) {
  SendScriptMsgs(kSS_Zero, mgr);
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
    SendScriptMsgs(kSS_Entered, mgr);
  }
  if (mFrustumExited) {
    mFrustumExited = false;
    SendScriptMsgs(kSS_Exited, mgr);
  }
}

void CScriptSpecialFunction::PreRenderAllViewports(CStateManager& mgr) {
  switch (mFunction) {
  case kSF_ExtraRenderClipPlane: {
    if (!GetActive()) {
      return;
    }
    if (mIntParm1 == 0) {
      return;
    }

    const CTransform4f camXf(
        mgr.GetCurrentRenderCameraManager()->GetCurrentCameraTransform(mgr, true));
    if (mIntParm2 != 0) {
      if (mgr.GetRenderVisorMode() == CStateManager::kRVM_Echo) {
        return;
      }
      const CGameArea::CAreaFog* areaFog =
          mgr.World()->GetAreaAlways(GetCurrentAreaId()).GetAreaFog();
      const CGameArea::CAreaFog& camFog = mgr.GetCurrentRenderCameraManager()->GetFog();
      const CGameArea::CAreaFog* fog = camFog.IsFogDisabled() ? areaFog : &camFog;
      if (fog->GetFogMode() == kRFM_None) {
        return;
      }
      mgr.SetAreaClipPlane(GetCurrentAreaId(),
                           CPlane(fog->GetRange().GetY() +
                                      CVector3f::Dot(camXf.GetForward(), camXf.GetTranslation()),
                                  CUnitVector3f(camXf.GetForward(), CUnitVector3f::kN_No)));
    } else {
      CPlane plane(GetTranslation(), CUnitVector3f(GetTransform().GetUp()));
      const CVector3f point = plane.GetNormal() * plane.GetConstant();
      if (CVector3f::Dot(camXf.GetTranslation() - point, plane.GetNormal()) > 0.f) {
        plane = CPlane(point, -plane.GetNormal());
      }
      mgr.SetAreaClipPlane(GetCurrentAreaId(), plane);
    }
    break;
  }
  default:
    break;
  }
}

CEntity* LoadSpecialFunction(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSpecialFunction sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSpecialFunction.inc"

  return rs_new CScriptSpecialFunction(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      static_cast< CScriptSpecialFunction::ESpecialFunction >(sldrThis.function),
      sldrThis.stringParm, sldrThis.valueParm, sldrThis.valueParm2, sldrThis.valueParm3,
      sldrThis.valueParm4, sldrThis.intParm1, sldrThis.intParm2, CVector3f::Zero(), CColor::Black(),
      CDamageInfo(), static_cast< CPlayerState::EItemType >(sldrThis.inventoryItemParm.value),
      static_cast< ushort >(sldrThis.sound1), static_cast< ushort >(sldrThis.sound2),
      static_cast< ushort >(sldrThis.sound3));
}

CEntity* LoadFogVolume(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrFogVolume sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrFogVolume.inc"

  return rs_new CScriptSpecialFunction(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      CScriptSpecialFunction::kSF_FogVolume, rstl::string_l(""), sldrThis.fogBobHeight,
      sldrThis.fogBobFreq, 0.f, 0.f, 0, 0, sldrThis.editorProperties.transform.scale,
      sldrThis.fogColor, CDamageInfo(), CPlayerState::kIT_Invalid,
      CSfxManager::kInternalInvalidSfxId, CSfxManager::kInternalInvalidSfxId,
      CSfxManager::kInternalInvalidSfxId);
}

CEntity* LoadSilhouette(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSilhouette sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSilhouette.inc"

  return rs_new CScriptSpecialFunction(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      CScriptSpecialFunction::kSF_Silhouette, rstl::string_l(""), sldrThis.unknown_0x82bad3ee,
      sldrThis.fadeInTime, sldrThis.fadeOutTime, 0.f, 0, 0,
      sldrThis.editorProperties.transform.scale, sldrThis.silhouetteColor, CDamageInfo(),
      CPlayerState::kIT_Invalid, CSfxManager::kInternalInvalidSfxId,
      CSfxManager::kInternalInvalidSfxId, CSfxManager::kInternalInvalidSfxId);
}

CEntity* LoadRadialDamage(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrRadialDamage sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrRadialDamage.inc"

  int flags = 0;
  if (sldrThis.autoAction) {
    flags |= 1;
  }
  if (sldrThis.autoDelete) {
    flags |= 2;
  }
  if (sldrThis.originator) {
    flags |= 4;
  }

  return rs_new CScriptSpecialFunction(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      CScriptSpecialFunction::kSF_RadialDamage, rstl::string_l(""), sldrThis.radius, 0.f, 0.f, 0.f,
      flags, 0, CVector3f::Zero(), CColor::Black(), LdrToDamageInfo(sldrThis.damage),
      CPlayerState::kIT_Invalid, CSfxManager::kInternalInvalidSfxId,
      CSfxManager::kInternalInvalidSfxId, CSfxManager::kInternalInvalidSfxId);
}

CEntity* LoadEnvFxDensityController(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrEnvFxDensityController sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrEnvFxDensityController.inc"

  return rs_new CScriptSpecialFunction(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), CTransform4f::Identity(),
      CScriptSpecialFunction::kSF_EnvFxDensityController, rstl::string_l(""), sldrThis.density,
      static_cast< float >(sldrThis.fadeSpeed), 0.f, 0.f, 0, 0, CVector3f::Zero(), CColor::Black(),
      CDamageInfo(), CPlayerState::kIT_Invalid, CSfxManager::kInternalInvalidSfxId,
      CSfxManager::kInternalInvalidSfxId, CSfxManager::kInternalInvalidSfxId);
}

CEntity* LoadRumbleEffect(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrRumbleEffect sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrRumbleEffect.inc"

  return rs_new CScriptSpecialFunction(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties),
      ConvertEditorEulerToTransform4f(CVector3f::Zero(),
                                      sldrThis.editorProperties.transform.position),
      CScriptSpecialFunction::kSF_RumbleEffect, rstl::string_l(""), sldrThis.radius,
      static_cast< float >(sldrThis.effect), static_cast< float >(sldrThis.flagsRumble), 0.f, 0, 0,
      CVector3f::Zero(), CColor::Black(), CDamageInfo(), CPlayerState::kIT_Invalid,
      CSfxManager::kInternalInvalidSfxId, CSfxManager::kInternalInvalidSfxId,
      CSfxManager::kInternalInvalidSfxId);
}

CEntity* LoadSpinner(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSpinner sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSpinner.inc"

  int flags = 0;
  if (sldrThis.allowWrap) {
    flags |= 1;
  }
  if (sldrThis.noBackward) {
    flags |= 2;
  }
  if (sldrThis.splineControl) {
    flags |= 4;
  }

  return rs_new CScriptSpecialFunction(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties),
      ConvertEditorEulerToTransform4f(CVector3f::Zero(),
                                      sldrThis.editorProperties.transform.position),
      sldrThis.shotSpinner ? CScriptSpecialFunction::kSF_ShotSpinnerController
                           : CScriptSpecialFunction::kSF_SpinnerController,
      rstl::string(), sldrThis.forwardSpeed, sldrThis.backwardSpeed, sldrThis.unknown_0x449dd059,
      sldrThis.unknown_0xfc849759, flags, 0, CVector3f::Zero(), CColor::Black(), CDamageInfo(),
      CPlayerState::kIT_Invalid, static_cast< ushort >(sldrThis.loopSound),
      static_cast< ushort >(sldrThis.startSound), static_cast< ushort >(sldrThis.stopSound));
}

CEntity* LoadDamageActor(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrDamageActor sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrDamageActor.inc"

  return rs_new CScriptSpecialFunction(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      CScriptSpecialFunction::kSF_DamageActor, rstl::string_l(""), 0.f, 0.f, 0.f, 0.f, 0, 0,
      CVector3f::Zero(), CColor::Black(), LdrToDamageInfo(sldrThis.damage),
      CPlayerState::kIT_Invalid, CSfxManager::kInternalInvalidSfxId,
      CSfxManager::kInternalInvalidSfxId, CSfxManager::kInternalInvalidSfxId);
}

CScriptSpecialFunction::~CScriptSpecialFunction() {}
