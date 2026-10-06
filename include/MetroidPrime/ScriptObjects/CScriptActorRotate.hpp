#ifndef _CSCRIPTACTORROTATE
#define _CSCRIPTACTORROTATE

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "rstl/pair.hpp"

class CScriptActorRotate : public CEntity {
public:
  // Guessed flag names from constructor, message and Think behavior.
  enum EFlag {
    kF_AutoStart = 1,
    kF_Loop = 2,
    kF_LocalRotation = 4,
    kF_DurationFromSplines = 8,
    kF_AdvanceTime = 0x10,
    kF_ExternalTime = 0x20,
    kF_AngularVelocity = 0x40,
  };

  CScriptActorRotate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, uint flags,
                     const SLdrSpline& xRotation, const SLdrSpline& yRotation,
                     const SLdrSpline& zRotation, const SLdrSpline& xScale,
                     const SLdrSpline& yScale, const SLdrSpline& zScale, float duration);

  // CEntity
  ~CScriptActorRotate() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  void StartRotation();            // Guessed name.
  void StopRotation();             // Guessed name.
  void SetCurrentTime(float time); // Guessed name.
  void UpdateActors(bool next, CStateManager& mgr);
  void UpdateTargetRotation(CStateManager& mgr);           // Guessed name.
  void SetActorTransforms(const CTransform4f& xf);         // Guessed name.
  void UpdateActorRotations(float dt, CStateManager& mgr); // Guessed name.
  void CheckEnd(CStateManager& mgr);                       // Guessed name.
  // Guessed names.
  void SetExternalTime() { mFlags |= kF_ExternalTime; }
  bool IsPlaying() const { return mPlaying; }
  float GetDuration() const { return mDuration; }

private:
  float mDuration;
  CMayaSpline mXRotation;
  CMayaSpline mYRotation;
  CMayaSpline mZRotation;
  CMayaSpline mXScale;
  CMayaSpline mYScale;
  CMayaSpline mZScale;
  uint mFlags;
  float mCurrentTime;
  CTransform4f mCurrentTransform;
  rstl::vector< rstl::pair< TUniqueId, CTransform4f > > mActors;
  TUniqueId mTargetId;
  bool mPlaying : 1;
};
CHECK_SIZEOF(CScriptActorRotate, 0x20c)

#endif // _CSCRIPTACTORROTATE
