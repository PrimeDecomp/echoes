#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiWidget.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CGuiWidget* CGuiCamera::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp, uint version) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  EProjection proj = static_cast< EProjection >(in.ReadInt32());
  CGuiCamera* camera = nullptr;
  if (proj == kProjection_Perspective) {
    const float fov = in.ReadFloat();
    const float aspect = in.ReadFloat();
    const float znear = in.ReadFloat();
    const float zfar = in.ReadFloat();
    camera = rs_new CGuiCamera(parms, fov, aspect, znear, zfar);
  } else if (proj == kProjection_Orthographic) {
    const float left = in.ReadFloat();
    const float right = in.ReadFloat();
    const float top = in.ReadFloat();
    const float bottom = in.ReadFloat();
    const float znear = in.ReadFloat();
    const float zfar = in.ReadFloat();
    camera = rs_new CGuiCamera(parms, left, right, top, bottom, znear, zfar);
  }

  frame->SetFrameCamera(camera);
  camera->ParseBaseInfo(frame, in, parms, version);
  return camera;
}

CGuiCamera::CGuiCamera(const CGuiWidgetParms& parms, float fov, float aspect, float znear,
                       float zfar)
: CGuiWidget(parms) {
  mProjection = kProjection_Perspective;
  CVector3f(1.f, 0.f, 0.f).Normalize();
  mCameraParms.mPerspective.mFov = fov;
  mCameraParms.mPerspective.mAspect = aspect;
  mCameraParms.mPerspective.mNear = znear;
  mCameraParms.mPerspective.mFar = zfar;
}

CGuiCamera::CGuiCamera(const CGuiWidgetParms& parms, float left, float right, float top,
                       float bottom, float znear, float zfar)
: CGuiWidget(parms) {
  mProjection = kProjection_Orthographic;
  mCameraParms.mOrthographic.mLeft = left;
  mCameraParms.mOrthographic.mRight = right;
  mCameraParms.mOrthographic.mTop = top;
  mCameraParms.mOrthographic.mBottom = bottom;
  mCameraParms.mOrthographic.mNear = znear;
  mCameraParms.mOrthographic.mFar = zfar;
}

void CGuiCamera::Draw(const CGuiWidgetDrawParms& parms) const {
  if (mProjection == kProjection_Perspective) {
    CGraphics::SetPerspective(mCameraParms.mPerspective.mFov, mCameraParms.mPerspective.mAspect,
                              mCameraParms.mPerspective.mNear, mCameraParms.mPerspective.mFar);
  } else {
    CGraphics::SetOrtho(mCameraParms.mOrthographic.mLeft, mCameraParms.mOrthographic.mRight,
                        mCameraParms.mOrthographic.mTop, mCameraParms.mOrthographic.mBottom,
                        mCameraParms.mOrthographic.mNear, mCameraParms.mOrthographic.mFar);
  }

  CGraphics::SetViewPointMatrix(CTransform4f::Translate(parms.GetCameraOffset()) *
                                GetWorldTransform());
  CGuiWidget::Draw(parms);
}

CVector3f CGuiCamera::ConvertToScreenSpace(const CVector3f& point) const {
  CVector3f rotated = RotateTranslateW2O(point);

  if (rotated.IsNonZero() && mProjection == kProjection_Perspective) {
    CMatrix4f xf = CGraphics::CalculatePerspectiveMatrix(
        mCameraParms.mPerspective.mFov, mCameraParms.mPerspective.mAspect,
        mCameraParms.mPerspective.mNear, mCameraParms.mPerspective.mFar);

    return xf.MultiplyOneOverW(rotated);
  }

  return CVector3f(-1.f, -1.f, 1.f);
}
