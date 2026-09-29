#include "MetroidPrime/CConsoleOutputWindow.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CArchitectureMessage.hpp"
#include "MetroidPrime/Decode.hpp"

#include <rstl/math.hpp>

CConsoleOutputWindow* CConsoleOutputWindow::mInstance = nullptr;

CConsoleOutputWindow::CConsoleOutputWindow(int lineCount, float duration, float fontScale)
: CIOWin(rstl::string_l("ConsoleOutputWindow"))
, mFont(fontScale)
, mUnresolvedFloat(duration)
, mCharsPerLine(632.f / mFont.CharWidth('0'))
, mLineIndex(0)
, mUnresolvedCounter(0) {
  mLines.reserve(lineCount);
  mLineTimers.reserve(lineCount);
  for (int i = 0; i < lineCount; ++i) {
    mLines.push_back_unsafe(rstl::string("", mCharsPerLine + 1));
    mLineTimers.push_back_unsafe(0.f);
  }
  mInstance = this;
}

CConsoleOutputWindow::~CConsoleOutputWindow() { mInstance = nullptr; }

CIOWin::EMessageReturn CConsoleOutputWindow::OnMessage(const CArchitectureMessage& msg,
                                                       CArchitectureQueue&) {
  switch (msg.GetType()) {
  case kAM_UserInput:
    return kMR_Normal;
  case kAM_TimerTick:
    Update(MakeMsg::GetParmTimerTick(msg).GetReal());
    return kMR_Normal;
  default:
    return kMR_Normal;
  }
}

void CConsoleOutputWindow::Update(float dt) {
  for (int i = 0; i < mLines.size(); ++i) {
    mLineTimers[i] = rstl::max_val(0.f, mLineTimers[i] - dt);
  }
}

void CConsoleOutputWindow::Draw() const {
  int row = 0;
  const int startIndex = (mLineIndex + mLines.size() - 1) % mLines.size();
  int index = startIndex;
  const CColor color = CColor::White();
  CGraphics::SetDepthRange(0.f, 1.f);
  gpRender->SetBlendMode_AlphaBlended();

  if (startIndex >= 0 && startIndex < mLines.size()) {
    const int lineCount = mLines.size();
    do {
      mFont.DrawString(mLines[index].c_str(), 18, row * (mFont.GetFontSize() + 2) + 12, color);
      index = (index + lineCount - 1) % lineCount;
      ++row;
    } while (mLineTimers[index] > 0.f && row < lineCount);
  }
}
