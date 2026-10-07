#include "MetroidPrime/CSortedLists.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "MetroidPrime/CActor.hpp"
#include "rstl/algorithm.hpp"

namespace SL {
static inline float GetPointForSL(ESortedLists list, const CAABox& box) {
  // The six list indices address the contiguous min/max floats of the box.
  return box.GetMinPoint()[int(list)];
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
  int count = sorted.mSize;
  int half;
  int first = 0;
  while (count > 0) {
    half = count / 2;
    const int middle = first + half;
    if (GetPointForSL(list, mNodes[sorted.mIds[middle]].mBox) < value) {
      first = middle + 1;
      count = count - half - 1;
    } else {
      count = half;
    }
  }
  return first;
}

short CSortedListManager::FindInListUpper(ESortedLists list, float value) const {
  const SSortedList& sorted = mSortedLists[list];
  int count = sorted.mSize;
  int half;
  int first = 0;
  while (count > 0) {
    half = count / 2;
    const int middle = first + half;
    if (value < GetPointForSL(list, mNodes[sorted.mIds[middle]].mBox)) {
      count = half;
    } else {
      first = middle + 1;
      count = count - half - 1;
    }
  }
  return first;
}

void CSortedListManager::InsertInList(ESortedLists list, SNode& node) {
  SSortedList& sorted = mSortedLists[list];
  const float value = GetPointForSL(list, node.mBox);
  int count = sorted.mSize;
  int half;
  int first = 0;
  while (count > 0) {
    half = count / 2;
    const int middle = first + half;
    if (GetPointForSL(list, mNodes[sorted.mIds[middle]].mBox) < value) {
      first = middle + 1;
      count = count - half - 1;
    } else {
      count = half;
    }
  }

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

void CSortedListManager::MoveInList(ESortedLists list, const short index) {
  SSortedList& sorted = mSortedLists[list];
  short idx = index;
  while (true) {
    if (idx > 0 && GetPointForSL(list, mNodes[sorted.mIds[idx - 1]].mBox) >
                       GetPointForSL(list, mNodes[sorted.mIds[idx]].mBox)) {
      mNodes[sorted.mIds[idx - 1]].mSelfIdxs[list] = idx;
      mNodes[sorted.mIds[idx]].mSelfIdxs[list] = idx - 1;
      rstl::swap(sorted.mIds[idx - 1], sorted.mIds[idx]);
      --idx;
    } else {
      if (idx >= static_cast< int >(sorted.mSize) - 1) {
        return;
      }
      if (!(GetPointForSL(list, mNodes[sorted.mIds[idx + 1]].mBox) <
            GetPointForSL(list, mNodes[sorted.mIds[idx]].mBox))) {
        return;
      }
      mNodes[sorted.mIds[idx + 1]].mSelfIdxs[list] = idx;
      mNodes[sorted.mIds[idx]].mSelfIdxs[list] = idx + 1;
      rstl::swap(sorted.mIds[idx + 1], sorted.mIds[idx]);
      ++idx;
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

void CSortedListManager::AddToLinkedList(const short nodeId, short& headId, short& tailId) const {
  if (headId == -1) {
    mNodes[nodeId].mNext = headId;
    tailId = nodeId;
    headId = nodeId;
    return;
  }
  if (mNodes[nodeId].mNext != -1) {
    return;
  }
  if (nodeId == tailId) {
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
  short headId = -1;
  short tailId = -1;
  for (short i = minBegin; i < minEnd; ++i) {
    AddToLinkedList(mSortedLists[minList].mIds[i], headId, tailId);
  }
  for (short i = maxBegin; i < maxEnd; ++i) {
    AddToLinkedList(mSortedLists[maxList].mIds[i], headId, tailId);
  }

  if (minBegin < static_cast< int >(mSortedLists[maxList].mSize) - maxEnd) {
    for (short i = 0; i < minBegin; ++i) {
      const short id = mSortedLists[minList].mIds[i];
      if (GetPointForSL(maxList, mNodes[id].mBox) > GetPointForSL(maxList, box)) {
        AddToLinkedList(id, headId, tailId);
      }
    }
  } else {
    for (short i = maxEnd; i < static_cast< int >(mSortedLists[maxList].mSize); ++i) {
      const short id = mSortedLists[maxList].mIds[i];
      if (GetPointForSL(minList, mNodes[id].mBox) < GetPointForSL(minList, box)) {
        AddToLinkedList(id, headId, tailId);
      }
    }
  }

  for (short* id = &headId; *id != -1;) {
    const SNode& node = mNodes[*id];
    if (GetPointForSL(otherMinA, node.mBox) > GetPointForSL(otherMaxA, box) ||
        GetPointForSL(otherMaxA, node.mBox) < GetPointForSL(otherMinA, box) ||
        GetPointForSL(otherMinB, node.mBox) > GetPointForSL(otherMaxB, box) ||
        GetPointForSL(otherMaxB, node.mBox) < GetPointForSL(otherMinB, box)) {
      *id = node.mNext;
      node.mNext = -1;
      continue;
    }
    id = &node.mNext;
  }
  return headId;
}

short CSortedListManager::ConstructIntersectionArray(const CAABox& box) const {
  const short minXa = FindInListLower(kSL_MinX, box.GetMinPoint().GetX());
  const short maxXa = FindInListUpper(kSL_MinX, box.GetMaxPoint().GetX());
  const short minXb = FindInListLower(kSL_MaxX, box.GetMinPoint().GetX());
  const short maxXb = FindInListUpper(kSL_MaxX, box.GetMaxPoint().GetX());
  const short xOutside = rstl::min_val< short >(minXa, mSortedLists[kSL_MaxX].mSize - maxXb);

  const short minYa = FindInListLower(kSL_MinY, box.GetMinPoint().GetY());
  const short maxYa = FindInListUpper(kSL_MinY, box.GetMaxPoint().GetY());
  const short minYb = FindInListLower(kSL_MaxY, box.GetMinPoint().GetY());
  const short maxYb = FindInListUpper(kSL_MaxY, box.GetMaxPoint().GetY());
  const short yOutside = rstl::min_val< short >(minYa, mSortedLists[kSL_MaxY].mSize - maxYb);

  const short minZa = FindInListLower(kSL_MinZ, box.GetMinPoint().GetZ());
  const short maxZa = FindInListUpper(kSL_MinZ, box.GetMaxPoint().GetZ());
  const short minZb = FindInListLower(kSL_MaxZ, box.GetMinPoint().GetZ());
  const short maxZb = FindInListUpper(kSL_MaxZ, box.GetMaxPoint().GetZ());
  const short zOutside = rstl::min_val< short >(minZa, mSortedLists[kSL_MaxZ].mSize - maxZb);

  const int xCount = xOutside + (maxXb + (maxXa - minXa) - minXb) / 2;
  const int yCount = yOutside + (maxYb + (maxYa - minYa) - minYb) / 2;
  const int zCount = zOutside + (maxZb + (maxZa - minZa) - minZb) / 2;

  if (xCount < yCount && xCount < zCount) {
    return CalculateIntersections(kSL_MinX, kSL_MaxX, minXa, maxXa, minXb, maxXb, kSL_MinY,
                                  kSL_MaxY, kSL_MinZ, kSL_MaxZ, box);
  } else if (yCount < zCount) {
    return CalculateIntersections(kSL_MinY, kSL_MaxY, minYa, maxYa, minYb, maxYb, kSL_MinX,
                                  kSL_MaxX, kSL_MinZ, kSL_MaxZ, box);
  } else {
    return CalculateIntersections(kSL_MinZ, kSL_MaxZ, minZa, maxZa, minZb, maxZb, kSL_MinX,
                                  kSL_MaxX, kSL_MinY, kSL_MaxY, box);
  }
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
  const float length = magnitude != 0.f ? magnitude : 8000.f;
  const CVector3f end = pos + dir * length;
  const CAABox box(rstl::min_val(pos.GetX(), end.GetX()), rstl::min_val(pos.GetY(), end.GetY()),
                   rstl::min_val(pos.GetZ(), end.GetZ()), rstl::max_val(pos.GetX(), end.GetX()),
                   rstl::max_val(pos.GetY(), end.GetY()), rstl::max_val(pos.GetZ(), end.GetZ()));
  BuildNearList(nearListOut, box, filter, actor);
}
} // namespace SL
