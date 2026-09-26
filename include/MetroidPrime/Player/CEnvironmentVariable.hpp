#ifndef _CENVIRONMENTVARIABLE
#define _CENVIRONMENTVARIABLE

#include "types.h"

// Guessed name
class CEnvironmentVariable {
public:
  int GetValue() const { return mValue; }

  void Set(int value);

private:
  int mMin;
  int mMax;
  int mValue;
};
CHECK_SIZEOF(CEnvironmentVariable, 0xc)

#endif // _CENVIRONMENTVARIABLE
