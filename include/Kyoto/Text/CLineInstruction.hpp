#ifndef _CLINEINSTRUCTION
#define _CLINEINSTRUCTION

#include "Kyoto/Text/CInstruction.hpp"
#include "Kyoto/Text/TextCommon.hpp"

class CLineInstruction : public CInstruction {
public:
  CLineInstruction(int words, int width, int height, EJustification justification,
                   EVerticalJustification verticalJustification, const bool imageBaseline);

  // CInstruction
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buffer) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buffer) const override;
  bool IsLineInstruction() const override { return true; }

  void InvokeTTB(CFontRenderState& state) const;
  void TestLargestFont(int width, int height, int baseline);
  void TestLargestImage(int width, int height, int baseline);
  int GetHeight() const;
  int GetBaseline() const;
  int GetWordCount() const { return mWordCount; }
  int GetWidth() const { return mCurrentX; }
  int GetY() const { return mCurrentY; }
  void IncWords() { ++mWordCount; }
  void DecWords() { --mWordCount; }
  void AddWidth(int width) { mCurrentX += width; }
  void SubWidth(int width) { mCurrentX -= width; }
  void AddHeight(int height) { mCurrentY += height; }
  void SetHeight(int height) { mCurrentY = height; }
  void SetJustification(EJustification justification) { mJustification = justification; }
  void SetVerticalJustification(EVerticalJustification justification) {
    mVerticalJustification = justification;
  }

private:
  int mWordCount;
  int mCurrentX;
  int mCurrentY;
  int mLargestFontHeight;
  int mLargestFontWidth;
  int mLargestFontBaseline;
  int mLargestImageHeight;
  int mLargestImageWidth;
  int mLargestImageBaseline;
  EJustification mJustification;
  EVerticalJustification mVerticalJustification;
  bool mImageBaseline;
};

CHECK_SIZEOF(CLineInstruction, 0x34)

#endif // _CLINEINSTRUCTION
