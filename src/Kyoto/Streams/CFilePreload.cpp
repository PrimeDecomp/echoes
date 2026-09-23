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

  rstl::string GetFilename() const { return x0_path; }
  bool IsReady();
  void Read(void* dest, int offset, int length);

  rstl::string x0_path;
  int x10_size;
  int x14_refCount;
  rstl::vector< rstl::auto_ptr< uchar > > x18_buffers;
  rstl::vector< rstl::auto_ptr< CDvdRequest > > x28_requests;
};
CHECK_SIZEOF(CFilePreloadData, 0x38)

static rstl::list< rstl::auto_ptr< CFilePreloadData > > sPreloadedFiles;

static void CopyAndFlush(void* dest, const void* src, int length) {
  memcpy(dest, src, length);
  DCFlushRange(dest, length);
}

CFilePreloadData::CFilePreloadData(const rstl::string& path)
: x0_path(path), x10_size(0), x14_refCount(1), x18_buffers(), x28_requests() {
  CDvdFile file(path.data());
  x10_size = file.GetFileSize();
  int offset;
  const int count = (x10_size + 0x3fff) / 0x4000;
  x18_buffers.reserve(count);
  x28_requests.reserve(count);

  int i = 0;
  offset = 0;
  for (; i < count; ++i, offset += 0x4000) {
    int length = 0x4000;
    const int remaining = x10_size - offset;
    if (remaining <= 0x4000) {
      length = remaining;
    }
    const uint alignedLength = (length + 31) & ~31;
    rstl::auto_ptr< uchar > buffer(static_cast< uchar* >(
        CMemory::Alloc(alignedLength, IAllocator::kHI_RoundUpLen)));
    rstl::auto_ptr< CDvdRequest > request(
        file.AsyncSeekRead(buffer.get(), alignedLength, kSO_Set, offset));
    x18_buffers.push_back_unsafe(buffer);
    x28_requests.push_back_unsafe(request);
  }
}

CFilePreloadData::~CFilePreloadData() {
  for (rstl::vector< rstl::auto_ptr< CDvdRequest > >::iterator it = x28_requests.begin();
       it != x28_requests.end(); ++it) {
    if (!(*it)->IsComplete()) {
      (*it)->PostCancelRequest();
    }
  }
}

bool CFilePreloadData::IsReady() {
  if (!x28_requests.empty()) {
    if (!x28_requests.back()->IsComplete()) {
      return false;
    }
    x28_requests = rstl::vector< rstl::auto_ptr< CDvdRequest > >();
  }
  return true;
}

void CFilePreloadData::Read(void* dest, int offset, int length) {
  const int chunk = offset / 0x4000;
  int firstLength = (chunk + 1) * 0x4000 - offset;
  if (length < firstLength) {
    firstLength = length;
  }
  CopyAndFlush(dest, x18_buffers[chunk].get() + (offset - chunk * 0x4000), firstLength);

  uchar* output = static_cast< uchar* >(dest) + firstLength;
  int remaining = length - firstLength;
  int nextChunk = chunk + 1;
  while (remaining != 0) {
    int count = 0x4000;
    if (remaining <= 0x4000) {
      count = remaining;
    }
    CopyAndFlush(output, x18_buffers[nextChunk].get(), count);
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
    ++(*it)->x14_refCount;
  }
  return (*it).get();
}

static void ReleaseFile(const rstl::string& path) {
  rstl::list< rstl::auto_ptr< CFilePreloadData > >::iterator it = FindFile(path);
  if (it != sPreloadedFiles.end()) {
    --(*it)->x14_refCount;
    if ((*it)->x14_refCount == 0) {
      sPreloadedFiles.erase(it);
    }
  }
}

CFilePreload::CFilePreload(const rstl::string& path) : x0_data(AcquireFile(path)) {}

CFilePreload::CFilePreload(const CFilePreload& other) : x0_data(other.x0_data) {
  ++x0_data->x14_refCount;
}

CFilePreload::~CFilePreload() { ReleaseFile(x0_data->GetFilename()); }

void CFilePreload::operator=(const CFilePreload& other) {
  if (x0_data != other.x0_data) {
    ReleaseFile(x0_data->GetFilename());
    x0_data = other.x0_data;
    ++x0_data->x14_refCount;
  }
}

bool CFilePreload::IsReady() const { return x0_data->IsReady(); }

void CFilePreload::Read(void* dest, int offset, int length) const {
  x0_data->Read(dest, offset, length);
}
