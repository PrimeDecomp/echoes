#include "MetroidPrime/ScriptObjects/CScriptSpawnPoint.hpp"

#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSpawnPoint.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/math.hpp"

CScriptSpawnPoint::CScriptSpawnPoint(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const rstl::reserved_vector< int, int(CPlayerState::kIT_Max) >& amountForItem,
    const rstl::reserved_vector< int, int(CPlayerState::kIT_Max) >& capacityForItem,
    bool firstSpawn, bool isMorphed)
: CEntity(uid, info, name, 0)
, m_xf(xf)
, m_amountForItem(amountForItem)
, m_capacityForItem(capacityForItem)
, m_firstSpawn(firstSpawn)
, m_morphed(isMorphed) {}

CScriptSpawnPoint::~CScriptSpawnPoint() {}

const CTransform4f& CScriptSpawnPoint::GetTransform() const { return m_xf; }

int CScriptSpawnPoint::GetItemAmount(CPlayerState::EItemType type) const {
  if (CPlayerState::kIT_Max <= type || type < 0) {
    return m_amountForItem.front();
  }
  return m_amountForItem[type];
}

int CScriptSpawnPoint::GetItemCapacity(CPlayerState::EItemType type) const {
  if (CPlayerState::kIT_Max <= type || type < 0) {
    return m_amountForItem.front();
  }
  return rstl::max_val(m_amountForItem[type], m_capacityForItem[type]);
}

void CScriptSpawnPoint::SendSpawnMessage(CStateManager& mgr, CEntity& player) {
  SendScriptMsgs(kSS_Zero, mgr, player.GetUniqueId(), kSM_None);
}

void CScriptSpawnPoint::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CEntity::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_Reset:
    for (int playerIndex = 0; playerIndex < mgr.GetNumPlayers(); ++playerIndex) {
      for (int i = 0; i < CPlayerState::kIT_Max; ++i) {
        const CPlayerState::EItemType e = CPlayerState::EItemType(i);
        mgr.PlayerState(playerIndex)->ReInitializePowerUp(e, GetItemCapacity(e));
        mgr.PlayerState(playerIndex)->ResetAndIncrPickUp(e, GetItemAmount(e));
      }
    }
  case kSM_SetToZero:
    if (GetActive()) {
      for (int playerIndex = 0; playerIndex < mgr.GetNumPlayers(); ++playerIndex) {
        CPlayer* player = mgr.Player(playerIndex);
        TAreaId thisAreaId = GetCurrentAreaId();
        TAreaId nextAreaId = mgr.GetNextAreaId();

        if (nextAreaId != thisAreaId) {
          bool propagateAgain = false;

          CGameArea* area = mgr.World()->Area(thisAreaId);
          CGameArea::EOcclusionState occlusionState = area->GetOcclusionState();

          if (occlusionState == CGameArea::kOS_Occluded) {
            while (!area->TryTakingOutOfARAM()) {
            }
            CWorld::PropogateAreaChain(CGameArea::kOS_Visible, area, mgr.World());
            propagateAgain = true;
          }

          mgr.SetCurrentAreaId(thisAreaId);
          mgr.SetActorAreaId(*player, thisAreaId);
          player->Teleport(m_xf, mgr, false);
          player->SetSpawnedMorphBallState(
              m_morphed ? CPlayer::kMS_Morphed : CPlayer::kMS_Unmorphed, mgr);

          if (propagateAgain) {
            CWorld::PropogateAreaChain(CGameArea::kOS_Occluded, mgr.World()->Area(nextAreaId),
                                       mgr.World());
          }

        } else {
          player->Teleport(m_xf, mgr, false);
          player->SetSpawnedMorphBallState(
              m_morphed ? CPlayer::kMS_Morphed : CPlayer::kMS_Unmorphed, mgr);
        }

        if (player->GetCameraManager()->IsInCinematicCamera()) {
          player->ResetPlayerState(mgr, 0);
        } else {
          player->ResetPlayerState(mgr, 1);
        }
      }
      CEntity::SendScriptMsgs(kSS_Zero, mgr);
    }
  }
}

