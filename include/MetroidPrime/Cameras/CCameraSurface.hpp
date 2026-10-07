#ifndef _CCAMERASURFACE
#define _CCAMERASURFACE

#include "types.h"

#include "Kyoto/Math/CCylinder.hpp"
#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CSphere.hpp"

class CScriptSurfaceCamera;

// Guessed names: the SurfaceCamera loader owns these projection/containment objects.
// No original class or method exports have been established for this family.
class CCameraSurface {
public:
  CCameraSurface();
  virtual CVector3f GetSurfacePoint(CVector3f point) = 0;
  virtual bool IsPointInside(const CVector3f& point) = 0;
  virtual ~CCameraSurface() = 0;
};
CHECK_SIZEOF(CCameraSurface, 0x4)

// Guessed name; native loader surface type 0.
class CSphereCameraSurface : public CCameraSurface {
public:
  explicit CSphereCameraSurface(const CSphere& sphere);

  // CCameraSurface
  CVector3f GetSurfacePoint(CVector3f point) override;
  bool IsPointInside(const CVector3f& point) override;
  ~CSphereCameraSurface() override;

  CSphere GetSphere() const { return mSphere; }

  friend class CScriptSurfaceCamera;

private:
  CSphere mSphere;
};
CHECK_SIZEOF(CSphereCameraSurface, 0x14)

// Guessed name; native loader surface type 1.
class CPlaneCameraSurface : public CCameraSurface {
public:
  CPlaneCameraSurface(const CPlane& plane, const CVector3f& axisA, const CVector3f& axisB,
                      const CVector3f& center, float width, float height);

  // CCameraSurface
  CVector3f GetSurfacePoint(CVector3f point) override;
  bool IsPointInside(const CVector3f& point) override;
  ~CPlaneCameraSurface() override;

  friend class CScriptSurfaceCamera;

private:
  CPlane mPlane;
  CVector3f mAxisA;
  CVector3f mAxisB;
  CVector3f mCenter;
  float mWidth;
  float mHeight;
};
CHECK_SIZEOF(CPlaneCameraSurface, 0x40)

// Guessed name; native loader surface type 2. The axial extent uses half mHeight.
class CCylinderCameraSurface : public CCameraSurface {
public:
  CCylinderCameraSurface(const CCylinder& cylinder, float height);

  // CCameraSurface
  CVector3f GetSurfacePoint(CVector3f point) override;
  bool IsPointInside(const CVector3f& point) override;
  ~CCylinderCameraSurface() override;

  const CCylinder& GetCylinder() const { return mCylinder; }

protected:
  float GetHeight() const { return mHeight; }

  friend class CScriptSurfaceCamera;

private:
  CCylinder mCylinder;
  float mHeight;
};
CHECK_SIZEOF(CCylinderCameraSurface, 0x24)

// Guessed name; native loader surface type 4. Spline amplitude offsets the radius.
class CSplineCylinderCameraSurface : public CCylinderCameraSurface {
public:
  CSplineCylinderCameraSurface(const CMayaSpline& spline, const CCylinder& cylinder,
                               const CVector3f& referenceDirection, float height);

  // CCameraSurface
  CVector3f GetSurfacePoint(CVector3f point) override;
  bool IsPointInside(const CVector3f& point) override;
  ~CSplineCylinderCameraSurface() override;

  friend class CScriptSurfaceCamera;

private:
  CMayaSpline mSpline;
  CVector3f mReferenceDirection;
};
CHECK_SIZEOF(CSplineCylinderCameraSurface, 0x74)

// Guessed name; native loader surface type 3. Spline amplitude offsets the plane.
class CSplinePlaneCameraSurface : public CCameraSurface {
public:
  CSplinePlaneCameraSurface(const CMayaSpline& spline, const CPlane& plane, const CVector3f& axisA,
                            const CVector3f& axisB, const CVector3f& center, float width,
                            float height);

  // CCameraSurface
  CVector3f GetSurfacePoint(CVector3f point) override;
  bool IsPointInside(const CVector3f& point) override;
  ~CSplinePlaneCameraSurface() override;

  friend class CScriptSurfaceCamera;

private:
  CMayaSpline mSpline;
  CPlane mPlane;
  CVector3f mAxisA;
  CVector3f mAxisB;
  CVector3f mCenter;
  float mWidth;
  float mHeight;
};
CHECK_SIZEOF(CSplinePlaneCameraSurface, 0x84)

#endif // _CCAMERASURFACE
