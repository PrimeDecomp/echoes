#include "MetroidPrime/CGameResultsScreen.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CDependencyGroupToken.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Streams/CFilePreload.hpp"
#include "Kyoto/Text/CGuiTextSupport.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CArchMsgParmUserInput.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/algorithm.hpp"

static const char* const skMusicFile = "Audio/samusjak_edit.dsp";
static const char* const skFontDeface18 = "FONT_Deface18BIO";
static const char* const skFontVoxBox24 = "FONT_VoxBox24O";
static const char* const skFontDeface32 = "FONT_Deface32BIO";
static const char* const skFontResultsLabel = "FONT_ResultsLabel";
static const char* const skGunLocator = "GUN_LCTR";

static const CColor skTitleFontColor(0x89D6FFFF);
static const CColor skTitleOutlineColor(0x000000FF);
static const CColor skTitlePulseFontColor(0xFF6705FF);
static const CColor skTitlePulseOutlineColor(0x000000FF);
static const CColor skPromptFontColor(0xFF6705FF);
static const CColor skPromptOutlineColor(0x000000FF);
static const CColor skLoserFontColor(0x89D6FFFF);
static const CColor skLoserOutlineColor(0x000000FF);
static const CColor skScoreFontColor(0x89D6FFFF);
static const CColor skScoreOutlineColor(0x000000FF);
static const CColor skScorePulseFontColor(0xFF6705FF);
static const CColor skScorePulseOutlineColor(0x000000FF);
static const CColor skScoreGeometryColor(uchar(255), uchar(255), uchar(255), uchar(200));
static const CColor skLoserGeometryColor(uchar(200), uchar(200), uchar(200), uchar(200));
static const CColor skFramePulseColor(0xFF6705FF);
static const CColor skFrameColor(0x89D6FFFF);
static const CColor skBackdropColor(0x3C3B38FF);
static const CColor skLightAmbientColor(0x151515FF);
static const CColor skLightColor(0xFFFFFFFF);

static const CVector3f skFourPlayerCameraPos(-1.f, 6.f, -0.9f);
static const CVector3f skTwoPlayerCameraPos(-0.9f, 6.f, -0.9f);
static const CVector3f skLightOffset(1.f, -4.f, 4.f);

static float skPromptZ = -196.f;
static float skTitleScale = 1.f;
static float skPromptScale = 1.f;
static float skFourPlayerTextInset = 40.f;
static float skTwoPlayerTextInset = 200.f;
static float skRankTextZ = 150.f;
static float skScoreTextZ = 108.f;
static float skDeathsTextZ = 69.f;
static float skFrameScale = 24.f;
static float skLightAttenuation = 0.75f;
static float skResultPulseRate = 1.5f;
static float skTitlePulseRate = 1.5f;
static float skPromptPulseRate = 1.5f;

namespace {
struct SFragScoreLess {
  bool operator()(const CGameResultsScreen::SPlayerScore& a,
                  const CGameResultsScreen::SPlayerScore& b) const {
    if (a.mFrags == b.mFrags) {
      return a.mDeaths < b.mDeaths;
    }
    return a.mFrags > b.mFrags;
  }
};

struct SCoinScoreLess {
  bool operator()(const CGameResultsScreen::SPlayerScore& a,
                  const CGameResultsScreen::SPlayerScore& b) const {
    if (a.mCoins == b.mCoins) {
      return a.mDeaths < b.mDeaths;
    }
    return a.mCoins > b.mCoins;
  }
};
} // namespace

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

  const CPASDatabase& pasDatabase = mSamusModel->AnimationData()->GetPASDatabase();
  const CPASAnimParmData parms(pas::kAS_Fall, CPASAnimParm::FromEnum(0), CPASAnimParm::FromEnum(0));
  const rstl::pair< float, int > anim = pasDatabase.FindBestAnimation(parms, -1);
  mSamusModel->AnimationData()->SetAnimation(CAnimPlaybackParms(anim.second, -1, 1.f, true), true);
  mSamusModel->EnableLooping(true);
  const float advanceTime = random.Range(0.f, 0.5f);
  mSamusModel->AdvanceAnimationIgnoreParticles(advanceTime, random, true);
  if (mResult == kPR_Loser) {
    mEmoteDelay += random.Range(1.f, 1.5f);
  }
}

CGameResultsScreen::SPlayerResults::~SPlayerResults() {}

