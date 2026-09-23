#ifndef _CLOSEENOUGH
#define _CLOSEENOUGH

#include "types.h"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "math.h"

struct Real32 {
  static inline float Epsilon() { return 1.e-5f; }
};
struct Double {
  static inline double Epsilon() { return 1.e-5; } // TODO check
};
static inline float vector3_epsilon() { return 1.e-4f; }
static inline float vector2_epsilon() { return 1.e-4f; }

class CQuaternion;

bool close_enough(const CQuaternion& a, const CQuaternion& b, float epsilon);
bool operator==(const CQuaternion& a, const CQuaternion& b);
bool close_enough(const CVector2f& a, const CVector2f& b, float epsilon = vector2_epsilon());
bool close_enough(const CVector3f& a, const CVector3f& b, float epsilon = vector3_epsilon());
inline bool close_enough(float a, float b, float epsilon = Real32::Epsilon()) {
  return CMath::AbsF(a - b) < epsilon;
}
inline bool close_enough(double a, double b, double epsilon = Double::Epsilon()) {
  return fabs(a - b) < epsilon;
}

#endif // _CLOSEENOUGH
