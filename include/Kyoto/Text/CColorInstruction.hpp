#ifndef _CCOLORINSTRUCTION
#define _CCOLORINSTRUCTION

#include "Kyoto/Text/CInstruction.hpp"

#include "Kyoto/Text/CTextColor.hpp"
#include "Kyoto/Text/TextCommon.hpp"

class CColorInstruction : public CInstruction {
public:
  CColorInstruction(EColorType type, const CTextColor& color) : mType(type), mColor(color) {}

  // CInstruction
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;

private:
  EColorType mType;
  CTextColor mColor;
};

#endif // _CCOLORINSTRUCTION
