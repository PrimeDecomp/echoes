#include "Kyoto/Text/CGuiTextSupport.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#include "Kyoto/Text/CTextParser.hpp"
#include "Kyoto/Text/ScreenText.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/math.hpp"
#include <math.h>

CGuiTextProperties::CGuiTextProperties(
    bool wordWrap, EJustification justification, EVerticalJustification verticalJustification,
    const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap)
: mWordWrap(wordWrap)
, mJustification(justification)
, mVerticalJustification(verticalJustification)
, mTextureMap(textureMap) {}

CGuiTextSupport::CGuiTextSupport(CAssetId font, int extentX, int extentY,
                                 const CGuiTextProperties& properties, const CColor& fontColor,
                                 const CColor& outlineColor, const CColor& geometryColor,
                                 CSimplePool* pool)
: mPool(pool)
, mCurrentTimeMod900(0.f)
, mProperties(properties)
, mFontColor(fontColor)
, mOutlineColor(outlineColor)
, mGeometryColor(geometryColor)
, mImageBaseline(false)
, mExtraCharacterSpacing(0)
, mExtraLineSpacing(0)
, mExtentX(extentX)
, mExtentY(extentY)
, mCurrentTime(0.f)
, mTypewriterEnabled(false)
, mCharacterFadeTime(0.1f)
, mCharacterRate(10.f)
, mFontId(font)
, mBounds(CVector2i(0, 0), CVector2i(0, 0))
, mPageCounter(0)
, mMultipage(false) {
  if (mFontId != kInvalidAssetId) {
    mFont = TToken< CRasterFont >(mPool->GetObj(SObjectTag('FONT', mFontId)));
    mFont->Lock();
  }
}

CGuiTextSupport::~CGuiTextSupport() {}

bool CGuiTextSupport::GetIsTextSupportFinishedLoading() const {
  CheckAndRebuildRenderBuffer();
  return _GetIsTextSupportFinishedLoading();
}

bool CGuiTextSupport::_GetIsTextSupportFinishedLoading() const {
  for (int i = 0; i < mAssets.size(); ++i) {
    if (!mAssets[i].HasLock()) {
      mAssets[i].Lock();
    }
    if (!mAssets[i].IsLoaded()) {
      return false;
    }
  }
  if (mFont.valid()) {
    TToken< CRasterFont > font = *mFont;
    if (!font.IsLoaded()) {
      return false;
    }
    return font->IsFinishedLoading();
  }
  if (!mFont.valid() && mAssets.size() == 0) {
    return false;
  }
  return true;
}

void CGuiTextSupport::SetText(const rstl::string& text, bool multipage) {
  const rstl::wstring wtext = CStringExtras::ConvertToUNICODE(text);
  SetText(wtext, multipage);
}

void CGuiTextSupport::SetText(const rstl::wstring& text, bool multipage) {
  if (mText != text) {
    mPrimitiveStartTimes.clear();
    mCurrentTime = 0.f;
    mText = text;
    ClearRenderBuffer();
    mMultipage = multipage;
    mPageCounter = 0;
  }
}

void CGuiTextSupport::AddText(const rstl::wstring& text) {
  if (mRenderBuffer) {
    mPrimitiveStartTimes.reserve(mPrimitiveStartTimes.size() + 1);
    mPrimitiveStartTimes.push_back_unsafe(
        rstl::pair< float, int >(rstl::max_val(GetCurrentAnimationOverAge(), mCurrentTime),
                                 mRenderBuffer->GetNumPrimitives()));
  }
  mText.append(text);
  ClearRenderBuffer();
}

void CGuiTextSupport::SetWordWrap(bool wordWrap) {
  if (wordWrap != mProperties.mWordWrap) {
    mProperties.mWordWrap = wordWrap;
    ClearRenderBuffer();
  }
}

void CGuiTextSupport::SetImageBaseline(bool baseline) {
  if (mImageBaseline != baseline) {
    mImageBaseline = baseline;
    ClearRenderBuffer();
  }
}

void CGuiTextSupport::SetFontColor(const CColor& color) {
  if (!(mFontColor == color)) {
    ClearRenderBuffer();
    mFontColor = color;
  }
}

void CGuiTextSupport::SetOutlineColor(const CColor& color) {
  if (!(mOutlineColor == color)) {
    ClearRenderBuffer();
    mOutlineColor = color;
  }
}

void CGuiTextSupport::SetGeometryColor(const CColor& color) { mGeometryColor = color; }

