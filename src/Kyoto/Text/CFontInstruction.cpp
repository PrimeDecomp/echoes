#include "Kyoto/Text/CFontInstruction.hpp"

#include "Kyoto/Text/CFontRenderState.hpp"
#include "Kyoto/Text/CTextRenderBuffer.hpp"

void CFontInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  if (buf != nullptr) {
    buf->AddFontChange(mFont);
  }

  state.SetFont(mFont);
  state.RefreshPalette();
}

void CFontInstruction::GetAssets(rstl::vector< CToken >& assets) const {
  assets.push_back_unsafe(mFont);
  if (mFont.IsLoaded()) {
    assets.push_back_unsafe(TToken< CRasterFont >(mFont)->GetTexture());
  }
}

uint CFontInstruction::GetAssetCount() const { return 2; }

void CFontInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  Invoke(state, buf);
}
