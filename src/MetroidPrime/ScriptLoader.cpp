#include "MetroidPrime/ScriptLoader.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CAnimationParameters.hpp"
#include "MetroidPrime/CBasicSwarmData.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGrappleParameters.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CPowerBombGuardianStageData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/SEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSporbBase.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAnimationSet.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrBasicSwarmProperties.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrGrappleParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrHealthInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPlasmaBeamInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrVector2f.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/Weapons/CBeamInfo.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/pair.hpp"

struct NamedScriptLoader {
  FourCC mType;
  FScriptLoader mLoader;

  bool operator<(const NamedScriptLoader& other) const {
    return static_cast< int >(mType) < static_cast< int >(other.mType);
  }
};

// Guessed name: a plain struct filled by an inline factory fits the original's inline budget; a
// constructor plus implicit copy constructor per table entry does not.
inline NamedScriptLoader MakeNamedScriptLoader(FourCC type, FScriptLoader loader) {
  NamedScriptLoader result;
  result.mType = type;
  result.mLoader = loader;
  return result;
}

NamedScriptLoader g_LoaderFuncs[] = {
    MakeNamedScriptLoader('ACTR', &LoadActor),
    MakeNamedScriptLoader('ACKF', &LoadActorKeyframe),
    MakeNamedScriptLoader('AROT', &LoadActorRotate),
    MakeNamedScriptLoader('ACNT', &LoadAdvancedCounter),
    MakeNamedScriptLoader('ADMG', &LoadAreaDamage),
    MakeNamedScriptLoader('AIHT', &LoadAIHint),
    MakeNamedScriptLoader('AJMP', &LoadAIJumpPoint),
    MakeNamedScriptLoader('AIKF', &LoadAIKeyframe),
    MakeNamedScriptLoader('AIMT', &REL_ENTRY(LoadAIMannedTurret)),
    MakeNamedScriptLoader('AIWP', &LoadAIWaypoint),
    MakeNamedScriptLoader('AMIA', &LoadAmbientAI),
    MakeNamedScriptLoader('REAA', &LoadAreaProperties),
    MakeNamedScriptLoader('ATMA', &REL_ENTRY(LoadAtomicAlpha)),
    MakeNamedScriptLoader('ATMB', &REL_ENTRY(LoadAtomicBeta)),
    MakeNamedScriptLoader('BSWM', &REL_ENTRY(LoadBacteriaSwarm)),
    MakeNamedScriptLoader('BALT', &LoadBallTrigger),
    MakeNamedScriptLoader('BLOG', &REL_ENTRY(LoadBlogg)),
    MakeNamedScriptLoader('BRZG', &REL_ENTRY(LoadBrizgee)),
    MakeNamedScriptLoader('CAMR', &LoadCamera),
    MakeNamedScriptLoader('BLUR', &LoadCameraBlurKeyframe),
    MakeNamedScriptLoader('FILT', &LoadCameraFilterKeyframe),
    MakeNamedScriptLoader('CAMH', &LoadCameraHint),
    MakeNamedScriptLoader('CAMP', &LoadCameraPitch),
    MakeNamedScriptLoader('CAMS', &LoadCameraShaker),
    MakeNamedScriptLoader('CAMW', &LoadCameraWaypoint),
    MakeNamedScriptLoader('CANB', &REL_ENTRY(LoadCannonBall)),
    MakeNamedScriptLoader('CHOG', &REL_ENTRY(LoadChozoGhost)),
    MakeNamedScriptLoader('COIN', &REL_ENTRY(LoadCoin)),
    MakeNamedScriptLoader('CLRM', &LoadColorModulate),
    MakeNamedScriptLoader('CMDO', &REL_ENTRY(LoadCommandPirate)),
    MakeNamedScriptLoader('CRLY', &LoadConditionalRelay),
    MakeNamedScriptLoader('CTLH', &LoadControlHint),
    MakeNamedScriptLoader('CNTA', &LoadControllerAction),
    MakeNamedScriptLoader('CNTR', &LoadCounter),
    MakeNamedScriptLoader('COVR', &LoadCoverPoint),
    MakeNamedScriptLoader('CRLT', &REL_ENTRY(LoadCrystallite)),
    MakeNamedScriptLoader('DTRG', &LoadDamageableTrigger),
    MakeNamedScriptLoader('DTRO', &LoadDamageableTriggerOriented),
    MakeNamedScriptLoader('DMGA', &LoadDamageActor),
    MakeNamedScriptLoader('DRKC', &REL_ENTRY(LoadDarkCommando)),
    MakeNamedScriptLoader('DRKS', &REL_ENTRY(LoadDarkSamus)),
    MakeNamedScriptLoader('DSBS', &REL_ENTRY(LoadDarkSamusBattleStage)),
    MakeNamedScriptLoader('DKTR', &REL_ENTRY(LoadDarkTrooper)),
    MakeNamedScriptLoader('DBR1', &LoadDebris),
    MakeNamedScriptLoader('DBR2', &LoadDebrisExtended),
    MakeNamedScriptLoader('DBAR', &REL_ENTRY(LoadDestructibleBarrier)),
    MakeNamedScriptLoader('DGRD', &REL_ENTRY(LoadDigitalGuardian)),
    MakeNamedScriptLoader('DGHD', &REL_ENTRY(LoadDigitalGuardianHead)),
    MakeNamedScriptLoader('DFOG', &LoadDistanceFog),
    MakeNamedScriptLoader('DOCK', &LoadDock),
    MakeNamedScriptLoader('DOOR', &LoadDoor),
    MakeNamedScriptLoader('EFCT', &LoadEffect),
    MakeNamedScriptLoader('EPRT', &REL_ENTRY(LoadElitePirate)),
    MakeNamedScriptLoader('EMS1', &REL_ENTRY(LoadEmperorIngStage1)),
    MakeNamedScriptLoader('EM2T', &REL_ENTRY(LoadEmperorIngStage2Tentacle)),
    MakeNamedScriptLoader('EMS3', &REL_ENTRY(LoadEmperorIngStage3)),
    MakeNamedScriptLoader('EMPU', &LoadEMPulse),
    MakeNamedScriptLoader('FXDC', &LoadEnvFxDensityController),
    MakeNamedScriptLoader('EYEB', &REL_ENTRY(LoadEyeBall)),
    MakeNamedScriptLoader('FISH', &REL_ENTRY(LoadFishCloud)),
    MakeNamedScriptLoader('FSHM', &REL_ENTRY(LoadFishCloudModifier)),
    MakeNamedScriptLoader('FSWM', &REL_ENTRY(LoadFlyerSwarm)),
    MakeNamedScriptLoader('FPRT', &REL_ENTRY(LoadFlyingPirate)),
    MakeNamedScriptLoader('FOGO', &REL_ENTRY(LoadFogOverlay)),
    MakeNamedScriptLoader('FOGV', &LoadFogVolume),
    MakeNamedScriptLoader('FGTO', &REL_ENTRY(LoadForgottenObject)),
    MakeNamedScriptLoader('FNWK', &REL_ENTRY(LoadFrontEndDataNetwork)),
    MakeNamedScriptLoader('GENR', &LoadGenerator),
    MakeNamedScriptLoader('GBUG', &REL_ENTRY(LoadGlowBug)),
    MakeNamedScriptLoader('GRAP', &LoadGrapplePoint),
    MakeNamedScriptLoader('GRCH', &REL_ENTRY(LoadGrenchler)),
    MakeNamedScriptLoader('GMNU', &REL_ENTRY(LoadGuiMenu)),
    MakeNamedScriptLoader('GPJN', &REL_ENTRY(LoadGuiPlayerJoinManager)),
    MakeNamedScriptLoader('GSCR', &REL_ENTRY(LoadGuiScreen)),
    MakeNamedScriptLoader('GSLD', &REL_ENTRY(LoadGuiSlider)),
    MakeNamedScriptLoader('GWIG', &REL_ENTRY(LoadGuiWidget)),
    MakeNamedScriptLoader('GNTB', &REL_ENTRY(LoadGunTurretBase)),
    MakeNamedScriptLoader('GNTT', &REL_ENTRY(LoadGunTurretTop)),
    MakeNamedScriptLoader('HHNT', &LoadHUDHint),
    MakeNamedScriptLoader('MEMO', &LoadHUDMemo),
    MakeNamedScriptLoader('INGS', &REL_ENTRY(LoadIngs)),
    MakeNamedScriptLoader('IBSM', &REL_ENTRY(LoadIngBlobSwarm)),
    MakeNamedScriptLoader('IBBG', &REL_ENTRY(LoadIngBoostBallGuardian)),
    MakeNamedScriptLoader('IPUD', &REL_ENTRY(LoadIngPuddle)),
    MakeNamedScriptLoader('ISSW', &REL_ENTRY(LoadIngSnatchingSwarm)),
    MakeNamedScriptLoader('ISJG', &REL_ENTRY(LoadIngSpaceJumpGuardian)),
    MakeNamedScriptLoader('ISBG', &REL_ENTRY(LoadIngSpiderBallGuardian)),
    MakeNamedScriptLoader('KRAL', &REL_ENTRY(LoadKralee)),
    MakeNamedScriptLoader('KROC', &REL_ENTRY(LoadKrocus)),
    MakeNamedScriptLoader('DLHT', &LoadDynamicLight),
    MakeNamedScriptLoader('LUMI', &REL_ENTRY(LoadLumite)),
    MakeNamedScriptLoader('MING', &REL_ENTRY(LoadMediumIng)),
    MakeNamedScriptLoader('MRLY', &LoadMemoryRelay),
    MakeNamedScriptLoader('MREE', &REL_ENTRY(LoadMetaree)),
    MakeNamedScriptLoader('MSWM', &REL_ENTRY(LoadMetareeSwarm)),
    MakeNamedScriptLoader('MTDA', &REL_ENTRY(LoadMetroidAlpha)),
    MakeNamedScriptLoader('MIDI', &LoadMidi),
    MakeNamedScriptLoader('MNNG', &REL_ENTRY(LoadMinorIng)),
    MakeNamedScriptLoader('MYSF', &REL_ENTRY(LoadMysteryFlyer)),
    MakeNamedScriptLoader('OCTS', &REL_ENTRY(LoadOctopedeSegment)),
    MakeNamedScriptLoader('PARA', &REL_ENTRY(LoadParasite)),
    MakeNamedScriptLoader('PCAM', &LoadPathCamera),
    MakeNamedScriptLoader('PMCT', &LoadPathMeshCtrl),
    MakeNamedScriptLoader('PCKP', &LoadPickup),
    MakeNamedScriptLoader('PKGN', &LoadPickupGenerator),
    MakeNamedScriptLoader('PILB', &REL_ENTRY(LoadPillBug)),
    MakeNamedScriptLoader('PLAT', &LoadPlatform),
    MakeNamedScriptLoader('PSSM', &REL_ENTRY(LoadPlantScarabSwarm)),
    MakeNamedScriptLoader('PLAC', &REL_ENTRY(LoadPlayerActor)),
    MakeNamedScriptLoader('PLCT', &REL_ENTRY(LoadPlayerController)),
    MakeNamedScriptLoader('HINT', &LoadPlayerHint),
    MakeNamedScriptLoader('PSCH', &LoadPlayerStateChange),
    MakeNamedScriptLoader('PLRT', &REL_ENTRY(LoadPlayerTurret)),
    MakeNamedScriptLoader('POIN', &LoadPointOfInterest),
    MakeNamedScriptLoader('PRTT', &LoadPortalTransition),
    MakeNamedScriptLoader('SPOR', &REL_ENTRY(LoadPuddleSpore)),
    MakeNamedScriptLoader('PUFR', &REL_ENTRY(LoadPuffer)),
    MakeNamedScriptLoader('RADD', &LoadRadialDamage),
    MakeNamedScriptLoader('RRLY', &LoadRandomRelay),
    MakeNamedScriptLoader('REPL', &LoadRepulsor),
    MakeNamedScriptLoader('REZB', &REL_ENTRY(LoadRezbit)),
    MakeNamedScriptLoader('RPTL', &REL_ENTRY(LoadRiftPortal)),
    MakeNamedScriptLoader('RIPR', &REL_ENTRY(LoadRipper)),
    MakeNamedScriptLoader('RIPL', &LoadRipple),
    MakeNamedScriptLoader('RMAC', &LoadRoomAcoustics),
    MakeNamedScriptLoader('RSFA', &REL_ENTRY(LoadRsfAudio)),
    MakeNamedScriptLoader('RBPZ', &REL_ENTRY(LoadRubiksPuzzle)),
    MakeNamedScriptLoader('RUMB', &LoadRumbleEffect),
    MakeNamedScriptLoader('SAFE', &REL_ENTRY(LoadSafeZone)),
    MakeNamedScriptLoader('SFZC', &REL_ENTRY(LoadSafeZoneCrystal)),
    MakeNamedScriptLoader('SNDB', &REL_ENTRY(LoadSandBoss)),
    MakeNamedScriptLoader('WORM', &REL_ENTRY(LoadSandworm)),
    MakeNamedScriptLoader('SLCT', &LoadScriptLayerController),
    MakeNamedScriptLoader('SRLY', &LoadRelay),
    MakeNamedScriptLoader('SQTR', &LoadSequenceTimer),
    MakeNamedScriptLoader('SHDW', &LoadShadowProjector),
    MakeNamedScriptLoader('SHRD', &REL_ENTRY(LoadShredder)),
    MakeNamedScriptLoader('SHRK', &REL_ENTRY(LoadShrieker)),
    MakeNamedScriptLoader('SILH', &LoadSilhouette),
    MakeNamedScriptLoader('SKRP', &REL_ENTRY(LoadSkyRipple)),
    MakeNamedScriptLoader('SNAK', &REL_ENTRY(LoadSnakeWeedSwarm)),
    MakeNamedScriptLoader('SOND', &LoadSound),
    MakeNamedScriptLoader('SNDM', &LoadSoundModifier),
    MakeNamedScriptLoader('PIRT', &REL_ENTRY(LoadSpacePirate)),
    MakeNamedScriptLoader('SPNK', &REL_ENTRY(LoadSpankWeed)),
    MakeNamedScriptLoader('SPWN', &LoadSpawnPoint),
    MakeNamedScriptLoader('SPFN', &LoadSpecialFunction),
    MakeNamedScriptLoader('BALS', &LoadSpiderBallAttractionSurface),
    MakeNamedScriptLoader('BALW', &LoadSpiderBallWaypoint),
    MakeNamedScriptLoader('SPND', &LoadSpindleCamera),
    MakeNamedScriptLoader('SPIN', &LoadSpinner),
    MakeNamedScriptLoader('SPTR', &REL_ENTRY(LoadSplinter)),
    MakeNamedScriptLoader('SPLU', &REL_ENTRY(LoadSplitterCommandModule)),
    MakeNamedScriptLoader('SPLL', &REL_ENTRY(LoadSplitterMainChassis)),
    MakeNamedScriptLoader('SPBB', &REL_ENTRY(LoadSporbBase)),
    MakeNamedScriptLoader('SPBN', &REL_ENTRY(LoadSporbNeedle)),
    MakeNamedScriptLoader('SPBT', &REL_ENTRY(LoadSporbTop)),
    MakeNamedScriptLoader('SPBP', &REL_ENTRY(LoadSporbProjectile)),
    MakeNamedScriptLoader('STEM', &LoadSteam),
    MakeNamedScriptLoader('STOD', &REL_ENTRY(LoadStoneToad)),
    MakeNamedScriptLoader('STAU', &LoadStreamedAudio),
    MakeNamedScriptLoader('MOVI', &REL_ENTRY(LoadStreamedMovie)),
    MakeNamedScriptLoader('SUBT', &LoadSubtitle),
    MakeNamedScriptLoader('SURC', &LoadSurfaceCamera),
    MakeNamedScriptLoader('SWTC', &LoadSwitch),
    MakeNamedScriptLoader('SBS1', &REL_ENTRY(LoadSwampBossStage1)),
    MakeNamedScriptLoader('SBS2', &REL_ENTRY(LoadSwampBossStage2)),
    MakeNamedScriptLoader('TGPT', &LoadTargetingPoint),
    MakeNamedScriptLoader('TMAI', &LoadTeamAI),
    MakeNamedScriptLoader('TXPN', &LoadTextPane),
    MakeNamedScriptLoader('TKEY', &LoadTimeKeyframe),
    MakeNamedScriptLoader('TIMR', &LoadTimer),
    MakeNamedScriptLoader('TRGR', &LoadTrigger),
    MakeNamedScriptLoader('TRGE', &LoadTriggerEllipsoid),
    MakeNamedScriptLoader('TRGO', &LoadTriggerOrientated),
    MakeNamedScriptLoader('TRYC', &REL_ENTRY(LoadTryclops)),
    MakeNamedScriptLoader('FLAR', &LoadVisorFlare),
    MakeNamedScriptLoader('VGOO', &LoadVisorGoo),
    MakeNamedScriptLoader('WATR', &LoadWater),
    MakeNamedScriptLoader('WLWK', &REL_ENTRY(LoadWallWalker)),
    MakeNamedScriptLoader('WAYP', &LoadWaypoint),
    MakeNamedScriptLoader('WISP', &REL_ENTRY(LoadWispTentacle)),
    MakeNamedScriptLoader('WLIT', &LoadWorldLightFader),
    MakeNamedScriptLoader('TEL1', &LoadWorldTeleporter),
};