void CGuiTextSupport::SetControlTXTRMap(
    const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap) {
  if (mProperties.mTextureMap != textureMap) {
    mProperties.mTextureMap = textureMap;
    ClearRenderBuffer();
  }
}

void CGuiTextSupport::Render() const {
  CheckAndRebuildRenderBuffer();
  const CTransform4f oldModel = CGraphics::GetModelMatrix();
  CGraphics::SetModelMatrix(oldModel * CTransform4f::Scale(CVector3f(1.f, 1.f, -1.f)));
  if (const CTextRenderBuffer* buffer = GetCurrentPageRenderBuffer()) {
    buffer->Render(mGeometryColor, mCurrentTimeMod900);
  }
  CGraphics::SetModelMatrix(oldModel);
}

void CGuiTextSupport::CheckAndRebuildTextBuffer() const {
  mExecuteBuffer.Clear();
  mExecuteBuffer.AddWordWrapping(mProperties.mWordWrap);
  mExecuteBuffer.BeginBlock(0, 0, mExtentX, mExtentY, mImageBaseline, kTD_Horizontal,
                            mProperties.mJustification, mProperties.mVerticalJustification);
  mExecuteBuffer.AddColor(kCT_Main, CTextColor(mFontColor.GetRedu8(), mFontColor.GetGreenu8(),
                                               mFontColor.GetBlueu8(), mFontColor.GetAlphau8()));
  mExecuteBuffer.AddColor(kCT_Outline,
                          CTextColor(mOutlineColor.GetRedu8(), mOutlineColor.GetGreenu8(),
                                     mOutlineColor.GetBlueu8(), mOutlineColor.GetAlphau8()));
  mExecuteBuffer.AddCharacterExtraSpace(mExtraCharacterSpacing);
  mExecuteBuffer.AddLineExtraSpace(mExtraLineSpacing);
  rstl::wstring text;
  if (mFontId != kInvalidAssetId) {
    text =
        CStringExtras::ConvertToUNICODE(rstl::string(CBasics::Stringize("&font=%8.8X;", mFontId)));
  }
  text.append(mText);
  CTextParser parser(*mPool);
  parser.ParseText(mExecuteBuffer, text.c_str(), text.size(), mProperties.mTextureMap);
  mExecuteBuffer.EndBlock();
}

bool CGuiTextSupport::CheckAndRebuildRenderBuffer() const {
  if ((!mMultipage && !mRenderBuffer) || (mMultipage && mPages.empty())) {
    CheckAndRebuildTextBuffer();
    mAssets = mExecuteBuffer.GetAssets();
    if (_GetIsTextSupportFinishedLoading()) {
      CheckAndRebuildTextBuffer();
      if (mMultipage) {
        mPages = mExecuteBuffer.BuildRenderBufferPages(CVector2i(mExtentX, mExtentY));
      } else {
        mRenderBuffer = mExecuteBuffer.BuildRenderBuffer();
        mBounds = mRenderBuffer->GetTextBounds();
      }
      mExecuteBuffer.Clear();
    } else {
      return false;
    }
    const_cast< CGuiTextSupport* >(this)->Update(0.f);
  }
  return true;
}

void CGuiTextSupport::ClearRenderBuffer() {
  mRenderBuffer.clear();
  mPages = rstl::list< CTextRenderBuffer >();
}

void CGuiTextSupport::Update(float dt) {
  if (mTypewriterEnabled) {
    CTextRenderBuffer* buffer = GetCurrentPageRenderBuffer();
    if (buffer != nullptr) {
      float characterStartTime = 0.f;
      for (int i = 0; i < buffer->GetNumPrimitives(); ++i) {
        for (int j = 0; j < mPrimitiveStartTimes.size(); ++j) {
          const rstl::pair< float, int >& start = mPrimitiveStartTimes[j];
          if (start.second < i) {
            continue;
          }
          if (start.second != i) {
            break;
          }
          characterStartTime = start.first;
          break;
        }
        CTextRenderBuffer::Primitive primitive = buffer->GetPrimitive(i);
        float alpha = rstl::min_val(
            1.f, rstl::max_val(0.f, (mCurrentTime - characterStartTime) / mCharacterFadeTime));
        characterStartTime += 1.f / mCharacterRate;
        CColor color(primitive.mColor);
        color.SetAlpha(alpha);
        primitive.mColor = color.GetColor_u32();
        buffer->SetPrimitive(primitive, i);
      }
    }
    mCurrentTime += dt;
  }
  mCurrentTimeMod900 = fmod(mCurrentTimeMod900 + dt, 900.0);
}

