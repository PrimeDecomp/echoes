#ifndef _CCHARACTEREXTRASPACEINSTRUCTION
#define _CCHARACTEREXTRASPACEINSTRUCTION

#include "Kyoto/Text/CInstruction.hpp"

// Guessed name
class CCharacterExtraSpaceInstruction : public CInstruction {
public:
  explicit CCharacterExtraSpaceInstruction(int spacing) : mSpacing(spacing) {}

  // CInstruction
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buffer) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buffer) const override;

private:
  int mSpacing;
};

CHECK_SIZEOF(CCharacterExtraSpaceInstruction, 0x8)

#endif // _CCHARACTEREXTRASPACEINSTRUCTION
