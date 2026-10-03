#ifndef _CPATHFINDPOINTSEARCHFILTER
#define _CPATHFINDPOINTSEARCHFILTER

#include <types.h>

class CPathFindPointSearchFilter {
public:
  CPathFindPointSearchFilter(float maxDistance, uint flags, int connectedPoint);

private:
  float mMaxDistance;  // Guessed name
  uint mFlags;         // Guessed name
  int mConnectedPoint; // Guessed name
};
CHECK_SIZEOF(CPathFindPointSearchFilter, 0xc)

#endif
