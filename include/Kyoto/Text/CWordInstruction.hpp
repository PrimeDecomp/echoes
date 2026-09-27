#ifndef _CWORDINSTRUCTION
#define _CWORDINSTRUCTION

#include "Kyoto/Text/CInstruction.hpp"

class CWordInstruction : public CInstruction {
public:
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void InvokeLTR(CFontRenderState& state) const;
};

CHECK_SIZEOF(CWordInstruction, 0x4)

#endif // _CWORDINSTRUCTION
