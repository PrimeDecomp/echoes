#include "Kyoto/Animation/CSkinnedModel.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CPoseAsTransforms_Linear.hpp"
#include "Kyoto/Animation/CSkinRules.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/DolphinGPUMemory.hpp"

#include "dolphin/gx.h"
#include "dolphin/os.h"

CSkinnedModel::TPointGenFunc CSkinnedModel::sPointGen;
void* CSkinnedModel::sPointGenData;

namespace {
// Descriptive name for the scoped release of a temporary GX matrix allocation.
class CMatrixPoolGuard {
public:
  explicit CMatrixPoolGuard(bool active) : mActive(active) {}

  ~CMatrixPoolGuard() {
    if (mActive) {
      GPUMemory::ReleaseAllocation();
    }
  }

private:
  bool mActive;
};

// Descriptive names for the target's surface-drawing functors.
struct SDrawSurfaceFlat {
  void operator()(const CCubeModel& model, const CCubeSurface& surface) const;
};

struct SDrawSurface {
  void operator()(const CCubeModel& model, const CCubeSurface& surface,
                  const CModelFlags& flags) const;
};

struct SDrawFlat {
  void operator()(const CCubeModel& model, const CCubeSurface& surface,
                  const CModelFlags& flags) const;
};

struct SDrawMasked {
  explicit SDrawMasked(u64 mask) : mMask(mask) {}

  void operator()(const CCubeModel& model, const CCubeSurface& surface,
                  const CModelFlags& flags) const;

  u64 mMask;
};

struct SDrawFlatMasked {
  explicit SDrawFlatMasked(u64 mask) : mMask(mask) {}

  void operator()(const CCubeModel& model, const CCubeSurface& surface,
                  const CModelFlags& flags) const;

  u64 mMask;
};
} // namespace

template < class TDraw, class TFlatDraw >
void CSkinnedModel::DolphinDrawInternal(const SSkinningWorkspace& workspace, uint drawFlags,
                                        const CModelFlags& flags, const TDraw& draw,
                                        const TFlatDraw& flat) const {
  CTransform4f saved(CGraphics::GetModelMatrix());
  int currentBank = -1;
  mModel->PreDrawModel(flags);
  const CCubeModel* model = mModel->GetModelInstance();
  if (model == nullptr || !model->TryLockTextures()) {
    return;
  }

  CCubeSurface surfaces[] = {model->GetNormalSurfaces(), model->GetAlphaSurfaces()};
  model->SetArraysCurrent();
  CMatrixPoolGuard guard(workspace.mMatrices == nullptr);
  SSkinningMatrices* matrices = workspace.mMatrices;
  if (matrices == nullptr) {
    matrices = static_cast< SSkinningMatrices* >(
        GPUMemory::EnsureAllocation(mSkinRules->GetNumVirtualBones() * sizeof(SSkinningMatrices)));
    BuildSkinningMatrices(workspace.mTransforms, matrices, workspace.mUniformScale);
  }
  CGX::SetArray(GX_POS_MTX_ARRAY, matrices, sizeof(SSkinningMatrices));
  CGX::SetArray(GX_NRM_MTX_ARRAY, matrices->mNormal, sizeof(SSkinningMatrices));

  for (int i = 0; i < sizeof(surfaces) / sizeof(surfaces[0]); ++i) {
    if (!(drawFlags & kDF_Unsorted) && i == 0) {
      continue;
    }
    if (!(drawFlags & kDF_Sorted) && i == 1) {
      continue;
    }
    if (drawFlags & kDF_Flat) {
      for (CCubeSurface surface = surfaces[i]; surface.IsValid();
           surface = surface.GetNextSurface()) {
        LoadMatrixBank(currentBank, surface.GetMatrixBank());
        flat(*model, surface, flags);
      }
    } else {
      for (CCubeSurface surface = surfaces[i]; surface.IsValid();
           surface = surface.GetNextSurface()) {
        LoadMatrixBank(currentBank, surface.GetMatrixBank());
        draw(*model, surface, flags);
      }
    }
  }

  GXSetCurrentMtx(GX_PNMTX0);
  const int extraCoord = CCubeMaterial::GetExtraTexCoord();
  if (extraCoord != GX_TEXCOORD_NULL) {
    CGX::SetTexCoordGen(static_cast< GXTexCoordID >(extraCoord), GX_TG_MTX3x4, GX_TG_POS,
                        static_cast< GXTexMtx >(0), GX_FALSE,
                        static_cast< GXPTTexMtx >(CCubeMaterial::GetExtraPostTexMtx()));
    CCubeMaterial::ResetExtraTexCoord();
  }
  CGraphics::SetModelMatrix(saved);
}

