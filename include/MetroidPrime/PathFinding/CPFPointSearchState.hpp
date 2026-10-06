#ifndef _CPFPOINTSEARCHSTATE
#define _CPFPOINTSEARCHSTATE

#include "rstl/vector.hpp"

class CPFPointSearchState { // Guessed name
public:
  struct SPointData { // Guessed name
    SPointData()
    : mPointIndex(-1)
    , mParent(nullptr)
    , mPathCost(0.f)
    , mHeuristic(0.f)
    , mDiscovered(false)
    , mClosed(false) {}

    int mPointIndex;      // Guessed name
    SPointData* mParent;  // Guessed name
    float mPathCost;      // Guessed name
    float mHeuristic;     // Guessed name
    bool mDiscovered : 1; // Target-derived: retained after removal from the heap.
    bool mClosed : 1;     // Target-derived: set after expansion.
  };
  typedef char SPointDataSizeCheck[sizeof(SPointData) == 0x14 ? 1 : -1];

  explicit CPFPointSearchState(int pointCount);
  void UpdateOpenPoint(const SPointData& point); // Guessed name
  SPointData* PopOpenPoint();                    // Guessed name
  void PushOpenPoint(SPointData* point);         // Guessed name
  SPointData& GetPointData(int point);           // Guessed name
  void Reset();                                  // Guessed name
  bool HasOpenPoints() const { return !mOpenPoints.empty(); }

private:
  int mPointCount;                         // Guessed name
  rstl::vector< SPointData > mPointData;   // Guessed name
  rstl::vector< SPointData* > mOpenPoints; // Guessed name
};
CHECK_SIZEOF(CPFPointSearchState, 0x24)

#endif
