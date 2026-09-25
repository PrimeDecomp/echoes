#include "Kyoto/Streams/CFilePreload.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/vector.hpp"

#include "dolphin/os/OSCache.h"

#include <string.h>

class CFilePreloadData {
public:
  CFilePreloadData(const rstl::string& path);
  ~CFilePreloadData();

  rstl::string GetFilename() const { return mPath; }
  bool IsReady();
  void Read(void* dest, int offset, int length);

  rstl::string mPath;
  int mSize;
  int mRefCount;
  rstl::vector< rstl::auto_ptr< uchar > > mBuffers;
  rstl::vector< rstl::auto_ptr< CDvdRequest > > mRequests;
};
CHECK_SIZEOF(CFilePreloadData, 0x38)

static rstl::list< rstl::auto_ptr< CFilePreloadData > > sPreloadedFiles;

static void CopyAndFlush(void* dest, const void* src, int length) {
  memcpy(dest, src, length);
  DCFlushRange(dest, length);
}

CFilePreloadData::CFilePreloadData(const rstl::string& path)
: mPath(path), mSize(0), mRefCount(1), mBuffers(), mRequests() {
  CDvdFile file(path.data());
  mSize = file.GetFileSize();
  int offset;
  const int count = (mSize + 0x3fff) / 0x4000;
  mBuffers.reserve(count);
  mRequests.reserve(count);

  int i = 0;
  offset = 0;
  for (; i < count; offset += 0x4000, ++i) {
    int length = 0x4000;
    const int remaining = mSize - offset;
    if (remaining <= 0x4000) {
      length = remaining;
    }
    const uint alignedLength = (length + 31) & ~31;
    rstl::auto_ptr< uchar > buffer(static_cast< uchar* >(
        CMemory::Alloc(alignedLength, IAllocator::kHI_RoundUpLen)));
    rstl::auto_ptr< CDvdRequest > request(
        file.AsyncSeekRead(buffer.get(), alignedLength, kSO_Set, offset));
    mBuffers.push_back_unsafe(buffer);
    mRequests.push_back_unsafe(request);
  }
}

CFilePreloadData::~CFilePreloadData() {
  for (rstl::vector< rstl::auto_ptr< CDvdRequest > >::iterator it = mRequests.begin();
       it != mRequests.end(); ++it) {
    if (!(*it)->IsComplete()) {
      (*it)->PostCancelRequest();
    }
  }
}

bool CFilePreloadData::IsReady() {
  if (!mRequests.empty()) {
    if (!mRequests.back()->IsComplete()) {
      return false;
    }
    mRequests = rstl::vector< rstl::auto_ptr< CDvdRequest > >();
  }
  return true;
}

void CFilePreloadData::Read(void* dest, int offset, int length) {
  const int chunk = offset / 0x4000;
  int firstLength = (chunk + 1) * 0x4000 - offset;
  if (length < firstLength) {
    firstLength = length;
  }
  CopyAndFlush(dest, mBuffers[chunk].get() + (offset - chunk * 0x4000), firstLength);

  uchar* output = static_cast< uchar* >(dest) + firstLength;
  int remaining = length - firstLength;
  int nextChunk = chunk + 1;
  while (remaining != 0) {
    int count = 0x4000;
    if (remaining <= 0x4000) {
      count = remaining;
    }
    CopyAndFlush(output, mBuffers[nextChunk].get(), count);
    remaining -= count;
    output += count;
    ++nextChunk;
  }
}

static rstl::list< rstl::auto_ptr< CFilePreloadData > >::iterator FindFile(
    const rstl::string& path) {
  for (rstl::list< rstl::auto_ptr< CFilePreloadData > >::iterator it = sPreloadedFiles.begin();
       it != sPreloadedFiles.end(); ++it) {
    const int comparison = CStringExtras::CompareCaseInsensitive((*it)->GetFilename(), path);
    if (comparison == 0) {
      return it;
    }
  }
  return sPreloadedFiles.end();
}

static CFilePreloadData* AcquireFile(const rstl::string& path) {
  rstl::list< rstl::auto_ptr< CFilePreloadData > >::iterator it = FindFile(path);
  if (it == sPreloadedFiles.end()) {
    rstl::auto_ptr< CFilePreloadData > data(rs_new CFilePreloadData(path));
    it = sPreloadedFiles.insert(sPreloadedFiles.end(), data);
  } else {
    ++(*it)->mRefCount;
  }
  return (*it).get();
}

static void ReleaseFile(const rstl::string& path) {
  rstl::list< rstl::auto_ptr< CFilePreloadData > >::iterator it = FindFile(path);
  if (it != sPreloadedFiles.end()) {
    --(*it)->mRefCount;
    if ((*it)->mRefCount == 0) {
      sPreloadedFiles.erase(it);
    }
  }
}

CFilePreload::CFilePreload(const rstl::string& path) : mData(AcquireFile(path)) {}

CFilePreload::CFilePreload(const CFilePreload& other) : mData(other.mData) {
  ++mData->mRefCount;
}

CFilePreload::~CFilePreload() { ReleaseFile(mData->GetFilename()); }

void CFilePreload::operator=(const CFilePreload& other) {
  if (mData != other.mData) {
    ReleaseFile(mData->GetFilename());
    mData = other.mData;
    ++mData->mRefCount;
  }
}

bool CFilePreload::IsReady() const { return mData->IsReady(); }

void CFilePreload::Read(void* dest, int offset, int length) const {
  mData->Read(dest, offset, length);
}
