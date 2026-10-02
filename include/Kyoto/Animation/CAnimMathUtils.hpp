#ifndef _CANIMMATHUTILS
#define _CANIMMATHUTILS

class CQuaternion;

class CAnimMathUtils {
public:
  static const float kInterpolationThreshold;
  static bool sUseFastSlerp; // Guessed name.

  static CQuaternion Slerp(const CQuaternion& start, const CQuaternion& end, float t);
  static CQuaternion SlerpLocal(const CQuaternion& start, const CQuaternion& end, float t);
};

#endif // _CANIMMATHUTILS
