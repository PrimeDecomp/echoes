#include "MetroidPrime/PathFinding/CPathFindPointSearch.hpp"

CPathFindPointSearch::CPathFindPointSearch(CPFArea* area) {}

int CPathFindPointSearch::Search(const CPFPoint& source, const CPFPoint& destination) {}

int CPathFindPointSearch::FindClosestPhysicalPoint(const CVector3f& position, int& point,
                                                   const CPathFindPointSearchFilter& filter) const {
}

CVector3f CPathFindPointSearch::GetSplinePoint(int waypoint, float t) const {}

int CPathFindPointSearch::SearchInternal(const CPFPoint& source, const CPFPoint& destination) {}

float CPathFindPointSearch::Heuristic(int point, const CVector3f& destination) const {}

CPathFindPointSearchFilter::CPathFindPointSearchFilter(float maxDistance, uint flags,
                                                       int connectedPoint) {}
