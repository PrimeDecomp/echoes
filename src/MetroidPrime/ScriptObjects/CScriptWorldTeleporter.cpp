#include "MetroidPrime/ScriptObjects/CScriptWorldTeleporter.hpp"

#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CPortalTransition.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CScriptCameraSpline.hpp"
#include "MetroidPrime/Factories/CCharacterFactory.hpp"
#include "MetroidPrime/Factories/CCharacterFactoryBuilder.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Player/CWorldTransManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrWorldTeleporter.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPortalTransition.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSafeZone.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

CScriptWorldTeleporter::CScriptWorldTeleporter(
    const TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CAssetId worldId,
    const CAssetId areaId, const CAssetId playerAncs, const int defaultAnim, const int charIdx,
    const CVector3f& playerScale, const CAssetId platformModel, const CVector3f& platformScale,
    const CAssetId backgroundModel, const CVector3f& backgroundScale, const bool upElevator,
    const CAssetId soundGroup, const ushort soundId, const uchar volume, const uchar panning)
: CEntity(uid, info, name, 0)
, mWorldId(worldId)
, mAreaId(areaId)
, mType(kTT_Elevator)
, mUpElevator(upElevator)
, mInTransition(false)
, mFadeWhite(false)
, mDisplaySubtitles(false)
, mIntroText(false)
, mCharFadeTime(0.1f)
, mCharsPerSecond(8.f)
, mStartDelay(0.f)
, mPlayerAncs(playerAncs)
, mPlayerDefaultAnim(defaultAnim)
, mPlayerCharIdx(charIdx)
, mPlayerScale(playerScale)
, mPlatformModel(platformModel)
, mPlatformScale(platformScale)
, mBackgroundModel(backgroundModel)
, mBackgroundScale(backgroundScale)
, mSoundGroup(soundGroup)
, mSoundId(soundId)
, mVolume(volume)
, mPanning(panning)
, mAudioStream() {}

CScriptWorldTeleporter::CScriptWorldTeleporter(
    const TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CAssetId worldId,
    const CAssetId areaId, const ushort soundId, const uchar volume, const uchar panning,
    const CAssetId fontId, const CAssetId stringId, const bool fadeWhite,
    const rstl::string& audioStream, const bool displaySubtitles, const bool introText,
    const float charFadeTime, const float charsPerSecond, const float startDelay,
    const float endDelay, const float subtitleFadeInDelay, const float subtitleFadeTime)
: CEntity(uid, info, name, 0)
, mWorldId(worldId)
, mAreaId(areaId)
, mType(kTT_Text)
, mUpElevator(false)
, mInTransition(false)
, mFadeWhite(fadeWhite)
, mDisplaySubtitles(displaySubtitles)
, mIntroText(introText)
, mCharFadeTime(charFadeTime)
, mCharsPerSecond(charsPerSecond)
, mStartDelay(startDelay)
, mEndDelay(endDelay)
, mSubtitleFadeInDelay(subtitleFadeInDelay)
, mSubtitleFadeTime(subtitleFadeTime)
, mPlayerAncs(kInvalidAssetId)
, mPlayerDefaultAnim(-1)
, mPlayerCharIdx(0)
, mPlayerScale(CVector3f::Zero())
, mPlatformModel(kInvalidAssetId)
, mPlatformScale(CVector3f::Zero())
, mBackgroundModel(kInvalidAssetId)
, mBackgroundScale(CVector3f::Zero())
, mSoundGroup(kInvalidAssetId)
, mSoundId(soundId)
, mVolume(volume)
, mPanning(panning)
, mFontId(fontId)
, mStringId(stringId)
, mAudioStream(audioStream) {}

void CScriptWorldTeleporter::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();

  if (GetActive()) {
    CWorldTransManager* transMgr = mgr.WorldTransManager();
    switch (message) {
    case kSM_SetToZero: {
      mgr.World()->SetLoadPauseState(true);
      const CAssetId currentWorld = gpGameState->CurrentWorldAssetId();
      gpGameState->SetCurrentWorldId(mWorldId);

      if (gpResourceFactory->GetResLoader().GetResourceTypeById(mWorldId) == 'MLVL') {
        StartTransition(mgr);
        gpGameState->SetCurrentWorldId(mWorldId);
        gpGameState->CurrentWorldState().SetDesiredAreaAssetId(mAreaId);
        gpMain->SetRestartMode(CMain::kRM_None);
        mgr.QuitGame();
      } else {
        mInTransition = false;
        transMgr->DisableTransition();
        gpGameState->SetCurrentWorldId(currentWorld);
      }
      break;
    }
    case kSM_Play:
      StartTransition(mgr);
      transMgr->SetSfx(mSoundId, mVolume, mPanning);
      transMgr->SfxStart();
      break;

    case kSM_Stop:
      mInTransition = false;
      transMgr->DisableTransition();
      transMgr->SfxStop();
      break;

    default:
      break;
    }
  }

  CEntity::AcceptScriptMsg(mgr, msg);
}

