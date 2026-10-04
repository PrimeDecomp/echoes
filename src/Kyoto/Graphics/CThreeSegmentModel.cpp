#include "Kyoto/Graphics/CThreeSegmentModel.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CDisplayListReader.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

#include "rstl/vector.hpp"

#include <dolphin/os.h>
#include <string.h>

namespace {
// Display-list counts and position indices are unaligned big-endian shorts.
inline ushort ReadDisplayListShort(const uchar* data) {
  uchar bytes[2];
  bytes[0] = data[0];
  bytes[1] = data[1];
#ifdef __MWERKS__
  return CBasics::SwapBytes(*reinterpret_cast< const ushort* >(bytes));
#else
  ushort value;
  memcpy(&value, bytes, sizeof(value));
  return CBasics::SwapBytes(value);
#endif
}
} // namespace

CThreeSegmentModel::CThreeSegmentModel(const TToken< CModel >& model, float lowerX, float upperX,
                                       const CVector3f& lowerOffset, const CVector3f& middleOffset,
                                       const CVector3f& upperOffset)
: mModel(model, true), mPositions(nullptr), mVertexCount(0), mDisplayListSize(0) {
  const CCubeModel& cubeModel = *mModel.GetObject()->GetModelInstance();
  const rstl::vector< void* >& surfaces = cubeModel.GetModelInstance().Surfaces();
  const CCubeMaterial material = cubeModel.GetMaterial(CCubeSurface(surfaces.front()));
  const uint vertexStride = CDisplayListReader::GetVertexStride(material.GetVertexDesc());
  int maxVertexIndex = 0;
  uint displayListSize = 0;
  for (int i = 0; i < surfaces.size(); ++i) {
    const CCubeSurface surface(surfaces[i]);
    const uchar* cursor = static_cast< const uchar* >(surface.GetDisplayList());
    const uchar* end = cursor + surface.GetDisplayListSize();
    while (cursor < end && *cursor != 0) {
      const ushort count = ReadDisplayListShort(cursor + 1);
      displayListSize += 3 + (vertexStride + 1) * count;
      cursor += 3;
      for (int j = 0; j < count; ++j) {
        const ushort index = ReadDisplayListShort(cursor);
        if (index > maxVertexIndex) {
          maxVertexIndex = index;
        }
        cursor += vertexStride;
      }
    }
  }

  mVertexCount = maxVertexIndex + 1;
  rstl::vector< uchar > segmentMatrixIndices(mVertexCount);
  mPositions = static_cast< CVector3f* >(
      CMemory::Alloc(mVertexCount * sizeof(CVector3f), IAllocator::kHI_RoundUpLen));
  const CVector3f* positions =
      static_cast< const CVector3f* >(cubeModel.GetModelInstance().GetVertexPointer());
  for (int i = 0; i < mVertexCount; ++i) {
    if (positions[i].GetX() < lowerX) {
      mPositions.get()[i] = positions[i] + lowerOffset;
      segmentMatrixIndices[i] = kS_LowerX * 3;
    } else if (positions[i].GetX() < upperX) {
      mPositions.get()[i] = positions[i] + middleOffset;
      segmentMatrixIndices[i] = kS_MiddleX * 3;
    } else {
      mPositions.get()[i] = positions[i] + upperOffset;
      segmentMatrixIndices[i] = kS_UpperX * 3;
    }
  }
  DCFlushRange(mPositions.get(), mVertexCount * sizeof(CVector3f));

  mDisplayListSize = (displayListSize + 31) & ~31;
  for (int list = 0; list < kS_Count; ++list) {
    rstl::single_ptr< uchar > data(
        static_cast< uchar* >(CMemory::Alloc(mDisplayListSize, IAllocator::kHI_RoundUpLen)));
    uchar* output = data.get();
    for (int i = 0; i < surfaces.size(); ++i) {
      const CCubeSurface surface(surfaces[i]);
      const uchar* source = static_cast< const uchar* >(surface.GetDisplayList());
      const uchar* cursor = source;
      const uchar* end = source + surface.GetDisplayListSize();
      while (cursor < end && *cursor != 0) {
        output[0] = cursor[0];
        output[1] = cursor[1];
        output[2] = cursor[2];
        output += 3;
        const ushort count = ReadDisplayListShort(cursor + 1);
        cursor += 3;
        for (int j = 0; j < count; ++j) {
          *output++ =
              GX_PNMTX1 + list * kS_Count * 3 + segmentMatrixIndices[ReadDisplayListShort(cursor)];
          memcpy(output, cursor, vertexStride);
          cursor += vertexStride;
          output += vertexStride;
        }
      }
      DCInvalidateRange(const_cast< uchar* >(source), surface.GetDisplayListSize());
    }
    memset(output, 0, data.get() + mDisplayListSize - output);
    DCFlushRange(data.get(), mDisplayListSize);
    mDisplayLists[list] = data;
  }
}

CThreeSegmentModel::~CThreeSegmentModel() {
  CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                        mPositions.release());
  for (int i = 0; i < kS_Count; ++i) {
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                          mDisplayLists[i].release());
  }
}

void CThreeSegmentModel::SetSegmentTransforms(int matrixGroup, const CTransform4f& lower,
                                              const CTransform4f& middle,
                                              const CTransform4f& upper) const {
  Mtx lowerModelView;
  Mtx middleModelView;
  Mtx upperModelView;
  PSMTXConcat(CGraphics::GetCameraMtx(), lower.GetCStyleMatrix(), lowerModelView);
  PSMTXConcat(CGraphics::GetCameraMtx(), middle.GetCStyleMatrix(), middleModelView);
  PSMTXConcat(CGraphics::GetCameraMtx(), upper.GetCStyleMatrix(), upperModelView);
  const int firstMatrix = matrixGroup * kS_Count * 3;
  GXLoadPosMtxImm(lowerModelView, firstMatrix + GX_PNMTX1);
  GXLoadPosMtxImm(middleModelView, firstMatrix + GX_PNMTX2);
  GXLoadPosMtxImm(upperModelView, firstMatrix + GX_PNMTX3);
}

void CThreeSegmentModel::DrawDisplayList(int index) const {
  CGX::CallDisplayList(mDisplayLists[index].get(), mDisplayListSize);
}

void CThreeSegmentModel::SetMaterialCurrent(const CModelFlags& flags) const {
  const CModel& model = *mModel.GetObject();
  model.PreDrawModel(flags);
  model.Touch(0);
  const CCubeModel& cubeModel = *model.GetModelInstance();
  const CCubeSurface surface(cubeModel.GetModelInstance().Surfaces().front());
  cubeModel.GetMaterial(surface).SetCurrent(flags, surface, cubeModel);
  cubeModel.SetArraysCurrent();
  if (mVertexCount != 0) {
    CGX::SetArray(GX_VA_POS, mPositions.get(), sizeof(CVector3f));
  }
  CGX::SetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
}

void CThreeSegmentModel::ResetRenderState() const {
  CGX::SetVtxDesc(GX_VA_PNMTXIDX, GX_NONE);
  GXSetCurrentMtx(GX_PNMTX0);
}
