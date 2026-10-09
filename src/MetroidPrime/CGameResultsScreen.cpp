#include "MetroidPrime/CGameResultsScreen.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CDependencyGroupToken.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Streams/CStreamPreloadedToken.hpp"
#include "Kyoto/Text/CGuiTextSupport.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CArchMsgParmReal32.hpp"
#include "MetroidPrime/CArchMsgParmUserInput.hpp"
#include "MetroidPrime/CArchitectureMessage.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/algorithm.hpp"

#include <stdio.h>

static const char* const skMusicFile = "Audio/samusjak_edit.dsp";
static const char* const skScoreFont = "FONT_Deface18BIO";
static const char* const skPromptFont = "FONT_VoxBox24O";
static const char* const skRankFont = "FONT_Deface32BIO";
static const char* const skTitleFont = "FONT_ResultsLabel";
static const char* const skGunLocator = "GUN_LCTR";

static float skPromptZ = -196.f;
static float skTitleScale = 1.f;
static float skPromptScale = 1.f;
static float skFourPlayerTextX = 40.f;
static float skTwoPlayerTextX = 200.f;
static float skRankTextZ = 150.f;
static float skScoreTextZ = 108.f;
static float skDeathsTextZ = 69.f;
static float skFrameScale = 24.f;
static float skLightLinearAttenuation = 0.75f;
static float skPlayerPulseSpeed = 1.5f;
static float skTitlePulseSpeed = 1.5f;
static float skPromptPulseSpeed = 1.5f;

static CColor skTitleColor(0x89d6ffffu);
static CColor skTitleOutlineColor(0x000000ffu);
static CColor skTitlePulseColor(0xff6705ffu);
static CColor skTitlePulseOutlineColor(0x000000ffu);
static CColor skPromptColor(0xff6705ffu);
static CColor skPromptOutlineColor(0x000000ffu);
static CColor skLoserTextColor(0x89d6ffffu);
static CColor skLoserTextOutlineColor(0x000000ffu);
static CColor skWinnerTextColor(0x89d6ffffu);
static CColor skWinnerTextOutlineColor(0x000000ffu);
static CColor skWinnerPulseColor(0xff6705ffu);
static CColor skWinnerPulseOutlineColor(0x000000ffu);
static CColor skWinnerGeometryColor(static_cast< uchar >(255), static_cast< uchar >(255),
                                    static_cast< uchar >(255), static_cast< uchar >(200));
static CColor skLoserGeometryColor(static_cast< uchar >(200), static_cast< uchar >(200),
                                   static_cast< uchar >(200), static_cast< uchar >(200));
static CColor skFramePulseColor(0xff6705ffu);
static CColor skFrameColor(0x89d6ffffu);
static CColor skBackgroundColor(0x3c3b38ffu);
static CColor skAmbientColor(0x151515ffu);
static CColor skLightColor(0xffffffffu);
static CGameResultsScreen::SFragSorter sFragSorter;
static CGameResultsScreen::SCoinSorter sCoinSorter;

static CVector3f skFourPlayerCameraPos(-1.f, 6.f, -0.9f);
static CVector3f skTwoPlayerCameraPos(-0.9f, 6.f, -0.9f);
static CVector3f skLightOffset(1.f, -4.f, 4.f);

float CGameResultsScreen::CalculateFade(float start, float duration, float time) {
  return CMath::Clamp(0.f, (time - start) / duration, 1.f);
}

CGameResultsScreen::SPlayerScore::SPlayerScore(int playerIndex, int coins, int frags, int deaths)
: mPlayerIndex(playerIndex), mCoins(coins), mFrags(frags), mDeaths(deaths) {}

bool CGameResultsScreen::SPlayerScore::IsTiedWith(const SPlayerScore& other, bool coinGame) const {
  if (coinGame) {
    if (mCoins != other.mCoins) {
      return false;
    }
  } else if (mFrags != other.mFrags) {
    return false;
  }
  return mDeaths == other.mDeaths;
}

CGameResultsScreen::SPlayerResults::SPlayerResults(uint playerSelection, int playerIndex,
                                                   CPlayerState::EBeamId beam, int coins, int frags,
                                                   int deaths, int rank, EPlayerResult result,
                                                   bool coinGame, bool fourPlayers,
                                                   CRandom16& random)
