#ifndef _CGAMECAMERASPLINE
#define _CGAMECAMERASPLINE

#include "Kyoto/Math/CGameSpline.hpp"

// Guessed name.
class CGameCameraSpline : public CGameSpline {
public:
  // CGameSpline
  ~CGameCameraSpline() override;

private:
  CMayaSpline mFovSpline;
  CMayaSpline x170_; // The second scalar channel's meaning is unresolved.
};
CHECK_SIZEOF(CGameCameraSpline, 0x1b4)

#endif // _CGAMECAMERASPLINE
