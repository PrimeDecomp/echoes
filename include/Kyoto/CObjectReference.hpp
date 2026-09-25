#ifndef _COBJECTREFERENCE
#define _COBJECTREFERENCE

#include <Kyoto/CVParamTransfer.hpp>
#include <Kyoto/SObjectTag.hpp>
#include <rstl/auto_ptr.hpp>

class IObj;
class IObjectStore;
class CObjectReference {
public:
  CObjectReference(const rstl::auto_ptr< IObj >& obj);
  ~CObjectReference();
  CObjectReference(IObjectStore& store, const rstl::auto_ptr< IObj >& obj, const SObjectTag& tag,
                   CVParamTransfer xfer);

  bool IsLoaded() const { return mObject != nullptr; }

  void AddReference() { mRefCount++; }
  int RemoveReference();
  void Lock();
  void Unlock();
  IObj* GetObject();
  void Unload();
  void CancelLoad();
  bool IsLoading() const;
  const SObjectTag& GetTag() const { return mObjTag; }

private:
  int mRefCount;
  int mLockCount;
  int mLoading;
  SObjectTag mObjTag;
  IObjectStore* mObjectStore;
  IObj* mObject;
  CVParamTransfer mParams;
};
CHECK_SIZEOF(CObjectReference, 0x24)
#endif // _COBJECTREFERENCE