: mPlayerSelection(playerSelection)
, mPlayerIndex(playerIndex)
, mBeam(beam)
, mCoins(coins)
, mFrags(frags)
, mDeaths(deaths)
, mRank(rank)
, mResult(result)
, mCoinGame(coinGame)
, mFourPlayers(fourPlayers)
, mEmoteDelay(1.5f)
, mPulseTime(0.f)
, mPulse(0.f)
, mTransform(CTransform4f::Identity())
, mLights(rs_new CActorLights(8, CVector3f::Zero(), 4, 4))
, mSamusModel(rs_new CModelData(
      CAnimRes(gpResourceFactory->GetResourceIdByName("ACS_SamusResultsScreen")->GetId(), 0,
               CVector3f::One(), -1, true)))
, mGunModel(rs_new CModelData(
      CStaticRes(gpTweakPlayerRes->GetBallTransitionBeamResIdMultiplayer(CPlayerState::kBI_Power),
                 CVector3f::One()))) {
  SetupLights();
  const CPASDatabase& pasDatabase = mSamusModel->GetAnimationData()->GetPASDatabase();
  const CPASAnimParmData parms(pas::kAS_Fall, CPASAnimParm::FromEnum(0), CPASAnimParm::FromEnum(0));
  const rstl::pair< float, int > best = pasDatabase.FindBestAnimation(parms, -1);
  mSamusModel->AnimationData()->SetAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), true);
  mSamusModel->EnableLooping(true);
  const float startTime = random.Range(0.f, 0.5f);
  mSamusModel->AdvanceAnimationIgnoreParticles(startTime, random, true);
  if (mResult == kPR_Loser) {
    mEmoteDelay += random.Range(1.f, 1.5f);
  }
}

CGameResultsScreen::SPlayerResults::~SPlayerResults() {}

void CGameResultsScreen::SPlayerResults::LoadTextAndFrame() {
  mFrameModel = rs_new TLockedToken< CModel >(
      gpSimplePool->GetObj(mFourPlayers ? "CMDL_ResultsScreenFrame4" : "CMDL_ResultsScreenFrame2"));

  const CColor& geometryColor = mResult != kPR_Loser ? skWinnerGeometryColor : skLoserGeometryColor;
  const CColor& fontColor = mResult != kPR_Loser ? skWinnerTextColor : skLoserTextColor;
  const CColor& outlineColor =
      mResult != kPR_Loser ? skWinnerTextOutlineColor : skLoserTextOutlineColor;

  {
    const CAssetId font = gpResourceFactory->GetResourceIdByName(skRankFont)->GetId();
    mRankText = rs_new CGuiTextSupport(
        font, 0, 0, CGuiTextProperties(false, kJustification_Right, kVerticalJustification_Center),
        fontColor, outlineColor, geometryColor, gpSimplePool);
    rstl::wstring text(gpStringTable->GetString("Rank"));
    char buf[4];
    sprintf(buf, " %d", mRank);
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l(buf)));
    mRankText->SetText(text);
  }
  {
    const CAssetId font = gpResourceFactory->GetResourceIdByName(skScoreFont)->GetId();
    mScoreText = rs_new CGuiTextSupport(
        font, 0, 0, CGuiTextProperties(false, kJustification_Right, kVerticalJustification_Center),
        fontColor, outlineColor, geometryColor, gpSimplePool);
    rstl::wstring text(gpStringTable->GetString(mCoinGame ? "Coins" : "Frags"));
    char buf[8];
    sprintf(buf, " %d", mCoinGame ? mCoins : mFrags);
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l(buf)));
    mScoreText->SetText(text);
  }
  {
    const CAssetId font = gpResourceFactory->GetResourceIdByName(skScoreFont)->GetId();
    mDeathsText = rs_new CGuiTextSupport(
        font, 0, 0, CGuiTextProperties(false, kJustification_Right, kVerticalJustification_Center),
        fontColor, outlineColor, geometryColor, gpSimplePool);
    rstl::wstring text(gpStringTable->GetString("Deaths"));
    char buf[8];
    sprintf(buf, " %d", mDeaths);
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l(buf)));
    mDeathsText->SetText(text);
  }
}

