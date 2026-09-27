#ifndef _CREMOVECOLOROVERRIDEINSTRUCTION
#define _CREMOVECOLOROVERRIDEINSTRUCTION

#include "Kyoto/Text/CInstruction.hpp"

class CRemoveColorOverrideInstruction : public CInstruction {
public:
  explicit CRemoveColorOverrideInstruction(int idx) : mIdx(idx) {}
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;

private:
  int mIdx;
};

CHECK_SIZEOF(CRemoveColorOverrideInstruction, 0x8)

#endif // _CREMOVECOLOROVERRIDEINSTRUCTION
