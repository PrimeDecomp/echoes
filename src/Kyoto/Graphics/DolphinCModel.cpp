#include "Kyoto/Graphics/CModel.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

#include <dolphin/gx.h>
#include <dolphin/os.h>

// Unidentified GX scratch buffer helpers (not yet split).
extern "C" void* fn_8032F404(int size);
extern "C" void fn_8032F3CC();

static bool sIsTextureTimeoutEnabled = true;
uint CModel::sTotalMemory = 0;
CModel::SShader* CModel::sThisFrameList = nullptr;
CModel::SShader* CModel::sOneFrameList = nullptr;
CModel::SShader* CModel::sTwoFrameList = nullptr;
static uint sFrameCounter = 0;

namespace {
// GX matrix arrays must be 32-byte aligned.
struct SSkinMatrices {
  CTransform4f mModelView;
  float mNormal[3][3];
} ATTRIBUTE_ALIGN(32);

class CSkinMatricesGuard {
public:
  CSkinMatricesGuard(void* matrices) : mActive(matrices != nullptr) {}
  ~CSkinMatricesGuard() {
    if (mActive) {
      fn_8032F3CC();
      GXSetCurrentMtx(GX_PNMTX0);
    }
  }

private:
  bool mActive;
};
} // namespace

static uchar* MemoryFromPartData(uchar*& dataCur, int*& secSizeCur) {
  uchar* ret = *secSizeCur != 0 ? dataCur : nullptr;
  dataCur += *secSizeCur;
  secSizeCur++;
  return ret;
}

CModel::SShader::SShader(uchar* data, CModel* owner)
: mData(data), mOwner(owner), mPrev(nullptr), mNext(nullptr) {}

CModel::SShader::SShader(const SShader& other)
: mTextures(other.mTextures)
, mData(other.mData)
, mOwner(other.mOwner)
, mPrev(nullptr)
, mNext(nullptr) {}

CModel::SShader::~SShader() { RemoveFromList(); }

void CModel::SShader::MoveToThisFrameList() {
  if (sThisFrameList == this) {
    return;
  }
  RemoveFromList();

  SShader* head = sThisFrameList;
  if (head != nullptr) {
    mNext = head;
    head->mPrev = this;
  }

  sThisFrameList = this;
}

void CModel::SShader::RemoveFromList() {
  if (mPrev != nullptr) {
    mPrev->mNext = mNext;
  } else if (this == sThisFrameList) {
    sThisFrameList = mNext;
  } else if (this == sOneFrameList) {
    sOneFrameList = mNext;
  } else if (this == sTwoFrameList) {
    sTwoFrameList = mNext;
  }

  if (mNext != nullptr) {
    mNext->mPrev = mPrev;
  }

  mPrev = nullptr;
  mNext = nullptr;
}

void CModel::SShader::UnlockTextures() {
  rstl::vector< TCachedToken< CTexture > >::iterator it = mTextures.begin();
  rstl::vector< TCachedToken< CTexture > >::iterator end = mTextures.end();
  for (; it != end; ++it) {
    it->Unlock();
  }
}

CModel::CModel(const rstl::auto_ptr< uchar >& data, int length, IObjectStore& store)
: mData(data.release())
, mDataLen(length)
, mModelInstance(nullptr)
, mLastFrame(CGraphics::GetFrameCounter() - 2)
, mCurrentMatxIdx(0)
, x30_16_(false)
, mHasSkinMatrices(false) {
  uchar* dataPtr = data.get();
  mHasSkinMatrices = *reinterpret_cast< const uint* >(dataPtr + 8) & 1;
  const int sectionCount = *reinterpret_cast< const int* >(dataPtr + 0x24);
  const uint flags = *reinterpret_cast< const uint* >(dataPtr + 8);
  const uint visorFlags = (flags >> 1) & 1;
  const uint hasShortUvs = (flags >> 2) & 1;
  const int numMatSets = *reinterpret_cast< const int* >(dataPtr + 0x28);

  uchar* dataCur = dataPtr + ((0x2c + sectionCount * 4 + 31) & ~31);
  int* secSizeCur = reinterpret_cast< int* >(dataPtr + 0x2c);
  mMatSets.reserve(numMatSets);
  for (int i = 0; i < numMatSets; ++i) {
    mMatSets.push_back_unsafe(SShader(MemoryFromPartData(dataCur, secSizeCur), this));
    SShader& shader = mMatSets.back();
    CCubeModel::MakeTexturesFromMats(shader.mData, shader.mTextures, store, true);
    for (int j = 0; j < shader.mTextures.size(); ++j) {
      shader.mTextures[j].Lock();
    }
    mDataLen += shader.mTextures.size() * 12;
  }

  const void* positions = MemoryFromPartData(dataCur, secSizeCur);
  const void* normals = MemoryFromPartData(dataCur, secSizeCur);
  const void* vtxColors = MemoryFromPartData(dataCur, secSizeCur);
  const void* floatUvs = MemoryFromPartData(dataCur, secSizeCur);
  const void* shortUvs = nullptr;
  if (hasShortUvs) {
    shortUvs = MemoryFromPartData(dataCur, secSizeCur);
  }

  const uint surfaceCount = *reinterpret_cast< uint* >(MemoryFromPartData(dataCur, secSizeCur));
  mSurfaces.reserve(surfaceCount);
  for (uint i = 0; i < surfaceCount; ++i) {
    mSurfaces.push_back_unsafe(MemoryFromPartData(dataCur, secSizeCur));
  }

  mModelInstance = rs_new CCubeModel(
      &mSurfaces, &mMatSets.front().mTextures, mMatSets.front().mData, positions, normals,
      vtxColors, floatUvs, shortUvs, *reinterpret_cast< const CAABox* >(dataPtr + 0xc),
      visorFlags ? 1 : 0, true, -1);
  mDataLen += mSurfaces.size() * 4;
  AddToTotal(mDataLen);
  DCFlushRange(mData.get(), length);
}

