#ifndef _CEULERANGLES
#define _CEULERANGLES

#include "Kyoto/Math/CVector3f.hpp"

class CQuaternion;

class CEulerAngles : public CVector3f {
public:
  CEulerAngles(float roll, float pitch, float yaw) : CVector3f(roll, pitch, yaw) {}

  static CEulerAngles FromQuaternion(const CQuaternion& quat);
  float GetYaw() const { return GetZ(); }
};
CHECK_SIZEOF(CEulerAngles, 0xc)

#endif // _CEULERANGLES
