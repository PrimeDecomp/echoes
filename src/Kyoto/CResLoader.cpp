#include "Kyoto/CResLoader.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CPakFile.hpp"
#include "Kyoto/Streams/CLZOInputStream.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "rstl/StringExtras.hpp"

static inline int align_size(const int size) { return (size + 31) & ~31; }

CResLoader::CResLoader()
: mCurPak(mPakLoadedList.end())
, mCachedResId(kInvalidAssetId)
, mCachedResInfo(nullptr)
, mForwardSeek(false) {}

CResLoader::~CResLoader() {
  for (PakList::iterator it = mPakLoadingList.begin(); it != mPakLoadingList.end(); ++it) {
    CPakFile* pak = it->get();
    while (!pak->IsCompletelyLoaded()) {
      pak->AsyncIdle();
    }
  }
}

void CResLoader::MoveToCorrectLoadedList(const rstl::auto_ptr< CPakFile >& pak) {
  if (pak->IsARAMPak()) {
    mAramList.push_back(pak);
  } else {
    mPakLoadedList.push_back(pak);
  }
}

bool CResLoader::CacheFromPak(const CPakFile& pak, const CAssetId asset) const {
  const CPakFile::CResInfo* resInfo = pak.GetResInfo(asset);
  if (!resInfo) {
    return false;
  }

  mCachedResId = asset;
  mCachedResInfo = resInfo;

  return true;
}

bool CResLoader::CacheFromPakForLoad(CPakFile& pak, const CAssetId asset) {
  const CPakFile::CResInfo* resInfo = nullptr;
  if (mForwardSeek) {
    resInfo = pak.GetResInfoForLoadPreferForward(asset);
    mForwardSeek = false;
  } else {
    resInfo = pak.GetResInfoForLoadDirectionless(asset);
  }

  if (resInfo == nullptr) {
    return false;
  }

  mCachedResId = asset;
  mCachedResInfo = resInfo;

  return true;
}

CPakFile* CResLoader::FindResourceForLoad(const SObjectTag& tag) {
  return FindResourceForLoad(tag.GetId());
}

CPakFile* CResLoader::FindResourceForLoad(const CAssetId asset) {
  PakList::iterator it;
  for (it = mAramList.begin(); it != mAramList.end(); ++it) {
    CPakFile* pak = it->get();
    if (CacheFromPak(*pak, asset)) {
      return pak;
    }
  }

  if (mCurPak != mPakLoadedList.end()) {
    CPakFile* pak = mCurPak->get();
    if (CacheFromPakForLoad(*pak, asset)) {
      return pak;
    }
  }

  for (it = mPakLoadedList.begin(); it != mPakLoadedList.end(); ++it) {
    CPakFile* pak = it->get();
    if (mCurPak != it && CacheFromPakForLoad(*pak, asset)) {
      mCurPak = it;
      return pak;
    }
  }

  return nullptr;
}

CPakFile* CResLoader::FindResource(const SObjectTag& tag) {
  mForwardSeek = false;
  CPakFile* ret = FindResourceForLoad(tag);
  mForwardSeek = true;
  return ret;
}

void CResLoader::ClearCache() {
  mCurPak = mPakLoadedList.end();
  mCachedResId = kInvalidAssetId;
  mCachedResInfo = nullptr;
}

void CResLoader::AsyncIdlePakLoading() {
  bool skipIdle = false;
  for (PakList::iterator it = mPakLoadingList.begin(); it != mPakLoadingList.end();) {
    CPakFile* pak = it->get();
    const bool aramPak = pak->IsARAMPak();
    if (aramPak || !skipIdle) {
      pak->AsyncIdle();
    }

    if (pak->IsCompletelyLoaded()) {
      MoveToCorrectLoadedList(*it);
      it = mPakLoadingList.erase(it);
    } else {
      if (!aramPak) {
        skipIdle = true;
      }
      ++it;
    }
  }
}

bool CResLoader::AreAllPaksLoaded() const { return mPakLoadingList.empty(); }

const SObjectTag* CResLoader::GetResourceIdByName(const char* name) const {
  for (PakList::const_iterator it = mAramList.begin(); it != mAramList.end(); ++it) {
    const SObjectTag* id = (*it)->GetResIdByName(name);
    if (id != nullptr) {
      return id;
    }
  }

  for (PakList::const_iterator it = mPakLoadedList.begin(); it != mPakLoadedList.end(); ++it) {
    const SObjectTag* id = (*it)->GetResIdByName(name);
    if (id != nullptr) {
      return id;
    }
  }

  return nullptr;
}

FourCC CResLoader::GetResourceTypeById(const CAssetId asset) const {
  if (const_cast< CResLoader& >(*this).FindResourceForLoad(asset)) {
    return mCachedResInfo->GetType();
  }

  return 0;
}

