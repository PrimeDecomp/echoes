#define MSL_NO_INLINE_SQRT
#include "Kyoto/Math/CMath.hpp"

#include "Kyoto/Math/CVector3f.hpp"

float CMath::SqrtF(const float x) { return sqrtf(x); }

double CMath::SqrtD(const double x) { return sqrt(x); }

float CMath::InvSqrtF(float x) { return 1.f / sqrtf(x); }

float CMath::CeilingF(float x) {
  float tmp = floor(x);
  if (tmp == x) {
    return x;
  }
  return tmp + 1.f;
}

CVector3f CMath::GetCatmullRomSplinePoint(const CVector3f& a, const CVector3f& b,
                                          const CVector3f& c, const CVector3f& d, float t) {
  if (t <= 0.0f)
    return b;
  if (t >= 1.0f)
    return c;

  return (
      a * (-0.5f * t * t * t + t * t - 0.5f * t) + b * (1.5f * t * t * t + -2.5f * t * t + 1.0f) +
      c * (-1.5f * t * t * t + 2.0f * t * t + 0.5f * t) + d * (0.5f * t * t * t - 0.5f * t * t));
}

float CMath::GetCatmullRomSplinePoint(float a, float b, float c, float d, float t) {
  if (t <= 0.0f)
    return b;
  if (t >= 1.0f)
    return c;

  return (
      a * (-0.5f * t * t * t + t * t - 0.5f * t) + b * (1.5f * t * t * t + -2.5f * t * t + 1.0f) +
      c * (-1.5f * t * t * t + 2.0f * t * t + 0.5f * t) + d * (0.5f * t * t * t - 0.5f * t * t));
}

CVector3f CMath::GetBezierPoint(const CVector3f& a, const CVector3f& b, const CVector3f& c,
                                const CVector3f& d, float t) {
  CVector3f ab = CVector3f::Lerp(a, b, t);
  CVector3f bc = CVector3f::Lerp(b, c, t);
  CVector3f cd = CVector3f::Lerp(c, d, t);

  return CVector3f::Lerp(CVector3f::Lerp(ab, bc, t), CVector3f::Lerp(bc, cd, t), t);
}

CVector3f CMath::BaryToWorld(const CVector3f& p0, const CVector3f& p1, const CVector3f& p2,
                             const CVector3f& bary) {
  return bary.GetX() * p0 + bary.GetY() * p1 + bary.GetZ() * p2;
}

static const uint skSinX1 = 0x3f7ff347;
static const uint skSinX2 = 0xbe2a34ae;
static const uint skSinX3 = 0x3c047fca;
static const uint skSinX4 = 0xb9206873;

float CMath::FastSinR(float x) {
  if (fabsf(x) > M_PIF) {
    x = WrapPi(x);
  }

  float x2 = x * x;
  float acc = x;
  acc *= reinterpret_cast< const float& >(skSinX1);
  x *= x2;
  acc += x * reinterpret_cast< const float& >(skSinX2);
  x *= x2;
  acc += x * reinterpret_cast< const float& >(skSinX3);
  x *= x2;
  acc += x * reinterpret_cast< const float& >(skSinX4);
  return acc;
}

static const uint skCosX1 = 0x3f800000;
static const uint skCosX2 = 0xbefffd62;
static const uint skCosX3 = 0x3d2a7a18;
static const uint skCosX4 = 0xbab2bb2b;
static const uint skCosX5 = 0x37a93188;

float CMath::FastCosR(float x) {
  if (fabsf(x) > M_PIF) {
    x = WrapPi(x);
  }

  float x2 = x * x;
  float acc = reinterpret_cast< const float& >(skCosX1);
  acc += x2 * reinterpret_cast< const float& >(skCosX2);
  float xn = x2 * x2;
  acc += xn * reinterpret_cast< const float& >(skCosX3);
  xn *= x2;
  acc += xn * reinterpret_cast< const float& >(skCosX4);
  xn *= x2;
  acc += xn * reinterpret_cast< const float& >(skCosX5);
  return acc;
}

