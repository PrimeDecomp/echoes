#ifndef _CSTATICINTERFERENCE
#define _CSTATICINTERFERENCE

#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/vector.hpp"

class CStateManager;

class CStaticInterferenceSource {
  TUniqueId x0_id;
  float x4_magnitude;
  float x8_timeLeft;
};

class CStaticInterference {
public:
  explicit CStaticInterference(int sourceCount);
  ~CStaticInterference();
  void Update(const CStateManager&, float dt);

private:
  rstl::vector< CStaticInterferenceSource > sources;
};

namespace rstl {
template <>
struct is_trivially_destructible< CStaticInterferenceSource > {
  enum { value = true };
};
} // namespace rstl

#endif // _CSTATICINTERFERENCE
