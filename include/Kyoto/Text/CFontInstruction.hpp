#ifndef _CFONTINSTRUCTION
#define _CFONTINSTRUCTION

#include "Kyoto/TToken.hpp"
#include "Kyoto/Text/CInstruction.hpp"
#include "Kyoto/Text/CRasterFont.hpp"

class CFontInstruction : public CInstruction {
public:
  explicit CFontInstruction(const TToken< CRasterFont >& font) : mFont(font) { mFont.Lock(); }

  // CInstruction
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buffer) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buffer) const override;
  void GetAssets(rstl::vector< CToken >& assets) const override;
  uint GetAssetCount() const override;

private:
  TToken< CRasterFont > mFont;
};

CHECK_SIZEOF(CFontInstruction, 0xc)

#endif // _CFONTINSTRUCTION
