#ifndef _CFILTEREDOBJECTLIST
#define _CFILTEREDOBJECTLIST

#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/list.hpp"

class CEntity;

// Guessed name. Echoes's small, linked-list counterpart to CObjectList.
class CFilteredObjectList {
public:
  explicit CFilteredObjectList(bool dynamic);
  virtual ~CFilteredObjectList();
  virtual bool IsQualified(const CEntity& entity) const;

  void AddObject(CEntity& entity);
  void RemoveObject(TUniqueId uid);
  void RemoveObject(CEntity& entity);
  bool Contains(const CEntity& entity) const;

  const rstl::list< CEntity* >& GetObjects() const { return mObjects; }
  bool IsDynamic() const { return mDynamic; }

private:
  rstl::list< CEntity* > mObjects;
  bool mDynamic;
};
CHECK_SIZEOF(CFilteredObjectList, 0x20)

// Guessed class names, describing the native qualification predicates.
class CFilteredGrapplePointList : public CFilteredObjectList {
public:
  CFilteredGrapplePointList();

  // CFilteredObjectList
  ~CFilteredGrapplePointList() override {}
  bool IsQualified(const CEntity& entity) const override;
};
CHECK_SIZEOF(CFilteredGrapplePointList, 0x20)

class CFilteredGameCameraList : public CFilteredObjectList {
public:
  CFilteredGameCameraList();

  // CFilteredObjectList
  ~CFilteredGameCameraList() override {}
  bool IsQualified(const CEntity& entity) const override;
};
CHECK_SIZEOF(CFilteredGameCameraList, 0x20)

class CFilteredForgottenObjectList : public CFilteredObjectList {
public:
  CFilteredForgottenObjectList();

  // CFilteredObjectList
  ~CFilteredForgottenObjectList() override {}
  bool IsQualified(const CEntity& entity) const override;
};
CHECK_SIZEOF(CFilteredForgottenObjectList, 0x20)

// The class selected by native entity type 124 remains unidentified.
class CFilteredType124List : public CFilteredObjectList {
public:
  CFilteredType124List();

  // CFilteredObjectList
  ~CFilteredType124List() override {}
  bool IsQualified(const CEntity& entity) const override;
};
CHECK_SIZEOF(CFilteredType124List, 0x20)

class CFilteredDockList : public CFilteredObjectList {
public:
  CFilteredDockList();

  // CFilteredObjectList
  ~CFilteredDockList() override {}
  bool IsQualified(const CEntity& entity) const override;
};
CHECK_SIZEOF(CFilteredDockList, 0x20)

class CFilteredDoorList : public CFilteredObjectList {
public:
  CFilteredDoorList();

  // CFilteredObjectList
  ~CFilteredDoorList() override {}
  bool IsQualified(const CEntity& entity) const override;
};
CHECK_SIZEOF(CFilteredDoorList, 0x20)

#endif // _CFILTEREDOBJECTLIST