void CGameResultsScreen::SPlayerResults::Update(float dt, CRandom16& random) {
  mPulseTime += dt * skPlayerPulseSpeed;
  if (mPulseTime > 900.f) {
    mPulseTime -= 900.f;
  }
  mPulse = CMath::AbsF(static_cast< float >(fmod(mPulseTime, 2.0)) - 1.f);

  if (mEmoteDelay > 0.f) {
    mEmoteDelay = rstl::max_val(0.f, mEmoteDelay - dt);
    if (mEmoteDelay == 0.f) {
      const CPASDatabase& pasDatabase = mSamusModel->GetAnimationData()->GetPASDatabase();
      int emote;
      if (mResult == kPR_Winner) {
        emote = 2;
      } else {
        emote = 0;
        if (mResult == kPR_Loser) {
          emote = 3;
        }
      }
      const CPASAnimParmData parms(pas::kAS_Fall, CPASAnimParm::FromEnum(emote),
                                   CPASAnimParm::FromEnum(0));
      const rstl::pair< float, int > best = pasDatabase.FindBestAnimation(parms, -1);
      mSamusModel->AnimationData()->SetAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true),
                                                 false);
      mSamusModel->EnableLooping(true);
    }
  }

  const CVector3f& cameraPos = mFourPlayers ? skFourPlayerCameraPos : skTwoPlayerCameraPos;
  mTransform =
      CTransform4f::LookAt(cameraPos, CVector3f(0.f, 0.f, cameraPos.GetZ()), CVector3f::Up());
  SetupLights();
  mSamusModel->AdvanceAnimationIgnoreParticles(dt, random, true);
  mSamusModel->Touch(CModelData::kWM_Normal, mPlayerSelection);

  const CColor& geometryColor = mResult != kPR_Loser ? skWinnerGeometryColor : skLoserGeometryColor;
  mRankText->SetGeometryColor(geometryColor);
  mScoreText->SetGeometryColor(geometryColor);
  mDeathsText->SetGeometryColor(geometryColor);
  if (mResult != kPR_Loser) {
    const CColor fontColor = CColor::Lerp(skWinnerTextColor, skWinnerPulseColor, mPulse);
    const CColor outlineColor =
        CColor::Lerp(skWinnerTextOutlineColor, skWinnerPulseOutlineColor, mPulse);
    mRankText->SetFontColor(fontColor);
    mScoreText->SetFontColor(fontColor);
    mDeathsText->SetFontColor(fontColor);
    mRankText->SetOutlineColor(outlineColor);
    mScoreText->SetOutlineColor(outlineColor);
    mDeathsText->SetOutlineColor(outlineColor);
  } else {
    mRankText->SetFontColor(skLoserTextColor);
    mScoreText->SetFontColor(skLoserTextColor);
    mDeathsText->SetFontColor(skLoserTextColor);
    mRankText->SetOutlineColor(skLoserTextOutlineColor);
    mScoreText->SetOutlineColor(skLoserTextOutlineColor);
    mDeathsText->SetOutlineColor(skLoserTextOutlineColor);
  }
}

void CGameResultsScreen::SPlayerResults::PreRender() { mSamusModel->AnimationData()->PreRender(); }

