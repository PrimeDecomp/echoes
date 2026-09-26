#ifndef _CGUIWIDGETDRAWPARMS
#define _CGUIWIDGETDRAWPARMS

#include "Kyoto/Math/CVector3f.hpp"

class CGuiWidgetDrawParms {
public:
  CGuiWidgetDrawParms(float alpha, const CVector3f& offset)
  : mAlpha(alpha), mCameraOffset(offset) {}

  float GetAlpha() const { return mAlpha; }
  const CVector3f& GetCameraOffset() const { return mCameraOffset; }

private:
  float mAlpha;
  CVector3f mCameraOffset;
};
CHECK_SIZEOF(CGuiWidgetDrawParms, 0x10)

#endif // _CGUIWIDGETDRAWPARMS
