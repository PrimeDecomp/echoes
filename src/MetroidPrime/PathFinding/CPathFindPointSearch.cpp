#include "MetroidPrime/PathFinding/CPathFindPointSearch.hpp"
#include "MetroidPrime/PathFinding/CPFPointSearchState.hpp"

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

CPFPointSearchState::CPFPointSearchState(int pointCount) {}

void CPFPointSearchState::UpdateOpenPoint(const SPointData& point) {}

CPFPointSearchState::SPointData* CPFPointSearchState::PopOpenPoint() {}

void CPFPointSearchState::PushOpenPoint(SPointData* point) {}

CPFPointSearchState::SPointData& CPFPointSearchState::GetPointData(int point) {}

void CPFPointSearchState::Reset() {}

namespace {
typedef CPFPointSearchState::SPointData PointData;
typedef rstl::vector< PointData* >::iterator OpenPointIterator;

struct SPointCompare { // Guessed name
  bool operator()(const PointData* lhs, const PointData* rhs) const {}
};

// Guessed name
void PushPointHeap(OpenPointIterator first, OpenPointIterator last, SPointCompare compare) {}

// Guessed name
void PopPointHeap(OpenPointIterator first, OpenPointIterator last, SPointCompare compare) {}

// Guessed name
void AdjustPointHeap(OpenPointIterator first, int length, int hole, PointData* const& point,
                     SPointCompare compare) {}
} // namespace