int g_LoaderFuncCount = ARRAY_SIZE(g_LoaderFuncs);

CAABox LoadCAABox(CStateManager& mgr, const TAreaId& areaId, const CVector3f& collisionSize,
                  const CVector3f& collisionOffset) {
  const CAABox box(-collisionSize.GetX() * 0.5f + collisionOffset.GetX(),
                   -collisionSize.GetY() * 0.5f + collisionOffset.GetY(),
                   -collisionSize.GetZ() * 0.5f + collisionOffset.GetZ(),
                   collisionSize.GetX() * 0.5f + collisionOffset.GetX(),
                   collisionSize.GetY() * 0.5f + collisionOffset.GetY(),
                   collisionSize.GetZ() * 0.5f + collisionOffset.GetZ());
  return box.GetTransformedAABox(mgr.GetWorld()->GetAreaAlways(areaId).GetTM().GetRotation());
}

CAABox LoadCAABox(CStateManager& mgr, const TAreaId& areaId, const CVector3f& scale,
                  const CTransform4f& transform, const CVector3f& collisionSize,
                  const CVector3f& collisionOffset) {
  const CVector3f scaledSize = collisionSize * scale;
  const CVector3f offset = collisionOffset * scale;
  CAABox box(-scaledSize.GetX() * 0.5f + offset.GetX(), -scaledSize.GetY() * 0.5f + offset.GetY(),
             -scaledSize.GetZ() * 0.5f + offset.GetZ(), scaledSize.GetX() * 0.5f + offset.GetX(),
             scaledSize.GetY() * 0.5f + offset.GetY(), scaledSize.GetZ() * 0.5f + offset.GetZ());
  box = box.GetTransformedAABox(transform.GetRotation());
  return box.GetTransformedAABox(mgr.GetWorld()->GetAreaAlways(areaId).GetTM().GetRotation());
}