CModel::~CModel() {
  RemoveFromTotal(mDataLen);
  const int frame = CGraphics::GetFrameCounter();
  if (mLastFrame == frame) {
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                          mData.release());
  } else if (mLastFrame == frame - 1) {
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_ThisFrame,
                                          mData.release());
  }
}

void CModel::PreDrawModel(const CModelFlags& flags) const {
  CCubeMaterial::ResetCachedMaterials();
  UpdateLastFrame();
  VerifyCurrentShader(flags.GetShaderSet());
}

void* CModel::SetupSkinMatrices() const {
  if (!mHasSkinMatrices) {
    return nullptr;
  }

  SSkinMatrices* matrices = static_cast< SSkinMatrices* >(fn_8032F404(sizeof(SSkinMatrices)));
  matrices->mModelView = CGraphics::GetGXModelView();
  float (*normal)[3] = matrices->mNormal;
  const Mtx& invXpose = CGraphics::GetGXModelViewInvXpose();
  for (int i = 0; i < 3; ++i) {
    normal[i][0] = invXpose[i][0];
    normal[i][1] = invXpose[i][1];
    normal[i][2] = invXpose[i][2];
  }
  DCFlushRange(matrices, 0x54);
  CGX::SetArray(GX_POS_MTX_ARRAY, &matrices->mModelView, sizeof(SSkinMatrices));
  CGX::SetArray(GX_NRM_MTX_ARRAY, normal, sizeof(SSkinMatrices));
  for (int i = 1; i < 10; ++i) {
    GXLoadPosMtxIndx(0, i * 3);
    GXLoadNrmMtxIndx3x3(0, i * 3);
  }
  return matrices;
}

void CModel::Draw(const CModelFlags& flags) const {
  PreDrawModel(flags);
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(kSS_All);
  }
  CSkinMatricesGuard guard(SetupSkinMatrices());
  mModelInstance->Draw(flags);
}

void CModel::Draw(u64 mask, const CModelFlags& flags) const {
  PreDrawModel(flags);
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(kSS_All);
  }
  CSkinMatricesGuard guard(SetupSkinMatrices());
  mModelInstance->Draw(mask, flags);
}

void CModel::DrawUnsortedParts(const CModelFlags& flags) const {
  PreDrawModel(flags);
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(kSS_Unsorted);
  }
  CSkinMatricesGuard guard(SetupSkinMatrices());
  mModelInstance->DrawNormal(flags);
}

void CModel::DrawSortedParts(const CModelFlags& flags) const {
  PreDrawModel(flags);
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(kSS_Sorted);
  }
  CSkinMatricesGuard guard(SetupSkinMatrices());
  mModelInstance->DrawAlpha(flags);
}

void CModel::DolphinDrawFlat(EDrawFlatFlags flags) const {
  CCubeMaterial::ResetCachedMaterials();
  UpdateLastFrame();
  CSkinMatricesGuard guard(SetupSkinMatrices());
  mModelInstance->DrawFlat(flags);
}

