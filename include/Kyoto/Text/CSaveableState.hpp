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
  TToken<CRasterFont>& GetFont() { return *mFont; }
  rstl::vector<CTextColor>& GetColors() { return mColors; }
  rstl::vector<bool>& GetOverride() { return mColorOverrides; }
  void SetLineSpacing(float spacing) { mLineSpacing = spacing; }
  void SetLineExtraSpace(int spacing) { mExtraLineSpacing = spacing; }

  
private:
  CDrawStringOptions mDrawStringOptions;
  rstl::optional_object< TToken< CRasterFont > > mFont;
  rstl::vector< CTextColor > mColors;
  rstl::vector<bool> mColorOverrides;
  float mLineSpacing;
  int mExtraLineSpacing;
  bool mEnableWordWrap;
  EJustification mJust;
  EVerticalJustification mVjust;
};

#endif // _CSAVEABLESTATE
