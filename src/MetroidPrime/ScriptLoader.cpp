#include "MetroidPrime/ScriptLoader.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/CActorParameters.hpp"
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
#include "MetroidPrime/ScriptLoader/SLdrActorRotate.hpp"
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
#include "MetroidPrime/Weapons/CBeamInfo.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/pair.hpp"

struct NamedScriptLoader {
  NamedScriptLoader(FourCC type, FScriptLoader loader) : mType(type), mLoader(loader) {}

  FourCC mType;
  FScriptLoader mLoader;

  bool operator<(const NamedScriptLoader& other) const {
    return static_cast< int >(mType) < static_cast< int >(other.mType);
  }
};

NamedScriptLoader g_LoaderFuncs[] = {
    NamedScriptLoader('ACTR', &LoadActor),
    NamedScriptLoader('ACKF', &LoadActorKeyframe),
    NamedScriptLoader('AROT', &LoadActorRotate),
    NamedScriptLoader('ACNT', &LoadAdvancedCounter),
    NamedScriptLoader('ADMG', &LoadAreaDamage),
    NamedScriptLoader('AIHT', &LoadAIHint),
    NamedScriptLoader('AJMP', &LoadAIJumpPoint),
    NamedScriptLoader('AIKF', &LoadAIKeyframe),
    NamedScriptLoader('AIMT', &LoadAIMannedTurret),
    NamedScriptLoader('AIWP', &LoadAIWaypoint),
    NamedScriptLoader('AMIA', &LoadAmbientAI),
    NamedScriptLoader('REAA', &LoadAreaAttributes),
    NamedScriptLoader('ATMA', &LoadAtomicAlpha),
    NamedScriptLoader('ATMB', &LoadAtomicBeta),
    NamedScriptLoader('BSWM', &LoadBacteriaSwarm),
    NamedScriptLoader('BALT', &LoadBallTrigger),
    NamedScriptLoader('BLOG', &LoadBlogg),
    NamedScriptLoader('BRZG', &LoadBrizgee),
    NamedScriptLoader('CAMR', &LoadCamera),
    NamedScriptLoader('BLUR', &LoadCameraBlurKeyframe),
    NamedScriptLoader('FILT', &LoadCameraFilterKeyframe),
    NamedScriptLoader('CAMH', &LoadCameraHint),
    NamedScriptLoader('CAMP', &LoadCameraPitch),
    NamedScriptLoader('CAMS', &LoadCameraShaker),
    NamedScriptLoader('CAMW', &LoadCameraWaypoint),
    NamedScriptLoader('CANB', &LoadCannonBall),
    NamedScriptLoader('CHOG', &LoadChozoGhost),
    NamedScriptLoader('COIN', &LoadCoin),
    NamedScriptLoader('CLRM', &LoadColorModulate),
    NamedScriptLoader('CMDO', &LoadCommandPirate),
    NamedScriptLoader('CRLY', &LoadConditionalRelay),
    NamedScriptLoader('CTLH', &LoadControlHint),
    NamedScriptLoader('CNTA', &LoadControllerAction),
    NamedScriptLoader('CNTR', &LoadCounter),
    NamedScriptLoader('COVR', &LoadCoverPoint),
    NamedScriptLoader('CRLT', &LoadCrystallite),
    NamedScriptLoader('DTRG', &LoadDamageableTrigger),
    NamedScriptLoader('DTRO', &LoadDamageableTriggerOriented),
    NamedScriptLoader('DMGA', &LoadDamageActor),
    NamedScriptLoader('DRKC', &LoadDarkCommando),
    NamedScriptLoader('DRKS', &LoadDarkSamus),
    NamedScriptLoader('DSBS', &LoadDarkSamusBattleStage),
    NamedScriptLoader('DKTR', &LoadDarkTrooper),
    NamedScriptLoader('DBR1', &LoadDebris),
    NamedScriptLoader('DBR2', &LoadDebrisExtended),
    NamedScriptLoader('DBAR', &LoadDestructableBarrier),
    NamedScriptLoader('DGRD', &LoadDigitalGuardian),
    NamedScriptLoader('DGHD', &LoadDigitalGuardianHead),
    NamedScriptLoader('DFOG', &LoadDistanceFog),
    NamedScriptLoader('DOCK', &LoadDock),
    NamedScriptLoader('DOOR', &LoadDoor),
    NamedScriptLoader('EFCT', &LoadEffect),
    NamedScriptLoader('EPRT', &LoadElitePirate),
    NamedScriptLoader('EMS1', &LoadEmperorIngStage1),
    NamedScriptLoader('EM2T', &LoadEmperorIngStage2Tentacle),
    NamedScriptLoader('EMS3', &LoadEmperorIngStage3),
    NamedScriptLoader('EMPU', &LoadEMPulse),
    NamedScriptLoader('FXDC', &LoadEnvFxDensityController),
    NamedScriptLoader('EYEB', &LoadEyeBall),
    NamedScriptLoader('FISH', &LoadFishCloud),
    NamedScriptLoader('FSHM', &LoadFishCloudModifier),
    NamedScriptLoader('FSWM', &LoadFlyerSwarm),
    NamedScriptLoader('FPRT', &LoadFlyingPirate),
    NamedScriptLoader('FOGO', &LoadFogOverlay),
    NamedScriptLoader('FOGV', &LoadFogVolume),
    NamedScriptLoader('FGTO', &LoadForgottenObject),
    NamedScriptLoader('FNWK', &LoadFrontEndDataNetwork),
    NamedScriptLoader('GENR', &LoadGenerator),
    NamedScriptLoader('GBUG', &LoadGlowBug),
    NamedScriptLoader('GRAP', &LoadGrapplePoint),
    NamedScriptLoader('GRCH', &LoadGrenchler),
    NamedScriptLoader('GMNU', &LoadGuiMenu),
    NamedScriptLoader('GPJN', &LoadGuiPlayerJoinManager),
    NamedScriptLoader('GSCR', &LoadGuiScreen),
    NamedScriptLoader('GSLD', &LoadGuiSlider),
    NamedScriptLoader('GWIG', &LoadGuiWidget),
    NamedScriptLoader('GNTB', &LoadGunTurretBase),
    NamedScriptLoader('GNTT', &LoadGunTurretTop),
    NamedScriptLoader('HHNT', &LoadHUDHint),
    NamedScriptLoader('MEMO', &LoadHUDMemo),
    NamedScriptLoader('INGS', &LoadIngs),
    NamedScriptLoader('IBSM', &LoadIngBlobSwarm),
    NamedScriptLoader('IBBG', &LoadIngBoostBallGuardian),
    NamedScriptLoader('IPUD', &LoadIngPuddle),
    NamedScriptLoader('ISSW', &LoadIngSnatchingSwarm),
    NamedScriptLoader('ISJG', &LoadIngSpaceJumpGuardian),
    NamedScriptLoader('ISBG', &LoadIngSpiderBallGuardian),
    NamedScriptLoader('KRAL', &LoadKralee),
    NamedScriptLoader('KROC', &LoadKrocus),
    NamedScriptLoader('DLHT', &LoadDynamicLight),
    NamedScriptLoader('LUMI', &LoadLumite),
    NamedScriptLoader('MING', &LoadMediumIng),
    NamedScriptLoader('MRLY', &LoadMemoryRelay),
    NamedScriptLoader('MREE', &LoadMetaree),
    NamedScriptLoader('MSWM', &LoadMetareeSwarm),
    NamedScriptLoader('MTDA', &LoadMetroidAlpha),
    NamedScriptLoader('MIDI', &LoadMidi),
    NamedScriptLoader('MNNG', &LoadMinorIng),
    NamedScriptLoader('MYSF', &LoadMysteryFlyer),
    NamedScriptLoader('OCTS', &LoadOctopedeSegment),
    NamedScriptLoader('PARA', &LoadParasite),
    NamedScriptLoader('PCAM', &LoadPathCamera),
    NamedScriptLoader('PMCT', &LoadPathMeshCtrl),
    NamedScriptLoader('PCKP', &LoadPickup),
    NamedScriptLoader('PKGN', &LoadPickupGenerator),
    NamedScriptLoader('PILB', &LoadPillBug),
    NamedScriptLoader('PLAT', &LoadPlatform),
    NamedScriptLoader('PSSM', &LoadPlantScarabSwarm),
    NamedScriptLoader('PLAC', &LoadPlayerActor),
    NamedScriptLoader('PLCT', &LoadPlayerController),
    NamedScriptLoader('HINT', &LoadPlayerHint),
    NamedScriptLoader('PSCH', &LoadPlayerStateChange),
    NamedScriptLoader('PLRT', &LoadPlayerTurret),
    NamedScriptLoader('POIN', &LoadPointOfInterest),
    NamedScriptLoader('PRTT', &LoadPortalTransition),
    NamedScriptLoader('SPOR', &LoadPuddleSpore),
    NamedScriptLoader('PUFR', &LoadPuffer),
    NamedScriptLoader('RADD', &LoadRadialDamage),
    NamedScriptLoader('RRLY', &LoadRandomRelay),
    NamedScriptLoader('REPL', &LoadRepulsor),
    NamedScriptLoader('REZB', &LoadRezbit),
    NamedScriptLoader('RPTL', &LoadRiftPortal),
    NamedScriptLoader('RIPR', &LoadRipper),
    NamedScriptLoader('RIPL', &LoadRipple),
    NamedScriptLoader('RMAC', &LoadRoomAcoustics),
    NamedScriptLoader('RSFA', &LoadRsfAudio),
    NamedScriptLoader('RBPZ', &LoadRubiksPuzzle),
    NamedScriptLoader('RUMB', &LoadRumbleEffect),
    NamedScriptLoader('SAFE', &LoadSafeZone),
    NamedScriptLoader('SFZC', &LoadSafeZoneCrystal),
    NamedScriptLoader('SNDB', &LoadSandBoss),
    NamedScriptLoader('WORM', &LoadSandworm),
    NamedScriptLoader('SLCT', &LoadScriptLayerController),
    NamedScriptLoader('SRLY', &LoadRelay),
    NamedScriptLoader('SQTR', &LoadSequenceTimer),
    NamedScriptLoader('SHDW', &LoadShadowProjector),
    NamedScriptLoader('SHRD', &LoadShredder),
    NamedScriptLoader('SHRK', &LoadShrieker),
    NamedScriptLoader('SILH', &LoadSilhouette),
    NamedScriptLoader('SKRP', &LoadSkyRipple),
    NamedScriptLoader('SNAK', &LoadSnakeWeedSwarm),
    NamedScriptLoader('SOND', &LoadSound),
    NamedScriptLoader('SNDM', &LoadSoundModifier),
    NamedScriptLoader('PIRT', &LoadSpacePirate),
    NamedScriptLoader('SPNK', &LoadSpankWeed),
    NamedScriptLoader('SPWN', &LoadSpawnPoint),
    NamedScriptLoader('SPFN', &LoadSpecialFunction),
    NamedScriptLoader('BALS', &LoadSpiderBallAttractionSurface),
    NamedScriptLoader('BALW', &LoadSpiderBallWaypoint),
    NamedScriptLoader('SPND', &LoadSpindleCamera),
    NamedScriptLoader('SPIN', &LoadSpinner),
    NamedScriptLoader('SPTR', &LoadSplinter),
    NamedScriptLoader('SPLU', &LoadSplitterCommandModule),
    NamedScriptLoader('SPLL', &LoadSplitterMainChassis),
    NamedScriptLoader('SPBB', &LoadSporbBase),
    NamedScriptLoader('SPBN', &LoadSporbNeedle),
    NamedScriptLoader('SPBT', &LoadSporbTop),
    NamedScriptLoader('SPBP', &LoadSporbProjectile),
    NamedScriptLoader('STEM', &LoadSteam),
    NamedScriptLoader('STOD', &LoadStoneToad),
    NamedScriptLoader('STAU', &LoadStreamedAudio),
    NamedScriptLoader('MOVI', &LoadStreamedMovie),
    NamedScriptLoader('SUBT', &LoadSubtitle),
    NamedScriptLoader('SURC', &LoadSurfaceCamera),
    NamedScriptLoader('SWTC', &LoadSwitch),
    NamedScriptLoader('SBS1', &LoadSwampBossStage1),
    NamedScriptLoader('SBS2', &LoadSwampBossStage2),
    NamedScriptLoader('TGPT', &LoadTargetingPoint),
    NamedScriptLoader('TMAI', &LoadTeamAI),
    NamedScriptLoader('TXPN', &LoadTextPane),
    NamedScriptLoader('TKEY', &LoadTimeKeyframe),
    NamedScriptLoader('TIMR', &LoadTimer),
    NamedScriptLoader('TRGR', &LoadTrigger),
    NamedScriptLoader('TRGE', &LoadTriggerEllipsoid),
    NamedScriptLoader('TRGO', &LoadTriggerOrientated),
    NamedScriptLoader('TRYC', &LoadTryclops),
    NamedScriptLoader('FLAR', &LoadVisorFlare),
    NamedScriptLoader('VGOO', &LoadVisorGoo),
    NamedScriptLoader('WATR', &LoadWater),
    NamedScriptLoader('WLWK', &LoadWallWalker),
    NamedScriptLoader('WAYP', &LoadWaypoint),
    NamedScriptLoader('WISP', &LoadWispTentacle),
    NamedScriptLoader('WLIT', &LoadWorldLightFader),
    NamedScriptLoader('TEL1', &LoadWorldTeleporter),
};

