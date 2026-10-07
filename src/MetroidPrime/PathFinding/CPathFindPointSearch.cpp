#include "MetroidPrime/PathFinding/CPathFindPointSearch.hpp"

#include "MetroidPrime/PathFinding/CPFPointSearchState.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"

#include "Kyoto/Math/CMath.hpp"

#include "rstl/algorithm.hpp"
#include <float.h>

namespace {
const float kMaxPointDistanceSq = FLT_MAX;

typedef CPFPointSearchState::SPointData PointData;
typedef rstl::vector< PointData* >::iterator OpenPointIterator;

struct SPointCompare { // Guessed name
  bool operator()(const PointData* lhs, const PointData* rhs) const {
    return lhs->mPathCost + lhs->mHeuristic > rhs->mPathCost + rhs->mHeuristic;
  }
};

// Guessed name
void AdjustPointHeap(OpenPointIterator first, int length, int hole, PointData* const& point,
                     SPointCompare compare) {
  int child = 2 * (hole + 1);
  while (child <= length) {
    if (child == length || compare(*(first + child), *(first + child - 1))) {
      --child;
    }
    if (!compare(point, *(first + child))) {
      break;
    }
    *(first + hole) = *(first + child);
    hole = child;
    child = 2 * (child + 1);
  }
  *(first + hole) = point;
}

// Guessed name
void PopPointHeap(OpenPointIterator first, OpenPointIterator last, SPointCompare compare) {
  const int length = last - first;
  if (length < 2) {
    return;
  }
  if (length == 2) {
    rstl::swap(*first, *--last);
  } else {
    PointData* point = *--last;
    *last = *first;
    AdjustPointHeap(first, length - 1, 0, point, compare);
  }
}

// Guessed name
void PushPointHeap(OpenPointIterator first, OpenPointIterator last, SPointCompare compare) {
  const int length = last - first;
  if (length < 2) {
    return;
  }

  int hole = length - 1;
  int parent = (length - 2) / 2;
  PointData* point = *(first + hole);
  if (!compare(*(first + parent), point)) {
    return;
  }
  do {
    *(first + hole) = *(first + parent);
    hole = parent;
    parent = (parent - 1) / 2;
  } while (hole > 0 && compare(*(first + parent), point));
  *(first + hole) = point;
}
} // namespace

CPFPointSearchState::CPFPointSearchState(int pointCount)
: mPointCount(pointCount), mPointData(pointCount, SPointData()) {
  mOpenPoints.reserve(pointCount);
}

void CPFPointSearchState::Reset() {
  mPointData.assign(mPointCount, SPointData());
  mOpenPoints.clear();
}

CPFPointSearchState::SPointData& CPFPointSearchState::GetPointData(int point) {
  return mPointData[point];
}

void CPFPointSearchState::PushOpenPoint(SPointData* point) {
  mOpenPoints.push_back_unsafe(point);
  PushPointHeap(mOpenPoints.begin(), mOpenPoints.end(), SPointCompare());
}

CPFPointSearchState::SPointData* CPFPointSearchState::PopOpenPoint() {
  SPointData* point = mOpenPoints.front();
  PopPointHeap(mOpenPoints.begin(), mOpenPoints.end(), SPointCompare());
  mOpenPoints.pop_back();
  return point;
}

void CPFPointSearchState::UpdateOpenPoint(const SPointData& point) {
  for (OpenPointIterator it = mOpenPoints.begin(); it != mOpenPoints.end(); ++it) {
    if ((*it)->mPointIndex == point.mPointIndex) {
      PushPointHeap(mOpenPoints.begin(), it + 1, SPointCompare());
    }
  }
}

CPathFindPointSearchFilter::CPathFindPointSearchFilter(float maxDistance, uint flags,
                                                       int connectedPoint)
: mMaxDistance(maxDistance), mFlags(flags), mConnectedPoint(connectedPoint) {}

CPathFindPointSearch::CPathFindPointSearch(CPFArea* area) : mArea(area) {}

