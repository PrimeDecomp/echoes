#include "Kyoto/CDependencyGroup.hpp"

#include "Kyoto/CDependencyGroupToken.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CDependencyGroup::CDependencyGroup(CInputStream& in) { ReadFromStream(in); }

void CDependencyGroup::ReadFromStream(CInputStream& in) {
  int numTags = in.ReadInt32();
  mObjectTags.reserve(numTags);

  for (int i = 0; i < numTags; ++i) {
    FourCC type = in.ReadInt32();
    CAssetId id = in.ReadInt32();
    mObjectTags.push_back_unsafe(SObjectTag(type, id));
  }
}

int CDependencyGroup::GetCountForResType(FourCC type) const {
  int ret = 0;
  for (rstl::vector< SObjectTag >::const_iterator it = mObjectTags.begin();
       it != mObjectTags.end(); ++it) {
    if (it->type == type) {
      ++ret;
    }
  }

  return ret;
}

CFactoryFnReturn FDependencyGroupFactory(const SObjectTag& tag, CInputStream& in,
                                         const CVParamTransfer& xfer) {
  return rs_new CDependencyGroup(in);
}

CDependencyGroupToken::CDependencyGroupToken(const TToken< CDependencyGroup >& group,
                                             IObjectStore& store)
: mGroup(group), mLockCount(0), mLoaded(false) {
  const rstl::vector< SObjectTag >& tags = mGroup->GetObjectTagVector();
  mDependencies.reserve(tags.size());
  for (int i = 0; i < tags.size(); ++i) {
    mDependencies.push_back_unsafe(store.GetObj(tags[i]));
  }
}

void CDependencyGroupToken::Lock() {
  ++mLockCount;
  if (mLockCount == 1) {
    for (int i = 0; i < mDependencies.size(); ++i) {
      mDependencies[i].Lock();
    }
    mLoaded = false;
  }
}

void CDependencyGroupToken::Unlock() {
  --mLockCount;
  if (mLockCount == 0) {
    for (int i = 0; i < mDependencies.size(); ++i) {
      mDependencies[i].Unlock();
    }
    mLoaded = false;
  }
}

bool CDependencyGroupToken::IsLocked() const { return mLockCount != 0; }

bool CDependencyGroupToken::IsLoaded() {
  if (mLockCount == 0) {
    return false;
  }
  if (mLoaded) {
    return true;
  }
  bool loaded = true;
  for (int i = 0; i < mDependencies.size(); ++i) {
    if (!mDependencies[i].IsLoaded()) {
      loaded = false;
    }
  }
  mLoaded = loaded;
  return loaded;
}