void CGameResultsScreen::SPlayerResults::Draw() const {
  bool flip = false;
  if ((mFourPlayers && mPlayerSelection > 1) || (!mFourPlayers && mPlayerSelection == 1)) {
    flip = true;
  }

  gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  const CVector3f frameScale = skFrameScale * CVector3f::One();
  CTransform4f frameXf = CTransform4f::Scale(frameScale);
  if (flip) {
    frameXf = frameXf * CTransform4f::RotateY(CRelAngle::FromRadians(M_PIF));
  }
  gpRender->SetModelMatrix(frameXf);
  const CModel* frame = **mFrameModel;
  CColor frameColor = skFrameColor;
  if (mResult != kPR_Loser) {
    frameColor = CColor::Lerp(frameColor, skFramePulseColor, mPulse);
  }
  frame->Draw(
      CModelFlags::Additive(frameColor.WithAlphaModulatedBy(1.f)).DepthCompareUpdate(false, false));

  gpRender->SetBlendMode_AlphaBlended();
  gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
  const float textX = static_cast< float >(CGraphics::GetViewport().mWidth) -
                      (mFourPlayers ? skFourPlayerTextX : skTwoPlayerTextX);
  CGraphics::SetModelMatrix(CTransform4f::Translate(textX, 0.f, skRankTextZ));
  mRankText->Render();
  CGraphics::SetModelMatrix(CTransform4f::Translate(textX, 0.f, skScoreTextZ));
  mScoreText->Render();
  CGraphics::SetModelMatrix(CTransform4f::Translate(textX, 0.f, skDeathsTextZ));
  mDeathsText->Render();

  gpRender->SetPerspective(30.f, static_cast< float >(CGraphics::GetViewport().mWidth),
                           static_cast< float >(CGraphics::GetViewport().mHeight), 0.2f, 4096.f);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  mSamusModel->Render(CModelData::kWM_Normal, mTransform, mLights.get(),
                      CModelFlags::Normal().UseShaderSet(mPlayerSelection));
  const CTransform4f locatorXf =
      mSamusModel->GetScaledLocatorTransform(rstl::string_l(skGunLocator));
  const CTransform4f gunXf = mTransform * locatorXf;
  mGunModel->Render(CModelData::kWM_Normal, gunXf, mLights.get(),
                    CModelFlags::Normal().UseShaderSet(mBeam));
  CGraphics::DisableAllLights();
  CGraphics::SetAmbientColor(CColor::White());
}

void CGameResultsScreen::SPlayerResults::SetupLights() {
  CLight light(CLight::BuildPoint(mTransform.GetTranslation() + skLightOffset, skLightColor));
  light.SetAttenuation(0.f, skLightLinearAttenuation, 0.f);
  const rstl::vector< CLight > lights(1, light);
  mLights->BuildFakeLightList(lights, skAmbientColor);
}

CGameResultsScreen::CGameResultsScreen(CGameState& state)
: CIOWin(rstl::string_l("GameResultsScreen"))
, mLoadState(kLS_Initial)
, mPhase(kP_Initial)
, mInputEnabled(false)
, mTie(false)
, mDraw(false)
, mFinished(false)
, mTime(0.f)
, mFade(1.f)
, mWhiteFade(1.f)
, mTitlePulseTime(0.f)
, mTitlePulse(0.f)
, mPromptPulseTime(0.f)
, mPromptPulse(0.f)
, mTitleFade(0.f)
, mWinnerFade(0.f)
, mGameMode(state.GetGameMode().GetGameModeType())
, mDependencies(
      rs_new CDependencyGroupToken(gpSimplePool->GetObj("ResultsScreen_DGRP"), *gpSimplePool))
, mMusicPath(rstl::string_l("mem:") + skMusicFile)
, mRandom(static_cast< uint >(CGraphics::GetSecondsMod900())) {
  GatherResults(state);
}

CGameResultsScreen::~CGameResultsScreen() {}

