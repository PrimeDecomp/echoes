#ifndef _CCOLOROVERRIDEINSTRUCTION
#define _CCOLOROVERRIDEINSTRUCTION

#include "Kyoto/Text/CInstruction.hpp"
#include "Kyoto/Text/CTextColor.hpp"

class CColorOverrideInstruction : public CInstruction {
public:
  explicit CColorOverrideInstruction(int idx, const CTextColor& color) : mIdx(idx), mColor(color) {}
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;

private:
  int mIdx;
  CTextColor mColor;
};

CHECK_SIZEOF(CColorOverrideInstruction, 0xc)

#endif // _CCOLOROVERRIDEINSTRUCTION
