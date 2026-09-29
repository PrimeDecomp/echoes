#ifndef _CCONSOLEOUTPUTWINDOW
#define _CCONSOLEOUTPUTWINDOW

#include "MetroidPrime/CIOWin.hpp"

#include "Kyoto/Text/CFont.hpp"

#include <rstl/vector.hpp>

class CConsoleOutputWindow : public CIOWin {
  static CConsoleOutputWindow* mInstance;

public:
  CConsoleOutputWindow(int lineCount, float duration, float fontScale);

  // CIOWin
  ~CConsoleOutputWindow() override;
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  void Draw() const override;

  void Update(float);

private:
  CFont mFont;
  float mUnresolvedFloat;
  rstl::vector< rstl::string > mLines;
  rstl::vector< float > mLineTimers;
  int mCharsPerLine;
  int mLineIndex;
  int mUnresolvedCounter;
};
CHECK_SIZEOF(CConsoleOutputWindow, 0x4c)

#endif // _CCONSOLEOUTPUTWINDOW
