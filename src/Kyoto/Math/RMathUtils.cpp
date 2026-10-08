#define MSL_NO_INLINE_SQRT
#include "Kyoto/Math/CMath.hpp"

#include "Kyoto/Math/CVector3f.hpp"

static const double skSqrtThree = CMath::SqrtD(3.0);

// Guessed names for the Perlin-noise helpers and their permutation table.
// Ken Perlin's reference permutation, stored twice so lookups never need to wrap.
static uchar skNoisePermutation[512] = {
    151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233, 7,   225, 140, 36,  103,
    30,  69,  142, 8,   99,  37,  240, 21,  10,  23,  190, 6,   148, 247, 120, 234, 75,  0,   26,
    197, 62,  94,  252, 219, 203, 117, 35,  11,  32,  57,  177, 33,  88,  237, 149, 56,  87,  174,
    20,  125, 136, 171, 168, 68,  175, 74,  165, 71,  134, 139, 48,  27,  166, 77,  146, 158, 231,
    83,  111, 229, 122, 60,  211, 133, 230, 220, 105, 92,  41,  55,  46,  245, 40,  244, 102, 143,
    54,  65,  25,  63,  161, 1,   216, 80,  73,  209, 76,  132, 187, 208, 89,  18,  169, 200, 196,
    135, 130, 116, 188, 159, 86,  164, 100, 109, 198, 173, 186, 3,   64,  52,  217, 226, 250, 124,
    123, 5,   202, 38,  147, 118, 126, 255, 82,  85,  212, 207, 206, 59,  227, 47,  16,  58,  17,
    182, 189, 28,  42,  223, 183, 170, 213, 119, 248, 152, 2,   44,  154, 163, 70,  221, 153, 101,
    155, 167, 43,  172, 9,   129, 22,  39,  253, 19,  98,  108, 110, 79,  113, 224, 232, 178, 185,
    112, 104, 218, 246, 97,  228, 251, 34,  242, 193, 238, 210, 144, 12,  191, 179, 162, 241, 81,
    51,  145, 235, 249, 14,  239, 107, 49,  192, 214, 31,  181, 199, 106, 157, 184, 84,  204, 176,
    115, 121, 50,  45,  127, 4,   150, 254, 138, 236, 205, 93,  222, 114, 67,  29,  24,  72,  243,
    141, 128, 195, 78,  66,  215, 61,  156, 180, 151, 160, 137, 91,  90,  15,  131, 13,  201, 95,
    96,  53,  194, 233, 7,   225, 140, 36,  103, 30,  69,  142, 8,   99,  37,  240, 21,  10,  23,
    190, 6,   148, 247, 120, 234, 75,  0,   26,  197, 62,  94,  252, 219, 203, 117, 35,  11,  32,
    57,  177, 33,  88,  237, 149, 56,  87,  174, 20,  125, 136, 171, 168, 68,  175, 74,  165, 71,
    134, 139, 48,  27,  166, 77,  146, 158, 231, 83,  111, 229, 122, 60,  211, 133, 230, 220, 105,
    92,  41,  55,  46,  245, 40,  244, 102, 143, 54,  65,  25,  63,  161, 1,   216, 80,  73,  209,
    76,  132, 187, 208, 89,  18,  169, 200, 196, 135, 130, 116, 188, 159, 86,  164, 100, 109, 198,
    173, 186, 3,   64,  52,  217, 226, 250, 124, 123, 5,   202, 38,  147, 118, 126, 255, 82,  85,
    212, 207, 206, 59,  227, 47,  16,  58,  17,  182, 189, 28,  42,  223, 183, 170, 213, 119, 248,
    152, 2,   44,  154, 163, 70,  221, 153, 101, 155, 167, 43,  172, 9,   129, 22,  39,  253, 19,
    98,  108, 110, 79,  113, 224, 232, 178, 185, 112, 104, 218, 246, 97,  228, 251, 34,  242, 193,
    238, 210, 144, 12,  191, 179, 162, 241, 81,  51,  145, 235, 249, 14,  239, 107, 49,  192, 214,
    31,  181, 199, 106, 157, 184, 84,  204, 176, 115, 121, 50,  45,  127, 4,   150, 254, 138, 236,
    205, 93,  222, 114, 67,  29,  24,  72,  243, 141, 128, 195, 78,  66,  215, 61,  156, 180};

