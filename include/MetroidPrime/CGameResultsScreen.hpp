#ifndef _CGAMERESULTSSCREEN
#define _CGAMERESULTSSCREEN

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CIOWin.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CActorLights;
class CDependencyGroupToken;
class CFilePreload;
class CGameState;
class CGuiTextSupport;
class CModel;
class CModelData;

// Guessed type, member and private method names, based on the native GameResultsScreen role.
class CGameResultsScreen : public CIOWin {
public:
  explicit CGameResultsScreen(CGameState& state);

  // CIOWin
  ~CGameResultsScreen() override;
  EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) override;
  void Draw() const override;

  enum EGameMode { kGM_Deathmatch = 'DTHM', kGM_Coin = 'COIN' };
  enum ELoadState { kLS_Initial, kLS_PreloadMusic, kLS_LoadDependencies, kLS_Ready = 4 };
  enum EPhase { kP_Initial, kP_Intro, kP_Results, kP_FadeOut };
  enum EPlayerResult { kPR_Winner, kPR_Loser, kPR_Tied };

  struct SPlayerScore {
    SPlayerScore(int playerIndex, int coins, int frags, int deaths);
    bool IsTiedWith(const SPlayerScore& other, bool coinGame) const;

    int mPlayerIndex;
    int mCoins;
    int mFrags;
    int mDeaths;
  };

  struct SPlayerResults {
    SPlayerResults(uint playerSelection, int playerIndex, CPlayerState::EBeamId beam, int coins,
                   int frags, int deaths, int rank, EPlayerResult result, bool coinGame,
                   bool fourPlayers, CRandom16& random);
    ~SPlayerResults();

    void LoadTextAndFrame();
    void Update(float dt, CRandom16& random);
    void PreRender();
    void Draw() const;
    void SetupLights();

    uint mPlayerSelection;
    int mPlayerIndex;
    CPlayerState::EBeamId mBeam;
    int mCoins;
    int mFrags;
    int mDeaths;
    int mRank;
    EPlayerResult mResult;
    bool mCoinGame;
    bool mFourPlayers;
    float mEmoteDelay;
    float mPulseTime;
    float mPulse;
    CTransform4f mTransform;
    rstl::auto_ptr< CActorLights > mLights;
    rstl::auto_ptr< CModelData > mSamusModel;
    rstl::auto_ptr< CModelData > mGunModel;
    rstl::auto_ptr< TLockedToken< CModel > > mFrameModel;
    rstl::auto_ptr< CGuiTextSupport > mRankText;
    rstl::auto_ptr< CGuiTextSupport > mScoreText;
    rstl::auto_ptr< CGuiTextSupport > mDeathsText;
  };
  typedef char SPlayerScoreSizeCheck[sizeof(SPlayerScore) == 0x10 ? 1 : -1];
  typedef char SPlayerResultsSizeCheck[sizeof(SPlayerResults) == 0x98 ? 1 : -1];

private:
  void GatherResults(CGameState& state);
  void BuildText();
  void Update(float dt);
  void UpdateIntro();
  static void SetViewport(int playerSelection, bool fourPlayers);
  static float CalculateFade(float start, float duration, float time);

  ELoadState mLoadState;
  EPhase mPhase;
  bool mInputEnabled : 1;
  bool mTie : 1;
  bool mDraw : 1;
  bool mFinished : 1;
  float mTime;
  float mFade;
  float mWhiteFade;
  float mTitlePulseTime;
  float mTitlePulse;
  float mPromptPulseTime;
  float mPromptPulse;
  float mTitleFade;
  float mWinnerFade;
  int mGameMode;
  rstl::auto_ptr< CDependencyGroupToken > mDependencies;
  rstl::auto_ptr< CGuiTextSupport > mResultsText;
  rstl::auto_ptr< CGuiTextSupport > mPromptText;
  rstl::string mMusicPath;
  rstl::auto_ptr< CFilePreload > mMusicPreload;
  CRandom16 mRandom;
  rstl::vector< SPlayerResults > mPlayerResults;
};
CHECK_SIZEOF(CGameResultsScreen, 0x8c)

#endif // _CGAMERESULTSSCREEN