CTransform4f ConvertEditorEulerToTransform4f(const CVector3f& orientation,
                                             const CVector3f& position) {
  const CQuaternion rotation = CQuaternion::ZRotation(CRelAngle::FromDegrees(orientation.GetZ())) *
                               CQuaternion::YRotation(CRelAngle::FromDegrees(orientation.GetY())) *
                               CQuaternion::XRotation(CRelAngle::FromDegrees(orientation.GetX()));
  const CMatrix3f matrix = rotation.BuildTransform();
  return CTransform4f::FromColumns(matrix.GetColumn(kDX), matrix.GetColumn(kDY),
                                   matrix.GetColumn(kDZ), position);
}

#include "MetroidPrime/ScriptLoader/ScriptLoaderDefinitions.inc"

CDamageInfo LdrToDamageInfo(const SLdrDamageInfo& data) {
  const EWeaponType type = static_cast< EWeaponType >(data.dI_WeaponType);
  CDamageInfo result(CWeaponMode(type, false, false, type == kWT_UnknownSource), data.dI_Damage,
                     data.dI_Radius, data.dI_KnockBackPower);
  result.SetApplyRadiusDamage(false);
  return result;
}

CHealthInfo LdrToHealthInfo(const SLdrHealthInfo& data) {
  return CHealthInfo(data.health, data.hI_KnockBackResistance);
}