float CMath::SqrtF(const float x) { return sqrt(x); }

double CMath::SqrtD(const double x) { return sqrt(x); }

float CMath::InvSqrtF(float x) { return 1.f / sqrt(x); }

float CMath::CeilingF(float x) {
  float tmp = floor(x);
  if (tmp == x) {
    return x;
  }
  return tmp + 1.f;
}

CVector3f CMath::GetHermiteSplinePoint(const CVector3f& a, const CVector3f& b,
                                       const CVector3f& tangentA, const CVector3f& tangentB,
                                       float t) {
  if (t <= 0.f) {
    return a;
  }
  if (t >= 1.f) {
    return b;
  }

  const float t2 = t * t;
  const float t3 = t2 * t;
  const float h00 = 1.f + (2.f * t3 - 3.f * t2);
  const float h01 = -2.f * t3 + 3.f * t2;
  const float h10 = t + (t3 - 2.f * t2);
  const float h11 = t3 - t2;
  return h00 * a + h01 * b + h10 * tangentA + h11 * tangentB;
}

CVector3f CMath::GetHermiteSplineTangent(const CVector3f& a, const CVector3f& b,
                                         const CVector3f& tangentA, const CVector3f& tangentB,
                                         float t) {
  const float t2 = t * t;
  const float h00 = 6.f * t2 - 6.f * t;
  const float h01 = -6.f * t2 + 6.f * t;
  const float h10 = 1.f + (3.f * t2 - 4.f * t);
  const float h11 = 3.f * t2 - 2.f * t;
  return h00 * a + h01 * b + h10 * tangentA + h11 * tangentB;
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

CVector3f CMath::GetCatmullRomSplineTangent(const CVector3f& a, const CVector3f& b,
                                            const CVector3f& c, const CVector3f& d, float t) {
  const float t2 = t * t;
  return 0.5f * ((-3.f * t2 + 4.f * t - 1.f) * a + (9.f * t2 - 10.f * t) * b +
                 (1.f + (-9.f * t2 + 8.f * t)) * c + (3.f * t2 - 2.f * t) * d);
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

CVector3f CMath::GetRoundedCatmullRomSplinePoint(const CVector3f& a, const CVector3f& b,
                                                 const CVector3f& c, const CVector3f& d, float t) {
  if (t <= 0.f) {
    return b;
  }
  if (t >= 1.f) {
    return c;
  }

  const CVector3f span = c - b;
  if (!span.CanBeNormalized()) {
    return b;
  }

  CVector3f incoming = a - b;
  if (!incoming.CanBeNormalized()) {
    incoming = CVector3f(0.f, 1.f, 0.f);
  }
  CVector3f tangentA = span.AsNormalized() - incoming.AsNormalized();
  if (tangentA.CanBeNormalized()) {
    tangentA.Normalize();
  } else {
    tangentA = CVector3f(0.f, 1.f, 0.f);
  }

  CVector3f outgoing = d - c;
  if (!outgoing.CanBeNormalized()) {
    outgoing = CVector3f(0.f, 1.f, 0.f);
  }
  const CVector3f backwardSpan = -span;
  CVector3f tangentB = outgoing.AsNormalized() - backwardSpan.AsNormalized();
  if (tangentB.CanBeNormalized()) {
    tangentB.Normalize();
  } else {
    tangentB = CVector3f(0.f, 1.f, 0.f);
  }

  const float length = span.Magnitude();
  return GetHermiteSplinePoint(b, c, length * tangentA, length * tangentB, t);
}

CVector3f CMath::GetRoundedCatmullRomSplineTangent(const CVector3f& a, const CVector3f& b,
                                                   const CVector3f& c, const CVector3f& d,
                                                   float t) {
  const CVector3f span = c - b;
  if (!span.CanBeNormalized()) {
    return b;
  }

  CVector3f incoming = a - b;
  if (!incoming.CanBeNormalized()) {
    incoming = CVector3f(0.f, 1.f, 0.f);
  }
  CVector3f tangentA = span.AsNormalized() - incoming.AsNormalized();
  if (tangentA.CanBeNormalized()) {
    tangentA.Normalize();
  } else {
    tangentA = CVector3f(0.f, 1.f, 0.f);
  }

  CVector3f outgoing = d - c;
  if (!outgoing.CanBeNormalized()) {
    outgoing = CVector3f(0.f, 1.f, 0.f);
  }
  const CVector3f backwardSpan = -span;
  CVector3f tangentB = outgoing.AsNormalized() - backwardSpan.AsNormalized();
  if (tangentB.CanBeNormalized()) {
    tangentB.Normalize();
  } else {
    tangentB = CVector3f(0.f, 1.f, 0.f);
  }

  const float length = span.Magnitude();
  return GetHermiteSplineTangent(b, c, length * tangentA, length * tangentB, t);
}

CVector3f CMath::GetBezierPoint(const CVector3f& a, const CVector3f& b, const CVector3f& c,
                                const CVector3f& d, float t) {
  CVector3f ab = CVector3f::Lerp(a, b, t);
  CVector3f bc = CVector3f::Lerp(b, c, t);
  CVector3f cd = CVector3f::Lerp(c, d, t);

  return CVector3f::Lerp(CVector3f::Lerp(ab, bc, t), CVector3f::Lerp(bc, cd, t), t);
}

CVector3f CMath::GetBezierTangent(const CVector3f& a, const CVector3f& b, const CVector3f& c,
                                  const CVector3f& d, float t) {
  const float t2 = t * t;
  return (-3.f + 6.f * t - 3.f * t2) * a + (-(12.f * t - 3.f) + 9.f * t2) * b +
         (6.f * t - 9.f * t2) * c + (3.f * t2) * d;
}

CVector3f CMath::GetBSplinePoint(const CVector3f& a, const CVector3f& b, const CVector3f& c,
                                 const CVector3f& d, float t) {
  const float clamped = Clamp(0.f, t, 1.f);
  const float t2 = clamped * clamped;
  const float t3 = t2 * clamped;
  return (1.f / 6.f) *
         ((1.f + (-3.f * t + (-t3 + 3.f * t2))) * a + (4.f + (3.f * t3 + -6.f * t2)) * b +
          (1.f + (3.f * t + (-3.f * t3 + 3.f * t2))) * c + t3 * d);
}

CVector3f CMath::GetBSplineTangent(const CVector3f& a, const CVector3f& b, const CVector3f& c,
                                   const CVector3f& d, float t) {
  t = Clamp(0.f, t, 1.f);
  const float t2 = t * t;
  return (1.f / 6.f) * ((-3.f * t2 + 6.f * t - 3.f) * a + (9.f * t2 + -12.f * t) * b +
                        (3.f + (-9.f * t2 + 6.f * t)) * c + (3.f * t2) * d);
}

static float NoiseFade(float t) { return t * t * t * (t * (6.f * t - 15.f) + 10.f); }

static float NoiseLerp(float t, float a, float b) { return a + t * (b - a); }

static float NoiseGradient3d(uint hash, float x, float y, float z) {
  const int h = hash & 15;
  float u = h < 8 ? x : y;
  float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
  return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

static float NoiseGradient4d(uint hash, float x, float y, float z, float w) {
  int h = hash & 31;
  float b = y;
  switch (h >> 3) {
  case 1:
    b = w;
    z = x;
    w = y;
    break;
  case 2:
    b = z;
    z = w;
    w = x;
    break;
  case 3:
    b = y;
    break;
  }
  if ((h & 2) == 0) {
    z = -z;
  }
  if ((h & 4) == 0) {
    b = -b;
  }
  if ((h & 1) == 0) {
    w = -w;
  }
  return w + (b + z);
}

static float PerlinNoise3d(float x, float y, float z) {
  const float floorX = floorf(x);
  const float floorY = floorf(y);
  const float floorZ = floorf(z);
  const int cellX = static_cast< int >(floorX) & 255;
  const int cellY = static_cast< int >(floorY) & 255;
  const int cellZ = static_cast< int >(floorZ) & 255;
  x -= floorX;
  y -= floorY;
  z -= floorZ;

  const float u = NoiseFade(x);
  const float v = NoiseFade(y);
  const float w = NoiseFade(z);
  const int a = skNoisePermutation[cellX] + cellY;
  const int aa = skNoisePermutation[a] + cellZ;
  const int ab = skNoisePermutation[a + 1] + cellZ;
  const int b = skNoisePermutation[cellX + 1] + cellY;
  const int ba = skNoisePermutation[b] + cellZ;
  const int bb = skNoisePermutation[b + 1] + cellZ;

  return NoiseLerp(
      w,
      NoiseLerp(v,
                NoiseLerp(u, NoiseGradient3d(skNoisePermutation[aa], x, y, z),
                          NoiseGradient3d(skNoisePermutation[ba], x - 1.f, y, z)),
                NoiseLerp(u, NoiseGradient3d(skNoisePermutation[ab], x, y - 1.f, z),
                          NoiseGradient3d(skNoisePermutation[bb], x - 1.f, y - 1.f, z))),
      NoiseLerp(v,
                NoiseLerp(u, NoiseGradient3d(skNoisePermutation[aa + 1], x, y, z - 1.f),
                          NoiseGradient3d(skNoisePermutation[ba + 1], x - 1.f, y, z - 1.f)),
                NoiseLerp(u, NoiseGradient3d(skNoisePermutation[ab + 1], x, y - 1.f, z - 1.f),
                          NoiseGradient3d(skNoisePermutation[bb + 1], x - 1.f, y - 1.f, z - 1.f))));
}

static float PerlinNoise4d(float x, float y, float z, float w) {
  const float floorX = floorf(x);
  const float floorY = floorf(y);
  const float floorZ = floorf(z);
  const float floorW = floorf(w);
  const int cellX = static_cast< int >(floorX) & 255;
  const int cellY = static_cast< int >(floorY) & 255;
  const int cellZ = static_cast< int >(floorZ) & 255;
  const int cellW = static_cast< int >(floorW) & 255;
  x -= floorX;
  y -= floorY;
  z -= floorZ;
  w -= floorW;

  const float u = NoiseFade(x);
  const float v = NoiseFade(y);
  const float s = NoiseFade(z);
  const float t = NoiseFade(w);
  const int a = skNoisePermutation[cellX] + cellY;
  const int aa = skNoisePermutation[a] + cellZ;
  const int ab = skNoisePermutation[a + 1] + cellZ;
  const int b = skNoisePermutation[cellX + 1] + cellY;
  const int ba = skNoisePermutation[b] + cellZ;
  const int bb = skNoisePermutation[b + 1] + cellZ;
  const int aaa = skNoisePermutation[aa] + cellW;
  const int aab = skNoisePermutation[aa + 1] + cellW;
  const int aba = skNoisePermutation[ab] + cellW;
  const int abb = skNoisePermutation[ab + 1] + cellW;
  const int baa = skNoisePermutation[ba] + cellW;
  const int bab = skNoisePermutation[ba + 1] + cellW;
  const int bba = skNoisePermutation[bb] + cellW;
  const int bbb = skNoisePermutation[bb + 1] + cellW;

  return NoiseLerp(
      t,
      NoiseLerp(
          s,
          NoiseLerp(v,
                    NoiseLerp(u, NoiseGradient4d(skNoisePermutation[aaa], x, y, z, w),
                              NoiseGradient4d(skNoisePermutation[baa], x - 1.f, y, z, w)),
                    NoiseLerp(u, NoiseGradient4d(skNoisePermutation[aba], x, y - 1.f, z, w),
                              NoiseGradient4d(skNoisePermutation[bba], x - 1.f, y - 1.f, z, w))),
          NoiseLerp(
              v,
              NoiseLerp(u, NoiseGradient4d(skNoisePermutation[aab], x, y, z - 1.f, w),
                        NoiseGradient4d(skNoisePermutation[bab], x - 1.f, y, z - 1.f, w)),
              NoiseLerp(u, NoiseGradient4d(skNoisePermutation[abb], x, y - 1.f, z - 1.f, w),
                        NoiseGradient4d(skNoisePermutation[bbb], x - 1.f, y - 1.f, z - 1.f, w)))),
      NoiseLerp(
          s,
          NoiseLerp(v,
                    NoiseLerp(u, NoiseGradient4d(skNoisePermutation[aaa + 1], x, y, z, w - 1.f),
                              NoiseGradient4d(skNoisePermutation[baa + 1], x - 1.f, y, z, w - 1.f)),
                    NoiseLerp(u,
                              NoiseGradient4d(skNoisePermutation[aba + 1], x, y - 1.f, z, w - 1.f),
                              NoiseGradient4d(skNoisePermutation[bba + 1], x - 1.f, y - 1.f, z,
                                              w - 1.f))),
          NoiseLerp(
              v,
              NoiseLerp(u, NoiseGradient4d(skNoisePermutation[aab + 1], x, y, z - 1.f, w - 1.f),
                        NoiseGradient4d(skNoisePermutation[bab + 1], x - 1.f, y, z - 1.f, w - 1.f)),
              NoiseLerp(u,
                        NoiseGradient4d(skNoisePermutation[abb + 1], x, y - 1.f, z - 1.f, w - 1.f),
                        NoiseGradient4d(skNoisePermutation[bbb + 1], x - 1.f, y - 1.f, z - 1.f,
                                        w - 1.f)))));
}

float CMath::Noise1d(float x) { return PerlinNoise3d(x, 0.f, 0.f); }

float CMath::Noise2d(float x, float y) { return PerlinNoise3d(x, y, 0.f); }

float CMath::Noise3d(float x, float y, float z) { return PerlinNoise3d(x, y, z); }

float CMath::Noise4d(float x, float y, float z, float w) { return PerlinNoise4d(x, y, z, w); }

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

int CMath::FloorLog2(uint v) {
  if (static_cast< int >(v) == 0) {
    return 0;
  }

  const uint s1 = (0xffffU - v) >> 0x1b & 0x10;
  const uint sb1 = v >> s1 & 0xffff;
  const uint s2 = (0xff - sb1) >> 0x1c & 8;
  const uint sb2 = sb1 >> s2 & 0xff;
  const uint s3 = (0xf - sb2) >> 0x1d & 4;
  const uint sb3 = sb2 >> s3 & 0xf;
  const uint s4 = (3 - sb3) >> 0x1e & 2;
  const uint finalSig = sb3 >> s4 & 3;
  const uint totalShift = s1 + s2 + s3 + s4;
  return ((1 - finalSig) >> 0x1f) + totalShift;
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

uint CMath::SolveCubic(const float* coefficients, float* roots) {
  uint count = 0;
  if (coefficients[3] != 0.f) {
    const float shift = coefficients[2] / (3.f * coefficients[3]);
    const float p = coefficients[1] / (3.f * coefficients[3]) - shift * shift;
    const float q = -0.5f * (shift * ((2.f * shift) * shift) -
                             (coefficients[1] * shift - coefficients[0]) / coefficients[3]);
    const float p3 = p * (p * p);
    const float discriminant = q * q + p3;
    if (discriminant < 0.f) {
      const float negativeP3 = -p3;
      const float angle = acosf(Clamp(-1.f, q / SqrtF(negativeP3), 1.f));
      const float amplitude = 2.f * powf(negativeP3, 1.f / 6.f);
      float* out = roots;
      for (float phase = 0.f; phase < 2.01f; phase += 1.f) {
        *out++ = amplitude * cosf((M_PIF * (2.f * phase) + angle) / 3.f) - shift;
        ++count;
      }
      if (roots[1] < roots[0]) {
        Swap(roots[0], roots[1]);
      }
      if (roots[2] < roots[1]) {
        Swap(roots[1], roots[2]);
      }
      if (roots[1] < roots[0]) {
        Swap(roots[0], roots[1]);
      }
    } else {
      const float root = SqrtF(discriminant);
      const float positive = q + root;
      float u = powf(fabsf(positive), 1.f / 3.f);
      const float negative = q - root;
      float v = powf(fabsf(negative), 1.f / 3.f);
      v = negative > 0.f ? v : -v;
      u = positive > 0.f ? u : -u;
      roots[0] = u + v - shift;
      count = 1;
    }

    for (uint i = 0; i < count; ++i) {
      const float& x = roots[i];
      const float slope = x * (2.f * coefficients[2] + coefficients[3] * (3.f * x));
      const float derivative = coefficients[1] + slope;
      if (derivative != 0.f) {
        roots[i] = x - (((coefficients[3] * x + coefficients[2]) * x + coefficients[1]) * x +
                        coefficients[0]) /
                           derivative;
      }
    }
  } else if (coefficients[2] != 0.f) {
    const float halfB = (0.5f * coefficients[1]) / coefficients[2];
    const float discriminant = halfB * halfB - coefficients[0] / coefficients[2];
    if (discriminant >= 0.f) {
      const float root = SqrtF(discriminant);
      roots[0] = -halfB - root;
      roots[1] = -halfB + root;
      count = 2;
    }
  } else if (coefficients[1] != 0.f) {
    roots[0] = -coefficients[0] / coefficients[1];
    count = 1;
  }
  for (uint i = 0; i < count; ++i) {
    // Native keeps an empty pass over the roots here (stripped check).
  }
  return count;
}

uint CMath::SolveQuartic(const float* coefficients, float* roots) {
  uint count = 0;
  if (coefficients[4] == 0.f) {
    float cubic[4];
    cubic[0] = coefficients[0];
    cubic[1] = coefficients[1];
    cubic[2] = coefficients[2];
    cubic[3] = coefficients[3];
    return SolveCubic(cubic, roots);
  }

  const float quadratic = coefficients[2] / coefficients[4];
  const float shift = coefficients[3] / (4.f * coefficients[4]);
  const float p = (-6.f * shift) * shift + quadratic;
  const float q = shift * ((8.f * shift) * shift - (2.f * coefficients[2]) / coefficients[4]) +
                  coefficients[1] / coefficients[4];
  const float r =
      shift * (shift * ((-3.f * shift) * shift + quadratic) - coefficients[1] / coefficients[4]) +
      coefficients[0] / coefficients[4];
  float resolvent[4];
  resolvent[0] = (4.f * r) * p - q * q;
  resolvent[1] = -8.f * r;
  resolvent[2] = -4.f * p;
  resolvent[3] = 8.f;
  float cubicRoots[4];
  const uint cubicCount = SolveCubic(resolvent, cubicRoots);
  if (cubicCount != 0) {
    const float y = cubicRoots[cubicCount - 1];
    const float squaredU = 2.f * y - p;
    const float u = SqrtF(squaredU);
    float v;
    if (u == 0.f) {
      const float discriminant = y * y - r;
      if (discriminant < 0.f) {
        return 0;
      }
      v = SqrtF(discriminant);
    } else {
      v = q / (2.f * u);
    }

    const float firstDiscriminant = -(4.f * (y + v) - squaredU);
    const float secondDiscriminant = -(4.f * (y - v) - squaredU);
    if (firstDiscriminant >= 0.f) {
      const float root = SqrtF(firstDiscriminant);
      roots[count++] = 0.5f * (u - root) - shift;
      roots[count++] = 0.5f * (u + root) - shift;
    }
    if (secondDiscriminant >= 0.f) {
      const float root = SqrtF(secondDiscriminant);
      roots[count++] = 0.5f * (-u - root) - shift;
      roots[count++] = 0.5f * (-u + root) - shift;
    }

    for (uint i = 0; i < count; ++i) {
      const float& x = roots[i];
      const float slope =
          x * (2.f * coefficients[2] + x * (3.f * coefficients[3] + coefficients[4] * (4.f * x)));
      const float derivative = coefficients[1] + slope;
      if (derivative != 0.f) {
        roots[i] = x - (x * (x * (x * (coefficients[4] * x + coefficients[3]) + coefficients[2]) +
                             coefficients[1]) +
                        coefficients[0]) /
                           derivative;
      }
    }

    if (count > 2) {
      if (roots[2] < roots[0]) {
        Swap(roots[0], roots[2]);
      }
      if (roots[3] < roots[1]) {
        Swap(roots[1], roots[3]);
      }
      if (roots[1] < roots[0]) {
        Swap(roots[0], roots[1]);
      }
      if (roots[3] < roots[2]) {
        Swap(roots[2], roots[3]);
      }
      if (roots[2] < roots[1]) {
        Swap(roots[1], roots[2]);
      }
    }
  }
  return count;
}

int CMath::SolveCubicDouble(double c0, double c1, double c2, double c3, double epsilon,
                            double* roots) {
  if (fabs(c3) <= epsilon) {
    if (fabs(c2) <= epsilon) {
      if (fabs(c1) >= epsilon) {
        roots[0] = -c0 / c1;
        return 1;
      }
      return -1;
    }

    double discriminant = c1 * c1 - (4.0 * c0) * c2;
    if (fabs(discriminant) <= epsilon) {
      discriminant = 0.0;
    }
    if (discriminant < 0.0) {
      return 0;
    }
    const double scale = 0.5 / c2;
    if (discriminant > 0.0) {
      const double root = SqrtD(discriminant);
      roots[0] = scale * (-c1 - root);
      roots[1] = scale * (-c1 + root);
      return 2;
    }
    roots[0] = -scale * c1;
    return 1;
  }

  const double inverse = 1.0 / c3;
  const double a = c2 * inverse;
  const double b = c1 * inverse;
  const double c = c0 * inverse;
  const double shift = (1.0 / 3.0) * a;
  const double p = -(a * shift - b);
  double q = 0.5 * ((1.0 / 27.0) * (a * ((2.0 * a) * a - 9.0 * b)) + c);
  double discriminant = q * q + (1.0 / 27.0) * (p * (p * p));
  if (fabs(discriminant) <= epsilon) {
    discriminant = 0.0;
  }

  if (discriminant > 0.0) {
    const double root = SqrtD(discriminant);
    q = -q;
    double term = q + root;
    if (term >= 0.0) {
      roots[0] = pow(term, 1.0 / 3.0);
    } else {
      roots[0] = -pow(-term, 1.0 / 3.0);
    }
    term = q - root;
    if (term >= 0.0) {
      roots[0] += pow(term, 1.0 / 3.0);
    } else {
      roots[0] -= pow(-term, 1.0 / 3.0);
    }
    roots[0] -= shift;
    return 1;
  }
  if (discriminant < 0.0) {
    const double radius = SqrtD((-1.0 / 3.0) * p);
    const double angle = (1.0 / 3.0) * atan2(SqrtD(-discriminant), -q);
    const double cosine = cos(angle);
    const double sine = sin(angle);
    roots[0] = (2.0 * radius) * cosine - shift;
    roots[1] = -radius * (skSqrtThree * sine + cosine) - shift;
    roots[2] = -radius * (-(skSqrtThree * sine - cosine)) - shift;
    return 3;
  }

  const double root = q >= 0.0 ? -pow(q, 1.0 / 3.0) : pow(-q, 1.0 / 3.0);
  roots[0] = 2.0 * root - shift;
  roots[1] = -root - shift;
  return 2;
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

template < typename T >
void CMath::Swap(T& a, T& b) {
  T tmp = a;
  a = b;
  b = tmp;
}
