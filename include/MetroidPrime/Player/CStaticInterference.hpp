#ifndef _CSTATICINTERFERENCE
#define _CSTATICINTERFERENCE

#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/vector.hpp"

class CStateManager;

class CStaticInterferenceSource {
public:
  CStaticInterferenceSource(TUniqueId id, float magnitude, float timeLeft)
  : mId(id), mMagnitude(magnitude), mTimeLeft(timeLeft) {}

  const TUniqueId GetSourceId() const { return mId; }
  const float GetIntensity() const { return mMagnitude; }
  void SetIntensity(const float v) { mMagnitude = v; }
  const float GetTime() const { return mTimeLeft; }
  void SetTime(const float v) { mTimeLeft = v; }

private:
  TUniqueId mId;
  float mMagnitude;
  float mTimeLeft;
};

namespace rstl {
template <>
struct is_trivially_destructible< CStaticInterferenceSource > {
  enum { value = true };
};
} // namespace rstl

class CStaticInterference {
public:
  explicit CStaticInterference(int sourceCount);
  ~CStaticInterference();

  void AddSource(TUniqueId id, float magnitude, float duration);
  void RemoveSource(TUniqueId id);
  void Update(const CStateManager&, float dt);
  float GetTotalInterference() const;

private:
  rstl::vector< CStaticInterferenceSource > sources;
};

#endif // _CSTATICINTERFERENCE
