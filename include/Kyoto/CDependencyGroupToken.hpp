#ifndef _CDEPENDENCYGROUPTOKEN
#define _CDEPENDENCYGROUPTOKEN
#include "Kyoto/TToken.hpp"
#include "rstl/vector.hpp"
class CDependencyGroup;
class IObjectStore;

// Guessed name. G2ME01 80320a90 constructs the group token and its dependency tokens.
class CDependencyGroupToken {
public:
  CDependencyGroupToken(const TToken< CDependencyGroup >& group, IObjectStore& store);

private:
  TToken< CDependencyGroup > x0_group;
  rstl::vector< CToken > x8_dependencies;
  uint x18_; // Loading progress/state, precise meaning unresolved.
  bool x1c_24_ : 1;
};
CHECK_SIZEOF(CDependencyGroupToken, 0x20)
#endif // _CDEPENDENCYGROUPTOKEN
