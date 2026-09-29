#ifndef _CGUICAMERA
#define _CGUICAMERA

#include "GuiSys/CGuiWidget.hpp"

class CGuiCamera : public CGuiWidget {
public:
  enum EProjection { kProjection_Perspective, kProjection_Orthographic };

  union UCameraParms {
    struct {
      float mFov;
      float mAspect;
      float mNear;
      float mFar;
    } mPerspective;
    struct {
      float mLeft;
      float mRight;
      float mTop;
      float mBottom;
      float mNear;
      float mFar;
    } mOrthographic;
  };

  CGuiCamera(const CGuiWidgetParms& parms, float fov, float aspect, float znear, float zfar);
  CGuiCamera(const CGuiWidgetParms& parms, float left, float right, float top, float bottom,
             float znear, float zfar);

  // CGuiWidget
  FourCC GetWidgetTypeID() const override { return 'CAMR'; }
  EWidgetUsageFlags GetWidgetUsageFlags() const override { return kWUF_None; }
  void Draw(const CGuiWidgetDrawParms& parms) const override;

  static CGuiWidget* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* pool, uint version);

  CVector3f ConvertToScreenSpace(const CVector3f& point) const;

private:
  EProjection mProjection;
  UCameraParms mCameraParms;
};
CHECK_SIZEOF(CGuiCamera, 0xd8)

#endif // _CGUICAMERA
