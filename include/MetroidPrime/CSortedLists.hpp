#ifndef _CSORTEDLISTS
#define _CSORTEDLISTS

#include "types.h"

#include "Kyoto/Math/CAABox.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/reserved_vector.hpp"

class CActor;
class CMaterialFilter;

namespace SL {
enum ESortedLists {
  kSL_MinX,
  kSL_MinY,
  kSL_MinZ,
  kSL_MaxX,
  kSL_MaxY,
  kSL_MaxZ,
};

struct SNode {
  CActor* mActor;
  CAABox mBox;
  short mSelfIdxs[6];  // Position in each coordinate-sorted list.
  mutable short mNext; // Scratch intersection-chain index, not a TUniqueId.
  bool mPopulated;

  SNode();
  SNode(CActor* actor, const CAABox& box);
};
CHECK_SIZEOF(SNode, 0x2c)

struct SSortedList {
  short mIds[1024]; // Node indices, without TUniqueId version bits.
  int mSize;

  SSortedList() : mSize(0) {
    for (int i = 0; i < 1024; ++i) {
      mIds[i] = -1;
    }
  }
};
CHECK_SIZEOF(SSortedList, 0x804)

class CSortedListManager {
public:
  CSortedListManager();
  void Reset();
  bool ActorInLists(const CActor* actor) const;
  short FindInListLower(ESortedLists list, float value) const;
  short FindInListUpper(ESortedLists list, float value) const;
  void InsertInList(ESortedLists list, SNode& node);
  void RemoveFromList(ESortedLists list, short index);
  void MoveInList(ESortedLists list, short index);
  void Insert(CActor* actor, const CAABox& box);
  void Remove(const CActor* actor);
  void Move(const CActor* actor, const CAABox& box);
  void AddToLinkedList(short nodeId, short& headId, short& tailId) const;
  short CalculateIntersections(ESortedLists minList, ESortedLists maxList, short minBegin,
                               short minEnd, short maxBegin, short maxEnd, ESortedLists otherMinA,
                               ESortedLists otherMaxA, ESortedLists otherMinB,
                               ESortedLists otherMaxB, const CAABox& box) const;
  short ConstructIntersectionArray(const CAABox& box) const;
  void BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearListOut, const CAABox& box,
                     const CMaterialFilter& filter, const CActor* actor) const;
  void BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearListOut, const CActor& actor,
                     const CAABox& box) const;
  void BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearListOut, const CVector3f& pos,
                     const CVector3f& dir, float magnitude, const CMaterialFilter& filter,
                     const CActor* actor) const;

private:
  SNode mNodes[1024];
  SSortedList mSortedLists[6];
};
CHECK_SIZEOF(CSortedListManager, 0xe018)
} // namespace SL

#endif // _CSORTEDLISTS