void CGameResultsScreen::SPlayerResults::LoadTextAndFrame() {
  mFrameModel = rs_new TLockedToken< CModel >(
      gpSimplePool->GetObj(mFourPlayers ? "CMDL_ResultsScreenFrame4" : "CMDL_ResultsScreenFrame2"));

  const CColor* geometryColor = &skLoserGeometryColor;
  if (mResult != kPR_Loser) {
    geometryColor = &skScoreGeometryColor;
  }
  const CColor* fontColor = &skLoserFontColor;
  if (mResult != kPR_Loser) {
    fontColor = &skScoreFontColor;
  }
  const CColor* outlineColor = &skLoserOutlineColor;
  if (mResult != kPR_Loser) {
    outlineColor = &skScoreOutlineColor;
  }
  char buffer[16];

  const CAssetId rankFont = gpResourceFactory->GetResourceIdByName(skFontDeface32)->GetId();
  mRankText = rs_new CGuiTextSupport(
      rankFont, 0, 0,
      CGuiTextProperties(false, kJustification_Right, kVerticalJustification_Center), *fontColor,
      *outlineColor, *geometryColor, gpSimplePool);
  rstl::wstring rankText(gpStringTable->GetString("Rank"));
  sprintf(buffer, " %d", mRank);
  rankText.append(CStringExtras::ConvertToUNICODE(rstl::string_l(buffer)));
  mRankText->SetText(rankText);

  const CAssetId scoreFont = gpResourceFactory->GetResourceIdByName(skFontDeface18)->GetId();
  mScoreText = rs_new CGuiTextSupport(
      scoreFont, 0, 0,
      CGuiTextProperties(false, kJustification_Right, kVerticalJustification_Center), *fontColor,
      *outlineColor, *geometryColor, gpSimplePool);
  rstl::wstring scoreText(gpStringTable->GetString(mCoinGame ? "Coins" : "Frags"));
  sprintf(buffer, " %d", mCoinGame ? mCoins : mFrags);
  scoreText.append(CStringExtras::ConvertToUNICODE(rstl::string_l(buffer)));
  mScoreText->SetText(scoreText);

  const CAssetId deathsFont = gpResourceFactory->GetResourceIdByName(skFontDeface18)->GetId();
  mDeathsText = rs_new CGuiTextSupport(
      deathsFont, 0, 0,
      CGuiTextProperties(false, kJustification_Right, kVerticalJustification_Center), *fontColor,
      *outlineColor, *geometryColor, gpSimplePool);
  rstl::wstring deathsText(gpStringTable->GetString("Deaths"));
  sprintf(buffer, " %d", mDeaths);
  deathsText.append(CStringExtras::ConvertToUNICODE(rstl::string_l(buffer)));
  mDeathsText->SetText(deathsText);
}

