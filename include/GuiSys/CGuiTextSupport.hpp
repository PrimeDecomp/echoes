#ifndef _CGUITEXTSUPPORT
#define _CGUITEXTSUPPORT

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Text/CTextExecuteBuffer.hpp"
#include "Kyoto/Text/CTextRenderBuffer.hpp"
#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/string.hpp"

class CGuiTextProperties {
public:
  CGuiTextProperties(bool wordWrap, EJustification justification,
                     EVerticalJustification verticalJustification,
                     const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap = nullptr);

private:
  bool mWordWrap;
  EJustification mJustification;
  EVerticalJustification mVerticalJustification;
  const rstl::vector< rstl::pair< CAssetId, CAssetId > >* mTextureMap;
};

class CGuiTextSupport {
public:
  CGuiTextSupport(CAssetId font, int extentX, int extentY, const CGuiTextProperties& properties,
                  const CColor& fontColor, const CColor& outlineColor, const CColor& geometryColor,
                  CSimplePool* pool);
  ~CGuiTextSupport();
  void SetText(const rstl::wstring& text, bool multipage = false);
  void SetText(const rstl::string& text, bool multipage = false);
  void Update(float dt);
  void Render() const;

private:
  CSimplePool* mPool;
  CTextExecuteBuffer mExecuteBuffer;
  rstl::wstring mText;
  float mCurrentTimeMod900;
  CGuiTextProperties mProperties;
  CColor mFontColor;
  CColor mOutlineColor;
  CColor mGeometryColor;
  bool mImageBaseline;
  int mExtraCharacterSpacing;
  int mExtraLineSpacing;
  int mExtentX;
  int mExtentY;
  float mCurrentTime;
  rstl::vector< rstl::pair< float, int > > mPrimitiveStartTimes;
  bool mTypewriterEnabled;
  float mCharacterFadeTime;
  float mCharacterRate;
  CAssetId mFontId;
  mutable rstl::optional_object< CTextRenderBuffer > mRenderBuffer;
  mutable rstl::vector< CToken > mAssets;
  rstl::optional_object< TToken< CRasterFont > > mFont;
  mutable rstl::pair< CVector2i, CVector2i > mBounds;
  mutable rstl::list< CTextRenderBuffer > mPages;
  int mPageCounter;
  bool mMultipage;
};

CHECK_SIZEOF(CGuiTextProperties, 0x10)
CHECK_SIZEOF(CGuiTextSupport, 0xd08)

#endif // _CGUITEXTSUPPORT
