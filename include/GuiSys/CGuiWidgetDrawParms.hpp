#ifndef _CGUIWIDGETDRAWPARMS
#define _CGUIWIDGETDRAWPARMS

#include "Kyoto/Math/CVector3f.hpp"

class CGuiWidgetDrawParms {
public:
  static CGuiWidgetDrawParms sDefaultDrawParms;
  static const CGuiWidgetDrawParms& Default() { return sDefaultDrawParms; }

  explicit CGuiWidgetDrawParms(float alpha) : mAlpha(alpha), mCameraOffset(0.f, 0.f, 0.f) {}

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
