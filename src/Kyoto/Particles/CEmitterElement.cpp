#include "Kyoto/Particles/CEmitterElement.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

static CVector3f sUnitVector(1.f, 1.f, 1.f);

static void GetConeVelocity(float xRange, float yRange, float coneCos, float coneSin,
                            const CVector3f& pos, const CVector3f& vel, CVector3f& velOut);

CEESimpleEmitter::CEESimpleEmitter(CVectorElement* pos, CVectorElement* vel)
: mPos(pos), mVel(vel) {}

CEESimpleEmitter::~CEESimpleEmitter() {
  delete mPos;
  delete mVel;
}

bool CEESimpleEmitter::GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const {
  mPos->GetValue(frame, pPos);
  if (mVel != NULL) {
    mVel->GetValue(frame, pVel);
  } else {
    pVel = CVector3f::Zero();
  }

  return false;
}

CVESphere::CVESphere(CVectorElement* origin, CRealElement* radius, CRealElement* velocityMag)
: mSphereOrigin(origin), mSphereRadius(radius), mVelocityMag(velocityMag) {}

CVESphere::~CVESphere() {
  delete mSphereOrigin;
  delete mSphereRadius;
  delete mVelocityMag;
}

bool CVESphere::GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const {
  CVector3f origin = CVector3f::Zero();
  float radius;
  float mag;
  mSphereOrigin->GetValue(frame, origin);
  mSphereRadius->GetValue(frame, radius);

  CVector3f normVec1 = CVector3f(CRandom16::GetRandomNumber()->Range(-100, 100),
                                 CRandom16::GetRandomNumber()->Range(-100, 100),
                                 CRandom16::GetRandomNumber()->Range(-100, 100));
  normVec1 = (normVec1.CanBeNormalized() ? (0.01f * normVec1).AsNormalized() : (0.01f * normVec1));
  pPos = origin + (normVec1 * radius);

  CVector3f offset = (pPos - origin);
  CVector3f direction = offset.CanBeNormalized() ? offset.AsNormalized() : offset;
  mVelocityMag->GetValue(frame, mag);
  pVel = mag * direction;

  return false;
}

CVEEllipsoid::CVEEllipsoid(CVectorElement* origin, CVectorElement* radii,
                           CVectorElement* rotation, CRealElement* velocityMag, bool onSurface)
: mOrigin(origin)
, mRadii(radii)
, mRotation(rotation)
, mVelocityMag(velocityMag)
, mOnSurface(onSurface) {}

CVEEllipsoid::~CVEEllipsoid() {
  delete mOrigin;
  delete mRadii;
  delete mRotation;
  delete mVelocityMag;
}

bool CVEEllipsoid::GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const {
  CVector3f origin = CVector3f::Zero();
  CVector3f radii = CVector3f::Zero();
  CVector3f rotation = CVector3f::Zero();
  float mag = 0.f;
  mOrigin->GetValue(frame, origin);
  mRadii->GetValue(frame, radii);
  mRotation->GetValue(frame, rotation);
  mVelocityMag->GetValue(frame, mag);

  CVector3f dir = CVector3f(CRandom16::GetRandomNumber()->Range(-100, 100),
                            CRandom16::GetRandomNumber()->Range(-100, 100),
                            CRandom16::GetRandomNumber()->Range(-100, 100));
  bool canNormalize = dir.CanBeNormalized();
  dir *= 0.01f;
  if (canNormalize) {
    dir.Normalize();
  }
  if (!mOnSurface) {
    radii.SetX(CRandom16::GetRandomNumber()->Range(0.f, radii.GetX()));
    radii.SetY(CRandom16::GetRandomNumber()->Range(0.f, radii.GetY()));
    radii.SetZ(CRandom16::GetRandomNumber()->Range(0.f, radii.GetZ()));
  }
  dir = dir * radii;

  CTransform4f xf = CTransform4f::Identity();
  xf = CTransform4f::RotateZ(CRelAngle::FromDegrees(rotation.GetZ()));
  xf.RotateLocalY(CRelAngle::FromDegrees(rotation.GetY()));
  xf.RotateLocalX(CRelAngle::FromDegrees(rotation.GetX()));
  dir = xf * dir;

  pPos = origin + dir;
  pVel = dir.CanBeNormalized() ? dir.AsNormalized() : dir;
  pVel *= mag;

  return false;
}

CVEAngleSphere::CVEAngleSphere(CVectorElement* origin, CRealElement* radius,
                               CRealElement* velocityMag, CRealElement* angleXBias,
                               CRealElement* angleYBias, CRealElement* angleXRange,
                               CRealElement* angleYRange)
: mSphereOrigin(origin)
, mSphereRadius(radius)
, mVelocityMag(velocityMag)
, mAngleXBias(angleXBias)
, mAngleYBias(angleYBias)
, mAngleXRange(angleXRange)
, mAngleYRange(angleYRange) {}

CVEAngleSphere::~CVEAngleSphere() {
  delete mSphereOrigin;
  delete mSphereRadius;
  delete mVelocityMag;
  delete mAngleXBias;
  delete mAngleYBias;
  delete mAngleXRange;
  delete mAngleYRange;
}