CWeaponTypeVulnerability LdrToWeaponVulnerability(const SLdrWeaponVulnerability& data) {
  return CWeaponTypeVulnerability(data.damageMultiplier / 100.f,
                                  static_cast< CWeaponTypeVulnerability::EEffect >(data.effect),
                                  data.ignoreRadius);
}

CDamageVulnerability LdrToDamageVulnerability(const SLdrDamageVulnerability& data) {
  rstl::reserved_vector< CWeaponTypeVulnerability, kWT_Max > normal(
      CWeaponTypeVulnerability::Immune());
  rstl::reserved_vector< CWeaponTypeVulnerability, 4 > charged(CWeaponTypeVulnerability::Immune());
  rstl::reserved_vector< CWeaponTypeVulnerability, 4 > combo(CWeaponTypeVulnerability::Immune());

  normal[kWT_Power] = LdrToWeaponVulnerability(data.power);
  normal[kWT_Dark] = LdrToWeaponVulnerability(data.dark);
  normal[kWT_Light] = LdrToWeaponVulnerability(data.light);
  normal[kWT_Annihilator] = LdrToWeaponVulnerability(data.annihilator);
  normal[kWT_Bomb] = LdrToWeaponVulnerability(data.bomb);
  normal[kWT_PowerBomb] = LdrToWeaponVulnerability(data.powerBomb);
  normal[kWT_Missile] = LdrToWeaponVulnerability(data.missile);
  normal[kWT_BoostBall] = LdrToWeaponVulnerability(data.boostBall);
  normal[kWT_CannonBall] = LdrToWeaponVulnerability(data.cannonBall);
  normal[kWT_ScrewAttack] = LdrToWeaponVulnerability(data.screwAttack);
  normal[kWT_Phazon] = LdrToWeaponVulnerability(data.phazon);
  normal[kWT_AI] = LdrToWeaponVulnerability(data.ai);
  normal[kWT_PoisonWater1] = LdrToWeaponVulnerability(data.poisonWater);
  normal[kWT_PoisonWater2] = LdrToWeaponVulnerability(data.darkWater);
  normal[kWT_Lava] = LdrToWeaponVulnerability(data.lava);
  normal[kWT_Heat] = LdrToWeaponVulnerability(data.areaDamageHot);
  normal[kWT_Unused1] = LdrToWeaponVulnerability(data.areaDamageCold);
  normal[kWT_AreaDark] = LdrToWeaponVulnerability(data.areaDamageDark);
  normal[kWT_AreaLight] = LdrToWeaponVulnerability(data.areaDamageLight);
  normal[kWT_UnknownSource] = LdrToWeaponVulnerability(data.weaponVulnerability);
  normal[kWT_SafeZone] = LdrToWeaponVulnerability(data.normalSafeZone);

  charged[kWT_Power] = LdrToWeaponVulnerability(data.powerCharge);
  charged[kWT_Dark] = LdrToWeaponVulnerability(data.entangler);
  charged[kWT_Light] = LdrToWeaponVulnerability(data.lightBlast);
  charged[kWT_Annihilator] = LdrToWeaponVulnerability(data.sonicBoom);
  combo[kWT_Power] = LdrToWeaponVulnerability(data.superMissle);
  combo[kWT_Dark] = LdrToWeaponVulnerability(data.blackHole);
  combo[kWT_Light] = LdrToWeaponVulnerability(data.sunburst);
  combo[kWT_Annihilator] = LdrToWeaponVulnerability(data.imploder);
  return CDamageVulnerability(normal, charged, combo);
}

