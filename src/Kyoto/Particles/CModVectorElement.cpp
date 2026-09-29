#include "Kyoto/Particles/CModVectorElement.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CAABox.hpp"

CMVEConstant::CMVEConstant(CRealElement* a, CRealElement* b, CRealElement* c)
: mX(a), mY(b), mZ(c) {}

CMVEConstant::~CMVEConstant() {
  delete mX;
  delete mY;
  delete mZ;
}

bool CMVEConstant::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  float x, y, z;
  mX->GetValue(frame, x);
  mY->GetValue(frame, y);
  mZ->GetValue(frame, z);
  pVel = CVector3f(x, y, z);
  return false;
}

CMVEFastConstant::CMVEFastConstant(float a, float b, float c) : mVal(a, b, c) {}

CMVEFastConstant::~CMVEFastConstant() {}

bool CMVEFastConstant::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  pVel = mVal;
  return false;
}

CMVEGravity::CMVEGravity(CVectorElement* a) : mA(a) {}

CMVEGravity::~CMVEGravity() { delete mA; }

bool CMVEGravity::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  CVector3f grav(CVector3f::Zero());
  mA->GetValue(frame, grav);
  pVel += grav;
  return false;
}

CMVEWind::CMVEWind(CVectorElement* velocity, CRealElement* factor)
: mVelocity(velocity), mFactor(factor) {}

CMVEWind::~CMVEWind() {
  delete mVelocity;
  delete mFactor;
}

bool CMVEWind::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  CVector3f wVel(CVector3f::Zero());
  mVelocity->GetValue(frame, wVel);
  CVector3f diff = wVel - pVel;
  float factor;
  mFactor->GetValue(frame, factor);
  diff *= factor;
  pVel += diff;
  return false;
}

CMVEExplode::CMVEExplode(CRealElement* a, CRealElement* b) : mA(a), mB(b) {}

CMVEExplode::~CMVEExplode() {
  delete mA;
  delete mB;
}

bool CMVEExplode::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  if (frame == 0) {
    float x, y, z;
    do {
      x = CRandom16::GetRandomNumber()->Float() - 0.5f;
      y = CRandom16::GetRandomNumber()->Float() - 0.5f;
      z = CRandom16::GetRandomNumber()->Float() - 0.5f;
    } while (x * x + y * y + z * z > 1.f);
    float a;
    mA->GetValue(frame, a);
    CVector3f vec(x, y, z);
    vec.Normalize();
    vec *= a;
    pVel = vec;
  } else {
    float b;
    mB->GetValue(frame, b);
    pVel *= b;
  }
  return false;
}

CMVETimeChain::CMVETimeChain(CModVectorElement* a, CModVectorElement* b, CIntElement* c)
: mA(a), mB(b), mSwFrame(c) {}

CMVETimeChain::~CMVETimeChain() {
  delete mA;
  delete mB;
  delete mSwFrame;
}

bool CMVETimeChain::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  int v;
  mSwFrame->GetValue(frame, v);
  if (frame < v) {
    return mA->GetValue(frame, pVel, pPos);
  } else {
    return mB->GetValue(frame - v, pVel, pPos);
  }
}

CMVEPulse::CMVEPulse(CIntElement* a, CIntElement* b, CModVectorElement* c, CModVectorElement* d)
: mADuration(a), mBDuration(b), mAVal(c), mBVal(d) {}

CMVEPulse::~CMVEPulse() {
  delete mADuration;
  delete mBDuration;
  delete mAVal;
  delete mBVal;
}

bool CMVEPulse::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  int a, b;
  mADuration->GetValue(frame, a);
  mBDuration->GetValue(frame, b);
  int cv = a + b + 1;
  if (cv < 0) {
    cv = 1;
  }

  if (b >= 1) {
    if (frame % cv > a) {
      mBVal->GetValue(frame, pVel, pPos);
    } else {
      mAVal->GetValue(frame, pVel, pPos);
    }
  } else {
    mAVal->GetValue(frame, pVel, pPos);
  }
  return false;
}

CMVEImplosion::CMVEImplosion(CVectorElement* a, CRealElement* b, CRealElement* c, CRealElement* d,
                             bool e)
: mImplPoint(a), mMagScale(b), mMaxMag(c), mMinMag(d), mEnableMinMag(e) {}

CMVEImplosion::~CMVEImplosion() {
  delete mImplPoint;
  delete mMagScale;
  delete mMaxMag;
  delete mMinMag;
}

