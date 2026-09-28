#ifndef _CSCRIPTCAMERAHINT
#define _CSCRIPTCAMERAHINT

#include "MetroidPrime/Cameras/CCameraOverrideInfo.hpp"
#include "MetroidPrime/ScriptObjects/CScriptHint.hpp"

class CScriptCameraHint : public CScriptHint {
public:
  CScriptCameraHint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                    const CTransform4f& xf, int priority, float timer,
                    CBallCamera::EBallCameraBehaviour behaviour, uint flags, uint overrideFlags,
                    float minDist, float maxDist, float backwardsDist,
                    const CVector3f& lookAtOffset, const CVector3f& worldOffset, float fov,
                    float attitudeRange, float azimuthRange, float anglePerSecond, float elevation,
                    float interpolateOnTime, float interpolateOffTime, float controlInterpDur,
                    int interpolateOnType, int interpolationMode, int interpolateOffType,
                    int acrossAreas);

  // CEntity
  ~CScriptCameraHint() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // Guessed name. Updates the connected script path camera's three position splines.
  void SetPathCameraPosition(const CVector3f& position, CStateManager& mgr) const;

  const CCameraOverrideInfo& GetInfo() const { return mOverrideInfo; }
  TUniqueId GetDelegatedCameraId() const { return mDelegatedCameraId; }
  TUniqueId GetCameraTargetId() const { return mCameraTargetId; }
  const CTransform4f& GetOriginalTransform() const { return mOrigXf; }

private:
  CCameraOverrideInfo mOverrideInfo;
  TUniqueId mDelegatedCameraId;
  TUniqueId mCameraTargetId;
  CTransform4f mOrigXf;
};
CHECK_SIZEOF(CScriptCameraHint, 0x240)

#endif // _CSCRIPTCAMERAHINT