CEntity* LoadSpawnPoint(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSpawnPoint sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSpawnPoint.inc"

  rstl::reserved_vector< int, int(CPlayerState::kIT_Max) > amountForItem;
  rstl::reserved_vector< int, int(CPlayerState::kIT_Max) > capacityForItem;
  amountForItem.resize(CPlayerState::kIT_Max, 0);
  capacityForItem.resize(CPlayerState::kIT_Max, 0);

  amountForItem[CPlayerState::kIT_PowerBeam] = sldrThis.spawnInventory.powerBeam;
  amountForItem[CPlayerState::kIT_DarkBeam] = sldrThis.spawnInventory.darkBeam;
  amountForItem[CPlayerState::kIT_LightBeam] = sldrThis.spawnInventory.lightBeam;
  amountForItem[CPlayerState::kIT_AnnihilatorBeam] = sldrThis.spawnInventory.annihilatorBeam;
  amountForItem[CPlayerState::kIT_SuperMissile] = sldrThis.spawnInventory.powerBeamCombo;
  amountForItem[CPlayerState::kIT_Darkburst] = sldrThis.spawnInventory.darkBeamCombo;
  amountForItem[CPlayerState::kIT_Sunburst] = sldrThis.spawnInventory.lightBeamCombo;
  amountForItem[CPlayerState::kIT_SonicBoom] = sldrThis.spawnInventory.annihilatorBeamCombo;
  amountForItem[CPlayerState::kIT_CombatVisor] = sldrThis.spawnInventory.combatVisor;
  amountForItem[CPlayerState::kIT_ScanVisor] = sldrThis.spawnInventory.scanVisor;
  amountForItem[CPlayerState::kIT_DarkVisor] = sldrThis.spawnInventory.darkVisor;
  amountForItem[CPlayerState::kIT_EchoVisor] = sldrThis.spawnInventory.echoVisor;
  amountForItem[CPlayerState::kIT_VariaSuit] = sldrThis.spawnInventory.variaSuit;
  amountForItem[CPlayerState::kIT_DarkSuit] = sldrThis.spawnInventory.darkSuit;
  amountForItem[CPlayerState::kIT_LightSuit] = sldrThis.spawnInventory.lightSuit;
  amountForItem[CPlayerState::kIT_MorphBall] = sldrThis.spawnInventory.morphBall;
  amountForItem[CPlayerState::kIT_BoostBall] = sldrThis.spawnInventory.boostBall;
  amountForItem[CPlayerState::kIT_SpiderBall] = sldrThis.spawnInventory.spiderBall;
  amountForItem[CPlayerState::kIT_MorphBallBombs] = sldrThis.spawnInventory.bomb;
  amountForItem[CPlayerState::kIT_LightBomb] = sldrThis.spawnInventory.lightBomb;
  amountForItem[CPlayerState::kIT_DarkBomb] = sldrThis.spawnInventory.darkBomb;
  amountForItem[CPlayerState::kIT_AnnihilatorBomb] = sldrThis.spawnInventory.annihilatorBomb;
  amountForItem[CPlayerState::kIT_ChargeBeam] = sldrThis.spawnInventory.chargeUpgrade;
  amountForItem[CPlayerState::kIT_GrappleBeam] = sldrThis.spawnInventory.grappleBeam;
  amountForItem[CPlayerState::kIT_SpaceJumpBoots] = sldrThis.spawnInventory.doubleJump;
  amountForItem[CPlayerState::kIT_GravityBoost] = sldrThis.spawnInventory.gravityBoost;
  amountForItem[CPlayerState::kIT_SeekerLauncher] = sldrThis.spawnInventory.seeker;
  amountForItem[CPlayerState::kIT_ScrewAttack] = sldrThis.spawnInventory.screwAttack;
  amountForItem[CPlayerState::kIT_TranslatorUpgrade_TempETM] =
      sldrThis.spawnInventory.translatorUpgrade;
  amountForItem[CPlayerState::kIT_TempleKey1] = sldrThis.spawnInventory.templeKey1;
  amountForItem[CPlayerState::kIT_TempleKey2] = sldrThis.spawnInventory.templeKey2;
  amountForItem[CPlayerState::kIT_TempleKey3] = sldrThis.spawnInventory.templeKey3;
  amountForItem[CPlayerState::kIT_TempleKey4] = sldrThis.spawnInventory.templeKey4;
  amountForItem[CPlayerState::kIT_TempleKey5] = sldrThis.spawnInventory.templeKey5;
  amountForItem[CPlayerState::kIT_TempleKey6] = sldrThis.spawnInventory.templeKey6;
  amountForItem[CPlayerState::kIT_TempleKey7] = sldrThis.spawnInventory.templeKey7;
  amountForItem[CPlayerState::kIT_TempleKey8] = sldrThis.spawnInventory.templeKey8;
  amountForItem[CPlayerState::kIT_TempleKey9] = sldrThis.spawnInventory.templeKey9;
  amountForItem[CPlayerState::kIT_AgonKey1] = sldrThis.spawnInventory.sandKey1;
  amountForItem[CPlayerState::kIT_AgonKey2] = sldrThis.spawnInventory.sandKey2;
  amountForItem[CPlayerState::kIT_AgonKey3] = sldrThis.spawnInventory.sandKey3;
  amountForItem[CPlayerState::kIT_TorvusKey1] = sldrThis.spawnInventory.swampKey1;
  amountForItem[CPlayerState::kIT_TorvusKey2] = sldrThis.spawnInventory.swampKey2;
  amountForItem[CPlayerState::kIT_TorvusKey3] = sldrThis.spawnInventory.swampKey3;
  amountForItem[CPlayerState::kIT_HiveKey1] = sldrThis.spawnInventory.cliffsideKey1;
  amountForItem[CPlayerState::kIT_HiveKey2] = sldrThis.spawnInventory.cliffsideKey2;
  amountForItem[CPlayerState::kIT_HiveKey3] = sldrThis.spawnInventory.cliffsideKey3;
  amountForItem[CPlayerState::kIT_HealthRefill] = sldrThis.spawnInventory.energy;
  amountForItem[CPlayerState::kIT_EnergyTanks] = sldrThis.spawnInventory.energyTank;
  amountForItem[CPlayerState::kIT_Powerbomb] = sldrThis.spawnInventory.powerBomb;
  amountForItem[CPlayerState::kIT_Missile] = sldrThis.spawnInventory.missile;
  amountForItem[CPlayerState::kIT_DarkAmmo] = sldrThis.spawnInventory.darkBeamAmmo;
  amountForItem[CPlayerState::kIT_LightAmmo] = sldrThis.spawnInventory.lightBeamAmmo;
  amountForItem[CPlayerState::kIT_ItemPercentage] = sldrThis.spawnInventory.percentageIncrease;
  amountForItem[CPlayerState::kIT_Multiplayer_NumPlayersJoined] =
      sldrThis.spawnInventory.miscCounter1;
  amountForItem[CPlayerState::kIT_Multiplayer_NumPlayersInOptionsMenu] =
      sldrThis.spawnInventory.miscCounter2;
  amountForItem[CPlayerState::kIT_MiscCounter3] = sldrThis.spawnInventory.miscCounter3;
  amountForItem[CPlayerState::kIT_Multiplayer_Archenemy] = sldrThis.spawnInventory.miscCounter4;
  amountForItem[CPlayerState::kIT_SwitchWeaponPower] = sldrThis.spawnInventory.changeToPowerBeam;
  amountForItem[CPlayerState::kIT_SwitchWeaponDark] = sldrThis.spawnInventory.changeToDarkBeam;
  amountForItem[CPlayerState::kIT_SwitchWeaponLight] = sldrThis.spawnInventory.changeToLightBeam;
  amountForItem[CPlayerState::kIT_SwitchWeaponAnnihilator] =
      sldrThis.spawnInventory.changeToAnnihilatorBeam;
  amountForItem[CPlayerState::kIT_MultiChargeUpgrade] = sldrThis.spawnInventory.multiChargeUpgrade;
  amountForItem[CPlayerState::kIT_Invisibility] = sldrThis.spawnInventory.invisibility;
  amountForItem[CPlayerState::kIT_DoubleDamage] = sldrThis.spawnInventory.amplifiedDamage;
  amountForItem[CPlayerState::kIT_Invincibility] = sldrThis.spawnInventory.invincibility;
  amountForItem[CPlayerState::kIT_MiscCounter1a] = sldrThis.spawnInventory.miscCounter1a;
  amountForItem[CPlayerState::kIT_MiscCounter2a] = sldrThis.spawnInventory.miscCounter2a;
  amountForItem[CPlayerState::kIT_MiscCounter3a] = sldrThis.spawnInventory.miscCounter3a;
  amountForItem[CPlayerState::kIT_MiscCounter4a] = sldrThis.spawnInventory.miscCounter4a;
  amountForItem[CPlayerState::kIT_FragCount] = sldrThis.spawnInventory.fragCount;
  amountForItem[CPlayerState::kIT_DiedCount] = sldrThis.spawnInventory.diedCount;
  amountForItem[CPlayerState::kIT_ArchenemyCount] = sldrThis.spawnInventory.archenemyCount;
  amountForItem[CPlayerState::kIT_PersistentCounter1] = sldrThis.spawnInventory.persistentCounter1;
  amountForItem[CPlayerState::kIT_PersistentCounter2] = sldrThis.spawnInventory.persistentCounter2;
  amountForItem[CPlayerState::kIT_PersistentCounter3] = sldrThis.spawnInventory.persistentCounter3;
  amountForItem[CPlayerState::kIT_PersistentCounter4] = sldrThis.spawnInventory.persistentCounter4;
  amountForItem[CPlayerState::kIT_PersistentCounter5] = sldrThis.spawnInventory.persistentCounter5;
  amountForItem[CPlayerState::kIT_PersistentCounter6] = sldrThis.spawnInventory.persistentCounter6;
  amountForItem[CPlayerState::kIT_PersistentCounter7] = sldrThis.spawnInventory.persistentCounter7;
  amountForItem[CPlayerState::kIT_PersistentCounter8] = sldrThis.spawnInventory.persistentCounter8;
  amountForItem[CPlayerState::kIT_SwitchVisorCombat] = sldrThis.spawnInventory.changeToCombatVisor;
  amountForItem[CPlayerState::kIT_SwitchVisorScan] = sldrThis.spawnInventory.changeToScanVisor;
  amountForItem[CPlayerState::kIT_SwitchVisorDark] = sldrThis.spawnInventory.changeToDarkVisor;
  amountForItem[CPlayerState::kIT_SwitchVisorEcho] = sldrThis.spawnInventory.changeToEchoVisor;
  amountForItem[CPlayerState::kIT_CoinAmplifier] = sldrThis.spawnInventory.coinAmplifier;
  amountForItem[CPlayerState::kIT_CoinCounter] = sldrThis.spawnInventory.coinCounter;
  amountForItem[CPlayerState::kIT_VioletTranslator] = sldrThis.spawnInventory.translatorUpgrade1;
  amountForItem[CPlayerState::kIT_AmberTranslator] = sldrThis.spawnInventory.translatorUpgrade2;
  amountForItem[CPlayerState::kIT_EmeraldTranslator] = sldrThis.spawnInventory.translatorUpgrade3;
  amountForItem[CPlayerState::kIT_CobaltTranslator] = sldrThis.spawnInventory.translatorUpgrade4;
  amountForItem[CPlayerState::kIT_ChargeCombo] = sldrThis.spawnInventory.chargeComboUpgrade;

  capacityForItem[CPlayerState::kIT_PowerBeam] = sldrThis.spawnInventoryCapacity.powerBeam;
  capacityForItem[CPlayerState::kIT_DarkBeam] = sldrThis.spawnInventoryCapacity.darkBeam;
  capacityForItem[CPlayerState::kIT_LightBeam] = sldrThis.spawnInventoryCapacity.lightBeam;
  capacityForItem[CPlayerState::kIT_AnnihilatorBeam] =
      sldrThis.spawnInventoryCapacity.annihilatorBeam;
  capacityForItem[CPlayerState::kIT_SuperMissile] = sldrThis.spawnInventoryCapacity.powerBeamCombo;
  capacityForItem[CPlayerState::kIT_Darkburst] = sldrThis.spawnInventoryCapacity.darkBeamCombo;
  capacityForItem[CPlayerState::kIT_Sunburst] = sldrThis.spawnInventoryCapacity.lightBeamCombo;
  capacityForItem[CPlayerState::kIT_SonicBoom] =
      sldrThis.spawnInventoryCapacity.annihilatorBeamCombo;
  capacityForItem[CPlayerState::kIT_CombatVisor] = sldrThis.spawnInventoryCapacity.combatVisor;
  capacityForItem[CPlayerState::kIT_ScanVisor] = sldrThis.spawnInventoryCapacity.scanVisor;
  capacityForItem[CPlayerState::kIT_DarkVisor] = sldrThis.spawnInventoryCapacity.darkVisor;
  capacityForItem[CPlayerState::kIT_EchoVisor] = sldrThis.spawnInventoryCapacity.echoVisor;
  capacityForItem[CPlayerState::kIT_VariaSuit] = sldrThis.spawnInventoryCapacity.variaSuit;
  capacityForItem[CPlayerState::kIT_DarkSuit] = sldrThis.spawnInventoryCapacity.darkSuit;
  capacityForItem[CPlayerState::kIT_LightSuit] = sldrThis.spawnInventoryCapacity.lightSuit;
  capacityForItem[CPlayerState::kIT_MorphBall] = sldrThis.spawnInventoryCapacity.morphBall;
  capacityForItem[CPlayerState::kIT_BoostBall] = sldrThis.spawnInventoryCapacity.boostBall;
  capacityForItem[CPlayerState::kIT_SpiderBall] = sldrThis.spawnInventoryCapacity.spiderBall;
  capacityForItem[CPlayerState::kIT_MorphBallBombs] = sldrThis.spawnInventoryCapacity.bomb;
  capacityForItem[CPlayerState::kIT_LightBomb] = sldrThis.spawnInventoryCapacity.lightBomb;
  capacityForItem[CPlayerState::kIT_DarkBomb] = sldrThis.spawnInventoryCapacity.darkBomb;
  capacityForItem[CPlayerState::kIT_AnnihilatorBomb] =
      sldrThis.spawnInventoryCapacity.annihilatorBomb;
  capacityForItem[CPlayerState::kIT_ChargeBeam] = sldrThis.spawnInventoryCapacity.chargeUpgrade;
  capacityForItem[CPlayerState::kIT_GrappleBeam] = sldrThis.spawnInventoryCapacity.grappleBeam;
  capacityForItem[CPlayerState::kIT_SpaceJumpBoots] = sldrThis.spawnInventoryCapacity.doubleJump;
  capacityForItem[CPlayerState::kIT_GravityBoost] = sldrThis.spawnInventoryCapacity.gravityBoost;
  capacityForItem[CPlayerState::kIT_SeekerLauncher] = sldrThis.spawnInventoryCapacity.seeker;
  capacityForItem[CPlayerState::kIT_ScrewAttack] = sldrThis.spawnInventoryCapacity.screwAttack;
  capacityForItem[CPlayerState::kIT_TranslatorUpgrade_TempETM] =
      sldrThis.spawnInventoryCapacity.translatorUpgrade;
  capacityForItem[CPlayerState::kIT_TempleKey1] = sldrThis.spawnInventoryCapacity.templeKey1;
  capacityForItem[CPlayerState::kIT_TempleKey2] = sldrThis.spawnInventoryCapacity.templeKey2;
  capacityForItem[CPlayerState::kIT_TempleKey3] = sldrThis.spawnInventoryCapacity.templeKey3;
  capacityForItem[CPlayerState::kIT_TempleKey4] = sldrThis.spawnInventoryCapacity.templeKey4;
  capacityForItem[CPlayerState::kIT_TempleKey5] = sldrThis.spawnInventoryCapacity.templeKey5;
  capacityForItem[CPlayerState::kIT_TempleKey6] = sldrThis.spawnInventoryCapacity.templeKey6;
  capacityForItem[CPlayerState::kIT_TempleKey7] = sldrThis.spawnInventoryCapacity.templeKey7;
  capacityForItem[CPlayerState::kIT_TempleKey8] = sldrThis.spawnInventoryCapacity.templeKey8;
  capacityForItem[CPlayerState::kIT_TempleKey9] = sldrThis.spawnInventoryCapacity.templeKey9;
  capacityForItem[CPlayerState::kIT_AgonKey1] = sldrThis.spawnInventoryCapacity.sandKey1;
  capacityForItem[CPlayerState::kIT_AgonKey2] = sldrThis.spawnInventoryCapacity.sandKey2;
  capacityForItem[CPlayerState::kIT_AgonKey3] = sldrThis.spawnInventoryCapacity.sandKey3;
  capacityForItem[CPlayerState::kIT_TorvusKey1] = sldrThis.spawnInventoryCapacity.swampKey1;
  capacityForItem[CPlayerState::kIT_TorvusKey2] = sldrThis.spawnInventoryCapacity.swampKey2;
  capacityForItem[CPlayerState::kIT_TorvusKey3] = sldrThis.spawnInventoryCapacity.swampKey3;
  capacityForItem[CPlayerState::kIT_HiveKey1] = sldrThis.spawnInventoryCapacity.cliffsideKey1;
  capacityForItem[CPlayerState::kIT_HiveKey2] = sldrThis.spawnInventoryCapacity.cliffsideKey2;
  capacityForItem[CPlayerState::kIT_HiveKey3] = sldrThis.spawnInventoryCapacity.cliffsideKey3;
  capacityForItem[CPlayerState::kIT_HealthRefill] = sldrThis.spawnInventoryCapacity.energy;
  capacityForItem[CPlayerState::kIT_EnergyTanks] = sldrThis.spawnInventoryCapacity.energyTank;
  capacityForItem[CPlayerState::kIT_Powerbomb] = sldrThis.spawnInventoryCapacity.powerBomb;
  capacityForItem[CPlayerState::kIT_Missile] = sldrThis.spawnInventoryCapacity.missile;
  capacityForItem[CPlayerState::kIT_DarkAmmo] = sldrThis.spawnInventoryCapacity.darkBeamAmmo;
  capacityForItem[CPlayerState::kIT_LightAmmo] = sldrThis.spawnInventoryCapacity.lightBeamAmmo;
  capacityForItem[CPlayerState::kIT_ItemPercentage] =
      sldrThis.spawnInventoryCapacity.percentageIncrease;
  capacityForItem[CPlayerState::kIT_Multiplayer_NumPlayersJoined] =
      sldrThis.spawnInventoryCapacity.miscCounter1;
  capacityForItem[CPlayerState::kIT_Multiplayer_NumPlayersInOptionsMenu] =
      sldrThis.spawnInventoryCapacity.miscCounter2;
  capacityForItem[CPlayerState::kIT_MiscCounter3] = sldrThis.spawnInventoryCapacity.miscCounter3;
  capacityForItem[CPlayerState::kIT_Multiplayer_Archenemy] =
      sldrThis.spawnInventoryCapacity.miscCounter4;
  capacityForItem[CPlayerState::kIT_SwitchWeaponPower] =
      sldrThis.spawnInventoryCapacity.changeToPowerBeam;
  capacityForItem[CPlayerState::kIT_SwitchWeaponDark] =
      sldrThis.spawnInventoryCapacity.changeToDarkBeam;
  capacityForItem[CPlayerState::kIT_SwitchWeaponLight] =
      sldrThis.spawnInventoryCapacity.changeToLightBeam;
  capacityForItem[CPlayerState::kIT_SwitchWeaponAnnihilator] =
      sldrThis.spawnInventoryCapacity.changeToAnnihilatorBeam;
  capacityForItem[CPlayerState::kIT_MultiChargeUpgrade] =
      sldrThis.spawnInventoryCapacity.multiChargeUpgrade;
  capacityForItem[CPlayerState::kIT_Invisibility] = sldrThis.spawnInventoryCapacity.invisibility;
  capacityForItem[CPlayerState::kIT_DoubleDamage] = sldrThis.spawnInventoryCapacity.amplifiedDamage;
  capacityForItem[CPlayerState::kIT_Invincibility] = sldrThis.spawnInventoryCapacity.invincibility;
  capacityForItem[CPlayerState::kIT_MiscCounter1a] = sldrThis.spawnInventoryCapacity.miscCounter1a;
  capacityForItem[CPlayerState::kIT_MiscCounter2a] = sldrThis.spawnInventoryCapacity.miscCounter2a;
  capacityForItem[CPlayerState::kIT_MiscCounter3a] = sldrThis.spawnInventoryCapacity.miscCounter3a;
  capacityForItem[CPlayerState::kIT_MiscCounter4a] = sldrThis.spawnInventoryCapacity.miscCounter4a;
  capacityForItem[CPlayerState::kIT_FragCount] = sldrThis.spawnInventoryCapacity.fragCount;
  capacityForItem[CPlayerState::kIT_DiedCount] = sldrThis.spawnInventoryCapacity.diedCount;
  capacityForItem[CPlayerState::kIT_ArchenemyCount] =
      sldrThis.spawnInventoryCapacity.archenemyCount;
  capacityForItem[CPlayerState::kIT_PersistentCounter1] =
      sldrThis.spawnInventoryCapacity.persistentCounter1;
  capacityForItem[CPlayerState::kIT_PersistentCounter2] =
      sldrThis.spawnInventoryCapacity.persistentCounter2;
  capacityForItem[CPlayerState::kIT_PersistentCounter3] =
      sldrThis.spawnInventoryCapacity.persistentCounter3;
  capacityForItem[CPlayerState::kIT_PersistentCounter4] =
      sldrThis.spawnInventoryCapacity.persistentCounter4;
  capacityForItem[CPlayerState::kIT_PersistentCounter5] =
      sldrThis.spawnInventoryCapacity.persistentCounter5;
  capacityForItem[CPlayerState::kIT_PersistentCounter6] =
      sldrThis.spawnInventoryCapacity.persistentCounter6;
  capacityForItem[CPlayerState::kIT_PersistentCounter7] =
      sldrThis.spawnInventoryCapacity.persistentCounter7;
  capacityForItem[CPlayerState::kIT_PersistentCounter8] =
      sldrThis.spawnInventoryCapacity.persistentCounter8;
  capacityForItem[CPlayerState::kIT_SwitchVisorCombat] =
      sldrThis.spawnInventoryCapacity.changeToCombatVisor;
  capacityForItem[CPlayerState::kIT_SwitchVisorScan] =
      sldrThis.spawnInventoryCapacity.changeToScanVisor;
  capacityForItem[CPlayerState::kIT_SwitchVisorDark] =
      sldrThis.spawnInventoryCapacity.changeToDarkVisor;
  capacityForItem[CPlayerState::kIT_SwitchVisorEcho] =
      sldrThis.spawnInventoryCapacity.changeToEchoVisor;
  capacityForItem[CPlayerState::kIT_CoinAmplifier] = sldrThis.spawnInventoryCapacity.coinAmplifier;
  capacityForItem[CPlayerState::kIT_CoinCounter] = sldrThis.spawnInventoryCapacity.coinCounter;
  // Translator capacity uses the amount record in G2ME01.
  capacityForItem[CPlayerState::kIT_VioletTranslator] = sldrThis.spawnInventory.translatorUpgrade1;
  capacityForItem[CPlayerState::kIT_AmberTranslator] = sldrThis.spawnInventory.translatorUpgrade2;
  capacityForItem[CPlayerState::kIT_EmeraldTranslator] = sldrThis.spawnInventory.translatorUpgrade3;
  capacityForItem[CPlayerState::kIT_CobaltTranslator] = sldrThis.spawnInventory.translatorUpgrade4;
  capacityForItem[CPlayerState::kIT_ChargeCombo] =
      sldrThis.spawnInventoryCapacity.chargeComboUpgrade;

  return rs_new CScriptSpawnPoint(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      amountForItem, capacityForItem, sldrThis.firstSpawn, sldrThis.spawnInMorphBallMode);
}
