#include "MetroidPrime/CFilteredObjectList.hpp"

#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/Enemies/CParasite.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "rstl/algorithm.hpp"

class CGameCamera;
class CScriptDock;
class CScriptDoor;
class CScriptForgottenObject;
class CScriptGrapplePoint;

CFilteredObjectList::CFilteredObjectList(bool dynamic) : mDynamic(dynamic) {}

CFilteredObjectList::~CFilteredObjectList() {}

bool CFilteredObjectList::IsQualified(const CEntity& entity) const { return true; }

bool CFilteredObjectList::Contains(const CEntity& entity) const {
  return rstl::find(mObjects.begin(), mObjects.end(), &entity) != mObjects.end();
}

void CFilteredObjectList::AddObject(CEntity& entity) {
  if (IsQualified(entity)) {
    mObjects.push_back(&entity);
  }
}

void CFilteredObjectList::RemoveObject(CEntity& entity) {
  rstl::list< CEntity* >::iterator it = rstl::find(mObjects.begin(), mObjects.end(), &entity);
  if (it != mObjects.end()) {
    mObjects.erase(it);
  }
}

void CFilteredObjectList::RemoveObject(TUniqueId uid) {
  for (rstl::list< CEntity* >::iterator it = mObjects.begin(); it != mObjects.end(); ++it) {
    if ((*it)->GetUniqueId() == uid) {
      mObjects.erase(it);
      return;
    }
  }
}

CFilteredDoorList::CFilteredDoorList() : CFilteredObjectList(false) {}

bool CFilteredDoorList::IsQualified(const CEntity& entity) const {
  return TCastToConstPtr< CScriptDoor >(entity) != nullptr;
}

CFilteredDockList::CFilteredDockList() : CFilteredObjectList(false) {}

bool CFilteredDockList::IsQualified(const CEntity& entity) const {
  return TCastToConstPtr< CScriptDock >(entity) != nullptr;
}

CFilteredParasiteList::CFilteredParasiteList() : CFilteredObjectList(false) {}

bool CFilteredParasiteList::IsQualified(const CEntity& entity) const {
  return TCastToConstPtr< CParasite >(entity) != nullptr;
}

CFilteredForgottenObjectList::CFilteredForgottenObjectList() : CFilteredObjectList(false) {}

bool CFilteredForgottenObjectList::IsQualified(const CEntity& entity) const {
  return TCastToConstPtr< CScriptForgottenObject >(entity) != nullptr;
}

CFilteredGameCameraList::CFilteredGameCameraList() : CFilteredObjectList(false) {}

bool CFilteredGameCameraList::IsQualified(const CEntity& entity) const {
  return TCastToConstPtr< CGameCamera >(entity) != nullptr;
}

CFilteredGrapplePointList::CFilteredGrapplePointList() : CFilteredObjectList(false) {}

bool CFilteredGrapplePointList::IsQualified(const CEntity& entity) const {
  return TCastToConstPtr< CScriptGrapplePoint >(entity) != nullptr;
}
