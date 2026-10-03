#ifndef _CPATHFINDPOINTSEARCH
#define _CPATHFINDPOINTSEARCH

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/PathFinding/CPathFindPointSearchFilter.hpp"
#include "rstl/reserved_vector.hpp"

class CPFArea;
class CPFPoint;

class CPathFindPointSearch {
public:
  explicit CPathFindPointSearch(CPFArea* area);
  int Search(const CPFPoint& source, const CPFPoint& destination);
  int FindClosestPhysicalPoint(const CVector3f& position, int& point,
                               const CPathFindPointSearchFilter& filter) const;
  CVector3f GetSplinePoint(int waypoint, float t) const; // Guessed name

private:
  int SearchInternal(const CPFPoint& source, const CPFPoint& destination); // Guessed name
  float Heuristic(int point, const CVector3f& destination) const;          // Guessed name

  CPFArea* mArea;                                    // Guessed name
  rstl::reserved_vector< CVector3f, 32 > mWaypoints; // Guessed name
};
CHECK_SIZEOF(CPathFindPointSearch, 0x188)

#endif