bool CMVEImplosion::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  CVector3f av(CVector3f::Zero());
  mImplPoint->GetValue(frame, av);

  CVector3f dv = av - pPos;
  float dvm = dv.Magnitude();

  float c;
  mMaxMag->GetValue(frame, c);
  if (dvm > c) {
    return false;
  }

  float d;
  mMinMag->GetValue(frame, d);
  if (mEnableMinMag && dvm < d) {
    return true;
  }

  if (dvm == 0.f) {
    return false;
  }

  CVector3f dvs = (1.f / dvm) * dv;
  float b;
  mMagScale->GetValue(frame, b);
  pVel += b * dvs;
  return false;
}

CMVELinearImplosion::CMVELinearImplosion(CVectorElement* a, CRealElement* b, CRealElement* c,
                                         CRealElement* d, bool e)
: mImplPoint(a), mMagScale(b), mMaxMag(c), mMinMag(d), mEnableMinMag(e) {}

CMVELinearImplosion::~CMVELinearImplosion() {
  delete mImplPoint;
  delete mMagScale;
  delete mMaxMag;
  delete mMinMag;
}

bool CMVELinearImplosion::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  CVector3f av(CVector3f::Zero());
  mImplPoint->GetValue(frame, av);

  CVector3f dv = av - pPos;
  float dvm = dv.Magnitude();

  float c;
  mMaxMag->GetValue(frame, c);
  if (dvm > c) {
    return false;
  }

  float d;
  mMinMag->GetValue(frame, d);
  if (mEnableMinMag && dvm < d) {
    return true;
  }

  if (dvm == 0.f) {
    return false;
  }

  CVector3f dvs = (1.f / dvm) * dv;
  float b;
  mMagScale->GetValue(frame, b);
  pVel = b * dvs;
  return false;
}

CMVEExponentialImplosion::CMVEExponentialImplosion(CVectorElement* a, CRealElement* b,
                                                   CRealElement* c, CRealElement* d, bool e)
: mImplPoint(a), mMagScale(b), mMaxMag(c), mMinMag(d), mEnableMinMag(e) {}

CMVEExponentialImplosion::~CMVEExponentialImplosion() {
  delete mImplPoint;
  delete mMagScale;
  delete mMaxMag;
  delete mMinMag;
}

bool CMVEExponentialImplosion::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  CVector3f av(CVector3f::Zero());
  mImplPoint->GetValue(frame, av);

  CVector3f dv = av - pPos;
  float dvm = dv.Magnitude();

  float c;
  mMaxMag->GetValue(frame, c);
  if (dvm > c) {
    return false;
  }

  float d;
  mMinMag->GetValue(frame, d);
  if (mEnableMinMag && dvm < d) {
    return true;
  }

  if (dvm == 0.f) {
    return false;
  }

  CVector3f dvs = (1.f / dvm) * dv;
  float b;
  mMagScale->GetValue(frame, b);
  pVel += dvm * (b * dvs);
  return false;
}

CMVESwirl::CMVESwirl(CVectorElement* a, CVectorElement* b, CRealElement* c, CRealElement* d)
: mHelixPoint(a), mCurveBinormal(b), mFilterGain(c), mTangentialVelocity(d) {}

CMVESwirl::~CMVESwirl() {
  delete mHelixPoint;
  delete mCurveBinormal;
  delete mFilterGain;
  delete mTangentialVelocity;
}

bool CMVESwirl::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  CVector3f a(CVector3f::Zero());
  CVector3f b(CVector3f::Zero());
  mHelixPoint->GetValue(frame, a);
  mCurveBinormal->GetValue(frame, b);

  CVector3f posToOrigin = a - pPos;
  CVector3f posToHelix = posToOrigin - CVector3f::Dot(posToOrigin, b) * b;

  float c = 0.f;
  float d = 0.f;
  mFilterGain->GetValue(frame, c);
  mTangentialVelocity->GetValue(frame, d);

#if VERSION >= VERSION_GM8P_00
  const CVector3f tangent = CVector3f::Cross(b, posToHelix) * d;
  pVel = (b * CVector3f::Dot(b, pVel) + tangent) * c + (1.f - c) * pVel;
#else
  pVel = (b * CVector3f::Dot(b, pVel) + d * CVector3f::Cross(b, posToHelix)) * c +
         (1.f - c) * pVel;
