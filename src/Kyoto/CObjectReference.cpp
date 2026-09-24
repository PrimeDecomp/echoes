#include "Kyoto/CObjectReference.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/IObjectStore.hpp"

CObjectReference::CObjectReference(IObjectStore& store, const rstl::auto_ptr< IObj >& obj,
                                   const SObjectTag& tag, CVParamTransfer xfer)
: x0_refCount(0)
, x4_lockCount(0)
, x8_loading(false)
, xc_objTag(tag)
, x14_objectStore(&store)
, x18_object(obj.release())
, x1c_params(xfer) {}

CObjectReference::CObjectReference(const rstl::auto_ptr< IObj >& obj)
: x0_refCount(0)
, x4_lockCount(0)
, x8_loading(false)
, xc_objTag(kInvalidAssetId, kInvalidAssetId)
, x14_objectStore(nullptr)
, x18_object(obj.release())
, x1c_params(CVParamTransfer::Null()) {}

CObjectReference::~CObjectReference() {
  if (x18_object) {
    delete x18_object;
  } else if (x8_loading) {
    x14_objectStore->GetFactory().CancelBuild(xc_objTag);
  }
}

void CObjectReference::Lock() {
  ++x4_lockCount;
  if (x18_object == nullptr && !x8_loading) {
    x14_objectStore->GetFactory().BuildAsync(xc_objTag, x1c_params, &x18_object);
    x8_loading = true;
  }
}

IObj* CObjectReference::GetObject() {
  if (x18_object == nullptr) {
    x18_object = x14_objectStore->GetFactory()
                     .Build(xc_objTag, x1c_params)
                     .GetObjForTransfer()
                     .release();
  }
  x8_loading = false;
  return x18_object;
}

void CObjectReference::Unload() {
  delete x18_object;
  x18_object = nullptr;
  x8_loading = false;
}

void CObjectReference::CancelLoad() {
  if (x14_objectStore == nullptr) {
    return;
  }
  if (!IsLoading()) {
    return;
  }
  x14_objectStore->GetFactory().CancelBuild(xc_objTag);
  x8_loading = false;
}

int CObjectReference::RemoveReference() {
  --x0_refCount;
  if (x0_refCount == 0) {
    if (x18_object != nullptr) {
      Unload();
    } else if (IsLoading()) {
      CancelLoad();
    }
    if (x14_objectStore != nullptr) {
      x14_objectStore->ObjectUnreferenced(xc_objTag);
    }
  }
  return x0_refCount;
}

void CObjectReference::Unlock() {
  --x4_lockCount;
  if (x4_lockCount != 0) {
    return;
  }
  if (x18_object != nullptr && x14_objectStore != nullptr) {
    Unload();
  } else if (IsLoading()) {
    CancelLoad();
  }
}

bool CObjectReference::IsLoading() const { return x8_loading && x18_object == nullptr; }
