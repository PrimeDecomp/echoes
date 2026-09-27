#ifndef _CCREDITS
#define _CCREDITS

#include "types.h"

#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CIOWin.hpp"
#include "rstl/list.hpp"
#include "rstl/pair.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/single_ptr.hpp"

class CFilePreload;
class CFinalInput;
class CGuiTextSupport;
class CMoviePlayer;
class CStringTable;
class CTransform4f;

class CCredits : public CIOWin {
public:
  CCredits();

  // CIOWin
  ~CCredits() override;
  EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) override;
  bool GetIsContinueDraw() const override;
  void Draw() const override;

  EMessageReturn Update(float dt, CArchitectureQueue& queue);
  EMessageReturn ProcessUserInput(const CFinalInput& input);
  static void DrawText(CGuiTextSupport& text, const CTransform4f& transform);

private:
  typedef rstl::list< rstl::pair< rstl::ncrc_ptr< CGuiTextSupport >, CVector2i > > TextList;

  int mState;
  TToken< CStringTable > mCreditsTable;
  rstl::single_ptr< CMoviePlayer > mMoviePlayer;
  rstl::string mAudioFile;
  rstl::single_ptr< CFilePreload > mAudioPreload;
  TextList mText;
  float mScrollPosition;
  float mTotalScrollDistance;
  float mScrollSpeed;
  float mTextFadeRemaining;
  float mVideoFadeTime;
  bool mFinished : 1;
  bool mVideoFaded : 1;
  bool mTextFaded : 1;
  bool mFadingIn : 1;
  bool mFadingOut : 1;

  void DrawVideo() const;
  void DrawText() const;
};
CHECK_SIZEOF(CCredits, 0x68)

#endif // _CCREDITS
