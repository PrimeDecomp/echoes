#ifndef _CPOPSTATEINSTRUCTION
#define _CPOPSTATEINSTRUCTION

#include "Kyoto/Text/CInstruction.hpp"

class CPopStateInstruction : public CInstruction {
public:
  CPopStateInstruction() {}
  ~CPopStateInstruction() {}

  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
};

CHECK_SIZEOF(CPopStateInstruction, 0x4)

#endif // _CPOPSTATEINSTRUCTION
