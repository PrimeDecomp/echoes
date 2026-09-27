#ifndef _CPLAYMOVIE
#define _CPLAYMOVIE

#include "types.h"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CDvdKeepAlive.hpp"
#include "MetroidPrime/CIOWin.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CMoviePlayer;
class CQuitGameScreen;
class CFilePreload;
class CStringTable;
class CRasterFont;
class CGuiTextSupport;
class CFinalInput;

class CPlayMovie : public CIOWin {
public:
  enum EWhichMovie {
    kWM_EndingPart2,
    kWM_EndingPart2B,
    kWM_EndingPart3,
    kWM_Credits,
    kWM_SpecialEnding,
    kWM_Results,
    kWM_LoseGame
  };

  explicit CPlayMovie(int which);

  ~CPlayMovie() override;
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  bool GetIsContinueDraw() const override;
  void Draw() const override;

private:
  enum EState { kS_LoadText, kS_LoadMovies, kS_LoadAudio, kS_Playing };

  void SetMovieIndex(int index);
  EMessageReturn ProcessUserInput(const CFinalInput& input);
  void UpdateText(float dt);
  void DrawText() const;
  void DrawVideo() const;

  EState mState;
  int mWhich;
  rstl::reserved_vector< rstl::auto_ptr< CMoviePlayer >, 3 > mMovies;
  CMoviePlayer* mMoviePlayer;
  int mMovieIndex;
  rstl::single_ptr< CQuitGameScreen > mQuitScreen;
  rstl::string mAudioFile;
  rstl::single_ptr< CFilePreload > mAudioPreload;
  TToken< CStringTable > mCompletionScreenStrings;
  TToken< CRasterFont > mLargeFont;
  rstl::single_ptr< CGuiTextSupport > mTitleText;
  rstl::single_ptr< CGuiTextSupport > mResultsText;
  rstl::single_ptr< CGuiTextSupport > mUnlockText;
  rstl::single_ptr< CGuiTextSupport > mContinueText;
  float mTextDelay;
  float mContinueDelay;
  float mContinueAlpha;
  float mPulseTime;
  float mPrintedCharacters;
  CColor mPulseStartColor;
  CColor mPulseEndColor;
  CDvdKeepAlive mDvdKeepAlive;
  bool mHideVideo : 1;
  bool mFinished : 1;
  bool mExit : 1;
  bool mContinuePressed : 1;
};
CHECK_SIZEOF(CPlayMovie, 0xd4)

#endif // _CPLAYMOVIE