CVisorParameters LdrToVisorParameters(const SLdrVisorParameters& data) {
  return CVisorParameters(data.visorFlags, data.scanThrough);
}

CTransform4f LdrToTransform4f(const SLdrEditorProperties& data) {
  return ConvertEditorEulerToTransform4f(data.transform.rotation, data.transform.position);
}

CScannableParameters LdrToScannableParameters(const SLdrScannableParameters& data) {
  return CScannableParameters(data.scannableInfo0);
}

CAnimationParameters LdrToAnimationParameters(const SLdrAnimationSet& data) {
  return CAnimationParameters(data.ancs, data.character_index, data.initial_anim);
}

CLightParameters LdrToLightParameters(const SLdrLightParameters& data) {
  return CLightParameters(
      data.castShadow, data.shadowScale,
      static_cast< CLightParameters::EShadowTessellation >(data.unknown_0xecda4163),
      data.shadowAlpha, data.maxShadowHeight, data.ambientColor, data.affectedByDynamicLights,
      static_cast< CLightParameters::EWorldLightingOptions >(data.useWorldLighting),
      static_cast< CLightParameters::ELightRecalculationOptions >(data.lightRecalculation),
      data.lightingPositionOffset, data.numDynamicLights, data.numAreaLights, data.useOldLighting,
      data.useLightSet, data.ignoreAmbientLighting);
}

