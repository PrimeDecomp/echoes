#ifndef _CFONTRENDERSTATE
#define _CFONTRENDERSTATE

#include "Kyoto/Text/CBlockInstruction.hpp"
#include "Kyoto/Text/CDrawStringOptions.hpp"
#include "Kyoto/Text/CSaveableState.hpp"
#include "Kyoto/Text/TextCommon.hpp"
#include "rstl/list.hpp"

class CBlockInstruction;
class CLineInstruction;

class CFontRenderState {
public:
  CFontRenderState();
  void RefreshColor(EColorType type);
  uint ConvertToTextureSpace(const CTextColor& color) const;
  void PushState();
  void PopState();
  void SetColor(EColorType type, const CTextColor& color);
  void RefreshPalette();
  bool IsFinishedLoading() { return mState.IsFinishedLoading(); }
  CDrawStringOptions& GetOptions() { return mState.GetOptions(); }
  void SetFont(const TToken< CRasterFont >& font) { mState.SetFont(font); }
  TToken< CRasterFont >& GetFont() { return mState.GetFont(); }
  rstl::vector< CTextColor >& GetColors() { return mState.GetColors(); }
  rstl::vector< bool >& GetOverride() { return mState.GetOverride(); }
  float GetLineSpacing() const { return mState.GetLineSpacing(); }
  void SetLineSpacing(float spacing) { mState.SetLineSpacing(spacing); }
  int GetLineExtraSpacing() const { return mState.GetLineExtraSpacing(); }
  void SetExtraLineSpace(int spacing) { mState.SetLineExtraSpace(spacing); }
  const CBlockInstruction* GetBlock() const { return mCurBlock; }
  void SetBlock(const CBlockInstruction* block) {
    mCurBlock = const_cast< CBlockInstruction* >(block);
  }
  void SetX(int x) { mCurX = x; }
  int GetX() const { return mCurX; }
  void SetY(int y) { mCurY = y; }
  int GetY() const { return mCurY; }
  void AddY(const int y) { mCurY += y; }
  const CLineInstruction* GetLine() const { return mCurrentLineInst; }
  void SetLine(const CLineInstruction* line) { mCurrentLineInst = line; }
  bool IsFirstWordOnLine() const { return mLineInitialized; }
  void SetFirstWordOnLine(bool v) { mLineInitialized = v; }

  int GetSpacing(const int value) const {
    if (GetBlock()->GetVerticalJustification() == kVerticalJustification_Full) {
      return value;
    }

    return static_cast< int >(static_cast< float >(value) * GetLineSpacing()) +
           GetLineExtraSpacing();
  }

private:
  CSaveableState mState;
  CBlockInstruction* mCurBlock;
  CDrawStringOptions mDrawOpts;
  int mCurX;
  int mCurY;
  const CLineInstruction* mCurrentLineInst;
  uint xe8_;
  uint xec_;
  rstl::vector< uint > xf0_;
  rstl::vector< uchar > x100_;
  bool mLineInitialized;
  rstl::list< CSaveableState > mPushedStates;
};

CHECK_SIZEOF(CFontRenderState, 0x12c)

#endif // _CFONTRENDERSTATE
