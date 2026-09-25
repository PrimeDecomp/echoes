#ifndef _CEMITTERELEMENT
#define _CEMITTERELEMENT

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Particles/IElement.hpp"

class CEESimpleEmitter : public CEmitterElement {
public:
  CEESimpleEmitter(CVectorElement* pos, CVectorElement* vel);
  ~CEESimpleEmitter();

  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const;

private:
  CVectorElement* x4_pos;
  CVectorElement* x8_vel;
};

class CVESphere : public CEmitterElement {
public:
  CVESphere(CVectorElement* origin, CRealElement* radius, CRealElement* velocityMag);
  ~CVESphere();

  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const;

private:
  CVectorElement* x4_sphereOrigin;
  CRealElement* x8_sphereRadius;
  CRealElement* xc_velocityMag;
};

// Echoes ELPS emitter: random point in (or on) a rotated ellipsoid.
class CVEEllipsoid : public CEmitterElement {
public:
  CVEEllipsoid(CVectorElement* origin, CVectorElement* radii, CVectorElement* rotation,
               CRealElement* velocityMag, bool onSurface);
  ~CVEEllipsoid();

  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const;

private:
  CVectorElement* x4_origin;
  CVectorElement* x8_radii;
  CVectorElement* xc_rotation;
  CRealElement* x10_velocityMag;
  bool x14_onSurface;
};

class CVEAngleSphere : public CEmitterElement {
public:
  CVEAngleSphere(CVectorElement* origin, CRealElement* radius, CRealElement* velocityMag,
                 CRealElement* angleXBias, CRealElement* angleYBias, CRealElement* angleXRange,
                 CRealElement* angleYRange);
  ~CVEAngleSphere();

  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const;

private:
  CVectorElement* x4_sphereOrigin;
  CRealElement* x8_sphereRadius;
  CRealElement* xc_velocityMag;
  CRealElement* x10_angleXBias;
  CRealElement* x14_angleYBias;
  CRealElement* x18_angleXRange;
  CRealElement* x1c_angleYRange;
};

// Echoes PLNE emitter: random point on a rotated rectangle, with an optional cone spread.
class CEEPlaneEmitter : public CEmitterElement {
public:
  CEEPlaneEmitter(CVectorElement* translation, CVectorElement* rotation, CVectorElement* velocity,
                  CRealElement* xRange, CRealElement* yRange, CRealElement* coneAngle);
  ~CEEPlaneEmitter();

  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const;

private:
  CVectorElement* x4_translation;
  CVectorElement* x8_rotation;
  CVectorElement* xc_velocity;
  CRealElement* x10_xRange;
  CRealElement* x14_yRange;
  CRealElement* x18_coneAngle;
  mutable CTransform4f x1c_transform;
  mutable float x4c_coneAngle;
  mutable float x50_coneCos;
  mutable float x54_coneSin;
  bool x58_24_constantTransform : 1;
  bool x58_25_hasCone : 1;
  bool x58_26_constantCone : 1;
};

#endif // _CEMITTERELEMENT
