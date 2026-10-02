#ifndef _GAMEOBJECTLISTS
#define _GAMEOBJECTLISTS

#include "MetroidPrime/CObjectList.hpp"

class CActorList : public CObjectList {
public:
  CActorList();
  uchar IsQualified(const CEntity& ent) override;
};

class CPhysicsActorList : public CObjectList {
public:
  CPhysicsActorList();
  uchar IsQualified(const CEntity& ent) override;
};

class CGameLightList : public CObjectList {
public:
  CGameLightList();
  uchar IsQualified(const CEntity& ent) override;
};

class CListeningAiList : public CObjectList {
public:
  CListeningAiList();
  uchar IsQualified(const CEntity& ent) override;
};

class CAiWaypointList : public CObjectList {
public:
  CAiWaypointList();
  uchar IsQualified(const CEntity& ent) override;
};

// Guessed name.
class CPlatformList : public CObjectList {
public:
  CPlatformList();
  uchar IsQualified(const CEntity& ent) override;
};

// Guessed name.
class CTriggerList : public CObjectList {
public:
  CTriggerList();
  uchar IsQualified(const CEntity& ent) override;
};

#endif // _GAMEOBJECTLISTS
