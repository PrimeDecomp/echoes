#include "Kyoto/Text/CCharacterExtraSpaceInstruction.hpp"

#include "Kyoto/Text/CFontRenderState.hpp"

void CCharacterExtraSpaceInstruction::Invoke(CFontRenderState& state,
                                             CTextRenderBuffer* buffer) const {
  state.GetOptions().SetCharacterExtraSpace(mSpacing);
}

void CCharacterExtraSpaceInstruction::PageInvoke(CFontRenderState& state,
                                                 CTextRenderBuffer* buffer) const {
  Invoke(state, buffer);
}