CPathFindPointSearch::EClosestPointResult
CPathFindPointSearch::FindClosestPhysicalPoint(const CVector3f& position, int& point,
                                               const CPathFindPointSearchFilter& filter) const {
  CPFArea* area = mArea;
  if (area) {
    float closestDistanceSq = kMaxPointDistanceSq;
    bool found = false;
    const uint flags = filter.GetFlags();
    for (int i = 0; i < area->GetNumPoints(); ++i) {
      const CPFPoint& candidate = area->GetPoint(i);
      if (flags && !(flags & candidate.GetFlags())) {
        continue;
      }
      if (filter.GetConnectedPoint() != -1 &&
          !area->PointPathExists(i, filter.GetConnectedPoint())) {
        continue;
      }
      const CVector3f delta = candidate.GetPosition() - position;
      const float distanceSq = CVector3f::Dot(delta, delta);
      if (distanceSq < closestDistanceSq) {
        closestDistanceSq = distanceSq;
        point = i;
        found = true;
      }
    }
    if (found) {
      if (closestDistanceSq < filter.GetMaxDistance() * filter.GetMaxDistance()) {
        return kCPR_Success;
      }
      return kCPR_OutOfRange;
    }
  }
  return kCPR_NoPoint;
}

CPathFindPointSearch::EResult CPathFindPointSearch::Search(const CPFPoint& source,
                                                           const CPFPoint& destination) {
  mWaypoints.clear();
  if (mArea && mArea->PointPathExists(&source, &destination)) {
    return SearchInternal(destination, source);
  }
  return kR_NoConnection;
}

CVector3f CPathFindPointSearch::GetSplinePoint(int waypoint, float t) const {
  CVector3f point = CVector3f::Zero();
  const int count = mWaypoints.size();
  if (count != 0) {
    if (waypoint < 0) {
      point = mWaypoints.front();
    } else if (waypoint >= count - 1) {
      point = mWaypoints.back();
    } else {
      const int previous = waypoint > 0 ? waypoint - 1 : 0;
      const int next = waypoint < count - 2 ? waypoint + 2 : count - 1;
      point = CMath::GetCatmullRomSplinePoint(mWaypoints[previous], mWaypoints[waypoint],
                                              mWaypoints[waypoint + 1], mWaypoints[next], t);
    }
  }
  return point;
}

float CPathFindPointSearch::Heuristic(int point, const CVector3f& destination) const {
  const CVector3f delta = destination - mArea->GetPoint(point).GetPosition();
  return CMath::FastSqrtF(CVector3f::Dot(delta, delta));
}

CPathFindPointSearch::EResult CPathFindPointSearch::SearchInternal(const CPFPoint& source,
                                                                   const CPFPoint& destination) {
  CPFPointSearchState* state = mArea->GetPointSearchState();
  if (state) {
    state->Reset();
    const int sourceIndex = mArea->GetPointIndex(source);
    const int destinationIndex = mArea->GetPointIndex(destination);
    const CVector3f& destinationPosition = mArea->GetPoint(destinationIndex).GetPosition();
    PointData& sourceData = state->GetPointData(sourceIndex);
    sourceData.mPointIndex = sourceIndex;
    sourceData.mPathCost = 0.f;
    sourceData.mHeuristic = Heuristic(sourceIndex, destinationPosition);
    sourceData.mParent = nullptr;
    sourceData.mDiscovered = true;
    sourceData.mClosed = false;
    state->PushOpenPoint(&sourceData);

    while (state->HasOpenPoints()) {
      PointData* current = state->PopOpenPoint();
      if (current->mPointIndex == destinationIndex) {
        for (PointData* point = current; point && mWaypoints.size() < mWaypoints.capacity();
             point = point->mParent) {
          mWaypoints.push_back(mArea->GetPoint(point->mPointIndex).GetPosition());
        }
        return kR_Success;
      }

      const CPFPoint& point = mArea->GetPoint(current->mPointIndex);
      for (int i = 0; i < point.GetNumLinks(); ++i) {
        const int next = point.GetLink(i);
        if (current->mParent && current->mParent->mPointIndex == next) {
          continue;
        }
        const float pathCost = current->mPathCost + point.GetLinkCost(i);
        const float heuristic = Heuristic(next, destinationPosition);
        PointData& data = state->GetPointData(next);
        if (data.mDiscovered && pathCost > data.mPathCost) {
          continue;
        }
        if (data.mClosed && pathCost > data.mPathCost) {
          continue;
        }
        data.mClosed = false;
        data.mParent = current;
        data.mPathCost = pathCost;
        data.mHeuristic = heuristic;
        data.mPointIndex = next;
        if (data.mDiscovered) {
          state->UpdateOpenPoint(data);
        } else {
          state->PushOpenPoint(&data);
          data.mDiscovered = true;
        }
      }
      current->mClosed = true;
    }
  }
  return kR_NoPath;
}
