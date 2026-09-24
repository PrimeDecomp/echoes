#include "Kyoto/CSimplePool.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CResFactory.hpp"

rstl::vector< SObjectTag > CSimplePool::GetReferencedTags() {
  rstl::vector< SObjectTag > tags;
  tags.reserve(x4_resources.size());
  ResourceMap::const_iterator it = x4_resources.begin();
  ResourceMap::const_iterator end = x4_resources.end();
  for (; it != end; ++it) {
    tags.push_back_unsafe(it->first);
  }
  return tags;
}

void CSimplePool::DebugDumpPool() const {
  ResourceMap::const_iterator it = x4_resources.begin();
  for (; it != x4_resources.end(); ++it) {
  }
}

void CSimplePool::Flush() {}

bool CSimplePool::ObjectIsLive(const SObjectTag& tag) {
  ResourceMap::iterator it = x4_resources.find(tag);
  if (it == x4_resources.end()) {
    return false;
  }
  return it->second->IsLoaded();
}

bool CSimplePool::HasObject(const SObjectTag& tag) {
  ResourceMap::iterator it = x4_resources.find(tag);
  if (it != x4_resources.end()) {
    return true;
  }
  return x18_factory != nullptr && x18_factory->CanBuild(tag);
}

CToken CSimplePool::GetObj(const char* name, const CVParamTransfer& xfer) {
  const SObjectTag* tag = CSimplePool::GetFactory().GetResourceIdByName(name);
  return CSimplePool::GetObj(*tag, xfer);
}

CToken CSimplePool::GetObj(const char* name) { return CSimplePool::GetObj(name, x1c_paramXfr); }

CToken CSimplePool::GetObj(const SObjectTag& tag) { return CSimplePool::GetObj(tag, x1c_paramXfr); }

CToken CSimplePool::GetObj(const SObjectTag& tag, const CVParamTransfer& xfer) {
  ResourceMap::iterator it = x4_resources.find(tag);
  if (it != x4_resources.end()) {
    return CToken(it->second);
  }

  CObjectReference* ref =
      rs_new CObjectReference(*this, rstl::auto_ptr< IObj >(nullptr), tag, xfer);
  ResourceMap::value_type item(tag, ref);
  x4_resources.insert(item);
  return CToken(ref);
}

void CSimplePool::ObjectUnreferenced(const SObjectTag& tag) {
  x4_resources.erase(x4_resources.find(tag));
}

CSimplePool::~CSimplePool() {
  Flush();
  if (x4_resources.size() > 0) {
    DebugDumpPool();
  }
}

CSimplePool::CSimplePool(IFactory& factory)
: x18_factory(&factory), x1c_paramXfr(CVParamTransfer::Null()) {
  x1c_paramXfr = CVParamTransfer(rs_new TObjOwnerParam< IObjectStore* >(this));
}
