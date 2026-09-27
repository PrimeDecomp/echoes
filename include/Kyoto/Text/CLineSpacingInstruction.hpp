#ifndef _CLINESPACINGINSTRUCTION
#define _CLINESPACINGINSTRUCTION

#include "Kyoto/Text/CInstruction.hpp"

class CLineSpacingInstruction : public CInstruction {
public:
  CLineSpacingInstruction(float spacing) : mSpacing(spacing) {}
  ~CLineSpacingInstruction() {}

  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;

private:
  float mSpacing;
};

CHECK_SIZEOF(CLineSpacingInstruction, 0x8)

#endif // _CLINESPACINGINSTRUCTION
