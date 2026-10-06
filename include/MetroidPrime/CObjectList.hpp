#ifndef _COBJECTLIST
#define _COBJECTLIST

#include "TGameTypes.hpp"

class CEntity;

#define kMaxObjects 1024

enum EGameObjectList {
  kOL_Invalid = -1,
  kOL_All,
  kOL_Actor,
  kOL_PhysicsActor,
  kOL_GameLight,
  kOL_ListeningAi,
  kOL_AiWaypoint,
  kOL_Platform,
  kOL_Trigger,
};

class CObjectList {
  struct SObjectListEntry {
    CEntity* mEntity;
    short mNext;
    short mPrev;
    SObjectListEntry() : mEntity(nullptr), mNext(-1), mPrev(-1) {}
  };

public:
  CObjectList(EGameObjectList list, bool flag);
  virtual uchar IsQualified(const CEntity& entity);

  // Echoes names below are inferred from Prime and their implementations.
  void Clear();
  void AddObject(CEntity& entity);
  void AddObjectIfAbsent(CEntity& entity);
  void RemoveObject(TUniqueId uid);
  CEntity* GetObjectById(TUniqueId uid);
  const CEntity* GetObjectById(TUniqueId uid) const;
  CEntity* operator[](int idx);
  const CEntity* operator[](int idx) const;

  int size() const { return mCount; }
  bool IsDynamic() const { return mDynamic; } // Guessed name, from manager registration.
  int GetFirstObjectIndex() const { return mFirstId; }
  int GetNextObjectIndex(int idx) const {
    if (idx != -1) {
      return mObjects[idx].mNext;
    } else {
      return -1;
    }
  }

private:
  SObjectListEntry mObjects[kMaxObjects];
  EGameObjectList mListType;
  short mFirstId;
  short mCount;
  bool mDynamic; // Guessed name: qualification can change after registration.
};
CHECK_SIZEOF(CObjectList, 0x2010)

#endif // _COBJECTLIST
