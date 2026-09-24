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
: x4_pos(pos), x8_vel(vel) {}

CEESimpleEmitter::~CEESimpleEmitter() {
  delete x4_pos;
  delete x8_vel;
}

bool CEESimpleEmitter::GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const {
  x4_pos->GetValue(frame, pPos);
  if (x8_vel != NULL) {
    x8_vel->GetValue(frame, pVel);
  } else {
    pVel = CVector3f::Zero();
  }

  return false;
}

CVESphere::CVESphere(CVectorElement* origin, CRealElement* radius, CRealElement* velocityMag)
: x4_sphereOrigin(origin), x8_sphereRadius(radius), xc_velocityMag(velocityMag) {}

CVESphere::~CVESphere() {
  delete x4_sphereOrigin;
  delete x8_sphereRadius;
  delete xc_velocityMag;
}

bool CVESphere::GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const {
  CVector3f origin = CVector3f::Zero();
  float radius;
  float mag;
  x4_sphereOrigin->GetValue(frame, origin);
  x8_sphereRadius->GetValue(frame, radius);

  CVector3f normVec1 = CVector3f(CRandom16::GetRandomNumber()->Range(-100, 100),
                                 CRandom16::GetRandomNumber()->Range(-100, 100),
                                 CRandom16::GetRandomNumber()->Range(-100, 100));
  normVec1 = (normVec1.CanBeNormalized() ? (0.01f * normVec1).AsNormalized() : (0.01f * normVec1));
  pPos = origin + (normVec1 * radius);

  CVector3f offset = (pPos - origin);
  CVector3f direction = offset.CanBeNormalized() ? offset.AsNormalized() : offset;
  xc_velocityMag->GetValue(frame, mag);
  pVel = mag * direction;

  return false;
}

CVEEllipsoid::CVEEllipsoid(CVectorElement* origin, CVectorElement* radii,
                           CVectorElement* rotation, CRealElement* velocityMag, bool onSurface)
: x4_origin(origin)
, x8_radii(radii)
, xc_rotation(rotation)
, x10_velocityMag(velocityMag)
, x14_onSurface(onSurface) {}

CVEEllipsoid::~CVEEllipsoid() {
  delete x4_origin;
  delete x8_radii;
  delete xc_rotation;
  delete x10_velocityMag;
}