bool CVEAngleSphere::GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const {
  CVector3f origin = CVector3f::Zero();
  float radius;
  float mag;
  float xBias;
  float xRange;
  float yBias;
  float yRange;
  mSphereOrigin->GetValue(frame, origin);
  mSphereRadius->GetValue(frame, radius);

  mAngleXBias->GetValue(frame, xBias);

  mAngleYBias->GetValue(frame, yBias);

  mAngleXRange->GetValue(frame, xRange);

  mAngleYRange->GetValue(frame, yRange);

  xBias += ((0.5f * xRange) - (CRandom16::GetRandomNumber()->Float() * xRange));
  xBias *= (M_PIF / 180.f);

  yBias += ((0.5f * yRange) - (CRandom16::GetRandomNumber()->Float() * yRange));
  yBias *= (M_PIF / 180.f);

  CVector3f vec(-CMath::FastSinR(yBias) * CMath::FastCosR(xBias), CMath::FastSinR(xBias),
                CMath::FastCosR(xBias) * CMath::FastCosR(yBias));
  pPos = origin + (radius * vec);
  CVector3f dir = (pPos - origin).AsNormalized();
  mVelocityMag->GetValue(frame, mag);
  pVel = mag * dir;

  return false;
}

CEEPlaneEmitter::CEEPlaneEmitter(CVectorElement* translation, CVectorElement* rotation,
                                 CVectorElement* velocity, CRealElement* xRange,
                                 CRealElement* yRange, CRealElement* coneAngle)
: mTranslation(translation)
, mRotation(rotation)
, mVelocity(velocity)
, mXRange(xRange)
, mYRange(yRange)
, mConeAngleElement(coneAngle)
, mTransform(CTransform4f::Identity())
, mConeAngleRadians(0.f)
, mConeCos(0.f)
, mConeSin(0.f)
, mConstantTransform(false)
, mHasCone(true)
, mConstantCone(false) {
  if (mTranslation != nullptr && mRotation != nullptr &&
      mTranslation->IsFastConstant() && mRotation->IsFastConstant()) {
    mConstantTransform = true;
    CVector3f trans = CVector3f::Zero();
    CVector3f rot = CVector3f::Zero();
    mTranslation->GetValue(0, trans);
    mRotation->GetValue(0, rot);
    mTransform = CTransform4f::RotateZ(CRelAngle::FromDegrees(rot.GetZ()));
    mTransform.RotateLocalY(CRelAngle::FromDegrees(rot.GetY()));
    mTransform.RotateLocalX(CRelAngle::FromDegrees(rot.GetX()));
    mTransform.AddTranslation(trans);
  }

  if (coneAngle != nullptr && coneAngle->IsConstant()) {
    mConstantCone = true;
    mConeAngleElement->GetValue(0, mConeAngleRadians);
    if (mConeAngleRadians == 0.f) {
      mHasCone = false;
    } else {
      mConeAngleRadians *= M_PIF / 180.f;
      mConeCos = CMath::FastCosR(mConeAngleRadians);
      mConeSin = CMath::FastSinR(mConeAngleRadians);
    }
  }
}

CEEPlaneEmitter::~CEEPlaneEmitter() {
  delete mTranslation;
  delete mRotation;
  delete mVelocity;
  delete mXRange;
  delete mYRange;
  delete mConeAngleElement;
}

bool CEEPlaneEmitter::GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const {
  CVector3f vel = CVector3f::Zero();
  float xRange;
  float yRange;
  mVelocity->GetValue(frame, vel);
  mXRange->GetValue(frame, xRange);
  mYRange->GetValue(frame, yRange);

  if (!mConstantTransform) {
    CVector3f trans = CVector3f::Zero();
    CVector3f rot = CVector3f::Zero();
    mTranslation->GetValue(frame, trans);
    mRotation->GetValue(frame, rot);
    mTransform = CTransform4f::RotateZ(CRelAngle::FromDegrees(rot.GetZ()));
    mTransform.RotateLocalY(CRelAngle::FromDegrees(rot.GetY()));
    mTransform.RotateLocalX(CRelAngle::FromDegrees(rot.GetX()));
    mTransform.AddTranslation(trans);
  }

  CVector3f pos(CRandom16::GetRandomNumber()->Range(-xRange, xRange),
                CRandom16::GetRandomNumber()->Range(-yRange, yRange), 0.f);
  if (mHasCone) {
    if (!mConstantCone) {
      mConeAngleElement->GetValue(frame, mConeAngleRadians);
      mConeAngleRadians *= M_PIF / 180.f;
      mConeCos = CMath::FastCosR(mConeAngleRadians);
      mConeSin = CMath::FastSinR(mConeAngleRadians);
    }
    GetConeVelocity(xRange, yRange, mConeCos, mConeSin, pos, vel, vel);
  }

  pPos = mTransform * pos;
  pVel = mTransform.Rotate(vel);
  return false;
}

static void GetConeVelocity(float xRange, float yRange, float coneCos, float coneSin,
                            const CVector3f& pos, const CVector3f& vel, CVector3f& velOut) {
  float xx = pos.GetX() * pos.GetX();
  float yy = pos.GetY() * pos.GetY();
  float len = CMath::FastSqrtF(xx + yy);
  float dist = static_cast< float >(fabs(pos.GetX()));
  float range = xRange;
  if ((yRange - static_cast< float >(fabs(pos.GetY())) < xRange - dist || xRange == 0.f) &&
      yRange != 0.f) {
    range = yRange;
    dist = static_cast< float >(fabs(pos.GetY()));
  }

  if (range != 0.f && coneSin != 0.f && len != 0.f) {
    float z = len / coneSin;
    CVector3f coneDir(pos.GetX(), pos.GetY(), z * coneCos);
    coneDir.Normalize();
    coneDir *= vel.Magnitude();
    if (CMath::Sign(vel.GetZ()) != CMath::Sign(coneDir.GetZ())) {
      coneDir.SetZ(-1.f * coneDir.GetZ());
    }
    velOut = CVector3f::Lerp(vel, coneDir, dist / range);
  }
}
