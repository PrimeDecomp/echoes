#include "MetroidPrime/ScriptLoader/Structs/SLdrSequenceConnections.hpp"

#include "rstl/math.hpp"

SLdrSequenceConnections::SLdrSequenceConnections() : mConnections() {}

SLdrSequenceConnections::SLdrSequenceConnections(CInputStream& input) : mConnections(input) {}

rstl::pair< float, float >
FindMinMaxConnectionTimes(const rstl::vector< SLdrConnection >& connections) {
  float minTime = 3.402823466e+38f;
  float maxTime = 1.175494351e-38f; // Smallest positive normal float, even for an empty schedule.

  for (rstl::vector< SLdrConnection >::const_iterator connection = connections.begin();
       connection != connections.end(); ++connection) {
    const rstl::vector< float >& times = connection->mActivation.first;
    for (rstl::vector< float >::const_iterator time = times.begin(); time != times.end(); ++time) {
      minTime = rstl::min_val(minTime, *time);
      maxTime = rstl::max_val(maxTime, *time);
    }
  }

  return rstl::pair< float, float >(minTime, maxTime);
}
