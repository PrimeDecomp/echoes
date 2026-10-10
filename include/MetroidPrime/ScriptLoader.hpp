#ifndef _SCRIPTLOADER
#define _SCRIPTLOADER

#include "Kyoto/Graphics/CGraphics.hpp"

class CEntity;
class CInputStream;
class CStateManager;
class CEntityInfo;
class CActorParameters;
class CLightParameters;
class CVisorParameters;
class CAnimationParameters;
class CGrappleParameters;
class CBeamInfo;
class CBasicSwarmData;
class CPatternedInfo;
struct CPowerBombGuardianStageData;
class CDamageInfo;
class CTransform4f;
class CScannableParameters;
class CVector2f;
class CVector3f;
class CAABox;
struct TAreaId;
class CModelData;
class CHealthInfo;
class CDamageVulnerability;
namespace rstl {
template < class T >
class optional_object;
}
struct SLdrHealthInfo;
struct SLdrDamageVulnerability;
struct SLdrAnimationSet;
struct SEchoParameters;
struct SLdrActorParameters;
struct SLdrDamageInfo;
struct SLdrEchoParameters;
struct SLdrScannableParameters;
struct SLdrEditorProperties;
struct SLdrVector2f;
struct SLdrLightParameters;
struct SLdrVisorParameters;
struct SLdrGrappleParameters;
struct SLdrWeaponVulnerability;
struct CWeaponTypeVulnerability;
struct SLdrPlasmaBeamInfo;
struct SLdrBasicSwarmProperties;
struct SLdrPatternedAITypedef;
struct SLdrIngPossessionData;
struct SLdrPowerBombGuardianStageProperties;

// Names from the Echoes Wii SEL exports.
CDamageInfo LdrToDamageInfo(const SLdrDamageInfo& data);
CHealthInfo LdrToHealthInfo(const SLdrHealthInfo& data);
CDamageVulnerability LdrToDamageVulnerability(const SLdrDamageVulnerability& data);
rstl::optional_object< CModelData > LdrToModelData(const CVector3f& scale, CAssetId model,
                                                   const SLdrAnimationSet& animation, bool canLoop);
CAnimationParameters LdrToAnimationParameters(const SLdrAnimationSet& data);
CGrappleParameters LdrToGrappleParameters(const SLdrGrappleParameters& data);
CBeamInfo TLdrToBeamInfo(const SLdrPlasmaBeamInfo& data, int beamAttributes);
CBasicSwarmData LdrToBasicSwarmData(const SLdrBasicSwarmProperties& data);
CPatternedInfo LdrToPatternedInfo(const SLdrPatternedAITypedef& data,
                                  const SLdrIngPossessionData* possession);
CPowerBombGuardianStageData
LdrToPowerBombGuardianStageData(const SLdrPowerBombGuardianStageProperties& data);
// Reconstructed name, correlated with Prime and verified Echoes loader callers.
CAABox LoadCAABox(CStateManager& mgr, const TAreaId& areaId, const CVector3f& collisionSize,
                  const CVector3f& collisionOffset);
CAABox LoadCAABox(CStateManager& mgr, const TAreaId& areaId, const CVector3f& scale,
                  const CTransform4f& transform, const CVector3f& collisionSize,
                  const CVector3f& collisionOffset);
// Names from the Echoes Wii SEL exports.
CTransform4f LdrToTransform4f(const SLdrEditorProperties& data);
// Guessed name; shared by editor conversion and runtime parent attachment.
CTransform4f ConvertEditorEulerToTransform4f(const CVector3f& orientation,
                                             const CVector3f& position);
CVector2f LdrToVector2f(const SLdrVector2f& data);
// Guessed name for the native serialized-fog selector conversion.
ERglFogMode FogSelectionToFogMode(int selection);
// Guessed name, following the neighbouring SEL-derived conversions.
CScannableParameters LdrToScannableParameters(const SLdrScannableParameters& data);
// Guessed names for unexported conversions, supported by their native callers.
CLightParameters LdrToLightParameters(const SLdrLightParameters& data);
CVisorParameters LdrToVisorParameters(const SLdrVisorParameters& data);
CWeaponTypeVulnerability LdrToWeaponVulnerability(const SLdrWeaponVulnerability& data);
CActorParameters LdrToActorParameters(const SLdrActorParameters& data);
SEchoParameters LdrToEchoParameters(const SLdrEchoParameters& data);

typedef CEntity* (*FScriptLoader)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);

// Guessed name: target dispatches a FourCC to a loader callback.
FScriptLoader GetScriptLoaderForType(FourCC type);

CEntity* LoadActor(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadActorKeyframe(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadActorRotate(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadAdvancedCounter(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadAreaDamage(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadAIHint(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadAIJumpPoint(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadAIKeyframe(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadAIWaypoint(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadAmbientAI(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadAreaProperties(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadBallTrigger(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadCamera(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadCameraBlurKeyframe(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadCameraFilterKeyframe(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadCameraHint(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadCameraPitch(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadCameraShaker(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadCameraWaypoint(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadColorModulate(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadConditionalRelay(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadControlHint(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadControllerAction(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadCounter(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadCoverPoint(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadDamageableTrigger(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadDamageableTriggerOriented(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadDamageActor(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadDebris(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadDebrisExtended(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadDistanceFog(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadDock(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadDoor(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadEffect(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadEMPulse(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadEnvFxDensityController(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadFogVolume(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadGenerator(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadGrapplePoint(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadHUDHint(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadHUDMemo(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadDynamicLight(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadMemoryRelay(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadMidi(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadPathCamera(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadPathMeshCtrl(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadPickup(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadPickupGenerator(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadPlatform(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadPlayerHint(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadPlayerStateChange(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadPointOfInterest(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadPortalTransition(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadRadialDamage(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadRandomRelay(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadRepulsor(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadRipple(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadRoomAcoustics(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadRumbleEffect(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadScriptLayerController(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadRelay(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadSequenceTimer(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadShadowProjector(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadSilhouette(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadSound(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadSoundModifier(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadSpawnPoint(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadSpecialFunction(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadSpiderBallAttractionSurface(CStateManager& mgr, CInputStream& input,
                                         CEntityInfo& info);
CEntity* LoadSpiderBallWaypoint(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadSpindleCamera(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadSpinner(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadSteam(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadStreamedAudio(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadSubtitle(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadSurfaceCamera(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadSwitch(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadTargetingPoint(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadTeamAI(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadTextPane(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadTimeKeyframe(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadTimer(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadTrigger(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadTriggerEllipsoid(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadTriggerOrientated(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadVisorFlare(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadVisorGoo(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadWater(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadWaypoint(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadWorldLightFader(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadWorldTeleporter(CStateManager& mgr, CInputStream& input, CEntityInfo& info);

#endif // _SCRIPTLOADER