CActorParameters LdrToActorParameters(const SLdrActorParameters& data) {
  typedef rstl::pair< CAssetId, CAssetId > TModelAssets;
  const TModelAssets echo = gpResourceFactory->GetResourceTypeById(data.echoModel)
                                ? TModelAssets(data.echoModel, data.echoSkin)
                                : TModelAssets(0, 0);
  const TModelAssets dark = gpResourceFactory->GetResourceTypeById(data.darkModel)
                                ? TModelAssets(data.darkModel, data.darkSkin)
                                : TModelAssets(0, 0);
  return CActorParameters(
      LdrToLightParameters(data.lighting), LdrToScannableParameters(data.scannable), echo, dark,
      LdrToVisorParameters(data.visor), data.useGlobalRenderTime, data.forceRenderUnsorted,
      data.isHighlightedInDarkVisor, data.takesProjectedShadow, data.unknown_0xf07981e8,
      data.unknown_0x6df33845, data.maxVolume, data.maxEchoVolume, data.fadeInTime,
      data.fadeOutTime);
}

SEchoParameters LdrToEchoParameters(const SLdrEchoParameters& data) {
  return SEchoParameters(data.isEchoEmitter, data.onlyEmitDamage, data.numSoundWaves,
                         data.spaceBetweenWaves, data.waveLineSize, data.forcedMinimumVis);
}