#endif
  return false;
}

CMVEBounce::CMVEBounce(CVectorElement* planePoint, CVectorElement* planeNormal,
                       CRealElement* friction, CRealElement* restitution, bool e)
: mPlanePoint(planePoint)
, mPlaneNormal(planeNormal)
, mFriction(friction)
, mRestitution(restitution)
, mPlanePrecomputed(false)
, mDieOnPenetrate(e)
, mPlaneValidatedNormal(CVector3f::Zero())
, mPlaneD(0.f) {
  if (planePoint && planeNormal && planePoint->IsFastConstant() && planeNormal->IsFastConstant()) {
    // Precompute Hesse normal form of plane (for penetration testing)
    // https://en.wikipedia.org/wiki/Hesse_normal_form
    mPlanePrecomputed = true;
    planeNormal->GetValue(0, mPlaneValidatedNormal);

    if (mPlaneValidatedNormal.MagSquared() > 0.f) {
      mPlaneValidatedNormal.Normalize();
    }
    CVector3f a(CVector3f::Zero());
    planePoint->GetValue(0, a);
    mPlaneD = CVector3f::Dot(mPlaneValidatedNormal, a);
  }
}

CMVEBounce::~CMVEBounce() {
  delete mPlanePoint;
  delete mPlaneNormal;
  delete mFriction;
  delete mRestitution;
}

bool CMVEBounce::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  if (!mPlanePrecomputed) {
    // Compute Hesse normal form of plane (for penetration testing)
    mPlaneNormal->GetValue(frame, mPlaneValidatedNormal);
    mPlaneValidatedNormal.Normalize();

    CVector3f a(CVector3f::Zero());
    mPlanePoint->GetValue(frame, a);

    mPlaneD = CVector3f::Dot(mPlaneValidatedNormal, a);
  }

  float dot = CVector3f::Dot(mPlaneValidatedNormal, pPos);
  if (dot - mPlaneD > 0.f) {
    return false;
  } else if (mDieOnPenetrate) {
    return true;
  }

  // Deflection event
  if (CVector3f::Dot(mPlaneValidatedNormal, pVel) < 0.f) {
    float pd = CVector3f::Dot(pPos - pVel, mPlaneValidatedNormal) - mPlaneD;
    float pn = CVector3f::Dot(pVel, mPlaneValidatedNormal);
    pPos = pPos + pVel * (-pd / pn - 1.f);

    float d = 0.f;
    mRestitution->GetValue(frame, d);
    pVel -= d * pVel;

    float c = 0.f;
    mFriction->GetValue(frame, c);
    pVel -= (c + 1.f) * CVector3f::Dot(mPlaneValidatedNormal, pVel) * mPlaneValidatedNormal;
  }
  return false;
}

CMVESetPosition::CMVESetPosition(CVectorElement* a) : mA(a) {}

CMVESetPosition::~CMVESetPosition() { delete mA; }

bool CMVESetPosition::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  mA->GetValue(frame, pPos);
  return false;
}

CMVESphereVolume::CMVESphereVolume(CVectorElement* a, CRealElement* b, CModVectorElement* c)
: x4_(a), x8_(b), xc_(c) {}

CMVESphereVolume::~CMVESphereVolume() {
  delete x4_;
  delete x8_;
  delete xc_;
}

bool CMVESphereVolume::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  CVector3f center(CVector3f::Zero());
  x4_->GetValue(frame, center);
  float radius = 0.0f;
  x8_->GetValue(frame, radius);
  CVector3f delta = pPos - center;
  if (delta.MagSquared() < radius * radius) {
    xc_->GetValue(frame, pVel, pPos);
  }
  return false;
}

CMVEBoxVolume::CMVEBoxVolume(CVectorElement* a, CVectorElement* b, CModVectorElement* c)
: x4_(a), x8_(b), xc_(c) {}

CMVEBoxVolume::~CMVEBoxVolume() {
  delete x4_;
  delete x8_;
  delete xc_;
}

bool CMVEBoxVolume::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  CVector3f center(CVector3f::Zero());
  x4_->GetValue(frame, center);
  CVector3f extents(CVector3f::Zero());
  x8_->GetValue(frame, extents);
  extents *= 0.5f;
  CAABox box(center - extents, center + extents);
  if (box.PointInside(pPos)) {
    xc_->GetValue(frame, pVel, pPos);
  }
  return false;
}