CAABox LoadCAABox(CStateManager& mgr, const TAreaId& areaId, const CVector3f& collisionSize,
                  const CVector3f& collisionOffset) {
  const CAABox box(-collisionSize * 0.5f + collisionOffset, collisionSize * 0.5f + collisionOffset);
  return box.GetTransformedAABox(mgr.GetWorld()->GetAreaAlways(areaId).GetTM());
}

CAABox LoadCAABox(CStateManager& mgr, const TAreaId& areaId, const CVector3f& scale,
                  const CTransform4f& transform, const CVector3f& collisionSize,
                  const CVector3f& collisionOffset) {
  const CVector3f scaledSize = collisionSize * scale;
  const CVector3f offset = collisionOffset * scale;
  const CAABox box(-scaledSize * 0.5f + offset, scaledSize * 0.5f + offset);
  const CAABox transformed = box.GetTransformedAABox(transform);
  return transformed.GetTransformedAABox(mgr.GetWorld()->GetAreaAlways(areaId).GetTM());
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
  result.mAnimationParameters = LdrToAnimationParameters(data.animationInformation);
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
  result.mIsVulnerableToSafeZone = data.isVulnerableToSafeZone;
  result.xdc_1 = data.unknown_0x7eb5d9e8;
  result.mIsOrbitable = data.isOrbitable;
  result.mIndividuallyTargetable = data.individuallyTargetable;
  result.mLifeTime = data.lifeTime;
  return result;
}

