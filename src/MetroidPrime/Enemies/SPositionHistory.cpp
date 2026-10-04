#include "MetroidPrime/Enemies/SPositionHistory.hpp"

SPositionHistory::SPositionHistory(float mag) : mMagSquared(mag * mag) {}

void SPositionHistory::AddValue(CVector3f pos) {
  if (mValues.size() >= mValues.capacity()) {
    return;
  }

  if (mValues.empty()) {
    mValues.push_back(pos);
    return;
  }

  const CVector3f diff = pos - mValues.back();
  if (diff.MagSquared() > mMagSquared) {
    mValues.push_back(pos);
  }
}
