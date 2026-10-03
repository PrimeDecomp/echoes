#include "MetroidPrime/CGameResultsScreen.hpp"

#include "Kyoto/CDependencyGroupToken.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Streams/CFilePreload.hpp"
#include "Kyoto/Text/CGuiTextSupport.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"

// Nonfunctional runtime scaffold: loading, rendering and state-machine bodies remain unimplemented.

float CGameResultsScreen::CalculateFade(float start, float duration, float time) {
  const float fade = (time - start) / duration;
  if (fade < 0.f) {
    return 0.f;
  }
  if (fade > 1.f) {
    return 1.f;
  }
  return fade;
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

CGameResultsScreen::SPlayerResults::SPlayerResults(
    uint playerSelection, int playerIndex, CPlayerState::EBeamId beam, int coins, int frags,
    int deaths, int rank, EPlayerResult result, bool coinGame, bool fourPlayers,
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
, mTransform(CTransform4f::Identity()) {
  // TODO: allocate the native lights and Samus/gun models, then select the idle animation.
}

CGameResultsScreen::SPlayerResults::~SPlayerResults() {}

void CGameResultsScreen::SPlayerResults::LoadTextAndFrame() {}

void CGameResultsScreen::SPlayerResults::Update(float dt, CRandom16& random) {}

void CGameResultsScreen::SPlayerResults::PreRender() {}

void CGameResultsScreen::SPlayerResults::Draw() const {}

void CGameResultsScreen::SPlayerResults::SetupLights() {}

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
, mMusicPath(rstl::string_l("mem:Audio/samusjak_edit.dsp"))
, mRandom(static_cast< uint >(CGraphics::GetSecondsMod900())) {
  // TODO: create the native dependency-group token before gathering the player results.
  GatherResults(state);
}

CGameResultsScreen::~CGameResultsScreen() {}

void CGameResultsScreen::GatherResults(CGameState& state) {}

void CGameResultsScreen::BuildText() {}

CIOWin::EMessageReturn CGameResultsScreen::OnMessage(const CArchitectureMessage& msg,
                                                    CArchitectureQueue& queue) {
  return kMR_Normal;
}

void CGameResultsScreen::Update(float dt) {}

void CGameResultsScreen::UpdateIntro() {}

void CGameResultsScreen::Draw() const {}

void CGameResultsScreen::SetViewport(int playerSelection, bool fourPlayers) {}
