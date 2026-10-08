#ifndef _CSCOPEDPROFILER
#define _CSCOPEDPROFILER

#include "rstl/string.hpp"

// Guessed class/name. Native callers construct stack scopes around named profiling regions;
// the release build keeps only empty stubs.
class CScopedProfiler {
public:
  CScopedProfiler(const rstl::string& name, bool enabled);
  static void BeginFrame(); // Guessed name; empty stub next to the constructor.
};

#endif // _CSCOPEDPROFILER
