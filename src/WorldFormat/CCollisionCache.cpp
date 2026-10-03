#include "WorldFormat/CCollisionCache.hpp"

void CCollisionCacheWriter::FinishLeaf() {}

void CCollisionCacheWriter::BeginLeaf(const CAABox& bounds) {}

void CCollisionCacheWriter::ReserveWords(int count) {}

void CCollisionCacheWriter::BeginGeometry(const CCollisionPrimitiveData& geometry,
                                          const CTransform4f* transform, short id, u64 flags) {}

CCollisionCacheWriter::~CCollisionCacheWriter() {}

CCollisionCacheWriter::CCollisionCacheWriter(CCollisionCache& cache) : mCache(cache) {}

void CCollisionCache::RemoveGeometry(CCollisionCacheIterator& iterator) {}

uint CCollisionCache::SkipGeometry(CCollisionCacheIterator& iterator) {}

uint CCollisionCache::ReadGeometry(CCollisionCacheIterator& iterator) {}

void CCollisionCache::ReadLeaf(CCollisionCacheIterator& iterator) {}

const CCachedCollisionSurface*
CCollisionCache::NextTriangle(CCollisionCacheIterator& iterator) const {}

int CCollisionCache::GetTriangleStride() const {}

void CCollisionCache::Reset() {}

void CCollisionCache::SetBounds(const CAABox& bounds) {}

CCollisionCache::CCollisionCache(const CAABox& bounds, int x2c, int x30, ushort x34)
: mBounds(bounds) {}

CCollisionCache::~CCollisionCache() {}

CCollisionCacheIterator::CCollisionCacheIterator() {}

CCollisionCacheIterator::CCollisionCacheIterator(CCollisionCache& cache) {}

bool CCollisionCacheIterator::MatchesGeometry(short id, const CCollisionPrimitiveData* geometry,
                                              const CTransform4f& transform, u64 flags) const {}

void CCollisionCacheIterator::Reset() {}

void rstl::locked_cache_allocator::Allocate(void*& out, uint size) {}