CPatternedInfo LdrToPatternedInfo(const SLdrPatternedAITypedef& data,
                                  const SLdrIngPossessionData* possession) {
  CPatternedInfo result(LdrToHealthInfo(data.health), LdrToDamageVulnerability(data.vulnerability),
                        data.stateMachine, data.stateMachine2);
  result.mMass = data.mass;
  result.mSpeed = data.speed;
  result.mTurnSpeed = data.turnSpeed;
  result.mDetectionRange = data.detectionRange;
  result.mDetectionHeightRange = data.detectionHeightRange;
  result.mDetectionAngle = data.detectionAngle;
  result.mMinAttackRange = data.minAttackRange;
  result.mMaxAttackRange = data.maxAttackRange;
  result.mAverageAttackTime = data.averageAttackTime;
  result.mAttackTimeVariation = data.attackTimeVariation;
  result.mLeashRadius = data.leashRadius;
  result.mPlayerLeashRadius = data.playerLeashRadius;
  result.mPlayerLeashTime = data.playerLeashTime;
  result.mContactDamageInfo = LdrToDamageInfo(data.contactDamage);
  result.mDamageWaitTime = data.damageWaitTime;

  result.mHalfExtent = data.collisionRadius;
  result.mHeight = data.collisionHeight;
  result.mBodyOrigin = data.collisionOffset;
  result.mStepUpHeight = data.stepUpHeight;
  result.mXDamageThreshold = data.unknown_0xe287d8dd;
  result.mFrozenXDamageThreshold = data.unknown_0x66cdc6e8;
  result.mXDamageDelay = data.xDamageDelay;
  result.mDeathSfx = data.sound_XDamage;
  result.mAnimationParameters = CAnimationParameters(data.animationInformation.ancs,
                                                     data.animationInformation.character_index,
                                                     data.animationInformation.initial_anim);
  result.mIntoFreezeDuration = data.unknown_0x87d22d43;
  result.mOutOfFreezeDuration = data.unknown_0xf0790c1b;
  result.mFreezeDuration = data.freezeDuration;
  result.mPathfindingIndex = data.pathMeshIndex;

  result.mDeathExplosionOffset = data.gibParticlesOffset;
  result.mDeathExplosionParticle = data.gibParticles;
  result.mDeathExplosionElectric = data.gibElectricParticles;
  result.mIceDeathExplosionOffset = data.iceGibParticlesOffset;
  result.mIceDeathExplosionParticle = data.iceGibParticles;
  result.mIceShatterSfx = data.sound_IceXDamage;
  result.mIceVocalSfx = data.sound_IceXDamageVocal;
  result.mFrozenSfx = data.sound_Frozen;
  if (possession) {
    result.mIngPossessionData = *possession;
  }
  result.mKnockBackRules = data.knockbackRules;
  result.mCreatureSize = data.creatureSize;
  result.mEchoParameters = LdrToEchoParameters(data.echoParameters);
  return result;
}

rstl::optional_object< CModelData > LdrToModelData(const CVector3f& scale, CAssetId model,
                                                   const SLdrAnimationSet& animation,
                                                   bool canLoop) {
  const FourCC modelType = gpResourceFactory->GetResourceTypeById(model);
  const FourCC animationType = gpResourceFactory->GetResourceTypeById(animation.ancs);
  if (!modelType && !animationType) {
    return rstl::optional_object< CModelData >();
  }
  if (animationType == 'ANCS') {
    return CModelData(CAnimRes(animation.ancs, animation.character_index, scale,
                               animation.initial_anim, canLoop));
  }
  return CModelData(CStaticRes(model, scale));
}

CGrappleParameters LdrToGrappleParameters(const SLdrGrappleParameters& data) {
  return CGrappleParameters(data.grappleLength, data.grappleAttachLength,
                            data.grappleSpringConstant, data.grappleSpringLength,
                            data.grappleSpringTardis, data.swingForce, data.swingMaxForce,
                            data.swingArcAngle, data.swingTurnAngle, data.swingCameraPitch,
                            data.swingCameraMaxPitch, data.constrainToAxis);
}

CVector2f LdrToVector2f(const SLdrVector2f& data) { return CVector2f(data.x, data.y); }

