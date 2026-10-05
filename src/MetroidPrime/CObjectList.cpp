#include "MetroidPrime/CObjectList.hpp"

#include "MetroidPrime/CEntity.hpp"

CObjectList::CObjectList(EGameObjectList list, bool dynamic)
: mListType(list), mFirstId(-1), mCount(0), mDynamic(dynamic) {
  for (int i = 0; i < kMaxObjects; ++i) {
    mObjects[i] = SObjectListEntry();
  }
}

uchar CObjectList::IsQualified(const CEntity& entity) { return true; }

void CObjectList::Clear() {
  short id = mFirstId;
  while (id != -1) {
    short next = mObjects[id].mNext;
    mObjects[id] = SObjectListEntry();
    id = next;
  }
  mFirstId = -1;
  mCount = 0;
}

void CObjectList::AddObject(CEntity& entity) {
  if (IsQualified(entity)) {
    short next = -1;
    if (mFirstId != -1) {
      mObjects[mFirstId].mPrev = entity.GetUniqueId().Value();
      next = mFirstId;
    }
    mFirstId = entity.GetUniqueId().Value();
    SObjectListEntry* entry = &mObjects[entity.GetUniqueId().Value()];
    entry->mEntity = &entity;
    entry->mNext = next;
    entry->mPrev = -1;
    ++mCount;
  }
}

void CObjectList::AddObjectIfAbsent(CEntity& entity) {
  if (mObjects[entity.GetUniqueId().Value()].mEntity == nullptr) {
    AddObject(entity);
  }
}

void CObjectList::RemoveObject(TUniqueId uid) {
  if (mObjects[uid.Value()].mEntity == nullptr) {
    return;
  }

  if (mObjects[uid.Value()].mEntity->GetUniqueId() != uid) {
    return;
  }

  if (mFirstId == uid.Value()) {
    mFirstId = mObjects[uid.Value()].mNext;
    short next = mObjects[uid.Value()].mNext;
    if (next != -1) {
      mObjects[next].mPrev = -1;
    }
  } else {
    mObjects[mObjects[uid.Value()].mPrev].mNext = mObjects[uid.Value()].mNext;
    short next = mObjects[uid.Value()].mNext;
    if (next != -1) {
      mObjects[next].mPrev = mObjects[uid.Value()].mPrev;
    }
  }
  --mCount;
  mObjects[uid.Value()].mEntity = nullptr;
  ushort index = uid.Value();
  mObjects[index].mNext = -1;
  mObjects[index].mPrev = -1;
}

CEntity* CObjectList::GetObjectById(TUniqueId uid) {
  if (uid == kInvalidUniqueId) {
    return nullptr;
  }
  CEntity* ret = mObjects[uid.Value()].mEntity;
  return ret && uid == ret->GetUniqueId() ? ret : nullptr;
}

const CEntity* CObjectList::GetObjectById(TUniqueId uid) const {
  if (uid == kInvalidUniqueId) {
    return nullptr;
  }
  const CEntity* ret = mObjects[uid.Value()].mEntity;
  return ret && uid == ret->GetUniqueId() ? ret : nullptr;
}

CEntity* CObjectList::operator[](int idx) { return mObjects[idx].mEntity; }

const CEntity* CObjectList::operator[](int idx) const { return mObjects[idx].mEntity; }