void CGameResultsScreen::GatherResults(CGameState& state) {
  CGameMode& mode = state.GetGameMode();
  mPlayerResults.reserve(mode.GetNumPlayers());
  rstl::vector< SPlayerScore > scores;
  scores.reserve(mode.GetNumPlayers());
  for (int i = 0; i < static_cast< int >(mode.GetNumPlayers()); ++i) {
    const CPlayerState* playerState = state.GetPlayerState(i).GetPtr();
    const int coins = playerState->GetItemAmount(CPlayerState::kIT_CoinCounter);
    const int frags = playerState->GetItemAmount(CPlayerState::kIT_FragCount);
    const int deaths = playerState->GetItemAmount(CPlayerState::kIT_DiedCount);
    scores.push_back_unsafe(SPlayerScore(i, coins, frags, deaths));
  }

  if (mGameMode == 'DTHM') {
    rstl::sort(scores.begin(), scores.end(), sFragSorter);
  } else if (mGameMode == 'COIN') {
    rstl::sort(scores.begin(), scores.end(), sCoinSorter);
  }

  const SPlayerScore& best = scores[0];
  mTie = false;
  mDraw = true;
  bool coinGame = mGameMode == 'COIN';
  for (int i = 0; i < static_cast< int >(mode.GetNumPlayers()); ++i) {
    if (best.IsTiedWith(scores[i], coinGame)) {
      mTie = i > 0;
    } else {
      mDraw = false;
    }
  }

  const bool fourPlayers = mode.GetNumPlayers() > 2;
  int rank = 1;
  for (int i = 0; i < static_cast< int >(mode.GetNumPlayers()); ++i) {
    const SPlayerScore& score = scores[i];
    const CPlayerState* playerState = state.GetPlayerState(score.mPlayerIndex).GetPtr();
    bool tiedWithBest = best.IsTiedWith(score, coinGame) != false;
    if (i > 0 && !score.IsTiedWith(scores[i - 1], coinGame)) {
      ++rank;
    }
    EPlayerResult result;
    if (tiedWithBest) {
      if (mTie || mDraw) {
        result = kPR_Tied;
      } else {
        result = kPR_Winner;
      }
    } else {
      result = kPR_Loser;
    }
    mPlayerResults.push_back_unsafe(
        SPlayerResults(playerState->GetPlayerSelection(), score.mPlayerIndex,
                       playerState->GetCurrentBeam(), score.mCoins, score.mFrags, score.mDeaths,
                       rank, result, mGameMode == 'COIN', fourPlayers, mRandom));
  }
}

void CGameResultsScreen::BuildText() {
  {
    const CAssetId font = gpResourceFactory->GetResourceIdByName(skPromptFont)->GetId();
    mPromptText = rs_new CGuiTextSupport(
        font, 0, 0, CGuiTextProperties(false, kJustification_Center, kVerticalJustification_Center),
        skPromptColor, skPromptOutlineColor, CColor::White(), gpSimplePool);
    mPromptText->SetText(rstl::wstring(gpStringTable->GetString("ResultsPressA")));
  }
  {
    const CAssetId font = gpResourceFactory->GetResourceIdByName(skTitleFont)->GetId();
    mResultsText = rs_new CGuiTextSupport(
        font, 0, 0, CGuiTextProperties(false, kJustification_Center, kVerticalJustification_Center),
        skTitleColor, skTitleOutlineColor, CColor::White(), gpSimplePool);
  }
  const wchar_t* title;
  if (mDraw) {
    title = gpStringTable->GetString("GameResultDraw");
  } else if (mTie) {
    title = gpStringTable->GetString("GameResultTie");
  } else {
    title = gpStringTable->GetString(
        CBasics::Stringize("PlayerWin%d", mPlayerResults[0].mPlayerSelection + 1));
  }
  mResultsText->SetText(rstl::wstring(title));
}

CIOWin::EMessageReturn CGameResultsScreen::OnMessage(const CArchitectureMessage& msg,
                                                     CArchitectureQueue& queue) {
  switch (msg.GetType()) {
  case kAM_UserInput: {
    const CFinalInput& input = MakeMsg::GetParmUserInput(msg).GetUserInput();
    if (mInputEnabled && input.PA()) {
      mInputEnabled = false;
      mPhase = kP_FadeOut;
      mTime = 0.f;
      CSfxManager::SfxStart(0x277b, 0x7f, 0x40);
      CStreamAudioManager::StopSoftwareAudio(CStreamAudioManager::kSC_Default, mMusicPath);
    }
    return kMR_Exit;
  }
  case kAM_TimerTick:
    Update(MakeMsg::GetParmTimerTick(msg).GetReal());
    if (mFinished && CStreamAudioManager::GetSoftwareAudioFileName(
                         CStreamAudioManager::kSC_Default) != mMusicPath) {
      return kMR_RemoveIOWinAndExit;
    }
    return kMR_Exit;
  case kAM_QuitGameplay:
    return kMR_Normal;
  default:
    return kMR_Exit;
  }
}

