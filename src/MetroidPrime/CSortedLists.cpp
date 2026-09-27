#include "MetroidPrime/CSortedLists.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "MetroidPrime/CActor.hpp"
#include "rstl/algorithm.hpp"

namespace SL {
static inline float GetPointForSL(ESortedLists list, const CAABox& box) {
  return list < kSL_MaxX ? box.GetMinPoint()[int(list)] : box.GetMaxPoint()[int(list) - kSL_MaxX];
}

SNode::SNode() : mActor(nullptr), mBox(CAABox::Identity()), mNext(-1), mPopulated(false) {}

SNode::SNode(CActor* actor, const CAABox& box)
: mActor(actor), mBox(box), mNext(-1), mPopulated(true) {
  for (int i = 0; i < 6; ++i) {
    mSelfIdxs[i] = -1;
  }
}

CSortedListManager::CSortedListManager() { Reset(); }

void CSortedListManager::Reset() {
  const SNode node;
  for (uint i = 0; i < 1024; ++i) {
    mNodes[i] = node;
  }

  const SSortedList sorted;
  for (int i = 0; i < 6; ++i) {
    mSortedLists[i] = sorted;
  }
}

bool CSortedListManager::ActorInLists(const CActor* actor) const {
  return actor != nullptr && mNodes[actor->GetUniqueId().Value()].mPopulated;
}

short CSortedListManager::FindInListLower(ESortedLists list, float value) const {
  const SSortedList& sorted = mSortedLists[list];
  int first = 0;
  int count = sorted.mSize;
  while (count > 0) {
    const int half = count / 2;
    const int middle = first + half;
    if (GetPointForSL(list, mNodes[sorted.mIds[middle]].mBox) < value) {
      first = middle + 1;
      count -= half + 1;
    } else {
      count = half;
    }
  }
  return first;
}

short CSortedListManager::FindInListUpper(ESortedLists list, float value) const {
  const SSortedList& sorted = mSortedLists[list];
  int first = 0;
  int count = sorted.mSize;
  while (count > 0) {
    const int half = count / 2;
    const int middle = first + half;
    if (GetPointForSL(list, mNodes[sorted.mIds[middle]].mBox) <= value) {
      first = middle + 1;
      count -= half + 1;
    } else {
      count = half;
    }
  }
  return first;
}

void CSortedListManager::InsertInList(ESortedLists list, SNode& node) {
  SSortedList& sorted = mSortedLists[list];
  const short first = FindInListLower(list, GetPointForSL(list, node.mBox));
  for (int i = sorted.mSize; i > first; --i) {
    mNodes[sorted.mIds[i - 1]].mSelfIdxs[list] = i;
    sorted.mIds[i] = sorted.mIds[i - 1];
  }
  sorted.mIds[first] = node.mActor->GetUniqueId().Value();
  ++sorted.mSize;
  node.mSelfIdxs[list] = first;
}

void CSortedListManager::RemoveFromList(ESortedLists list, short index) {
  SSortedList& sorted = mSortedLists[list];
  for (int i = index; i < int(sorted.mSize) - 1; ++i) {
    mNodes[sorted.mIds[i + 1]].mSelfIdxs[list] = i;
    sorted.mIds[i] = sorted.mIds[i + 1];
  }
  --sorted.mSize;
}

void CSortedListManager::MoveInList(ESortedLists list, short index) {
  SSortedList& sorted = mSortedLists[list];
  while (true) {
    if (index > 0 && GetPointForSL(list, mNodes[sorted.mIds[index - 1]].mBox) >
                         GetPointForSL(list, mNodes[sorted.mIds[index]].mBox)) {
      mNodes[sorted.mIds[index - 1]].mSelfIdxs[list] = index;
      mNodes[sorted.mIds[index]].mSelfIdxs[list] = index - 1;
      rstl::swap(sorted.mIds[index - 1], sorted.mIds[index]);
      --index;
    } else {
      if (index >= int(sorted.mSize) - 1 ||
          !(GetPointForSL(list, mNodes[sorted.mIds[index + 1]].mBox) <
            GetPointForSL(list, mNodes[sorted.mIds[index]].mBox))) {
        return;
      }
      mNodes[sorted.mIds[index + 1]].mSelfIdxs[list] = index;
      mNodes[sorted.mIds[index]].mSelfIdxs[list] = index + 1;
      rstl::swap(sorted.mIds[index + 1], sorted.mIds[index]);
      ++index;
    }
  }
}

void CSortedListManager::Insert(CActor* actor, const CAABox& box) {
  if (mNodes[actor->GetUniqueId().Value()].mPopulated) {
    Move(actor, box);
    return;
  }

  SNode node(actor, box);
  InsertInList(kSL_MinX, node);
  InsertInList(kSL_MaxX, node);
  InsertInList(kSL_MinY, node);
  InsertInList(kSL_MaxY, node);
  InsertInList(kSL_MinZ, node);
  InsertInList(kSL_MaxZ, node);
  mNodes[actor->GetUniqueId().Value()] = node;
}

void CSortedListManager::Remove(const CActor* actor) {
  if (actor == nullptr) {
    return;
  }

  SNode& node = mNodes[actor->GetUniqueId().Value()];
  if (node.mPopulated) {
    RemoveFromList(kSL_MinX, node.mSelfIdxs[kSL_MinX]);
    RemoveFromList(kSL_MaxX, node.mSelfIdxs[kSL_MaxX]);
    RemoveFromList(kSL_MinY, node.mSelfIdxs[kSL_MinY]);
    RemoveFromList(kSL_MaxY, node.mSelfIdxs[kSL_MaxY]);
    RemoveFromList(kSL_MinZ, node.mSelfIdxs[kSL_MinZ]);
    RemoveFromList(kSL_MaxZ, node.mSelfIdxs[kSL_MaxZ]);
    node.mPopulated = false;
  }
}

void CSortedListManager::Move(const CActor* actor, const CAABox& box) {
  SNode& node = mNodes[actor->GetUniqueId().Value()];
  node.mBox = box;
  MoveInList(kSL_MinX, node.mSelfIdxs[kSL_MinX]);
  MoveInList(kSL_MaxX, node.mSelfIdxs[kSL_MaxX]);
  MoveInList(kSL_MinY, node.mSelfIdxs[kSL_MinY]);
  MoveInList(kSL_MaxY, node.mSelfIdxs[kSL_MaxY]);
  MoveInList(kSL_MinZ, node.mSelfIdxs[kSL_MinZ]);
  MoveInList(kSL_MaxZ, node.mSelfIdxs[kSL_MaxZ]);
}

void CSortedListManager::AddToLinkedList(short nodeId, short& headId, short& tailId) const {
  if (headId == -1) {
    mNodes[nodeId].mNext = -1;
    tailId = headId = nodeId;
    return;
  }
  if (mNodes[nodeId].mNext != -1 || nodeId == tailId) {
    return;
  }

  mNodes[nodeId].mNext = headId;
  headId = nodeId;
}

short CSortedListManager::CalculateIntersections(ESortedLists minList, ESortedLists maxList,
                                                 short minBegin, short minEnd, short maxBegin,
                                                 short maxEnd, ESortedLists otherMinA,
                                                 ESortedLists otherMaxA, ESortedLists otherMinB,
                                                 ESortedLists otherMaxB, const CAABox& box) const {
  // TODO: Build the candidate chain from the selected axis and filter the other two axes.
  return -1;
}

short CSortedListManager::ConstructIntersectionArray(const CAABox& box) const {
  // TODO: Select the least-populated axis ranges and construct the intersection chain.
  return -1;
}

void CSortedListManager::BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearListOut,
                                       const CAABox& box, const CMaterialFilter& filter,
                                       const CActor* actor) const {
  for (short id = ConstructIntersectionArray(box); id != -1;) {
    const SNode& node = mNodes[id];
    const CActor* candidate = node.mActor;
    if (actor != candidate && filter.Passes(candidate->GetMaterialList())) {
      nearListOut.push_back(candidate->GetUniqueId());
    }
    id = node.mNext;
    node.mNext = -1;
  }
}

void CSortedListManager::BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearListOut,
                                       const CActor& actor, const CAABox& box) const {
  const CMaterialFilter& filter = actor.GetMaterialFilter();
  const CMaterialList& materials = actor.GetMaterialList();
  for (short id = ConstructIntersectionArray(box); id != -1;) {
    const SNode& node = mNodes[id];
    const CActor* candidate = node.mActor;
    if (&actor != candidate && filter.Passes(candidate->GetMaterialList()) &&
        candidate->GetMaterialFilter().Passes(materials)) {
      nearListOut.push_back(candidate->GetUniqueId());
    }
    id = node.mNext;
    node.mNext = -1;
  }
}

void CSortedListManager::BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearListOut,
                                       const CVector3f& pos, const CVector3f& dir, float magnitude,
                                       const CMaterialFilter& filter, const CActor* actor) const {
  // TODO: Construct the segment bounds (including the zero-length fallback) and query them.
}
} // namespace SL
