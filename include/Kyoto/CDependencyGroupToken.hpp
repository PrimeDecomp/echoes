#ifndef _CDEPENDENCYGROUPTOKEN
#define _CDEPENDENCYGROUPTOKEN
#include "Kyoto/TToken.hpp"
#include "rstl/vector.hpp"
class CDependencyGroup;
class IObjectStore;

// Guessed name. Owns the group token and its dependency tokens.
class CDependencyGroupToken {
public:
  CDependencyGroupToken(const TToken< CDependencyGroup >& group, IObjectStore& store);

  void Lock();
  void Unlock();
  bool IsLocked() const;
  bool IsLoaded();

private:
  TToken< CDependencyGroup > mGroup;
  rstl::vector< CToken > mDependencies;
  uint mLockCount;
  bool mLoaded : 1;
};
CHECK_SIZEOF(CDependencyGroupToken, 0x20)
#endif // _CDEPENDENCYGROUPTOKEN