void CScriptWorldTeleporter::StartTransition(CStateManager& mgr) {
  if (mInTransition) {
    return;
  }

  CWorldTransManager* transMgr = mgr.WorldTransManager();

  if (mType == kTT_Elevator && mPlayerAncs != kInvalidAssetId &&
      mPlayerDefaultAnim != kInvalidAssetId) {
    int suit = gpGameState->GetPlayerState()->ShouldDrawGravityBoost(mgr);
    {
      const CAnimRes factoryRes(mPlayerAncs, mPlayerCharIdx, mPlayerScale, mPlayerDefaultAnim,
                                true);
      TCachedToken< CCharacterFactory > factory(gpCharacterFactoryBuilder->GetFactory(factoryRes),
                                                true);
      const int suitCount = factory.GetObject()->GetCharacterCount();
      if (suit >= suitCount) {
        suit = gpGameState->GetPlayerState()->GetCurrentSuitRaw();
        if (suit >= suitCount) {
          suit = 0;
        }
      }
    }

    const CAnimRes animRes(mPlayerAncs, suit, mPlayerScale, mPlayerDefaultAnim, true);
    const CScriptCamera* firstPass = TCastToConstPtr< CScriptCamera >(
        mgr.GetObjectById(FindConnectedObject(mgr, kSS_XINF, kSM_None)));
    const CScriptCamera* secondPass = TCastToConstPtr< CScriptCamera >(
        mgr.GetObjectById(FindConnectedObject(mgr, kSS_XINB, kSM_None)));
    const CScriptSafeZone* safeZone = TCastToConstPtr< CScriptSafeZone >(
        mgr.GetObjectById(FindConnectedObject(mgr, kSS_Connect, kSM_None)));
    const CDarkWorldInfo* darkWorldInfo = nullptr;
    if (safeZone != nullptr) {
      darkWorldInfo = &safeZone->GetDarkWorldInfo();
    }

    rstl::optional_object< CToken > soundGroup;
    if (mSoundGroup != kInvalidAssetId) {
      soundGroup = gpSimplePool->GetObj(SObjectTag('AGSC', mSoundGroup));
    }

    transMgr->EnableTransition(
        animRes, gpGameState->GetPlayerState()->ShouldDrawGrapple(), mPlatformModel, mPlatformScale,
        mBackgroundModel, mBackgroundScale, mUpElevator,
        firstPass ? &firstPass->GetSpline() : nullptr,
        secondPass ? &secondPass->GetSpline() : nullptr,
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mInverseTransform,
        soundGroup, darkWorldInfo);
    mInTransition = true;
  } else if (mType == kTT_Text && mFontId != kInvalidAssetId && mStringId != kInvalidAssetId) {
    transMgr->EnableTransition(mFontId, mStringId, 0, mFadeWhite, mCharFadeTime, mCharsPerSecond,
                               mStartDelay, mEndDelay, mSubtitleFadeInDelay, mSubtitleFadeTime,
                               mAudioStream, mVolume, mDisplaySubtitles, false);
    mInTransition = true;
  } else {
    const CEntity* portal = mgr.GetObjectById(FindConnectedObject(mgr, kSS_Play, kSM_None));
    if (const CScriptPortalTransition* transition =
            TCastToConstPtr< CScriptPortalTransition >(portal)) {
      rstl::single_ptr< CPortalTransition > portalTransition(transition->CreateTransition(mgr));
      gpGameState->WorldTransitionManager()->EnableTransition(portalTransition, mVolume);
      mInTransition = true;
    } else {
      transMgr->DisableTransition();
    }
  }
}

CEntity* LoadWorldTeleporter(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrWorldTeleporter sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrWorldTeleporter.inc"

  if (sldrThis.isTeleport) {
    return rs_new CScriptWorldTeleporter(
        mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
        LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.world, sldrThis.area,
        sldrThis.elevator, sldrThis.volume, sldrThis.pan, sldrThis.displayFont, sldrThis.string,
        sldrThis.isFadeWhite, sldrThis.audioStream, sldrThis.displaySubtitles,
        sldrThis.unknown_0x5657ca1c, sldrThis.characterFadeTime, sldrThis.charactersPerSecond,
        sldrThis.startDelay, sldrThis.endDelay, sldrThis.subtitleFadeInDelay,
        sldrThis.subtitleFadeTime);
  }

  return rs_new CScriptWorldTeleporter(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.world, sldrThis.area,
      sldrThis.animationInformation.ancs, sldrThis.animationInformation.initial_anim,
      sldrThis.animationInformation.character_index, sldrThis.playerScale, sldrThis.platform,
      sldrThis.platformScale, sldrThis.shaft, sldrThis.shaftScale, sldrThis.unknown_0x2e997e0b,
      sldrThis.soundGroup, sldrThis.elevator, sldrThis.volume, sldrThis.pan);
}

CScriptWorldTeleporter::~CScriptWorldTeleporter() {}