CSkinnedModelState::CSkinnedModelState(int boneCount, bool transient) {
  mWorkspace.mTransforms =
      transient ? nullptr
                : static_cast< CTransform4f* >(CMemory::Alloc(boneCount * sizeof(CTransform4f)));
  mWorkspace.mMatrices = nullptr;
  mWorkspace.mBoneCount = boneCount;
  mWorkspace.mOwned = true;
  mWorkspace.mTransient = transient;
  mWorkspace.mUniformScale = false;
  if (mWorkspace.mTransient) {
    const uint transformSize = (boneCount * sizeof(CTransform4f) + 31) & ~31;
    const uint matrixSize = (boneCount * sizeof(SSkinningMatrices) + 31) & ~31;
    void* data = GPUMemory::EnsureAllocation(transformSize + matrixSize);
    mWorkspace.mTransforms = static_cast< CTransform4f* >(data);
    mWorkspace.mMatrices =
        reinterpret_cast< SSkinningMatrices* >(static_cast< uchar* >(data) + transformSize);
  }
}

CSkinnedModelState::CSkinnedModelState(const CSkinnedModelState& other) {
  mWorkspace.mTransforms = other.mWorkspace.mTransforms;
  mWorkspace.mMatrices = other.mWorkspace.mMatrices;
  mWorkspace.mBoneCount = other.mWorkspace.mBoneCount;
  mWorkspace.mOwned = other.mWorkspace.mOwned;
  mWorkspace.mTransient = other.mWorkspace.mTransient;
  mWorkspace.mUniformScale = other.mWorkspace.mUniformScale;
  other.mWorkspace.mOwned = false;
}

CSkinnedModelState::~CSkinnedModelState() {
  if (mWorkspace.mOwned) {
    if (mWorkspace.mTransient) {
      DCFlushRange(mWorkspace.mTransforms,
                   (mWorkspace.mBoneCount * sizeof(CTransform4f) + 31) & ~31);
      GPUMemory::ReleaseAllocation();
    } else {
      CMemory::Free(mWorkspace.mTransforms);
    }
  }
}

CSkinnedModel::CSkinnedModel(const TLockedToken< CModel >& model,
                             const TLockedToken< CSkinRules >& skinRules,
                             const TLockedToken< CCharLayoutInfo >& layoutInfo)
: mModel(model), mSkinRules(skinRules), mLayoutInfo(layoutInfo) {}

CSkinnedModel::~CSkinnedModel() {}

CSkinnedModelState CSkinnedModel::MakeStorage(bool transient) const {
  return CSkinnedModelState(mSkinRules->GetNumVirtualBones(), transient);
}

CSkinnedModelState CSkinnedModel::MakeDefaultStorage() const { return MakeStorage(false); }

void CSkinnedModel::StoreCalculation(CSkinnedModelState& state,
                                     const CPoseAsTransforms_Linear* pose) const {
  SSkinningWorkspace& workspace = state.mWorkspace;
  CTransform4f* transforms = workspace.mTransforms;
  const int count = mSkinRules->GetNumVirtualBones();
  bool uniformScale = false;
  if (pose == nullptr) {
    for (int i = 0; i < count; ++i) {
      *transforms = CTransform4f::Identity();
      ++transforms;
    }
  } else {
    mSkinRules->BuildAccumulatedTransforms(*pose, **mLayoutInfo, transforms);
    if (!pose->HasScale() && pose->HasUniformScale()) {
      uniformScale = true;
    }
  }
  workspace.mUniformScale = uniformScale;
  if (workspace.mMatrices != nullptr) {
    BuildSkinningMatrices(workspace.mTransforms, workspace.mMatrices, workspace.mUniformScale);
  }
  if (sPointGen != nullptr && mSkinRules->GetNumPoints() != 0) {
    sPointGen(*this, workspace, sPointGenData);
  }
}

void CSkinnedModel::Draw(const CPoseAsTransforms_Linear* pose, const CModelFlags& flags) const {
  CSkinnedModelState state(MakeStorage(true));
  StoreCalculation(state, pose);
  DrawFromState(state, flags);
}

void CSkinnedModel::Draw(const CPoseAsTransforms_Linear* pose, TDrawFunc callback,
                         void* context) const {
  CSkinnedModelState state(MakeStorage(true));
  StoreCalculation(state, pose);
  callback(state.GetWorkspace(), context);
}

