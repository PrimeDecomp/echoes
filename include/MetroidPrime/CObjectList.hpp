#ifndef _COBJECTLIST
#define _COBJECTLIST

#include "TGameTypes.hpp"

class CEntity;

enum EGameObjectList {
  kOL_Invalid = -1,
  kOL_All,
  kOL_Actor,
  kOL_PhysicsActor,
  kOL_GameCamera,
  kOL_GameLight,
  kOL_ListeningAi,
  kOL_AiWaypoint,
  kOL_PlatformAndDoor,
  kOL_Unk,
};

class CObjectList {
public:
  CObjectList(EGameObjectList list, bool flag);
  virtual uchar IsQualified(const CEntity& entity);
  CEntity* fn_8000B538(TUniqueId id) const;

private:
  struct SObjectListEntry {
    CEntity* mEntity;
    short mNext;
    short mPrev;
  };

  SObjectListEntry mObjects[1024];
  EGameObjectList mListType;
  short mFirstId;
  short mCount;
  bool x200c_;
};
CHECK_SIZEOF(CObjectList, 0x2010)

#endif // _COBJECTLIST
