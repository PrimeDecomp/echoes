#include "Kyoto/CResLoader.hpp"

#include "Kyoto/Alloc/CMemory.hpp"

// Echoes-only TU. CLookaheadRes is named by a Corruption diagnostic; CBufferedDvdRequest is still a
// guess.

CLookaheadRes::CLookaheadRes(uchar* buffer, CDvdFile* file, uint offset, uint size,
                             const rstl::auto_ptr< CDvdRequest >& request, CResLoader* owner)
: mBuffer(buffer)
, mFile(file)
, mOffset(offset)
, mSize(size)
, mReferenceCount(0)
, mInvalid(false)
, mRequest(request)
, mOwner(owner) {}

CLookaheadRes::~CLookaheadRes() {}

void CLookaheadRes::RequestHasDied() {
  --mReferenceCount;
  if (mReferenceCount == 0) {
    mOwner->KillLookahead(this);
  }
}

rstl::auto_ptr< CBufferedDvdRequest > CLookaheadRes::MakeRequest(uint offset) {
  ++mReferenceCount;
  return rstl::auto_ptr< CBufferedDvdRequest >(
      rs_new CBufferedDvdRequest(this, mRequest.get(), mBuffer.get() + (offset - mOffset)));
}

bool CLookaheadRes::Contains(const CDvdFile* file, uint offset, uint size) const {
  if (file != mFile) {
    return false;
  }
  if (offset >= mOffset && offset < mOffset + mSize) {
    return offset + size <= mOffset + mSize;
  }
  return false;
}

bool CLookaheadRes::Cancel() {
  if (mReferenceCount > 1) {
    return true;
  }
  mInvalid = true;
  mRequest->PostCancelRequest();
  return mRequest->IsComplete();
}

CBufferedDvdRequest::CBufferedDvdRequest(CLookaheadRes* cache, CDvdRequest* request, uchar* buffer)
: mCache(cache), mRequest(request), mBuffer(buffer), mMediaType(request->GetMediaType()) {
  if (mCache != nullptr) {
    mRequest.release();
    mBuffer.release();
  }
}

CBufferedDvdRequest::~CBufferedDvdRequest() {
  if (mCache != nullptr) {
    mCache->RequestHasDied();
    mCache = nullptr;
  }
}

rstl::auto_ptr< uchar >& CBufferedDvdRequest::GetBuffer() { return mBuffer; }

void CBufferedDvdRequest::WaitUntilComplete() {
  if (mRequest.get() != nullptr) {
    mRequest->WaitUntilComplete();
  }
}

bool CBufferedDvdRequest::IsComplete() {
  if (mRequest.get() == nullptr) {
    return true;
  }
  return mRequest->IsComplete();
}

void CBufferedDvdRequest::PostCancelRequest() {
  if (mRequest.get() == nullptr) {
    return;
  }
  if (mCache != nullptr) {
    if (mCache->Cancel()) {
      mCache->RequestHasDied();
      mCache = nullptr;
      mRequest = rstl::auto_ptr< CDvdRequest >();
    }
  } else {
    mRequest->PostCancelRequest();
  }
}

int CBufferedDvdRequest::GetMediaType() const { return mMediaType; }
