#ifndef _CSAVEABLESTATE
#define _CSAVEABLESTATE

#include "Kyoto/Text/CDrawStringOptions.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#include "Kyoto/Text/CTextColor.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

class CSaveableState {
public:
  CSaveableState();
  bool IsFinishedLoading();

  CDrawStringOptions& GetOptions() { return mDrawStringOptions; }
  TToken< CRasterFont >& GetFont() { return *mFont; }
  void SetFont(const TToken< CRasterFont >& font) { mFont = font; }
  rstl::vector< CTextColor >& GetColors() { return mColors; }
  rstl::vector< bool >& GetOverride() { return mColorOverrides; }
  void SetLineSpacing(float spacing) { mLineSpacing = spacing; }
  void SetLineExtraSpace(int spacing) { mExtraLineSpacing = spacing; }
  float GetLineSpacing() const { return mLineSpacing; }
  int GetLineExtraSpacing() const { return mExtraLineSpacing; }
  void SetWordWrapping(bool wrap) { mEnableWordWrap = wrap; }
  bool IsWordWrapping() const { return mEnableWordWrap; }
  void SetJustification(EJustification justification) { mJust = justification; }
  EJustification GetJustification() const { return mJust; }
  void SetVerticalJustification(EVerticalJustification justification) { mVjust = justification; }
  EVerticalJustification GetVerticalJustification() const { return mVjust; }

private:
  CDrawStringOptions mDrawStringOptions;
  rstl::optional_object< TToken< CRasterFont > > mFont;
  rstl::vector< CTextColor > mColors;
  rstl::vector< bool > mColorOverrides;
  float mLineSpacing;
  int mExtraLineSpacing;
  bool mEnableWordWrap;
  EJustification mJust;
  EVerticalJustification mVjust;
};

CHECK_SIZEOF(CSaveableState, 0x8c)

#endif // _CSAVEABLESTATE
