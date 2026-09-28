#ifndef _CPUSHSTATEINSTRUCTION
#define _CPUSHSTATEINSTRUCTION

#include "Kyoto/Text/CInstruction.hpp"

class CPushStateInstruction : public CInstruction {
public:
  CPushStateInstruction() {}
  ~CPushStateInstruction() {}

  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
};

CHECK_SIZEOF(CPushStateInstruction, 0x4)

#endif // _CPUSHSTATEINSTRUCTION
