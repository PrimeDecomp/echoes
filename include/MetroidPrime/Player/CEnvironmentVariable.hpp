#ifndef _CENVIRONMENTVARIABLE
#define _CENVIRONMENTVARIABLE

#include "types.h"

class CBitStreamReader;
class CBitStreamWriter;

// Guessed name
class CEnvironmentVariable {
public:
  CEnvironmentVariable(int minimum, int maximum, int value);
  CEnvironmentVariable(int minimum, int maximum, CBitStreamReader& in);

  int GetValue() const { return mValue; }
  int GetMinimum() const { return mMin; }
  int GetMaximum() const { return mMax; }

  void Set(int value);
  void PutTo(CBitStreamWriter& out) const;

private:
  void ClampToMinMax();
  static uint GetBitCount(uint value);

  int mMin;
  int mMax;
  int mValue;
};
CHECK_SIZEOF(CEnvironmentVariable, 0xc)

#endif // _CENVIRONMENTVARIABLE
