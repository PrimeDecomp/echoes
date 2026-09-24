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

  bool IsLoaded() const { return x18_object != nullptr; }

  void AddReference() { x0_refCount++; }
  int RemoveReference();
  void Lock();
  void Unlock();
  IObj* GetObject();
  void Unload();
  void CancelLoad();
  bool IsLoading() const;
  const SObjectTag& GetTag() const { return xc_objTag; }

private:
  int x0_refCount;
  int x4_lockCount;
  int x8_loading;
  SObjectTag xc_objTag;
  IObjectStore* x14_objectStore;
  IObj* x18_object;
  CVParamTransfer x1c_params;
};
CHECK_SIZEOF(CObjectReference, 0x24)
#endif // _COBJECTREFERENCE
