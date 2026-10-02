#include "MetroidPrime/GameObjectLists.hpp"

#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/TCastTo.hpp"

class CScriptAIHint;

CActorList::CActorList() : CObjectList(kOL_Actor, false) {}

uchar CActorList::IsQualified(const CEntity& ent) {
  return TCastToConstPtr< CActor >(ent) != nullptr;
}

CPhysicsActorList::CPhysicsActorList() : CObjectList(kOL_PhysicsActor, false) {}

uchar CPhysicsActorList::IsQualified(const CEntity& ent) {
  return TCastToConstPtr< CPhysicsActor >(ent) != nullptr;
}

CListeningAiList::CListeningAiList() : CObjectList(kOL_ListeningAi, true) {}

uchar CListeningAiList::IsQualified(const CEntity& ent) {
  bool ret = false;
  const CPatterned* pat = TCastToConstPtr< CPatterned >(ent);
  if (pat && pat->IsListening()) {
    ret = true;
  }
  return ret;
}

CAiWaypointList::CAiWaypointList() : CObjectList(kOL_AiWaypoint, false) {}

uchar CAiWaypointList::IsQualified(const CEntity& ent) {
  bool ret = false;
  if (TCastToConstPtr< CScriptCoverPoint >(ent) != nullptr) {
    ret = true;
  } else if (TCastToConstPtr< CScriptAiJumpPoint >(ent) != nullptr) {
    ret = true;
  } else if (TCastToConstPtr< CScriptAIHint >(ent) != nullptr) {
    ret = true;
  }
  return ret;
}

CPlatformList::CPlatformList() : CObjectList(kOL_Platform, false) {}

uchar CPlatformList::IsQualified(const CEntity& ent) {
  return TCastToConstPtr< CScriptPlatform >(ent) != nullptr;
}

CTriggerList::CTriggerList() : CObjectList(kOL_Trigger, false) {}

uchar CTriggerList::IsQualified(const CEntity& ent) {
  return TCastToConstPtr< CScriptTrigger >(ent) != nullptr;
}

CGameLightList::CGameLightList() : CObjectList(kOL_GameLight, false) {}

uchar CGameLightList::IsQualified(const CEntity& ent) {
  return TCastToConstPtr< CGameLight >(ent) != nullptr;
}
