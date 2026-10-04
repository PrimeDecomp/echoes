#ifndef _SPOSITIONHISTORY
#define _SPOSITIONHISTORY

#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/reserved_vector.hpp"

// Guessed Echoes name/signatures, correlated with Prime and the ElitePirate member.
struct SPositionHistory {
private:
  float mMagSquared;
  rstl::reserved_vector< CVector3f, 16 > mValues;

public:
  explicit SPositionHistory(float mag);
  void AddValue(CVector3f pos);
  void Clear() { mValues.clear(); }
};
CHECK_SIZEOF(SPositionHistory, 0xc8)

#endif // _SPOSITIONHISTORY