void CSkinnedModel::DolphinDrawWithFlags(const CPoseAsTransforms_Linear* pose, uint drawFlags,
                                         const CModelFlags& flags) const {
  CSkinnedModelState state(MakeStorage(true));
  StoreCalculation(state, pose);
  DolphinDrawFromWorkspace(state.GetWorkspace(), drawFlags, flags);
}

void CSkinnedModel::DolphinDrawFromWorkspace(const SSkinningWorkspace& workspace, uint drawFlags,
                                             const CModelFlags& flags) const {
  DolphinDrawInternal(workspace, drawFlags, flags, SDrawSurface(), SDrawFlat());
}

void CSkinnedModel::DolphinDrawFromWorkspace(const SSkinningWorkspace& workspace, uint drawFlags,
                                             const CModelFlags& flags, u64 mask) const {
  DolphinDrawInternal(workspace, drawFlags, flags, SDrawMasked(mask), SDrawFlatMasked(mask));
}

void CSkinnedModel::DrawFromState(const CSkinnedModelState& state, const CModelFlags& flags) const {
  DolphinDrawFromWorkspace(state.GetWorkspace(), kDF_Unsorted | kDF_Sorted, flags);
}

void CSkinnedModel::LoadMatrixBank(int& currentBank, int bank) const {
  if (bank != currentBank) {
    mSkinRules->LoadMatrixBank(bank);
    currentBank = bank;
  }
}

void CSkinnedModel::BuildSkinningMatrices(const CTransform4f* transforms,
                                          SSkinningMatrices* matrices, bool uniformScale) const {
  const rstl::vector< CVirtualBone >& bones = mSkinRules->GetVirtualBones();
  SSkinningMatrices* matrix = matrices;
  const CTransform4f* transform = transforms;
  for (rstl::vector< CVirtualBone >::const_iterator it = bones.begin(); it != bones.end();
       ++it, ++matrix, ++transform) {
    it->BuildSkinningMatrices(*transform, *matrix, uniformScale);
    DCFlushRange(matrix, sizeof(SSkinningMatrices));
  }
}

void CSkinnedModel::SetPointGeneratorFunc(void* context, TPointGenFunc callback) {
  sPointGen = callback;
  sPointGenData = context;
}

void CSkinnedModel::ClearPointGeneratorFunc() { sPointGen = nullptr; }

CVector3f CSkinnedModel::GetSkinnedPosition(const SSkinningWorkspace& workspace, int vertex) const {
  const CTransform4f* transforms = workspace.mTransforms;
  const int bone = mSkinRules->GetVertexToBoneMap()[vertex];
  return transforms[bone] * reinterpret_cast< const CVector3f* >(mModel->GetPositions())[vertex];
}

CVector3f CSkinnedModel::GetSkinnedNormal(const SSkinningWorkspace& workspace, int vertex) const {
  const int bone = mSkinRules->GetVertexToBoneMap()[vertex];
  const CVector3f& normal = reinterpret_cast< const CVector3f* >(mModel->GetNormals())[vertex];
  const CTransform4f& transform = workspace.mTransforms[bone];
  if (workspace.mUniformScale) {
    return CUnitVector3f(transform.Rotate(normal));
  }
  return CUnitVector3f(transform.GetQuickInverse().TransposeRotate(normal));
}

namespace {
inline void SDrawSurfaceFlat::operator()(const CCubeModel& model,
                                         const CCubeSurface& surface) const {
  const CCubeMaterial material = model.GetMaterial(surface);
  CGX::SetVtxDescv_Compressed(material.GetVertexDesc());
  CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
}

inline void SDrawSurface::operator()(const CCubeModel& model, const CCubeSurface& surface,
                                     const CModelFlags& flags) const {
  model.DrawSurface(surface, flags);
}

inline void SDrawFlat::operator()(const CCubeModel& model, const CCubeSurface& surface,
                                  const CModelFlags& flags) const {
  SDrawSurfaceFlat()(model, surface);
}

inline void SDrawMasked::operator()(const CCubeModel& model, const CCubeSurface& surface,
                                    const CModelFlags& flags) const {
  const CCubeMaterial material = model.GetMaterial(surface);
  if (material.GetMaterialMask() & mMask) {
    model.DrawSurface(surface, flags);
  }
}

inline void SDrawFlatMasked::operator()(const CCubeModel& model, const CCubeSurface& surface,
                                        const CModelFlags& flags) const {
  const CCubeMaterial material = model.GetMaterial(surface);
  if (material.GetMaterialMask() & mMask) {
    SDrawFlat()(model, surface, flags);
  }
}
} // namespace
