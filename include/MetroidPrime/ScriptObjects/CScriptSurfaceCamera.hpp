#ifndef _CSCRIPTSURFACECAMERA
#define _CSCRIPTSURFACECAMERA

#include "Kyoto/Math/CMotionSpline.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/Cameras/CCameraSurface.hpp"
#include "rstl/single_ptr.hpp"

// Guessed name; script settings and geometry are separate from the runtime camera.
class CScriptSurfaceCamera : public CActor {
public:
  // Guessed names for the two target-selection flags established by native consumers.
  enum ESurfaceFlags {
    kSF_TargetBallPosition = 0x8,
    kSF_ProjectTargetAlongHintForward = 0x10,
  };

  // Guessed names, supported by the five loader allocation/geometry branches.
  enum ESurfaceType {
    kST_Sphere,
    kST_Plane,
    kST_Cylinder,
    kST_SplinePlane,
    kST_SplineCylinder,
  };

  CScriptSurfaceCamera(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CTransform4f& xf, uint flags, CCameraSurface* surface,
                       ESurfaceType surfaceType, const CVector3f& playerOffset,
                       CMotionSpline::ESplineType targetType,
                       const CMayaSpline& targetControlSpline, bool targetLoops,
                       CMotionSpline::ESplineType playerType, bool playerLoops,
                       const CMayaSpline& fovSpline);

  // CEntity
  ~CScriptSurfaceCamera() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  uint GetFlags() const { return mFlags; }
  ESurfaceType GetSurfaceType() const { return mSurfaceType; }
  CCameraSurface* GetSurface() const { return mSurface.get(); }
  const CVector3f& GetPlayerOffset() const { return mPlayerOffset; }
  const CMotionSpline& GetPlayerSpline() const { return mPlayerSpline; }
  const CMotionSpline& GetTargetSpline() const { return mTargetSpline; }
  CMayaSpline& GetTargetControlSpline() const { return mTargetControlSpline; }
  TUniqueId GetTargetId() const { return mTargetId; }
  CMayaSpline& GetFovSpline() const { return mFovSpline; }

private:
  uint mFlags;
  ESurfaceType mSurfaceType;
  rstl::single_ptr< CCameraSurface > mSurface;
  CVector3f mPlayerOffset;
  CMotionSpline mPlayerSpline;
  CMotionSpline mTargetSpline;
  mutable CMayaSpline mTargetControlSpline;
  TUniqueId mTargetId;
  mutable CMayaSpline mFovSpline;
};
CHECK_SIZEOF(CScriptSurfaceCamera, 0x288)

#endif // _CSCRIPTSURFACECAMERA
