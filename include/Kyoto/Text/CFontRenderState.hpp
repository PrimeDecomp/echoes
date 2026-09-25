#ifndef _CFONTRENDERSTATE
#define _CFONTRENDERSTATE

#include "Kyoto/Text/CDrawStringOptions.hpp"
#include "Kyoto/Text/CSaveableState.hpp"
#include "Kyoto/Text/TextCommon.hpp"

class CBlockInstruction;
class CLineInstruction;

class CFontRenderState {
public:
  uint ConvertToTextureSpace(const CTextColor& color) const;
  void PushState();
  void PopState();
  void SetColor(EColorType type, const CTextColor& color);
  void RefreshPalette();
  CDrawStringOptions& GetOptions() { return mState.GetOptions(); }
  TToken< CRasterFont >& GetFont() { return mState.GetFont(); }
  rstl::vector< CTextColor >& GetColors() { return mState.GetColors(); }
  rstl::vector< bool >& GetOverride() { return mState.GetOverride(); }
  void SetLineSpacing(float spacing) { mState.SetLineSpacing(spacing); }
  void SetExtraLineSpace(int spacing) { mState.SetLineExtraSpace(spacing); }
  const CBlockInstruction* GetBlock() const { return mCurBlock; }
  void SetBlock(const CBlockInstruction* block) {
    mCurBlock = const_cast< CBlockInstruction* >(block);
  }
  void SetX(int x) { mCurX = x; }
  int GetX() const { return mCurX; }
  void SetY(int y) { mCurY = y; }
  int GetY() const { return mCurY; }
  const CLineInstruction* GetLine() const { return mCurrentLineInst; }
  bool IsFirstWordOnLine() const { return mLineInitialized; }
  void SetFirstWordOnLine(bool v) { mLineInitialized = v; }

private:
  CSaveableState mState;
  CBlockInstruction* mCurBlock;
  CDrawStringOptions mDrawOpts;
  int mCurX;
  int mCurY;
  const CLineInstruction* mCurrentLineInst;
  uint xe0_;
  uint xe4_;
  rstl::vector< uint > xe8_;
  rstl::vector< uchar > xf8_;
  bool mLineInitialized;
  rstl::list< CSaveableState > mPushedStates;
};

#endif // _CFONTRENDERSTATE