CBasicSwarmData LdrToBasicSwarmData(const SLdrBasicSwarmProperties& data) {
  CBasicSwarmData result(LdrToDamageInfo(data.contactDamage), LdrToHealthInfo(data.health),
                         LdrToDamageVulnerability(data.damageVulnerability),
                         data.deathParticleEffect);
  result.mDamageWaitTime = data.damageWaitTime;
  result.mCollisionRadius = data.collisionRadius;
  result.mTouchRadius = data.touchRadius;
  result.mDamageRadius = data.damageRadius;
  result.mSpeed = data.speed;
  result.mCount = data.count;
  result.mMaxCount = data.maxCount;
  result.mInfluenceRadius = data.influenceRadius;
  result.mCohesionPriority = data.cohesionPriority;
  result.mAlignmentPriority = data.alignmentPriority;
  result.mSeparationPriority = data.separationPriority;
  result.mPathFollowingPriority = data.pathFollowingPriority;
  result.mPlayerAttractPriority = data.playerAttractPriority;
  result.mPlayerAttractDistance = data.playerAttractDistance;
  result.mSpawnSpeed = data.spawnSpeed;
  result.mNumDeathParticles = data.numDeathParticles;
  result.mAttackerCount = data.attackerCount;
  result.mAttackProximity = data.attackProximity;
  result.mSafeZoneAvoidancePriority = data.safeZoneAvoidancePriority;
  result.mTurnRate = data.turnRate;
  result.mAttackTimer = data.attackTimer;

  result.mLocomotionLoopedSound = static_cast< ushort >(data.locomotionLoopedSound);
  result.mAttackLoopedSound = static_cast< ushort >(data.attackLoopedSound);
  result.mSoundFallOff = data.soundFallOff;
  result.mMaxAudibleDistance = data.maxAudibleDistance;
  result.mMinVolume = static_cast< uchar >(data.minVolume);
  result.mMaxVolume = static_cast< uchar >(data.maxVolume);
  result.mFreezeDuration = data.freezeDuration;
  result.mLifeTime = data.lifeTime;
  result.mIsVulnerableToSafeZone = data.isVulnerableToSafeZone;
  result.xdc_1 = data.unknown_0x7eb5d9e8;
  result.mIsOrbitable = data.isOrbitable;
  result.mIndividuallyTargetable = data.individuallyTargetable;
  return result;
}

CPowerBombGuardianStageData
LdrToPowerBombGuardianStageData(const SLdrPowerBombGuardianStageProperties& data) {
  return CPowerBombGuardianStageData(
      data.minTimeBetweenAttacks, data.maxTimeBetweenAttacks, data.minTimeBetweenShots,
      data.maxTimeBetweenShots, static_cast< uchar >(data.minShotsInABurst),
      static_cast< uchar >(data.maxShotsInABurst), data.powerBombProjectileGravityMultiplier,
      data.waypointTargetSpreadRadius, data.doubleShotChance,
      static_cast< uchar >(data.minAttacksPerDoubleShot),
      static_cast< uchar >(data.maxAttacksPerDoubleShot));
}

CEntityInfo& LdrToEntityInfo(CEntityInfo& info, const SLdrEditorProperties& data) {
  info.SetActive(data.active);
  info.SetUpdateWhileOccluded((data.unknown_0x5d298a43 & 1) != 0);
  info.SetUpdateDuringCinematicSkip((data.unknown_0x5d298a43 & 2) != 0);
  return info;
}

CBeamInfo TLdrToBeamInfo(const SLdrPlasmaBeamInfo& data, int beamAttributes) {
  return CBeamInfo(beamAttributes, data.contactEffect, data.pulseEffect, data.glowTexture,
                   data.beamTexture, data.length, data.radius, data.expansionSpeed, data.lifeTime,
                   data.pulseSpeed, data.shutdownTime, data.contactEffectScale,
                   data.pulseEffectScale, data.innerColor, data.outerColor, data.travelSpeed,
                   data.beamStreaks);
}

ERglFogMode FogSelectionToFogMode(int selection) {
  // Guessed selector names; these values come from the serialized fog choice.
  enum EFogSelection { kFS_Linear = 1, kFS_Exp, kFS_Exp2, kFS_RevExp, kFS_RevExp2 };
  switch (selection) {
  case kFS_Linear:
    return kRFM_PerspLin;
  case kFS_Exp:
    return kRFM_PerspExp;
  case kFS_Exp2:
    return kRFM_PerspExp2;
  case kFS_RevExp:
    return kRFM_PerspRevExp;
  case kFS_RevExp2:
    return kRFM_PerspRevExp2;
  default:
    return kRFM_None;
  }
}

FScriptLoader GetScriptLoaderForType(FourCC type) {
  static bool sorted = false;
  if (!sorted) {
    rstl::sort(g_LoaderFuncs, g_LoaderFuncs + g_LoaderFuncCount);
    sorted = true;
  }

  NamedScriptLoader key;
  key.mType = type;
  key.mLoader = nullptr;
  const NamedScriptLoader* const loader =
      rstl::binary_find(g_LoaderFuncs, g_LoaderFuncs + g_LoaderFuncCount, key);
  if (loader && loader != g_LoaderFuncs + g_LoaderFuncCount) {
    return loader->mLoader;
  }
  return nullptr;
}