bool CResLoader::ResourceExists(const SObjectTag& tag) const {
  return const_cast< CResLoader& >(*this).FindResourceForLoad(tag.GetId()) != nullptr;
}

uint CResLoader::ResourceSize(const SObjectTag& tag) const {
  if (const_cast< CResLoader& >(*this).FindResourceForLoad(tag.GetId())) {
    return mCachedResInfo->GetSize();
  }

  return 0;
}

uint CResLoader::GetResourceOffset(const SObjectTag& tag) const {
  if (const_cast< CResLoader& >(*this).FindResourceForLoad(tag.GetId())) {
    return mCachedResInfo->GetOffset();
  }

  return 0;
}

CResLoader::ECompressionType CResLoader::GetResourceCompression(const SObjectTag& tag) const {
  if (const_cast< CResLoader& >(*this).FindResourceForLoad(tag.GetId())) {
    return mCachedResInfo->IsCompressed() ? kCompressionType_Compressed
                                          : kCompressionType_Uncompressed;
  }

  return kCompressionType_Uncompressed;
}

CDvdRequest* CResLoader::LoadResourceAsync(const SObjectTag& tag, char* extBuf) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const CPakFile::CResInfo* info = mCachedResInfo;
  return curPak->DvdFile().AsyncSeekRead(extBuf, align_size(info->GetSize()), kSO_Set,
                                         info->GetOffset());
}

CBufferedDvdRequest* CResLoader::LoadResourceAsync(const SObjectTag& tag) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const CPakFile::CResInfo* info = mCachedResInfo;
  GroupCacheList::iterator it =
      FindGroupCache(&curPak->DvdFile(), info->GetOffset(), info->GetSize());
  if (it == mGroupCaches.end()) {
    const uint groupedSize = info->GetLookaheadAfterResourceSize();
    if (groupedSize == 0) {
      const int size = align_size(info->GetSize());
      rstl::auto_ptr< uchar > buffer(
          static_cast< uchar* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen)));
      CDvdRequest* request =
          curPak->DvdFile().AsyncSeekRead(buffer.get(), size, kSO_Set, info->GetOffset());
      return rs_new CBufferedDvdRequest(nullptr, request, buffer.release());
    }
    it = AddGroupCache(&curPak->DvdFile(), info->GetOffset(), groupedSize + info->GetSize());
  }
  return it->MakeRequest(info->GetOffset()).release();
}

CDvdRequest* CResLoader::LoadResourcePartAsync(const SObjectTag& tag, const int offset,
                                               const int length, char* extBuf) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const CPakFile::CResInfo* info = mCachedResInfo;
  return curPak->DvdFile().AsyncSeekRead(extBuf, length, kSO_Set, info->GetOffset() + offset);
}

CInputStream* CResLoader::LoadNewResourceSync(const SObjectTag& tag, char* extBuf) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const CPakFile::CResInfo* info = mCachedResInfo;
  uint len = align_size(info->GetSize());
  void* dest = extBuf ? extBuf : CMemory::Alloc(len, IAllocator::kHI_RoundUpLen);

  curPak->DvdFile().SyncSeekRead(dest, len, kSO_Set, info->GetOffset());
  rstl::auto_ptr< CInputStream > input(rs_new CMemoryInStream(
      dest, info->GetSize(),
      extBuf == nullptr ? CMemoryInStream::kOS_Owned : CMemoryInStream::kOS_NotOwned));

  if (info->IsCompressed()) {
    const int length = input->ReadInt32();
    return rs_new CLZOInputStream(input, info->GetSize() - input->GetReadPosition(), length);
  }

  return input.release();
}

CInputStream* CResLoader::LoadResourceFromMemorySync(const SObjectTag& tag, const void* extBuf) {
  FindResourceForLoad(tag);
  const CPakFile::CResInfo* info = mCachedResInfo;
  rstl::auto_ptr< CInputStream > input(rs_new CMemoryInStream(extBuf, info->GetSize()));

  if (info->IsCompressed()) {
    const int length = input->ReadInt32();
    return rs_new CLZOInputStream(input, info->GetSize() - input->GetReadPosition(), length);
  }

  return input.release();
}

void CResLoader::LoadMemResourceSync(const SObjectTag& tag, char** bufOut, int* lenOut) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const CPakFile::CResInfo* info = mCachedResInfo;
  uint len = align_size(info->GetSize());
  char* buf = static_cast< char* >(CMemory::Alloc(len, IAllocator::kHI_RoundUpLen));
  curPak->DvdFile().SyncSeekRead(buf, len, kSO_Set, info->GetOffset());
  *bufOut = buf;
  *lenOut = info->GetSize();
}

