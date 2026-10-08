#include "MetroidPrime/CRelFile.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CDvdRequest.hpp"

#include "dolphin/os.h"
#include "dolphin/os/OSModule.h"

CRelFile::CRelFile(const rstl::string& name)
: mName(name)
, mDvdRequest(nullptr)
, mData(nullptr)
, mDataSize(0)
, mModule(nullptr)
, mReferenceCount(0)
, mLoadRequestCount(0)
, mState(kS_Unloaded) {}

CRelFile::~CRelFile() {
  if (mState != kS_Unloaded) {
    Update();
  }
}

void CRelFile::AddReference() { ++mReferenceCount; }

short CRelFile::RemoveReference() {
  --mReferenceCount;
  return mReferenceCount;
}

void CRelFile::FreeData() {
  CMemory::OffsetFakeStatics(-mDataSize);
  mDvdRequest = nullptr;
  mData = nullptr;
  mDataSize = 0;
  mModule = nullptr;
}

bool CRelFile::StartLoad() {
#ifdef MONOLITHIC
  // Completed REL sources are linked into the DOL; take the missing-file path
  // so the request reports loaded without linking anything.
  return false;
#endif
  if (!CDvdFile::FileExists(mName.data())) {
    return false;
  }
  CDvdFile file(mName.data());
  mDataSize = (file.Length() + 31) & ~31;
  mData = static_cast< uchar* >(CMemory::Alloc(mDataSize, IAllocator::kHI_RoundUpLen));
  mModule = reinterpret_cast< OSModuleHeader* >(mData.get());
  CMemory::OffsetFakeStatics(mDataSize);
  mDvdRequest = file.SyncRead(mModule, mDataSize);
  return true;
}

void CRelFile::Update() {
  switch (mState) {
  case kS_Cancelling:
    if (!mDvdRequest->IsComplete()) {
      break;
    }
    FreeData();
    mState = kS_Unloaded;
  case kS_Unloaded:
    if (mLoadRequestCount >= 1) {
      if (StartLoad()) {
        mState = kS_Loading;
      } else {
        mState = kS_Loaded;
      }
    }
    break;
  case kS_Loading:
    if (mDvdRequest->IsComplete()) {
      mState = kS_Loaded;
      Link();
    } else if (mLoadRequestCount == 0) {
      mDvdRequest->PostCancelRequest();
      mState = kS_Cancelling;
    }
    break;
  case kS_Loaded:
    if (mLoadRequestCount == 0) {
      Unlink();
      mState = kS_Unloaded;
    }
    break;
  }
}

void CRelFile::AddLoadRequest() { ++mLoadRequestCount; }

void CRelFile::RemoveLoadRequest() { --mLoadRequestCount; }

void CRelFile::Unlink() {
  mDebugInfo.Unregister();
  if (mModule != nullptr) {
    reinterpret_cast< void (*)() >(mModule->epilog)();
    OSUnlink(&mModule->info);
  }
  FreeData();
}

void CRelFile::Link() {
  OSGetTime();
  uint bssSize = mModule->bssSize;
  uint fixSize = (mModule->fixSize + 31) & ~31;
  if (mDataSize - fixSize < bssSize) {
    uint newSize = ((bssSize + 31) & ~31) + fixSize;
    rstl::single_ptr< uchar > newData(
        static_cast< uchar* >(CMemory::Alloc(newSize, IAllocator::kHI_RoundUpLen)));
    CBasics::CopyMemory(newData.get(), mData.get(), mDataSize);
    mDataSize = newSize;
    mData = newData;
    mModule = reinterpret_cast< OSModuleHeader* >(mData.get());
  }
  OSLinkFixed(&mModule->info, reinterpret_cast< uchar* >(mModule) + fixSize);
  mDebugInfo.Register(mName.data(), mData.get(), mDataSize);
  reinterpret_cast< void (*)() >(mModule->prolog)();
  OSGetTime();
}

bool CRelFile::IsLoaded() const { return mState == kS_Loaded && mLoadRequestCount > 0; }

bool CRelFile::IsDeletable() const { return mState == kS_Unloaded && mReferenceCount == 0; }
