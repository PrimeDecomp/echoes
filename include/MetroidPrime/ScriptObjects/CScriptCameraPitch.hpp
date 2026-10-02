#ifndef _CSCRIPTCAMERAPITCH
#define _CSCRIPTCAMERAPITCH

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CMotionSpline.hpp"
#include "MetroidPrime/CActor.hpp"

// Guessed name; Echoes's spline-driven successor to Prime's CScriptCameraPitchVolume.
class CScriptCameraPitch : public CActor {
public:
  CScriptCameraPitch(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, const CMayaSpline& forwardsPitch,
                     const CMayaSpline& backwardsPitch, bool playerSplineLoops,
                     const CMotionSpline::ESplineType& playerSplineType);

  // CEntity
  ~CScriptCameraPitch() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  float GetPitch(const CTransform4f& playerXf); // Guessed name; returns degrees.

private:
  CMayaSpline mForwardsPitch;
  CMayaSpline mBackwardsPitch;
  CMotionSpline mPlayerSpline;
  uint x224_; // Native allocation has four additional bytes; purpose and type unresolved.
};
CHECK_SIZEOF(CScriptCameraPitch, 0x228)

#endif // _CSCRIPTCAMERAPITCH