void CResLoader::AddPakFileAsync(const rstl::string& filePath, bool a, bool b) {
  const rstl::string pathWithExt(filePath + ".pak");

  if (CDvdFile::FileExists(pathWithExt.data())) {
    mPakLoadingList.push_back(rs_new CPakFile(pathWithExt, a, b));
  }
}

void CResLoader::RemovePakFile(const rstl::string& filePath) {
  rstl::string pathWithExt(filePath + ".pak");
  ClearCache();
  PakList* lists[] = {&mAramList, &mPakLoadedList};

  for (int i = 0; i < ARRAY_SIZE(lists); ++i) {
    PakList& list = *lists[i];
    for (PakList::iterator it = list.begin(); it != list.end(); ++it) {
      const CPakFile* pak = it->get();
      if (CStringExtras::CompareCaseInsensitive(pak->GetDvdFile().GetFilename(), pathWithExt) ==
          0) {
        list.erase(it);
        return;
      }
    }
  }

  for (PakList::iterator it = mPakLoadingList.begin(); it != mPakLoadingList.end(); ++it) {
    const CPakFile* pak = it->get();
    if (CStringExtras::CompareCaseInsensitive(pak->GetDvdFile().GetFilename(), pathWithExt) == 0) {
      while (!pak->IsCompletelyLoaded()) {
        AsyncIdlePakLoading();
      }
      mPakLoadingList.erase(it);
      return;
    }
  }
}

rstl::vector< rstl::pair< rstl::string, SObjectTag > > CResLoader::GetResourceIdToNameList() const {
  const PakList* lists[] = {&mAramList, &mPakLoadedList};
  int nameCount = 0;
  for (int i = 0; i < ARRAY_SIZE(lists); ++i) {
    const PakList& list = *lists[i];
    for (PakList::const_iterator it = list.begin(); it != list.end(); ++it) {
      const CPakFile* pak = it->get();
      if (!pak->IsStashedInARAM()) {
        nameCount += pak->GetStringToObjectList().size();
      }
    }
  }

  rstl::vector< rstl::pair< rstl::string, SObjectTag > > ret;
  ret.reserve(nameCount);

  for (int i = 0; i < ARRAY_SIZE(lists); ++i) {
    const PakList& list = *lists[i];
    for (PakList::const_iterator it = list.begin(); it != list.end(); ++it) {
      const CPakFile* pak = it->get();
      if (!pak->IsStashedInARAM()) {
        const rstl::vector< rstl::pair< rstl::string, SObjectTag > >& tagList =
            pak->GetStringToObjectList();
        ret.insert(ret.end(), tagList.begin(), tagList.end());
      }
    }
  }

  return ret;
}

int CResLoader::GetPakCount() const { return mAramList.size() + mPakLoadedList.size(); }

CPakFile* CResLoader::sub_802FBB64(const int idx) const {
  int numAramPaks = mAramList.size();
  if (idx < numAramPaks) {
    PakList::const_iterator it = mAramList.begin();
    for (int i = 0; i < idx; ++it, ++i) {
    }
    return it->get();
  }

  PakList::const_iterator it = mPakLoadedList.begin();
  for (int i = 0; i < idx - numAramPaks; ++it, ++i) {
  }
  return it->get();
}

CPakFile* CResLoader::GetPakFile(const int idx) const {
  int numAramPaks = mAramList.size();
  if (idx < numAramPaks) {
    PakList::const_iterator it = mAramList.begin();
    for (int i = 0; i < idx; ++it, ++i) {
    }
    return it->get();
  }

  PakList::const_iterator it = mPakLoadedList.begin();
  for (int i = 0; i < idx - numAramPaks; ++it, ++i) {
  }
  return it->get();
}

void CResLoader::KillLookahead(const CLookaheadRes* cache) {
  for (GroupCacheList::iterator it = mGroupCaches.begin(); it != mGroupCaches.end(); ++it) {
    if (&*it == cache) {
      mGroupCaches.erase(it);
      return;
    }
  }
}

CResLoader::GroupCacheList::iterator CResLoader::FindGroupCache(const CDvdFile* file, uint offset,
                                                                uint size) {
  GroupCacheList::iterator it = mGroupCaches.end();
  while (it != mGroupCaches.begin()) {
    --it;
    const CLookaheadRes& cache = *it;
    if (!cache.IsInvalid() && cache.Contains(file, offset, size)) {
      return it;
    }
  }
  return mGroupCaches.end();
}

CResLoader::GroupCacheList::iterator CResLoader::AddGroupCache(CDvdFile* file, uint offset,
                                                               uint size) {
  rstl::auto_ptr< uchar > buffer(
      static_cast< uchar* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen)));
  rstl::auto_ptr< CDvdRequest > request(file->AsyncSeekRead(buffer.get(), size, kSO_Set, offset));
  return mGroupCaches.insert(mGroupCaches.end(),
                             CLookaheadRes(buffer.release(), file, offset, size, request, this));
}