static const uint skArcCosX1 = 0x3fc90fdb;
static const uint skArcCosX2 = 0xbf7f8bd1;
static const uint skArcCosX3 = 0xbe52ce8c;
static const uint skArcCosX4 = 0x3de9fe20;
static const uint skArcCosX5 = 0xbe980d88;

float CMath::FastArcCosR(float x) {
  if (fabsf(x) > 0.925f) {
    return acosf(x);
  }

  float x2 = x * x;
  float acc = reinterpret_cast< const float& >(skArcCosX1);
  acc += x * reinterpret_cast< const float& >(skArcCosX2);
  float xn = x * x2;
  acc += xn * reinterpret_cast< const float& >(skArcCosX3);
  xn *= x2;
  acc += xn * reinterpret_cast< const float& >(skArcCosX4);
  xn *= x2;
  acc += xn * reinterpret_cast< const float& >(skArcCosX5);
  return acc;
}

int CMath::FloorPowerOfTwo(int v) {
  if (v == 0) {
    return 0;
  }
  const uint s1 = (0xffffU - v) >> 0x1b & 0x10;
  const uint sb1 = static_cast< uint >(v) >> s1 & 0xffff;
  const uint s2 = (0xff - sb1) >> 0x1c & 8;
  const uint sb2 = sb1 >> s2 & 0xff;
  const uint s3 = ((0xf - sb2) >> 0x1d) & 4;
  const uint sb3 = (sb2 >> s3) & 0xf;
  const uint s4 = (3 - sb3) >> 0x1e & 2;
  const uint totalShift = s1 + s2 + s3 + s4;
  const uint finalSig = sb3 >> s4 & 3;
  const uint finalShift = ((1 - finalSig) >> 0x1f) + totalShift;
  return 1 << finalShift;
}

bool CMath::SolveQuadratic(float a, float b, float c, float& plus, float& minus) {
  const float discriminant = b * b - (4.f * a) * c;
  if (discriminant < FLT_EPSILON || fabsf(a) < FLT_EPSILON) {
    return false;
  }
  const float root = SqrtF(discriminant);
  plus = (-b + root) / (2.f * a);
  minus = (-b - root) / (2.f * a);
  return true;
}

float CMath::PhongBlob(float t, float exponent) {
  t = Clamp(0.f, t, 1.f);
  return pow(0.5f * (1.f + cosf(M_PIF * t)), exponent);
}

float CMath::EaseInOut(float t, EEaseTypes ease, float easeIn, float easeOut, float low, float high,
                       float scale) {
  const float minimum = FastMin(low, high);
  const float maximum = FastMax(low, high);
  const float range = maximum - minimum;
  t = Clamp(0.f, t, 1.f);
  easeIn = Clamp(0.f, easeIn, 1.f);
  easeOut = Clamp(0.f, easeOut, 1.f);

  switch (ease) {
  case kET_Sinusoidal: {
    const float easeInWeight = 2.f * easeIn / M_PIF;
    const float easeOutWeight = 2.f * (1.f - easeOut) / M_PIF;
    const float middle = easeOut + easeInWeight - easeIn;
    const float total = middle + easeOutWeight;
    if (t <= easeIn) {
      t = easeInWeight * (1.f + sinf(M_PIF * 0.5f * (t / easeIn) - M_PIF * 0.5f)) / total;
    } else if (t >= easeOut) {
      t = (easeOutWeight * sinf(M_PIF * 0.5f * ((t - easeOut) / (1.f - easeOut))) + middle) / total;
    } else {
      t = (t + easeInWeight - easeIn) / total;
    }
    break;
  }
  case kET_Quadratic:
    if (t <= easeIn) {
      t = scale * (t * t / (2.f * easeIn));
    } else if (t >= easeOut) {
      const float delta = t - easeOut;
      const float base = 0.5f * (scale * easeIn) + scale * (easeOut - easeIn);
      const float falloff = -(0.5f * (scale * (delta / (1.f - easeOut))) - scale);
      const float falloffTerm = falloff * delta;
      t = base + falloffTerm;
    } else {
      t = scale * (0.5f * easeIn) + scale * (t - easeIn);
    }
    break;
  default:
    break;
  }

  return range * Clamp(0.f, t, 1.f) + minimum;
}
