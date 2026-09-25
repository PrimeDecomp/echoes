#ifndef _CTOKEN
#define _CTOKEN

#include "types.h"

#include "Kyoto/CObjectReference.hpp"
#include "Kyoto/IObj.hpp"

class CObjectReference;

class CToken {
public:
  CToken() {}
  CToken(CObjectReference* ref);
  CToken(IObj* obj); // : x0_objRef(new CObjectReference(obj)), x4_lockHeld(false) {}
  CToken(const CToken& other);
  ~CToken();

  CObjOwnerDerivedFromIObjUntyped* GetObj();
  void Lock();
  void Unlock();
  bool IsLoaded() const { return mObjRef->IsLoaded(); }
  void RemoveRef();
  CToken& operator=(const CToken&);
  bool HasReference() const { return mObjRef != nullptr; }
  const SObjectTag& GetTag() const { return mObjRef->GetTag(); }
  FourCC GetReferenceType() { return GetTag().type; }

  bool HasLock() { return mLockHeld; }

  const CObjectReference* GetRef() const { return mObjRef; }

private:
  CObjectReference* mObjRef;
  bool mLockHeld;
};

#endif // _CTOKEN
