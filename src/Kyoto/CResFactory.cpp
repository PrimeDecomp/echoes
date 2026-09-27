#include "Kyoto/CResFactory.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/Streams/CLZOSupport.hpp"

#include <dolphin/os.h>

// Written by CMFGameLoader; its purpose is not yet identified.
int gResFactoryUnknown = 2;

CResFactory::CResFactory() {}

CResFactory::~CResFactory() {}

void CResFactory::AddToLoadList(const SLoadingData& data) {
  LoadList::iterator pos = mLoadList.end();
  if (data.mDvdReq->GetMediaType() == 0) {
    pos = mLoadList.begin();
    while (pos != mLoadList.end()) {
      if (pos->mDvdReq->GetMediaType() == 1) {
        break;
      }
      ++pos;
    }
  }
  LoadList::iterator it = mLoadList.insert(pos, data);
  mLoadMap.insert(rstl::pair< SObjectTag, LoadList::iterator >(data.mTag, it));
}

void CResFactory::EraseFromLoadList(const LoadList::iterator& it) {
  mLoadMap.erase(it->mTag);
  mLoadList.erase(it);
}

CResFactory::LoadList::iterator CResFactory::FindInLoadList(const SObjectTag& tag) {
  rstl::map< SObjectTag, LoadList::iterator >::const_iterator it = mLoadMap.find(tag);
  if (it == mLoadMap.end()) {
    return mLoadList.end();
  }
  return it->second;
}

rstl::auto_ptr< IObj > CResFactory::Build(const SObjectTag& tag, const CVParamTransfer& params) {
  LoadList::iterator it = FindInLoadList(tag);
  if (it != mLoadList.end()) {
    IObj** target = it->mTarget;
    while (*target == nullptr) {
      while (!PumpResource(it, 0)) {
      }
    }
    return rstl::auto_ptr< IObj >(*target);
  }
  return BuildSync(tag, params);
}

rstl::auto_ptr< IObj > CResFactory::BuildSync(const SObjectTag& tag,
                                              const CVParamTransfer& params) {
  if (mFactoryMgr.CanMakeMemory(tag)) {
    char* buffer;
    int size;
    mResLoader.LoadMemResourceSync(tag, &buffer, &size);
    return mFactoryMgr.MakeObjectFromMemory(
        tag, rstl::auto_ptr< uchar >(reinterpret_cast< uchar* >(buffer)), size,
        mResLoader.GetResourceCompression(tag) != CResLoader::kCompressionType_Uncompressed,
        params);
  }
  CInputStream* in = mResLoader.LoadNewResourceSync(tag, nullptr);
  rstl::auto_ptr< IObj > result = mFactoryMgr.MakeObject(tag, *in, params);
  delete in;
  return result;
}

void CResFactory::BuildAsync(const SObjectTag& tag, const CVParamTransfer& params, IObj** target) {
  *target = nullptr;
  const uint size = ResourceSize(tag);
  CBufferedDvdRequest* request = mResLoader.LoadResourceAsync(tag);
  const bool ownsBuffer = request->OwnsBuffer();
  rstl::auto_ptr< uchar > buffer;
  if (ownsBuffer) {
    buffer = request->GetBuffer();
  } else {
    buffer = rstl::auto_ptr< uchar >(request->GetBufferPtr());
    buffer.release();
  }
  SLoadingData data(tag, request, target, buffer, size, mResLoader.GetResourceCompression(tag),
                    params);
  AddToLoadList(data);
}

void CResFactory::CancelBuild(const SObjectTag& tag) {
  LoadList::iterator it = FindInLoadList(tag);
  if (it != mLoadList.end()) {
    SLoadingData& data = *it;
    data.mDvdReq->PostCancelRequest();
    mCancelledList.push_back(data);
    EraseFromLoadList(it);
  }
}

void CResFactory::AsyncIdle(uint time, bool keepPumping) {
  CStopwatch timer;
  LoadList::iterator it = mCancelledList.begin();
  while (it != mCancelledList.end()) {
    LoadList::iterator current = it;
    ++it;
    if (current->mDvdReq->IsComplete()) {
      mCancelledList.erase(current);
    }
  }

  bool done = false;
  while (!done && mLoadList.size() != 0) {
    done = true;
    const uint elapsed = timer.GetElapsedMicros();
    if (elapsed < time && mLoadList.size() != 0) {
      done = !PumpResource(mLoadList.begin(), time - elapsed);
      if (keepPumping) {
        done = false;
      }
    }
  }
}

bool CResFactory::PumpResource(const LoadList::iterator& it, uint time) {
  if (it->mDvdReq->IsComplete()) {
    if (it->mCompression == CResLoader::kCompressionType_Compressed &&
        !it->PumpDecompression(time)) {
      return false;
    }
    SLoadingData data(*it);
    EraseFromLoadList(it);
    *data.mTarget =
        mFactoryMgr
            .MakeObjectFromMemory(data.mTag, data.mBuffer, data.mSize,
                                  data.mCompression != CResLoader::kCompressionType_Uncompressed,
                                  data.mParams)
            .release();
    return true;
  }
  return false;
}

CResFactory::SLoadingData::SLoadingData(const SObjectTag& tag, CDvdRequest* request, IObj** target,
                                        const rstl::auto_ptr< uchar >& buffer, int size,
                                        CResLoader::ECompressionType compression,
                                        const CVParamTransfer& params)
: mTag(tag)
, mDvdReq(request)
, mTarget(target)
, mBuffer(buffer)
, mCompressedPos(0)
, mDecompressedPos(0)
, mSize(size)
, mCompression(compression)
, mParams(params) {}

CResFactory::SLoadingData::~SLoadingData() {}

bool CResFactory::SLoadingData::PumpDecompression(uint time) {
  CStopwatch timer;
  uint* buffer = reinterpret_cast< uint* >(mBuffer.get());
  const uint length = *buffer;
  if (mDecompBuffer.null()) {
    mDecompBuffer = rstl::auto_ptr< uchar >(
        static_cast< uchar* >(CMemory::Alloc(length, IAllocator::kHI_RoundUpLen)));
  }

  uchar* compressed = reinterpret_cast< uchar* >(buffer + 1);
  while ((time == 0 || timer.GetElapsedMicros() < time) && length != mDecompressedPos) {
    const uint chunkSize = *reinterpret_cast< ushort* >(compressed + mCompressedPos);
    mCompressedPos += 2;
    uint outSize = length - mDecompressedPos;
    CLZOSupport::Inflate(compressed + mCompressedPos, chunkSize,
                         mDecompBuffer.get() + mDecompressedPos, outSize);
    DCStoreRange(mDecompBuffer.get() + mDecompressedPos, outSize);
    mCompressedPos += chunkSize;
    mDecompressedPos += outSize;
  }

  if (length == mDecompressedPos) {
    mBuffer = mDecompBuffer;
    mDecompBuffer = rstl::auto_ptr< uchar >();
    mCompression = CResLoader::kCompressionType_Uncompressed;
    mSize = length;
    return true;
  }
  return false;
}
