#ifndef _CGAMESPLINEDESC
#define _CGAMESPLINEDESC

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CMotionSpline.hpp"

// Class name corroborated by the Echoes Wii CScriptEffect constructor export.
class CGameSplineDesc {
public:
  CGameSplineDesc(const SLdrSpline& spline, CMotionSpline::ESplineType type, float duration,
                  bool closedLoop);
  ~CGameSplineDesc() {}

  const SLdrSpline& GetSpline() const { return mSpline; }
  CMotionSpline::ESplineType GetType() const { return mType; }
  float GetDuration() const { return mDuration; }
  bool IsClosedLoop() const { return mClosedLoop; }

private:
  SLdrSpline mSpline;
  CMotionSpline::ESplineType mType;
  float mDuration;
  bool mClosedLoop : 1;
};
CHECK_SIZEOF(CGameSplineDesc, 0x50)

#endif // _CGAMESPLINEDESC
