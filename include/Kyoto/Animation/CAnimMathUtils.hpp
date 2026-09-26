#ifndef _CANIMMATHUTILS
#define _CANIMMATHUTILS

class CQuaternion;

class CAnimMathUtils {
public:
  static CQuaternion Slerp(const CQuaternion& start, const CQuaternion& end, float t);
};

#endif // _CANIMMATHUTILS
