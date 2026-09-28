#ifndef _CLINEEXTRASPACINGINSTRUCTION
#define _CLINEEXTRASPACINGINSTRUCTION

#include "Kyoto/Text/CInstruction.hpp"

class CLineExtraSpaceInstruction : public CInstruction {
public:
  CLineExtraSpaceInstruction(int spacing) : mSpacing(spacing) {}
  ~CLineExtraSpaceInstruction() {}

  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;

private:
  int mSpacing;
};

CHECK_SIZEOF(CLineExtraSpaceInstruction, 0x8)

#endif // _CLINEEXTRASPACINGINSTRUCTION
