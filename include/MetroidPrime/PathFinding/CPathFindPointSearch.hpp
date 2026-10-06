#ifndef _CPATHFINDPOINTSEARCH
#define _CPATHFINDPOINTSEARCH

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/PathFinding/CPathFindPointSearchFilter.hpp"
#include "rstl/reserved_vector.hpp"

class CPFArea;
class CPFPoint;

class CPathFindPointSearch {
public:
  // Guessed result names, derived from native search and distance-check branches.
  enum EResult { kR_Success, kR_NoPath, kR_NoConnection };
  enum EClosestPointResult { kCPR_Success, kCPR_OutOfRange, kCPR_NoPoint };

  explicit CPathFindPointSearch(CPFArea* area);
  EResult Search(const CPFPoint& source, const CPFPoint& destination);
  EClosestPointResult FindClosestPhysicalPoint(const CVector3f& position, int& point,
                                              const CPathFindPointSearchFilter& filter) const;
  CVector3f GetSplinePoint(int waypoint, float t) const; // Guessed name

private:
  EResult SearchInternal(const CPFPoint& source, const CPFPoint& destination); // Guessed name
  float Heuristic(int point, const CVector3f& destination) const;          // Guessed name

  CPFArea* mArea;                                    // Guessed name
  rstl::reserved_vector< CVector3f, 32 > mWaypoints; // Guessed name
};
CHECK_SIZEOF(CPathFindPointSearch, 0x188)

#endif
