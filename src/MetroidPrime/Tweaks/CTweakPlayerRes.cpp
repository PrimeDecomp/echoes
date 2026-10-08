#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"

#include "Kyoto/CResFactory.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerRes.hpp"

// Guessed name
static CAssetId ResolveAssetId(const rstl::string& name) {
  return gpResourceFactory->GetResourceIdByName(name.c_str())->GetId();
}

void CTweakPlayerRes::CacheResources() {
  mSaveStationIcon = ResolveAssetId(mData->autoMapperIcons.saveStationIcon);
  mMissileStationIcon = ResolveAssetId(mData->autoMapperIcons.missileStationIcon);
  mElevatorIcon = ResolveAssetId(mData->autoMapperIcons.elevatorIconIcon);
  mPortalIcon = ResolveAssetId(mData->autoMapperIcons.portalIcon);
  mMinesFirstBreakTopIcon = ResolveAssetId(mData->autoMapperIcons.minesFirstBreakTopIcon);
  mMinesFirstBreakBottomIcon = ResolveAssetId(mData->autoMapperIcons.minesFirstBreakBottomIcon);
  mMinesSecondBreakTopIcon = ResolveAssetId(mData->autoMapperIcons.minesSecondBreakTopIcon);
  mMinesSecondBreakBottomIcon = ResolveAssetId(mData->autoMapperIcons.minesSecondBreakBottomIcon);
  mTranslatorDoorIcon = ResolveAssetId(mData->autoMapperIcons.translatorDoorIcon);

  mLStick.clear();
  mLStick.push_back(ResolveAssetId(mData->mapScreenIcons.lStickN));
  mLStick.push_back(ResolveAssetId(mData->mapScreenIcons.lStickU));
  mLStick.push_back(ResolveAssetId(mData->mapScreenIcons.lStickUL));
  mLStick.push_back(ResolveAssetId(mData->mapScreenIcons.lStickL));
  mLStick.push_back(ResolveAssetId(mData->mapScreenIcons.lStickDL));
  mLStick.push_back(ResolveAssetId(mData->mapScreenIcons.lStickD));
  mLStick.push_back(ResolveAssetId(mData->mapScreenIcons.lStickDR));
  mLStick.push_back(ResolveAssetId(mData->mapScreenIcons.lStickR));
  mLStick.push_back(ResolveAssetId(mData->mapScreenIcons.lStickUR));

  mCStick.clear();
  mCStick.push_back(ResolveAssetId(mData->mapScreenIcons.cStickN));
  mCStick.push_back(ResolveAssetId(mData->mapScreenIcons.cStickU));
  mCStick.push_back(ResolveAssetId(mData->mapScreenIcons.cStickUL));
  mCStick.push_back(ResolveAssetId(mData->mapScreenIcons.cStickL));
  mCStick.push_back(ResolveAssetId(mData->mapScreenIcons.cStickDL));
  mCStick.push_back(ResolveAssetId(mData->mapScreenIcons.cStickD));
  mCStick.push_back(ResolveAssetId(mData->mapScreenIcons.cStickDR));
  mCStick.push_back(ResolveAssetId(mData->mapScreenIcons.cStickR));
  mCStick.push_back(ResolveAssetId(mData->mapScreenIcons.cStickUR));

  mLTrigger.clear();
  mLTrigger.push_back(ResolveAssetId(mData->mapScreenIcons.lTriggerOut));
  mLTrigger.push_back(ResolveAssetId(mData->mapScreenIcons.lTriggerIn));

  mRTrigger.clear();
  mRTrigger.push_back(ResolveAssetId(mData->mapScreenIcons.rTriggerOut));
  mRTrigger.push_back(ResolveAssetId(mData->mapScreenIcons.rTriggerIn));

  mStartButton.clear();
  mStartButton.push_back(ResolveAssetId(mData->mapScreenIcons.startButtonOut));
  mStartButton.push_back(ResolveAssetId(mData->mapScreenIcons.startButtonIn));

  mAButton.clear();
  mAButton.push_back(ResolveAssetId(mData->mapScreenIcons.aButtonOut));
  mAButton.push_back(ResolveAssetId(mData->mapScreenIcons.aButtonIn));

  mBButton.clear();
  mBButton.push_back(ResolveAssetId(mData->mapScreenIcons.bButtonOut));
  mBButton.push_back(ResolveAssetId(mData->mapScreenIcons.bButtonIn));

  mXButton.clear();
  mXButton.push_back(ResolveAssetId(mData->mapScreenIcons.xButtonOut));
  mXButton.push_back(ResolveAssetId(mData->mapScreenIcons.xButtonIn));

  mYButton.clear();
  mYButton.push_back(ResolveAssetId(mData->mapScreenIcons.yButtonOut));
  mYButton.push_back(ResolveAssetId(mData->mapScreenIcons.yButtonIn));

  mBallTransitionsANCS = ResolveAssetId(mData->ballTransitionResources.suitANCS);
  mBallTransitions[0] = ResolveAssetId(mData->ballTransitionResources.gunResources.power_Beam);
  mBallTransitions[1] = ResolveAssetId(mData->ballTransitionResources.gunResources.ice_Beam);
  mBallTransitions[2] = ResolveAssetId(mData->ballTransitionResources.gunResources.wave_Beam);
  mBallTransitions[3] = ResolveAssetId(mData->ballTransitionResources.gunResources.plasma_Beam);

  mMultiplayerBallTransitions[0] =
      ResolveAssetId(mData->ballTransitionResources.multiPlayerGunResources.power_Beam);
  mMultiplayerBallTransitions[1] =
      ResolveAssetId(mData->ballTransitionResources.multiPlayerGunResources.ice_Beam);
  mMultiplayerBallTransitions[2] =
      ResolveAssetId(mData->ballTransitionResources.multiPlayerGunResources.wave_Beam);
  mMultiplayerBallTransitions[3] =
      ResolveAssetId(mData->ballTransitionResources.multiPlayerGunResources.plasma_Beam);

  mCineGun[0] = ResolveAssetId(mData->cinematicResources.power_Beam);
  mCineGun[1] = ResolveAssetId(mData->cinematicResources.ice_Beam);
  mCineGun[2] = ResolveAssetId(mData->cinematicResources.wave_Beam);
  mCineGun[3] = ResolveAssetId(mData->cinematicResources.plasma_Beam);

  mCinematicGrapple = ResolveAssetId(rstl::string_l("CinematicGrapple"));
  mCinematicMoveOutofIntoPlayerDistance = mData->unknown_0x36ad9d19;

  mMorphAlphaSpline = mData->ballTransitionResources.unknown_0xa342c3a6;
  mUnmorphAlphaSpline = mData->ballTransitionResources.unknown_0x15b6840d;
  mMultiplayerMorphAlphaSpline = mData->ballTransitionResources.unknown_0x23fb0e93;
  mMultiplayerUnmorphAlphaSpline = mData->ballTransitionResources.unknown_0x564262f0;
  mMovementControlSpline = mData->ballTransitionResources.movementControl;
}

CAssetId CTweakPlayerRes::GetBallTransitionBeamResId(CPlayerState::EBeamId beam) const {
  if (beam < CPlayerState::kBI_Power || beam > CPlayerState::kBI_Annihilator) {
    return mBallTransitions[0];
  }
  return mBallTransitions[beam];
}

CAssetId CTweakPlayerRes::GetBallTransitionBeamResIdMultiplayer(CPlayerState::EBeamId beam) const {
  if (beam < CPlayerState::kBI_Power || beam > CPlayerState::kBI_Annihilator) {
    return mMultiplayerBallTransitions[0];
  }
  return mMultiplayerBallTransitions[beam];
}

CAssetId CTweakPlayerRes::GetCinematicBeamResId(CPlayerState::EBeamId beam) const {
  if (beam < CPlayerState::kBI_Power || beam > CPlayerState::kBI_Annihilator) {
    return mCineGun[0];
  }
  return mCineGun[beam];
}

CAssetId CTweakPlayerRes::GetCinematicGrappleResId() const { return mCinematicGrapple; }