void CModel::VerifyCurrentShader(int shader) const {
  if (shader >= mMatSets.size()) {
    shader = 0;
  }
  SShader& material = mMatSets[shader];
  if (shader != mCurrentMatxIdx) {
    mModelInstance->RemapMaterialData(material.mData, &material.mTextures);
    mCurrentMatxIdx = shader;
  }
  if (x30_16_) {
    material.MoveToThisFrameList();
  }
}

CFactoryFnReturn FModelFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& ptr,
                                     int len, const CVParamTransfer& xfer) {
  rstl::rc_ptr< IVParamObj > obj = xfer.GetObj();
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  GXInvalidateVtxCache();
  return rs_new CModel(ptr, len, *pool);
}

const float* CModel::GetPositions() const {
  return static_cast< const float* >(mModelInstance->GetPositions());
}

const float* CModel::GetNormals() const {
  return static_cast< const float* >(mModelInstance->GetNormals());
}

void CModel::Touch(int shader) const {
  UpdateLastFrame();
  VerifyCurrentShader(shader);
  mModelInstance->TryLockTextures();
}

bool CModel::IsLoaded(int shader) const {
  VerifyCurrentShader(shader);

  const rstl::vector< TCachedToken< CTexture > >& textures = mModelInstance->GetTextures();
  for (rstl::vector< TCachedToken< CTexture > >::const_iterator it = textures.begin();
       it != textures.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  return true;
}

void CModel::FrameDone() {
  ++sFrameCounter;
  if (!sIsTextureTimeoutEnabled) {
    return;
  }

  for (SShader* shader = sTwoFrameList; shader != nullptr;) {
    SShader* next = shader->mNext;
    CCubeModel* instance = shader->mOwner->mModelInstance.get();
    if (instance->GetTexturesPtr() == &shader->mTextures) {
      instance->UnlockTextures();
    } else {
      shader->UnlockTextures();
    }
    shader->mNext = nullptr;
    shader->mPrev = nullptr;
    shader = next;
  }

  sTwoFrameList = sOneFrameList;
  sOneFrameList = sThisFrameList;
  sThisFrameList = nullptr;
}

void CModel::DisableTextureTimeout() { sIsTextureTimeoutEnabled = false; }

void CModel::EnableTextureTimeout() { sIsTextureTimeoutEnabled = true; }

uint CModel::GetDataSize() const { return mDataLen; }

rstl::auto_ptr< uchar > CModel::GetData() { return rstl::auto_ptr< uchar >(mData.get()); }

namespace {
inline void RemapPointer(const void*& pointer, uintptr_t offset) {
  if (pointer != nullptr) {
    pointer = reinterpret_cast< void* >(reinterpret_cast< uintptr_t >(pointer) + offset);
  }
}

template < typename T >
inline void RemapPointer(T*& pointer, uintptr_t offset) {
  RemapPointer(reinterpret_cast< const void*& >(pointer), offset);
}
} // namespace

void CModel::RemapData(uchar* data) {
  uintptr_t offset =
      reinterpret_cast< uintptr_t >(data) - reinterpret_cast< uintptr_t >(mData.release());
  mData = data;
  for (int i = 0; i < mMatSets.size(); ++i) {
    RemapPointer(mMatSets[i].mData, offset);
  }

  const CCubeModel::ModelInstance& instance = mModelInstance->GetModelInstance();
  const uchar* positions = static_cast< const uchar* >(instance.GetVertexPointer());
  const uchar* normals = static_cast< const uchar* >(instance.GetNormalPointer());
  const uchar* colors = static_cast< const uchar* >(instance.GetColorPointer());
  const uchar* uvs = static_cast< const uchar* >(instance.GetTCPointer());
  const uchar* packedUvs = static_cast< const uchar* >(instance.GetPackedTCPointer());
  const CAABox bounds = mModelInstance->GetBoundingBox();
  uchar flags = mModelInstance->GetModelFlags();
  bool texturesLoaded = mModelInstance->AreTexturesLoaded();
  const int index = mModelInstance->GetModelIndex();
  RemapPointer(positions, offset);
  RemapPointer(normals, offset);
  RemapPointer(colors, offset);
  RemapPointer(uvs, offset);
  RemapPointer(packedUvs, offset);
  for (int i = 0; i < mSurfaces.size(); ++i) {
    RemapPointer(mSurfaces[i], offset);
  }

  mModelInstance = rs_new CCubeModel(&mSurfaces, &mMatSets.front().mTextures,
                                     mMatSets.front().mData, positions, normals, colors, uvs,
                                     packedUvs, bounds, flags, texturesLoaded, index);
  UpdateLastFrame();
}

void CModel::UpdateLastFrame() const { mLastFrame = CGraphics::GetFrameCounter(); }
