#include "MetroidPrime/CTurretHud.hpp"

#include "GuiSys/CAuiEnergyBarT01.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiFrameLoader.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"

class CScriptPlayerTurret;

static const char* const skTurretFrameNames[] = {"FRME_TurretHud4Combat", "FRME_TurretHud2Combat",
                                                 "FRME_TurretHud4Combat"};

rstl::pair< CVector3f, CVector3f > CTurretHud::GetEnergyBarCoords(float t) {
  const float x = 8.5f * t - 4.25f;
  return rstl::pair< CVector3f, CVector3f >(CVector3f(x, 0.f, 0.f), CVector3f(x, 0.f, 0.4f));
}

CTurretHud::CTurretHud(const CStateManager& mgr, int playerIndex)
: mPlayerIndex(playerIndex)
, mFrameLoader(rs_new CGuiFrameLoader(
      gpResourceFactory->GetResourceIdByName(skTurretFrameNames[mgr.GetViewportLayoutIndex()])->id,
      *gpResourceFactory, *gpSimplePool))
, mFrame(nullptr)
, mHullEnergy(nullptr) {}

void CTurretHud::BindWidgets() {
  mHullEnergy = static_cast< CAuiEnergyBarT01* >(mFrame->FindWidget("energybart01_hullenergy"));
  if (mHullEnergy != nullptr) {
    mHullEnergy->SetCoordFunc(GetEnergyBarCoords);
    mHullEnergy->SetTesselation(0.2f);
    mHullEnergy->SetMaxEnergy(CPlayerState::GetBaseHealthCapacity());
    mHullEnergy->SetFilledColor(gpTweakGuiColors->GetTurretHUDEnergyBarFillColor());
    mHullEnergy->SetShadowColor(gpTweakGuiColors->GetTurretHUDEnergyBarShadowColor());
    mHullEnergy->SetEmptyColor(gpTweakGuiColors->GetTurretHUDEnergyBarEmptyColor());
    mHullEnergy->SetFilledDrainSpeed(gpTweakGui->GetEnergyBarFilledDrainSpeed());
    mHullEnergy->SetShadowDrainSpeed(gpTweakGui->GetEnergyBarShadowDrainSpeed());
    mHullEnergy->SetShadowDrainDelay(gpTweakGui->GetEnergyBarShadowDrainDelay());
    mHullEnergy->SetIsAlwaysResetTimer(gpTweakGui->GetEnergyBarAlwaysResetDelay());
  }

  if (CGuiTextPane* title = static_cast< CGuiTextPane* >(mFrame->FindWidget("textpane_title"))) {
    title->TextSupport().SetText(rstl::wstring(gpStringTable->GetString("HullHealth")), false);
    title->TextSupport().SetFontColor(gpTweakGuiColors->GetTurretHUDFontColor());
    title->TextSupport().SetOutlineColor(gpTweakGuiColors->GetTurretHUDFontOutlineColor());
  }
  if (CGuiWidget* frame = mFrame->FindWidget("model_frame")) {
    frame->SetColor(gpTweakGuiColors->GetTurretHUDFrameColor());
  }
}

void CTurretHud::UpdateEnergy(const CStateManager& mgr) {
  if (mHullEnergy == nullptr) {
    return;
  }
  const CPlayer* player = mgr.GetPlayer(mPlayerIndex);
  if (player->GetTurretState() != CPlayer::kTS_Active) {
    return;
  }
  if (player->GetTurretId() == kInvalidUniqueId) {
    return;
  }
  const TUniqueId turretId = player->GetTurretId();
  CEntity* turret = reinterpret_cast< CEntity* >(
      TCastToPtr< CScriptPlayerTurret >(const_cast< CEntity* >(mgr.GetObjectById(turretId))));
  if (turret == nullptr) {
    return;
  }

  const TUniqueId hullId = PlayerTurret_GetHullActorId(*turret);
  const CActor* hull = TCastToConstPtr< CActor >(mgr.GetObjectById(hullId));
  if (const CHealthInfo* health = hull->GetHealthInfo()) {
    mHullEnergy->SetMaxEnergy(health->GetInitialHP());
    mHullEnergy->SetCurrEnergy(health->GetHP(), CAuiEnergyBarT01::kSM_Instant);
    const float scale = health->GetInitialHP() / 100.f;
    mHullEnergy->SetFilledDrainSpeed(scale * gpTweakGui->GetEnergyBarFilledDrainSpeed());
    mHullEnergy->SetShadowDrainSpeed(scale * gpTweakGui->GetEnergyBarShadowDrainSpeed());
  }
}

void CTurretHud::Update(float dt, const CStateManager& mgr) {
  if (mFrameLoader.get() != nullptr) {
    if (!mFrameLoader->IsFinishedLoading()) {
      return;
    }
    mFrame = mFrameLoader->CreateFrame();
    mFrameLoader = nullptr;
    BindWidgets();
  }

  if (mFrame.get() != nullptr) {
    mFrame->Update(dt);
  }
  UpdateEnergy(mgr);
}

void CTurretHud::Draw() const {
  if (mFrame.get() != nullptr) {
    mFrame->Draw(CGuiWidgetDrawParms::sDefaultDrawParms);
  }
}
