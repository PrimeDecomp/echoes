#ifndef _CFILTEREDOBJECTLIST
#define _CFILTEREDOBJECTLIST

#include "rstl/list.hpp"

class CEntity;

// Guessed name. Echoes's small, linked-list counterpart to CObjectList.
class CFilteredObjectList {
public:
  explicit CFilteredObjectList(bool dynamic);
  virtual ~CFilteredObjectList();
  virtual bool IsQualified(const CEntity& entity) const;

  const rstl::list< CEntity* >& GetObjects() const { return mObjects; }

private:
  rstl::list< CEntity* > mObjects;
  bool x1c_;
};
CHECK_SIZEOF(CFilteredObjectList, 0x20)

#endif // _CFILTEREDOBJECTLIST