void CGuiTextSupport::SetTypeWriteEffectOptions(bool enable, float fadeTime, float rate) {
  mTypewriterEnabled = enable;
  mCharacterFadeTime = rstl::max_val(fadeTime, 0.0001f);
  mCharacterRate = rstl::max_val(rate, 1.f);
}

float CGuiTextSupport::GetTotalAnimationTime() const {
  if (const CTextRenderBuffer* buffer = GetCurrentPageRenderBuffer()) {
    if (mTypewriterEnabled) {
      return buffer->GetNumPrimitives() / mCharacterRate;
    }
  }
  return 0.f;
}

float CGuiTextSupport::GetNumCharactersPrinted() const {
  if (const CTextRenderBuffer* buffer = GetCurrentPageRenderBuffer()) {
    if (mTypewriterEnabled) {
      return rstl::min_val(float(buffer->GetNumPrimitives()), mCurrentTime * mCharacterRate);
    }
  }
  return 0.f;
}

float CGuiTextSupport::GetCurrentAnimationOverAge() const {
  float time = 0.f;
  if (const CTextRenderBuffer* buffer = GetCurrentPageRenderBuffer()) {
    if (mTypewriterEnabled) {
      if (!mPrimitiveStartTimes.empty()) {
        const rstl::pair< float, int >& last = mPrimitiveStartTimes.back();
        time = rstl::max_val(time, (buffer->GetNumPrimitives() - last.second) / mCharacterRate +
                                       last.first);
      } else {
        time = rstl::max_val(time, buffer->GetNumPrimitives() / mCharacterRate);
      }
    }
  }
  return time;
}

int CGuiTextSupport::GetTotalPageCount() {
  if (CheckAndRebuildRenderBuffer()) {
    return mPages.size();
  }
  return -1;
}

void CGuiTextSupport::SetPage(int page) {
  mPageCounter = page;
  mPrimitiveStartTimes.clear();
  mCurrentTime = 0.f;
}

CTextRenderBuffer* CGuiTextSupport::GetCurrentPageRenderBuffer() const {
  if (mRenderBuffer && !mMultipage) {
    return mRenderBuffer.get_ptr();
  }
  if (mMultipage && mPages.size() > mPageCounter) {
    int i = 0;
    for (rstl::list< CTextRenderBuffer >::iterator it = mPages.begin();; ++it, ++i) {
      if (i == mPageCounter) {
        return &*it;
      }
    }
  }
  return nullptr;
}

const rstl::pair< CVector2i, CVector2i >& CGuiTextSupport::GetBounds() {
  CheckAndRebuildRenderBuffer();
  return mBounds;
}

void ScreenText::DrawExecuteBuffer(const CTextExecuteBuffer& buffer) {
  const CViewport viewport = CGraphics::GetViewport();
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetOrtho(static_cast< float >(viewport.mLeft),
                      static_cast< float >(viewport.mLeft + viewport.mWidth),
                      static_cast< float >(viewport.mTop + viewport.mHeight),
                      static_cast< float >(viewport.mTop), -4096.f, 4096.f);
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  CGraphics::SetCullMode(kCM_None);
  CGraphics::SetDepthWriteMode(true, kE_Always, false);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);

  const CTransform4f transform =
      CTransform4f::FromColumns(CVector3f::Right(), CVector3f::Forward(), CVector3f::Down(),
                                CVector3f(0.f, 0.f, static_cast< float >(viewport.mHeight)));
  CGraphics::SetModelMatrix(transform);

  buffer.BuildRenderBuffer().Render(CColor::White(), 0.f);
  CGraphics::SetCullMode(kCM_Front);
}

void ScreenText::DrawString(const rstl::string& text, int x, int y,
                            const TToken< CRasterFont >& font) {
  const CViewport viewport = CGraphics::GetViewport();
  CTextExecuteBuffer buffer;
  buffer.AddWordWrapping(true);
  buffer.BeginBlock(x, y, viewport.mWidth - x, viewport.mHeight - y, false, kTD_Horizontal,
                    kJustification_Left, kVerticalJustification_Center);

  {
    const TToken< CRasterFont > fontCopy = font;
    buffer.AddFont(fontCopy);
  }
  buffer.AddString(CStringExtras::ConvertToUNICODE(text));
  buffer.EndBlock();
  DrawExecuteBuffer(buffer);
}
