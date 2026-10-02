#ifndef _CMESSAGESCREEN
#define _CMESSAGESCREEN

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

class CFinalInput;
class CGuiFrame;
class CGuiModel;
class CGuiTextPane;
class CGuiWidget;
class CStringTable;

class CMessageScreen {
public:
  CMessageScreen(CAssetId msg, float time);
  ~CMessageScreen();
  void ProcessControllerInput(const CFinalInput& input);
  bool Update(float dt, float blurAmt);
  void Draw() const;

private:
  TCachedToken< CStringTable > mMsg;
  TCachedToken< CGuiFrame > mMsgScreen;
  CGuiFrame* mLoadedMsgScreen;
  CGuiTextPane* mTextpane_message;
  CGuiWidget* x20_;
  CGuiWidget* mBasewidget_center;
  CGuiWidget* mBasewidget_bottom;
  CGuiModel* mModel_abutton;
  CGuiModel* x30_;
  CGuiModel* mModel_center;
  CGuiModel* mModel_bottom;
  CGuiModel* mModel_bg;
  CGuiModel* x40_;
  CVector3f x44_;
  CVector3f mBottomPos;
  CVector3f x5c_;
  float mVideoBandOffset;
  int mPage;
  float mBlurAmt;
  float mDelayTime;
  bool mExit : 1;
};
CHECK_SIZEOF(CMessageScreen, 0x7c)

#endif
