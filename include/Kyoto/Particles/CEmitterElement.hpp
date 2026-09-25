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
  CVectorElement* mPos;
  CVectorElement* mVel;
};

class CVESphere : public CEmitterElement {
public:
  CVESphere(CVectorElement* origin, CRealElement* radius, CRealElement* velocityMag);
  ~CVESphere();

  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const;

private:
  CVectorElement* mSphereOrigin;
  CRealElement* mSphereRadius;
  CRealElement* mVelocityMag;
};

// Echoes ELPS emitter: random point in (or on) a rotated ellipsoid.
class CVEEllipsoid : public CEmitterElement {
public:
  CVEEllipsoid(CVectorElement* origin, CVectorElement* radii, CVectorElement* rotation,
               CRealElement* velocityMag, bool onSurface);
  ~CVEEllipsoid();

  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const;

private:
  CVectorElement* mOrigin;
  CVectorElement* mRadii;
  CVectorElement* mRotation;
  CRealElement* mVelocityMag;
  bool mOnSurface;
};

class CVEAngleSphere : public CEmitterElement {
public:
  CVEAngleSphere(CVectorElement* origin, CRealElement* radius, CRealElement* velocityMag,
                 CRealElement* angleXBias, CRealElement* angleYBias, CRealElement* angleXRange,
                 CRealElement* angleYRange);
  ~CVEAngleSphere();

  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const;

private:
  CVectorElement* mSphereOrigin;
  CRealElement* mSphereRadius;
  CRealElement* mVelocityMag;
  CRealElement* mAngleXBias;
  CRealElement* mAngleYBias;
  CRealElement* mAngleXRange;
  CRealElement* mAngleYRange;
};

// Echoes PLNE emitter: random point on a rotated rectangle, with an optional cone spread.
class CEEPlaneEmitter : public CEmitterElement {
public:
  CEEPlaneEmitter(CVectorElement* translation, CVectorElement* rotation, CVectorElement* velocity,
                  CRealElement* xRange, CRealElement* yRange, CRealElement* coneAngle);
  ~CEEPlaneEmitter();

  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const;

private:
  CVectorElement* mTranslation;
  CVectorElement* mRotation;
  CVectorElement* mVelocity;
  CRealElement* mXRange;
  CRealElement* mYRange;
  CRealElement* mConeAngleElement;
  mutable CTransform4f mTransform;
  mutable float mConeAngleRadians;
  mutable float mConeCos;
  mutable float mConeSin;
  bool mConstantTransform : 1;
  bool mHasCone : 1;
  bool mConstantCone : 1;
};

#endif // _CEMITTERELEMENT