CPowerBombGuardianStageData
LdrToPowerBombGuardianStageData(const SLdrPowerBombGuardianStageProperties& data) {
  return CPowerBombGuardianStageData(
      data.minTimeBetweenAttacks, data.maxTimeBetweenAttacks, data.minTimeBetweenShots,
      data.maxTimeBetweenShots, static_cast< uchar >(data.minShotsInABurst),
      static_cast< uchar >(data.maxShotsInABurst), data.powerBombProjectileGravityMultiplier,
      data.unknown_0xd356c997, data.doubleShotChance, static_cast< uchar >(data.unknown_0x87cc8ba4),
      static_cast< uchar >(data.unknown_0x6491357e));
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
  enum EFogSelection { kFS_None, kFS_Linear, kFS_Exp, kFS_Exp2, kFS_RevExp, kFS_RevExp2 };
  ERglFogMode mode = kRFM_None;
  switch (selection) {
  case kFS_None:
    mode = kRFM_None;
    break;
  case kFS_Linear:
    mode = kRFM_PerspLin;
    break;
  case kFS_Exp:
    mode = kRFM_PerspExp;
    break;
  case kFS_Exp2:
    mode = kRFM_PerspExp2;
    break;
  case kFS_RevExp:
    mode = kRFM_PerspRevExp;
    break;
  case kFS_RevExp2:
    mode = kRFM_PerspRevExp2;
    break;
  }
  return mode;
}

// Guessed name.
int g_LoaderFuncCount = ARRAY_SIZE(g_LoaderFuncs);

FScriptLoader GetScriptLoaderForType(FourCC type) {
  static bool sorted = false;
  if (!sorted) {
    rstl::sort(g_LoaderFuncs, g_LoaderFuncs + g_LoaderFuncCount);
    sorted = true;
  }

  const NamedScriptLoader key(type, nullptr);
  const NamedScriptLoader* const loader =
      rstl::binary_find(g_LoaderFuncs, g_LoaderFuncs + g_LoaderFuncCount, key);
  if (loader != nullptr && loader != g_LoaderFuncs + g_LoaderFuncCount) {
    return loader->mLoader;
  }
  return nullptr;
}