void CGameResultsScreen::SPlayerResults::Update(float dt, CRandom16& random) {
  mPulseTime += dt * skResultPulseRate;
  if (mPulseTime > 900.f) {
    mPulseTime -= 900.f;
  }
  mPulse = fabs(static_cast< float >(fmod(mPulseTime, 2.0)) - 1.f);

  if (mEmoteDelay > 0.f) {
    mEmoteDelay = rstl::max_val(0.f, mEmoteDelay - dt);
    if (mEmoteDelay == 0.f) {
      const CPASDatabase& pasDatabase = mSamusModel->AnimationData()->GetPASDatabase();
      int emote = 0;
      if (mResult == kPR_Winner) {
        emote = 2;
      } else if (mResult == kPR_Loser) {
        emote = 3;
      }
      const CPASAnimParmData parms(pas::kAS_Fall, CPASAnimParm::FromEnum(emote),
                                   CPASAnimParm::FromEnum(0));
      const rstl::pair< float, int > anim = pasDatabase.FindBestAnimation(parms, -1);
      mSamusModel->AnimationData()->SetAnimation(CAnimPlaybackParms(anim.second, -1, 1.f, true),
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

  const CColor* geometryColor = &skLoserGeometryColor;
  if (mResult != kPR_Loser) {
    geometryColor = &skScoreGeometryColor;
  }
  mRankText->SetGeometryColor(*geometryColor);
  mScoreText->SetGeometryColor(*geometryColor);
  mDeathsText->SetGeometryColor(*geometryColor);

  if (mResult != kPR_Loser) {
    const CColor fontColor = CColor::Lerp(skScoreFontColor, skScorePulseFontColor, mPulse);
    const CColor outlineColor = CColor::Lerp(skScoreOutlineColor, skScorePulseOutlineColor, mPulse);
    mRankText->SetFontColor(fontColor);
    mScoreText->SetFontColor(fontColor);
    mDeathsText->SetFontColor(fontColor);
    mRankText->SetOutlineColor(outlineColor);
    mScoreText->SetOutlineColor(outlineColor);
    mDeathsText->SetOutlineColor(outlineColor);
  } else {
    mRankText->SetFontColor(skLoserFontColor);
    mScoreText->SetFontColor(skLoserFontColor);
    mDeathsText->SetFontColor(skLoserFontColor);
    mRankText->SetOutlineColor(skLoserOutlineColor);
    mScoreText->SetOutlineColor(skLoserOutlineColor);
    mDeathsText->SetOutlineColor(skLoserOutlineColor);
  }
}

void CGameResultsScreen::SPlayerResults::PreRender() { mSamusModel->AnimationData()->PreRender(); }

void CGameResultsScreen::SPlayerResults::Draw() const {
  bool mirrored = false;
  if ((mFourPlayers && mPlayerSelection > 1) || (!mFourPlayers && mPlayerSelection == 1)) {
    mirrored = true;
  }

  gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  CTransform4f frameXf = CTransform4f::Scale(CVector3f::One() * skFrameScale);
  if (mirrored) {
    frameXf = frameXf * CTransform4f::RotateY(CRelAngle::FromRadians(M_PIF));
  }
  gpRender->SetModelMatrix(frameXf);

  CModel* frame = **mFrameModel;
  CColor frameColor = skFrameColor;
  if (mResult != kPR_Loser) {
    frameColor = CColor::Lerp(frameColor, skFramePulseColor, mPulse);
  }
  frame->Draw(CModelFlags::Additive(frameColor.WithAlphaModulatedBy(1.f)));
  gpRender->SetBlendMode_AlphaBlended();

  gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
  const float textX = CGraphics::GetViewport().mWidth -
                      (mFourPlayers ? skFourPlayerTextInset : skTwoPlayerTextInset);
  CGraphics::SetModelMatrix(CTransform4f::Translate(textX, 0.f, skRankTextZ));
  mRankText->Render();
  CGraphics::SetModelMatrix(CTransform4f::Translate(textX, 0.f, skScoreTextZ));
  mScoreText->Render();
  CGraphics::SetModelMatrix(CTransform4f::Translate(textX, 0.f, skDeathsTextZ));
  mDeathsText->Render();

  gpRender->SetPerspective(30.f, CGraphics::GetViewport().mWidth, CGraphics::GetViewport().mHeight,
                           0.2f, 4096.f);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());

  mSamusModel->Render(CModelData::kWM_Normal, mTransform, mLights.get(),
                      CModelFlags::Normal().UseShaderSet(mPlayerSelection));
  const CTransform4f gunTransform =
      mTransform * mSamusModel->GetScaledLocatorTransform(rstl::string_l(skGunLocator));
  mGunModel->Render(CModelData::kWM_Normal, gunTransform, mLights.get(),
                    CModelFlags::Normal().UseShaderSet(mPlayerSelection));
  CGraphics::DisableAllLights();
  CGraphics::SetAmbientColor(CColor::White());
}

void CGameResultsScreen::SPlayerResults::SetupLights() {
  CLight light = CLight::BuildPoint(mTransform.GetTranslation() + skLightOffset, skLightColor);
  light.SetAttenuation(0.f, skLightAttenuation, 0.f);
  rstl::vector< CLight > lights(1, light);
  mLights->BuildFakeLightList(lights, skLightAmbientColor);
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
  CGameMode& gameMode = state.GetGameMode();
  mPlayerResults.reserve(gameMode.GetNumPlayers());

  rstl::vector< SPlayerScore > scores;
  scores.reserve(gameMode.GetNumPlayers());
  for (int i = 0; i < int(gameMode.GetNumPlayers()); ++i) {
    CPlayerState* playerState = state.GetPlayerState(i).GetPtr();
    const int coins = playerState->GetItemAmount(CPlayerState::kIT_CoinCounter, true);
    const int frags = playerState->GetItemAmount(CPlayerState::kIT_FragCount, true);
    const int deaths = playerState->GetItemAmount(CPlayerState::kIT_DiedCount, true);
    scores.push_back_unsafe(SPlayerScore(i, coins, frags, deaths));
  }

  if (mGameMode == kGM_Deathmatch) {
    rstl::sort(scores.begin(), scores.end(), SFragScoreLess());
  } else if (mGameMode == kGM_Coin) {
    rstl::sort(scores.begin(), scores.end(), SCoinScoreLess());
  }

  mTie = false;
  mDraw = true;
  const SPlayerScore& top = scores[0];
  const bool coinGame = mGameMode == kGM_Coin;
  for (int i = 0; i < int(gameMode.GetNumPlayers()); ++i) {
    if (top.IsTiedWith(scores[i], coinGame)) {
      mTie = i > 0;
    } else {
      mDraw = false;
    }
  }

  const bool fourPlayers = gameMode.GetNumPlayers() > 2;
  int rank = 1;
  for (int i = 0; i < int(gameMode.GetNumPlayers()); ++i) {
    const SPlayerScore& score = scores[i];
    CPlayerState* playerState = state.GetPlayerState(score.mPlayerIndex).GetPtr();
    const bool tiedWithFirst = top.IsTiedWith(score, coinGame);
    if (i > 0 && !score.IsTiedWith(scores[i - 1], coinGame)) {
      ++rank;
    }
    EPlayerResult result;
    if (tiedWithFirst) {
      result = (mTie || mDraw) ? kPR_Tied : kPR_Winner;
    } else {
      result = kPR_Loser;
    }
    SPlayerResults results(playerState->GetPlayerSelection(), score.mPlayerIndex,
                           playerState->GetCurrentBeam(), score.mCoins, score.mFrags, score.mDeaths,
                           rank, result, mGameMode == kGM_Coin, fourPlayers, mRandom);
    mPlayerResults.push_back_unsafe(results);
  }
}

void CGameResultsScreen::BuildText() {
  const CAssetId promptFont = gpResourceFactory->GetResourceIdByName(skFontVoxBox24)->GetId();
  mPromptText = rs_new CGuiTextSupport(
      promptFont, 0, 0,
      CGuiTextProperties(false, kJustification_Center, kVerticalJustification_Center),
      skPromptFontColor, skPromptOutlineColor, CColor::White(), gpSimplePool);
  mPromptText->SetText(rstl::wstring(gpStringTable->GetString("ResultsPressA")));

  const CAssetId titleFont = gpResourceFactory->GetResourceIdByName(skFontResultsLabel)->GetId();
  mResultsText = rs_new CGuiTextSupport(
      titleFont, 0, 0,
      CGuiTextProperties(false, kJustification_Center, kVerticalJustification_Center),
      skTitleFontColor, skTitleOutlineColor, CColor::White(), gpSimplePool);
  const char* titleKey;
  if (mDraw) {
    titleKey = "GameResultDraw";
  } else if (mTie) {
    titleKey = "GameResultTie";
  } else {
    titleKey = CBasics::Stringize("PlayerWin%d", mPlayerResults[0].mPlayerSelection + 1);
  }
  mResultsText->SetText(rstl::wstring(gpStringTable->GetString(titleKey)));
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
      CSfxManager::SfxStart(0x277b, 127, 64, CSfxManager::kAllAreas, false, false,
                            CSfxManager::kMedPriority);
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
  case kLS_Initial:
    mLoadState = kLS_PreloadMusic;
    return;
  case kLS_PreloadMusic:
    if (mMusicPreload.null()) {
      mMusicPreload = rs_new CFilePreload(rstl::string_l(skMusicFile));
    }
    mTime += dt;
    if (!mMusicPreload->IsReady()) {
      return;
    }
    CStreamAudioManager::PlaySoftwareAudio(CStreamAudioManager::kSC_Default, mMusicPath, 0.f, 1.f,
                                           gpTweakGui->GetCompletionScreenVolume(), true);
    mLoadState = kLS_LoadDependencies;
    mTime = 0.f;
    mDependencies->Lock();
    // fall through
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
    // fall through
  case kLS_Ready:
    break;
  default:
    return;
  }

  mTime += dt;
  if (mTime > 900.f) {
    mTime -= 900.f;
  }

  mTitlePulseTime += dt * skTitlePulseRate;
  if (mTitlePulseTime > 900.f) {
    mTitlePulseTime -= 900.f;
  }
  mTitlePulse = fabs(static_cast< float >(fmod(mTitlePulseTime, 2.0)) - 1.f);

  if (mInputEnabled) {
    mPromptPulseTime += dt * skPromptPulseRate;
    if (mPromptPulseTime > 900.f) {
      mPromptPulseTime -= 900.f;
    }
    mPromptPulse = fabs(static_cast< float >(fmod(mPromptPulseTime, 2.0)) - 1.f);
  }

  mPromptText->SetFontColor(skPromptFontColor);
  mPromptText->SetOutlineColor(skPromptOutlineColor);
  mPromptText->SetGeometryColor(CColor::Lerp(CColor(0xFFFFFF00), CColor::White(), mPromptPulse));
  mResultsText->SetFontColor(CColor::Lerp(skTitleFontColor, skTitlePulseFontColor, mTitlePulse));
  mResultsText->SetOutlineColor(
      CColor::Lerp(skTitleOutlineColor, skTitlePulseOutlineColor, mTitlePulse));

  mWhiteFade = rstl::max_val(0.f, mWhiteFade - dt);

  for (int i = 0; i < mPlayerResults.size(); ++i) {
    mPlayerResults[i].Update(dt, mRandom);
    mPlayerResults[i].PreRender();
  }

  switch (mPhase) {
  case kP_Initial:
    mPhase = kP_Intro;
    // fall through
  case kP_Intro:
    UpdateIntro();
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

void CGameResultsScreen::UpdateIntro() {
  const float prevTitleFade = mTitleFade;
  mTitleFade = CalculateFade(0.5f, 0.25f, mTime);
  if (prevTitleFade == 0.f && mTitleFade > 0.f) {
    CSfxManager::SfxStart(0x277c, 0x69, 0x40, CSfxManager::kAllAreas, false, false,
                          CSfxManager::kMedPriority);
  }
  const float prevWinnerFade = mWinnerFade;
  mWinnerFade = CalculateFade(3.5f, 0.25f, mTime);
  if (prevWinnerFade == 0.f && mWinnerFade > 0.f) {
    CSfxManager::SfxStart(0x2780, 0x5f, 0x40, CSfxManager::kAllAreas, false, false,
                          CSfxManager::kMedPriority);
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

  const GXRenderModeObj& renderMode = CGraphics::GetRenderMode();
  CGraphics::SetViewport(0, 0, renderMode.fbWidth, renderMode.xfbHeight);
  CGraphics::SetScissor(0, 0, renderMode.fbWidth, renderMode.xfbHeight);
  gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
  gpRender->SetDepthReadWrite(false, false);

  const CViewport viewport = CGraphics::GetViewport();
  CGraphics::LoadDolphinSpareTexture(viewport.mWidth / 2, viewport.mHeight / 2, GX_TF_RGB565,
                                     CGraphics::mpSpareBuffer, GX_TEXMAP0);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  const float left = viewport.mLeft;
  const float right = viewport.mLeft + viewport.mWidth;
  const float top = viewport.mTop;
  const float bottom = viewport.mTop + viewport.mHeight;
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
                                skBackdropColor, nullptr, 1.f);

  for (int i = 0; i < mPlayerResults.size(); ++i) {
    const SPlayerResults& results = mPlayerResults[i];
    const bool fourPlayers = mPlayerResults.size() > 2;
    SetViewport(fourPlayers ? results.mPlayerSelection : results.mPlayerIndex, fourPlayers);
    results.Draw();
  }

  CGraphics::SetViewport(0, 0, renderMode.fbWidth, renderMode.xfbHeight);
  CGraphics::SetScissor(0, 0, renderMode.fbWidth, renderMode.xfbHeight);
  gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetBlendMode_AlphaBlended();

  CGraphics::SetModelMatrix(CTransform4f::Translate(0.f, 0.f, 0.f) *
                            CTransform4f::Scale(skTitleScale));
  mResultsText->SetGeometryColor(CColor::White().WithAlphaOf(mWinnerFade));
  mResultsText->Render();

  if (mInputEnabled) {
    CGraphics::SetModelMatrix(CTransform4f::Translate(0.f, 0.f, skPromptZ) *
                              CTransform4f::Scale(skPromptScale));
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

void CGameResultsScreen::SetViewport(int playerSelection, bool fourPlayers) {
  const int width = CGraphics::GetRenderMode().fbWidth;
  const uint height = CGraphics::GetRenderMode().xfbHeight;
  const int halfWidth = width / 2;
  const int halfHeight = height / 2;
  int x = 0;
  int y = 0;
  int viewportWidth;
  int viewportHeight;
  if (fourPlayers) {
    viewportWidth = halfWidth;
    viewportHeight = halfHeight;
    switch (playerSelection) {
    case 0:
      x = 0;
      y = halfHeight;
      break;
    case 1:
      x = halfWidth;
      y = halfHeight;
      break;
    case 2:
      x = 0;
      y = 0;
      break;
    case 3:
      x = halfWidth;
      y = 0;
      break;
    }
  } else {
    viewportWidth = width;
    viewportHeight = halfHeight;
    x = 0;
    switch (playerSelection) {
    case 0:
      y = halfHeight;
      break;
    case 1:
      y = 0;
      break;
    }
  }
  CGraphics::SetViewport(x, y, viewportWidth, viewportHeight);
  CGraphics::SetScissor(x, y, viewportWidth, viewportHeight);
}
