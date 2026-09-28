#ifndef _CSCRIPTCAMERA
#define _CSCRIPTCAMERA

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/Cameras/CScriptCameraSpline.hpp"

// Echoes stores cinematic settings in this actor, separately from CCinematicCamera.
class CScriptCamera : public CActor {
public:
  // Guessed names. Other flag bits are not yet identified.
  enum EFlags {
    kF_LookAtPlayer = 0x1,
    kF_CinematicPause = 0x100,
    kF_SlowMotion = 0x200,
    kF_VerticalFov = 0x400,
  };

  CScriptCamera(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, float duration, uint flags, uint splineFlags,
                const CMayaSpline& positionTimeSpline, const CMayaSpline& lookAtTimeSpline,
                const CMayaSpline& fovSpline, const CMayaSpline& rollSpline,
                CMotionSpline::ESplineType positionType, CMotionSpline::ESplineType lookAtType,
                const CMayaSpline& slowMotionSpline);

  // CEntity
  ~CScriptCamera() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // Guessed names; these query/update persistent cinematic history.
  bool WasViewed(const CStateManager& mgr) const;
  void MarkViewed(const CStateManager& mgr) const;
  void TranslateSplines(const CVector3f& offset);
  void RotateSplines(const CQuaternion& rotation, const CVector3f& origin);

  // Evaluation changes spline caches, not the scripted settings.
  CScriptCameraSpline& GetSpline() const { return mSpline; }
  CMayaSpline& GetSlowMotionSpline() const { return mSlowMotionSpline; }
  float GetDuration() const { return mDuration; }
  uint GetFlags() const { return mFlags; }
  TUniqueId GetCameraActorId() const { return mCameraActorId; }
  TUniqueId GetTimeKeyframeId() const { return mTimeKeyframeId; }
  bool HasBeenViewed() const { return mHasBeenViewed; }

private:
  mutable CScriptCameraSpline mSpline;
  float mDuration;
  uint mFlags;
  TUniqueId mCameraActorId;
  TUniqueId mTimeKeyframeId;
  mutable CMayaSpline mSlowMotionSpline;
  bool mHasBeenViewed : 1;
};
CHECK_SIZEOF(CScriptCamera, 0x368)

#endif // _CSCRIPTCAMERA