void CGameResultsScreen::Update(float dt) {
  switch (mLoadState) {
  default:
    return;
  case kLS_Initial:
    mLoadState = kLS_PreloadMusic;
    return;
  case kLS_PreloadMusic:
    if (mMusicPreload.null()) {
      mMusicPreload = rs_new CStreamPreloadedToken(rstl::string_l(skMusicFile));
    }
    mTime += dt;
    if (!mMusicPreload->IsReady()) {
      return;
    }
    CStreamAudioManager::PlaySoftwareAudio(
        CStreamAudioManager::kSC_Default, mMusicPath, 0.f, 1.f,
        static_cast< uchar >(gpTweakGui->GetCompletionScreenVolume()), true);
    mLoadState = kLS_LoadDependencies;
    mTime = 0.f;
    mDependencies->Lock();
  case kLS_LoadDependencies:
    mTime += dt;
    if (!mDependencies->IsLoaded()) {
      return;
    }
    for (int i = 0; i < mPlayerResults.size(); ++i) {
      mPlayerResults[i].LoadTextAndFrame();
    }
    BuildText();
    mLoadState = kLS_Ready;
    mTime = 0.f;
    CGraphics::SetIsBeginSceneClearFb(true);
  case kLS_Ready:
    break;
  }

  mTime += dt;
  if (mTime > 900.f) {
    mTime -= 900.f;
  }
  mTitlePulseTime += dt * skTitlePulseSpeed;
  if (mTitlePulseTime > 900.f) {
    mTitlePulseTime -= 900.f;
  }
  mTitlePulse = CMath::AbsF(static_cast< float >(fmod(mTitlePulseTime, 2.0)) - 1.f);
  if (mInputEnabled) {
    mPromptPulseTime += dt * skPromptPulseSpeed;
    if (mPromptPulseTime > 900.f) {
      mPromptPulseTime -= 900.f;
    }
    mPromptPulse = CMath::AbsF(static_cast< float >(fmod(mPromptPulseTime, 2.0)) - 1.f);
  }
  mPromptText->SetFontColor(skPromptColor);
  mPromptText->SetOutlineColor(skPromptOutlineColor);
  mPromptText->SetGeometryColor(CColor::Lerp(CColor(0xffffff00u), CColor::White(), mPromptPulse));
  mResultsText->SetFontColor(CColor::Lerp(skTitleColor, skTitlePulseColor, mTitlePulse));
  mResultsText->SetOutlineColor(
      CColor::Lerp(skTitleOutlineColor, skTitlePulseOutlineColor, mTitlePulse));
  mWhiteFade = rstl::max_val(0.f, mWhiteFade - dt);
  for (int i = 0; i < mPlayerResults.size(); ++i) {
    SPlayerResults& results = mPlayerResults[i];
    results.Update(dt, mRandom);
    results.PreRender();
  }

  switch (mPhase) {
  case kP_Initial:
    mPhase = kP_Intro;
  case kP_Intro:
    UpdateIntro(dt);
    break;
  case kP_Results:
    if (mTime > 3.f) {
      mInputEnabled = true;
    }
    if (mTime > 60.f) {
      mPhase = kP_FadeOut;
      mTime = 0.f;
      CStreamAudioManager::StopSoftwareAudio(CStreamAudioManager::kSC_Default, mMusicPath);
    }
    break;
  case kP_FadeOut:
    if (mTime > 1.f) {
      mFinished = true;
    } else {
      mFade = CMath::Clamp(0.f, 1.f - mTime, 1.f);
    }
    break;
  }
}

void CGameResultsScreen::UpdateIntro(float dt) {
  const float oldTitleFade = mTitleFade;
  mTitleFade = CalculateFade(0.5f, 0.25f, mTime);
  if (oldTitleFade == 0.f && mTitleFade > 0.f) {
    CSfxManager::SfxStart(0x277c, 0x69, 0x40);
  }
  const float oldWinnerFade = mWinnerFade;
  mWinnerFade = CalculateFade(3.5f, 0.25f, mTime);
  if (oldWinnerFade == 0.f && mWinnerFade > 0.f) {
    CSfxManager::SfxStart(0x2780, 0x5f, 0x40);
  }
  if (mTime > 5.f) {
    mPhase = kP_Results;
    mTime = 0.f;
  }
}

