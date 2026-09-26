#ifndef _CGUITEXTPANE
#define _CGUITEXTPANE

#include "GuiSys/CGuiPane.hpp"
#include "GuiSys/CGuiTextSupport.hpp"

class CGuiTextPane : public CGuiPane {
public:
  // Guessed name
  struct SFontInfo {
    int mExtentX;
    int mExtentY;
    CColor mFontColor;
    CColor mOutlineColor;
    CAssetId mFontId;
  };

  CGuiTextPane(const CGuiWidgetParms& parms, CSimplePool* pool, float width, float height,
               const CVector3f& scaleCenter, const CGuiTextProperties& properties,
               const SFontInfo& font, const SFontInfo& alternateFont, bool scaleToViewport);

  // CGuiObject
  ~CGuiTextPane() override;

  // CGuiWidget
  FourCC GetWidgetTypeID() const override;
  int GetWidgetTypeIndex() const override;
  void Update(float dt) override;
  void Draw(const CGuiWidgetDrawParms& parms) const override;
  void Initialize() override;

  // CGuiPane
  void ScaleDimensions(const CVector3f& scale) override;
  void SetDimensions(const CVector2f& dim, bool initVBO) override;

  virtual rstl::vector< SObjectTag > GetFontAssets() const;

  CGuiTextSupport& TextSupport() { return mTextSupport; }
  const CGuiTextSupport& GetTextSupport() const { return mTextSupport; }

private:
  mutable CGuiTextSupport mTextSupport;
  SFontInfo mFontInfo;          // Guessed name
  SFontInfo mAlternateFontInfo; // Guessed name
  bool mDrawShadow;
  bool mScaleToViewport; // Guessed name
};
NESTED_CHECK_SIZEOF(CGuiTextPane, SFontInfo, 0x14)
CHECK_SIZEOF(CGuiTextPane, 0xe08)

#endif // _CGUITEXTPANE
