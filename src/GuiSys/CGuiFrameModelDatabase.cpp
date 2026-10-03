#include "GuiSys/CGuiFrameModelDatabase.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include <dolphin/os.h>

// Guessed local helper name; serialized lengths determine the section boundaries.
static uchar* MemoryFromPartData(uchar*& dataCur, int*& sectionSizeCur) {
  uchar* result;
  if (*sectionSizeCur != 0) {
    result = dataCur;
    dataCur += *sectionSizeCur;
  } else {
    result = nullptr;
  }
  ++sectionSizeCur;
  return result;
}

CGuiFrameModelDatabase::CGuiFrameModelDatabase(CInputStream& in, CSimplePool* pool)
: mBufferSize(in.ReadInt32())
, mBuffer(mBufferSize != 0
              ? static_cast< uchar* >(CMemory::Alloc(mBufferSize, IAllocator::kHI_RoundUpLen))
              : nullptr)
, mModels(in.ReadInt32(), rstl::auto_ptr< CCubeModel >())
, mSurfaces(mModels.size(), rstl::vector< void* >()) {
  CModel::AddToTotal(mBufferSize);
  rstl::vector< int > sectionSizes(in);
  if (!sectionSizes.empty()) {
    in.Get(mBuffer.get(), mBufferSize);

    int* sectionSizeCur = sectionSizes.data();
    const int modelCount = mModels.size();
    uchar* dataCur = mBuffer.get();
    const void* materialData = MemoryFromPartData(dataCur, sectionSizeCur);
    CCubeModel::MakeTexturesFromMats(materialData, mTextures, *pool, true);

    for (int i = 0; i < modelCount; ++i) {
      const void* positions = MemoryFromPartData(dataCur, sectionSizeCur);
      const void* normals = MemoryFromPartData(dataCur, sectionSizeCur);
      const void* colors = MemoryFromPartData(dataCur, sectionSizeCur);
      const void* uvs = MemoryFromPartData(dataCur, sectionSizeCur);
      const void* packedUvs = MemoryFromPartData(dataCur, sectionSizeCur);
      const uint surfaceCount = CBasics::SwapBytes(
          *reinterpret_cast< const uint* >(MemoryFromPartData(dataCur, sectionSizeCur)));
      rstl::vector< void* >& surfaces = mSurfaces[i];
      surfaces.reserve(surfaceCount);
      for (uint j = 0; j < surfaceCount; ++j) {
        surfaces.push_back_unsafe(MemoryFromPartData(dataCur, sectionSizeCur));
      }

      const rstl::auto_ptr< CCubeModel > model(rs_new CCubeModel(
          &surfaces, &mTextures, materialData, positions, normals, colors, uvs, packedUvs,
          CAABox::Identity(), 0, true, i));
      mModels[i] = model;
    }
  }

  DCFlushRange(mBuffer.get(), mBufferSize);
}

CGuiFrameModelDatabase::~CGuiFrameModelDatabase() {
  CModel::RemoveFromTotal(mBufferSize);
  CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                       mBuffer.release());
}

void CGuiFrameModelDatabase::Draw(int index, const CModelFlags& flags) const {
  mModels[index]->Draw(flags);
}

const CCubeModel* CGuiFrameModelDatabase::GetModel(int index) const {
  return mModels[index].get();
}
