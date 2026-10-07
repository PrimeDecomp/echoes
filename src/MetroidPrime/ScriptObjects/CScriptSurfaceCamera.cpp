#include "MetroidPrime/ScriptObjects/CScriptSurfaceCamera.hpp"

#include "Kyoto/Math/CQuaternion.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CScriptCameraSpline.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSurfaceCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"

CScriptSurfaceCamera::CScriptSurfaceCamera(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    uint flags, CCameraSurface* surface, ESurfaceType surfaceType, const CVector3f& playerOffset,
    CMotionSpline::ESplineType targetType, const CMayaSpline& targetControlSpline, bool targetLoops,
    CMotionSpline::ESplineType playerType, bool playerLoops, const CMayaSpline& fovSpline)
: CActor(uid, name, info, 0, xf, CModelData(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mFlags(flags)
, mSurfaceType(surfaceType)
, mSurface(surface)
, mPlayerOffset(playerOffset)
, mPlayerSpline(playerLoops, 1.f, playerType)
, mTargetSpline(targetLoops, 1.f, targetType)
, mTargetControlSpline(targetControlSpline)
, mTargetId(kInvalidUniqueId)
, mFovSpline(fovSpline) {}

CScriptSurfaceCamera::~CScriptSurfaceCamera() {}

void CScriptSurfaceCamera::Think(float dt, CStateManager& mgr) {
  switch (mSurfaceType) {
  case kST_Sphere: {
    CSphereCameraSurface* surface = static_cast< CSphereCameraSurface* >(mSurface.get());
    surface->mSphere = CSphere(GetTranslation(), surface->mSphere.GetRadius());
    break;
  }
  case kST_Plane: {
    CPlaneCameraSurface* surface = static_cast< CPlaneCameraSurface* >(mSurface.get());
    surface->mPlane = CPlane(GetTranslation(), CUnitVector3f(GetTransform().GetForward()));
    surface->mCenter = GetTranslation();
    surface->mAxisA = GetTransform().GetRight();
    surface->mAxisB = GetTransform().GetUp();
    break;
  }
  case kST_Cylinder: {
    CCylinderCameraSurface* surface = static_cast< CCylinderCameraSurface* >(mSurface.get());
    surface->mCylinder = CCylinder(CLine(GetTranslation(), CUnitVector3f(GetTransform().GetUp())),
                                   surface->mCylinder.GetRadius());
    break;
  }
  case kST_SplinePlane: {
    CSplinePlaneCameraSurface* surface = static_cast< CSplinePlaneCameraSurface* >(mSurface.get());
    surface->mPlane = CPlane(GetTranslation(), CUnitVector3f(GetTransform().GetForward()));
    surface->mCenter = GetTranslation();
    surface->mAxisA = GetTransform().GetRight();
    surface->mAxisB = GetTransform().GetUp();
    break;
  }
  case kST_SplineCylinder: {
    CSplineCylinderCameraSurface* surface =
        static_cast< CSplineCylinderCameraSurface* >(mSurface.get());
    static_cast< CCylinderCameraSurface* >(surface)->mCylinder =
        CCylinder(CLine(GetTranslation(), CUnitVector3f(GetTransform().GetUp())),
                  surface->GetCylinder().GetRadius());
    surface->mReferenceDirection = GetTransform().GetForward().AsNormalized();
    break;
  }
  }
}

void CScriptSurfaceCamera::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  if (GetActive() && message == kSM_AreaLoaded) {
    mTargetId = FindConnectedObject(mgr, kSS_CameraTarget, kSM_Attach);
    if (TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mTargetId))) {
      rstl::vector< CVector3f > positions;
      rstl::vector< CQuaternion > orientations;
      ScriptCameraSpline::CollectWaypoints(*this, kSS_CameraTarget, kSM_Attach, positions,
                                           orientations, mgr);
      mTargetSpline.Initialise(positions);
    }
    rstl::vector< CVector3f > positions;
    rstl::vector< CQuaternion > orientations;
    const TUniqueId player = FindConnectedObject(mgr, kSS_CameraPlayer, kSM_Attach);
    if (TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(player))) {
      ScriptCameraSpline::CollectWaypoints(*this, kSS_CameraPlayer, kSM_Attach, positions,
                                           orientations, mgr);
      mPlayerSpline.Initialise(positions);
    }
  }
}

CEntity* LoadSurfaceCamera(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSurfaceCamera sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSurfaceCamera.inc"
  rstl::single_ptr< CCameraSurface > surface;
  const SLdrEditorProperties& editor = sldrThis.editorProperties;
  const CScriptSurfaceCamera::ESurfaceType type =
      static_cast< CScriptSurfaceCamera::ESurfaceType >(sldrThis.surfaceType);
  switch (type) {
  case CScriptSurfaceCamera::kST_Sphere:
    surface = rs_new CSphereCameraSurface(
        CSphere(editor.transform.position, editor.transform.scale.GetX()));
    break;
  case CScriptSurfaceCamera::kST_Plane:
  case CScriptSurfaceCamera::kST_SplinePlane: {
    const CTransform4f xf = LdrToTransform4f(editor);
    const CUnitVector3f normal(xf.GetForward().AsNormalized());
    const CPlane plane(xf.GetTranslation(), normal);
    const CVector3f axisA = xf.GetRight().AsNormalized();
    const CVector3f axisB = xf.GetUp().AsNormalized();
    const float width = 2.f * editor.transform.scale.GetX();
    const float height = 2.f * editor.transform.scale.GetZ();
    if (type == CScriptSurfaceCamera::kST_Plane) {
      surface = rs_new CPlaneCameraSurface(plane, axisA, axisB, xf.GetTranslation(), width, height);
    } else {
      surface = rs_new CSplinePlaneCameraSurface(sldrThis.spline, plane, axisA, axisB,
                                                 xf.GetTranslation(), width, height);
    }
    break;
  }
  case CScriptSurfaceCamera::kST_Cylinder:
  case CScriptSurfaceCamera::kST_SplineCylinder: {
    const CTransform4f xf = LdrToTransform4f(editor);
    const CUnitVector3f axis(xf.GetUp().AsNormalized());
    const CCylinder cylinder(CLine(xf.GetTranslation(), axis), editor.transform.scale.GetX());
    const float height = 2.f * editor.transform.scale.GetZ();
    if (type == CScriptSurfaceCamera::kST_Cylinder) {
      surface = rs_new CCylinderCameraSurface(cylinder, height);
    } else {
      surface = rs_new CSplineCylinderCameraSurface(sldrThis.spline, cylinder,
                                                    xf.GetForward().AsNormalized(), height);
    }
    break;
  }
  default:
    break;
  }
  return rs_new CScriptSurfaceCamera(
      mgr.AllocateUniqueId(), editor.name, LdrToEntityInfo(info, editor), LdrToTransform4f(editor),
      sldrThis.flagsSurfaceCamera, surface.release(), type, sldrThis.playerOffset,
      static_cast< CMotionSpline::ESplineType >(sldrThis.targetSplineType.type),
      sldrThis.targetControlSpline, sldrThis.targetSplineLoops,
      static_cast< CMotionSpline::ESplineType >(sldrThis.playerSplineType.type),
      sldrThis.playerSplineLoops, sldrThis.fOVSpline);
}
