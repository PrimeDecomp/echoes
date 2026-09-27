#ifndef _CIMAGEINSTRUCTION
#define _CIMAGEINSTRUCTION

#include "Kyoto/Text/CFontImageDef.hpp"
#include "Kyoto/Text/CInstruction.hpp"

class CImageInstruction : public CInstruction {
public:
  explicit CImageInstruction(const CFontImageDef& image) : mImage(image) {}

  // CInstruction
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buffer) const override;
  void GetAssets(rstl::vector< CToken >& assets) const override;
  uint GetAssetCount() const override { return mImage.GetImages().size(); }

private:
  CFontImageDef mImage;
};

CHECK_SIZEOF(CImageInstruction, 0x20)

#endif // _CIMAGEINSTRUCTION
