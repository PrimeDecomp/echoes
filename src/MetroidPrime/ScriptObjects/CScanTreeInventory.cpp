#include "MetroidPrime/ScriptObjects/CScanTreeInventory.hpp"

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScanTreeInventory.hpp"
#include "rstl/string.hpp"

static const CPlayerState::EItemType kInventorySlotToItemType[] = {
  CPlayerState::kIT_PowerBeam, 
  CPlayerState::kIT_DarkBeam, 
  CPlayerState::kIT_LightBeam,
  CPlayerState::kIT_AnnihilatorBeam,
  CPlayerState::kIT_SuperMissile,
  CPlayerState::kIT_Darkburst,
  CPlayerState::kIT_Sunburst,
  CPlayerState::kIT_SonicBoom,
  CPlayerState::kIT_CombatVisor,
  CPlayerState::kIT_ScanVisor,
  CPlayerState::kIT_DarkVisor,
  CPlayerState::kIT_EchoVisor,
  CPlayerState::kIT_VariaSuit,
  CPlayerState::kIT_DarkSuit,
  CPlayerState::kIT_LightSuit,
  CPlayerState::kIT_MorphBall,
  CPlayerState::kIT_BoostBall,
  CPlayerState::kIT_SpiderBall,
  CPlayerState::kIT_MorphBallBombs,
  CPlayerState::kIT_DarkBomb,
  CPlayerState::kIT_LightBomb,
  CPlayerState::kIT_AnnihilatorBomb,
  CPlayerState::kIT_ChargeBeam,
  CPlayerState::kIT_GrappleBeam,
  CPlayerState::kIT_SpaceJumpBoots,
  CPlayerState::kIT_GravityBoost,
  CPlayerState::kIT_SeekerLauncher,
  CPlayerState::kIT_ScrewAttack,
  CPlayerState::kIT_Powerbomb,
  CPlayerState::kIT_Missile,
  CPlayerState::kIT_DarkAmmo,
  CPlayerState::kIT_LightAmmo,
  CPlayerState::kIT_EnergyTanks,
  CPlayerState::kIT_TempleKey1,
  CPlayerState::kIT_TempleKey2,
  CPlayerState::kIT_TempleKey3,
  CPlayerState::kIT_TempleKey4,
  CPlayerState::kIT_TempleKey5,
  CPlayerState::kIT_TempleKey6,
  CPlayerState::kIT_TempleKey7,
  CPlayerState::kIT_TempleKey8,
  CPlayerState::kIT_TempleKey9,
  CPlayerState::kIT_AgonKey1,
  CPlayerState::kIT_AgonKey2,
  CPlayerState::kIT_AgonKey3,
  CPlayerState::kIT_TorvusKey1,
  CPlayerState::kIT_TorvusKey2,
  CPlayerState::kIT_TorvusKey3,
  CPlayerState::kIT_HiveKey1,
  CPlayerState::kIT_HiveKey2,
  CPlayerState::kIT_HiveKey3,
  CPlayerState::kIT_EnergyTransferModule,
  CPlayerState::kIT_ChargeCombo
};

CScanTreeInventory* LoadScanTreeInventory(int* id, CInputStream& input) {
  SLdrScanTreeInventory sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrScanTreeInventory.inc"

  return rs_new CScanTreeInventory(
    *id & 0xffff, sldrThis.editorProperties.transform, sldrThis.nodeName,
    sldrThis.scannableInfo.scannableInfo0, uint(sldrThis.unknown_0x3d326f90) < 0x35 ? kInventorySlotToItemType[sldrThis.unknown_0x3d326f90] : CPlayerState::kIT_PowerBeam, sldrThis.stringName
  );
}