bool CVEEllipsoid::GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const {
  CVector3f origin = CVector3f::Zero();
  CVector3f radii = CVector3f::Zero();
  CVector3f rotation = CVector3f::Zero();
  float mag = 0.f;
  x4_origin->GetValue(frame, origin);
  x8_radii->GetValue(frame, radii);
  xc_rotation->GetValue(frame, rotation);
  x10_velocityMag->GetValue(frame, mag);

  CVector3f dir = CVector3f(CRandom16::GetRandomNumber()->Range(-100, 100),
                            CRandom16::GetRandomNumber()->Range(-100, 100),
                            CRandom16::GetRandomNumber()->Range(-100, 100));
  bool canNormalize = dir.CanBeNormalized();
  dir *= 0.01f;
  if (canNormalize) {
    dir.Normalize();
  }
  if (!x14_onSurface) {
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
: x4_sphereOrigin(origin)
, x8_sphereRadius(radius)
, xc_velocityMag(velocityMag)
, x10_angleXBias(angleXBias)
, x14_angleYBias(angleYBias)
, x18_angleXRange(angleXRange)
, x1c_angleYRange(angleYRange) {}

CVEAngleSphere::~CVEAngleSphere() {
  delete x4_sphereOrigin;
  delete x8_sphereRadius;
  delete xc_velocityMag;
  delete x10_angleXBias;
  delete x14_angleYBias;
  delete x18_angleXRange;
  delete x1c_angleYRange;
}

bool CVEAngleSphere::GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const {
  CVector3f origin = CVector3f::Zero();
  float radius;
  float mag;
  float xBias;
  float xRange;
  float yBias;
  float yRange;
  x4_sphereOrigin->GetValue(frame, origin);
  x8_sphereRadius->GetValue(frame, radius);

  x10_angleXBias->GetValue(frame, xBias);

  x14_angleYBias->GetValue(frame, yBias);

  x18_angleXRange->GetValue(frame, xRange);

  x1c_angleYRange->GetValue(frame, yRange);

  xBias += ((0.5f * xRange) - (CRandom16::GetRandomNumber()->Float() * xRange));
  xBias *= (M_PIF / 180.f);

  yBias += ((0.5f * yRange) - (CRandom16::GetRandomNumber()->Float() * yRange));
  yBias *= (M_PIF / 180.f);

  CVector3f vec(-CMath::FastSinR(yBias) * CMath::FastCosR(xBias), CMath::FastSinR(xBias),
                CMath::FastCosR(xBias) * CMath::FastCosR(yBias));
  pPos = origin + (radius * vec);
  CVector3f dir = (pPos - origin).AsNormalized();
  xc_velocityMag->GetValue(frame, mag);
  pVel = mag * dir;

  return false;
}

CEEPlaneEmitter::CEEPlaneEmitter(CVectorElement* translation, CVectorElement* rotation,
                                 CVectorElement* velocity, CRealElement* xRange,
                                 CRealElement* yRange, CRealElement* coneAngle)
: x4_translation(translation)
, x8_rotation(rotation)
, xc_velocity(velocity)
, x10_xRange(xRange)
, x14_yRange(yRange)
, x18_coneAngle(coneAngle)
, x1c_transform(CTransform4f::Identity())
, x4c_coneAngle(0.f)
, x50_coneCos(0.f)
, x54_coneSin(0.f)
, x58_24_constantTransform(false)
, x58_25_hasCone(true)
, x58_26_constantCone(false) {
  if (x4_translation != nullptr && x8_rotation != nullptr &&
      x4_translation->IsFastConstant() && x8_rotation->IsFastConstant()) {
    x58_24_constantTransform = true;
    CVector3f trans = CVector3f::Zero();
    CVector3f rot = CVector3f::Zero();
    x4_translation->GetValue(0, trans);
    x8_rotation->GetValue(0, rot);
    x1c_transform = CTransform4f::RotateZ(CRelAngle::FromDegrees(rot.GetZ()));
    x1c_transform.RotateLocalY(CRelAngle::FromDegrees(rot.GetY()));
    x1c_transform.RotateLocalX(CRelAngle::FromDegrees(rot.GetX()));
    x1c_transform.AddTranslation(trans);
  }

  if (coneAngle != nullptr && coneAngle->IsConstant()) {
    x58_26_constantCone = true;
    x18_coneAngle->GetValue(0, x4c_coneAngle);
    if (x4c_coneAngle == 0.f) {
      x58_25_hasCone = false;
    } else {
      x4c_coneAngle *= M_PIF / 180.f;
      x50_coneCos = CMath::FastCosR(x4c_coneAngle);
      x54_coneSin = CMath::FastSinR(x4c_coneAngle);
    }
  }
}

CEEPlaneEmitter::~CEEPlaneEmitter() {
  delete x4_translation;
  delete x8_rotation;
  delete xc_velocity;
  delete x10_xRange;
  delete x14_yRange;
  delete x18_coneAngle;
}

bool CEEPlaneEmitter::GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const {
  CVector3f vel = CVector3f::Zero();
  float xRange;
  float yRange;
  xc_velocity->GetValue(frame, vel);
  x10_xRange->GetValue(frame, xRange);
  x14_yRange->GetValue(frame, yRange);

  if (!x58_24_constantTransform) {
    CVector3f trans = CVector3f::Zero();
    CVector3f rot = CVector3f::Zero();
    x4_translation->GetValue(frame, trans);
    x8_rotation->GetValue(frame, rot);
    x1c_transform = CTransform4f::RotateZ(CRelAngle::FromDegrees(rot.GetZ()));
    x1c_transform.RotateLocalY(CRelAngle::FromDegrees(rot.GetY()));
    x1c_transform.RotateLocalX(CRelAngle::FromDegrees(rot.GetX()));
    x1c_transform.AddTranslation(trans);
  }

  CVector3f pos(CRandom16::GetRandomNumber()->Range(-xRange, xRange),
                CRandom16::GetRandomNumber()->Range(-yRange, yRange), 0.f);
  if (x58_25_hasCone) {
    if (!x58_26_constantCone) {
      x18_coneAngle->GetValue(frame, x4c_coneAngle);
      x4c_coneAngle *= M_PIF / 180.f;
      x50_coneCos = CMath::FastCosR(x4c_coneAngle);
      x54_coneSin = CMath::FastSinR(x4c_coneAngle);
    }
    GetConeVelocity(xRange, yRange, x50_coneCos, x54_coneSin, pos, vel, vel);
  }

  pPos = x1c_transform * pos;
  pVel = x1c_transform.Rotate(vel);
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