void CGameResultsScreen::Draw() const {
  if (mLoadState < kLS_Ready) {
    return;
  }
  CGraphics::SetViewport(0, 0, CGraphics::GetRenderMode().fbWidth,
                         CGraphics::GetRenderMode().xfbHeight);
  CGraphics::SetScissor(0, 0, CGraphics::GetRenderMode().fbWidth,
                        CGraphics::GetRenderMode().xfbHeight);
  gpRender->SetBlendMode_AlphaBlended();
  gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
  gpRender->SetDepthReadWrite(false, false);
  const CViewport viewport = CGraphics::GetViewport();
  CGraphics::LoadDolphinSpareTexture(viewport.mWidth >> 1, viewport.mHeight >> 1, GX_TF_RGB565,
                                     CGraphics::GetDolphinSpareBuffer(), GX_TEXMAP0);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  const float left = static_cast< float >(viewport.mLeft);
  const float right = static_cast< float >(viewport.mLeft + viewport.mWidth);
  const float top = static_cast< float >(viewport.mTop);
  const float bottom = static_cast< float >(viewport.mTop + viewport.mHeight);
  CGraphics::StreamBegin(kP_TriangleStrip);
  CGraphics::StreamColor(CColor::White());
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(CVector3f(left, 0.f, bottom));
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(CVector3f(left, 0.f, top));
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(CVector3f(right, 0.f, bottom));
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(CVector3f(right, 0.f, top));
  CGraphics::StreamEnd();
  CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen,
                                skBackgroundColor, nullptr, 1.f);

  for (int i = 0; i < mPlayerResults.size(); ++i) {
    const SPlayerResults& results = mPlayerResults[i];
    const bool fourPlayers = mPlayerResults.size() > 2;
    SetViewport(fourPlayers ? results.mPlayerSelection : results.mPlayerIndex, fourPlayers);
    results.Draw();
  }

  CGraphics::SetViewport(0, 0, CGraphics::GetRenderMode().fbWidth,
                         CGraphics::GetRenderMode().xfbHeight);
  CGraphics::SetScissor(0, 0, CGraphics::GetRenderMode().fbWidth,
                        CGraphics::GetRenderMode().xfbHeight);
  gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetBlendMode_AlphaBlended();
  const CTransform4f titleScale = CTransform4f::Scale(skTitleScale);
  gpRender->SetModelMatrix(CTransform4f::Translate(0.f, 0.f, 0.f) * titleScale);
  mResultsText->SetGeometryColor(CColor::White().WithAlphaOf(mWinnerFade));
  mResultsText->Render();
  if (mInputEnabled) {
    const CTransform4f promptScale = CTransform4f::Scale(skPromptScale);
    gpRender->SetModelMatrix(CTransform4f::Translate(0.f, 0.f, skPromptZ) * promptScale);
    mPromptText->Render();
  }
  if (mFade < 1.f) {
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                  CColor::Black().WithAlphaOf(1.f - mFade), nullptr, 1.f);
  }
  if (mWhiteFade > 0.f) {
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                  CColor::White().WithAlphaOf(mWhiteFade), nullptr, 1.f);
  }
}

void CGameResultsScreen::SetViewport(int player, bool fourPlayers) {
  int left = 0;
  int top = 0;
  const int fullWidth = CGraphics::GetRenderMode().fbWidth;
  const int halfWidth = fullWidth / 2;
  const uint halfHeight = CGraphics::GetRenderMode().xfbHeight / 2u;
  int width;
  int height;
  if (fourPlayers) {
    width = halfWidth;
    height = halfHeight;
    switch (player) {
    case 0:
      top = halfHeight;
      left = 0;
      break;
    case 1:
      left = halfWidth;
      top = halfHeight;
      break;
    case 2:
      left = 0;
      top = 0;
      break;
    case 3:
      left = halfWidth;
      top = 0;
      break;
    }
  } else {
    width = fullWidth;
    height = halfHeight;
    left = 0;
    switch (player) {
    case 0:
      top = halfHeight;
      break;
    case 1:
      top = 0;
      break;
    }
  }
  CGraphics::SetViewport(left, top, width, height);
  CGraphics::SetScissor(left, top, width, height);
}
