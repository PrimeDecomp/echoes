#ifndef _CSCRIPTCAMERASPLINE
#define _CSCRIPTCAMERASPLINE

#include "Kyoto/Math/CGameCameraSpline.hpp"
#include "MetroidPrime/CEntityInfo.hpp"

class CEntity;
class CMaterialFilter;
class CStateManager;
class CTransform4f;

// Guessed name. A camera spline with script-object fallbacks for missing waypoint paths.
class CScriptCameraSpline : public CGameCameraSpline {
public:
  CScriptCameraSpline(float duration, uint flags, const CMayaSpline& positionTimeSpline,
                      const CMayaSpline& lookAtTimeSpline, const CMayaSpline& fovSpline,
                      const CMayaSpline& secondScalarSpline,
                      CMotionSpline::ESplineType positionType,
                      CMotionSpline::ESplineType lookAtType);

  // CGameSpline
  ~CScriptCameraSpline() override;

  CVector3f GetPositionByTime(float time, const CTransform4f& xf, const CStateManager& mgr);
  CQuaternion GetOrientationByTime(float time, const CTransform4f& xf, const CStateManager& mgr);
  using CGameSpline::GetPositionByLength;
  CVector3f GetPositionByLength(float distance, const CTransform4f& xf, const CStateManager& mgr);
  CQuaternion GetOrientationByLength(float positionDistance, float targetDistance,
                                     const CTransform4f& xf, const CStateManager& mgr);

  void SetPositionId(TUniqueId id) { mPositionId = id; }
  void SetTargetId(TUniqueId id) { mTargetId = id; }

private:
  TUniqueId mPositionId;
  TUniqueId mTargetId;
};
CHECK_SIZEOF(CScriptCameraSpline, 0x1b8)

// Guessed helper names/scope. Shared waypoint traversal is implemented in a separate TU.
namespace ScriptCameraSpline {
float ClampLength(const CMotionSpline& spline, const CVector3f& position, bool checkObstructions,
                  const CMaterialFilter& filter, const CStateManager& mgr);
void CollectWaypoints(const CEntity& entity, EScriptObjectState state, EScriptObjectMessage message,
                      rstl::vector< CVector3f >& positions,
                      rstl::vector< CQuaternion >& orientations, CStateManager& mgr);
void Initialise(const CEntity& entity, EScriptObjectState positionState,
                EScriptObjectMessage positionMessage, EScriptObjectState targetState,
                EScriptObjectMessage targetMessage, CStateManager& mgr, CGameSpline& spline);
} // namespace ScriptCameraSpline

#endif // _CSCRIPTCAMERASPLINE
