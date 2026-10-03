#include "MetroidPrime/SwarmRenderHelpers.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CSkinRules.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CDisplayListReader.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

#include <dolphin/os.h>
#include <string.h>

namespace SwarmRenderHelpers {

CSwarmDisplayList::CSwarmDisplayList(const CSkinnedModel& model)
: mSkinnedModel(model), mDisplayList(), mDisplayListSize(0) {
  const CCubeModel& cubeModel = *model.GetModel()->GetModelInstance();
  const uint vertexDesc = cubeModel.GetMaterialByIndex(0).GetVertexDesc();
  int matrixIndexBytes = 0;
  for (int attr = GX_VA_PNMTXIDX; attr <= GX_VA_TEX6MTXIDX; ++attr) {
    if (CDisplayListReader::GetAttributeType(vertexDesc, static_cast< GXAttr >(attr)) != GX_NONE) {
      ++matrixIndexBytes;
    }
  }
  const uint vertexStride = CDisplayListReader::GetVertexStride(vertexDesc);
  const uint arrayVertexStride = vertexStride - matrixIndexBytes;

  const rstl::vector< void* >& surfaces = cubeModel.GetModelInstance().Surfaces();
  uint totalSize = 0;
  for (int i = 0; i < surfaces.size(); ++i) {
    totalSize += CCubeSurface(surfaces[i]).GetDisplayListSize();
  }
  if (totalSize != 0) {
    mDisplayList = rstl::auto_ptr< uchar >(
        static_cast< uchar* >(CMemory::Alloc(totalSize, IAllocator::kHI_RoundUpLen)));
    uchar* output = mDisplayList.get();
    for (int i = 0; i < surfaces.size(); ++i) {
      const CCubeSurface surface(surfaces[i]);
      const uchar* input = static_cast< const uchar* >(surface.GetDisplayList());
      const uchar* end = input + surface.GetDisplayListSize();
      while (input < end && *input != GX_NOP) {
        // GX primitive counts are big-endian halfwords, potentially unaligned.
        const ushort vertexCount = (static_cast< ushort >(input[1]) << 8) | input[2];
        output[0] = input[0];
        output[1] = input[1];
        output[2] = input[2];
        input += 3;
        output += 3;

        for (int vertex = 0; vertex < vertexCount; ++vertex) {
          memcpy(output, input + matrixIndexBytes, arrayVertexStride);
          output += arrayVertexStride;
          input += vertexStride;
        }
      }
    }
    while ((output - mDisplayList.get()) & 31) {
      *output++ = GX_NOP;
    }
    mDisplayListSize = output - mDisplayList.get();
    DCFlushRange(mDisplayList.get(), mDisplayListSize);
  }
}

void CSwarmDisplayList::SetMaterialCurrent(const CModelFlags& flags) const {
  const CModel& model = **mSkinnedModel.GetModel();
  const CCubeModel& cubeModel = *model.GetModelInstance();
  model.PreDrawModel(flags);
  cubeModel.TryLockTextures();
  const CCubeMaterial material = cubeModel.GetMaterialByIndex(0);
  const CCubeSurface surface(cubeModel.GetModelInstance().Surfaces()[0]);
  material.SetCurrent(flags, surface, cubeModel);
  CGX::SetVtxDescv_Compressed(material.GetVertexDesc() & 0xffffff);
  cubeModel.SetArraysCurrent();
}

void CSwarmDisplayList::Draw(const CVector3f* positions, const CVector3f* normals) const {
  CGX::SetArray(GX_VA_POS, positions, sizeof(CVector3f));
  CGX::SetArray(GX_VA_NRM, normals, sizeof(CVector3f));
  CGX::CallDisplayList(mDisplayList.get(), mDisplayListSize);
}

void CSwarmSkinnedModelState::StateToArrays() {
  const SSkinningWorkspace& workspace = mState.GetWorkspace();
  const CTransform4f* transforms = workspace.mTransforms;
  const CModel& model = **mSkinnedModel.GetModel();
  const CSkinRules& skinRules = **mSkinnedModel.GetSkinRules();
  const CVector3f* positions = reinterpret_cast< const CVector3f* >(model.GetPositions());
  const CVector3f* normals = reinterpret_cast< const CVector3f* >(model.GetNormals());
  const rstl::vector< CVirtualBone >& bones = skinRules.GetVirtualBones();
  CVector3f* skinnedPositions = mPositions.get();
  CVector3f* skinnedNormals = mNormals.get();

  int vertex = 0;
  for (int bone = 0; bone < bones.size(); ++bone) {
    const CTransform4f& transform = transforms[bone];
    const int count = bones[bone].GetVertexCount();
    for (int i = 0; i < count; ++i, ++vertex) {
      skinnedPositions[vertex] = transform * positions[vertex];
    }
  }
  DCFlushRange(skinnedPositions, vertex * sizeof(CVector3f));

  vertex = 0;
  for (int bone = 0; bone < bones.size(); ++bone) {
    const CTransform4f& transform = transforms[bone];
    const int count = bones[bone].GetVertexCount();
    if (workspace.mUniformScale) {
      for (int i = 0; i < count; ++i, ++vertex) {
        skinnedNormals[vertex] = transform.Rotate(normals[vertex]);
      }
    } else {
      const CTransform4f inverse = transform.GetQuickInverse();
      for (int i = 0; i < count; ++i, ++vertex) {
        skinnedNormals[vertex] = inverse.TransposeRotate(normals[vertex]);
      }
    }
  }
  DCFlushRange(skinnedNormals, vertex * sizeof(CVector3f));
}

CSwarmSkinnedModelState::CSwarmSkinnedModelState(const CSkinnedModel& model)
: mSkinnedModel(model), mState(model.MakeDefaultStorage()), mPositions(), mNormals() {
  const CCubeModel& cubeModel = *model.GetModel()->GetModelInstance();
  const CSkinRules& skinRules = **model.GetSkinRules();
  mPositions = rstl::auto_ptr< CVector3f >(static_cast< CVector3f* >(
      CMemory::Alloc(skinRules.GetNumPoints() * sizeof(CVector3f), IAllocator::kHI_RoundUpLen)));
  mNormals = rstl::auto_ptr< CVector3f >(static_cast< CVector3f* >(
      CMemory::Alloc(skinRules.GetNumPoints() * sizeof(CVector3f), IAllocator::kHI_RoundUpLen)));

  memcpy(mPositions.get(), cubeModel.GetPositions(), skinRules.GetNumPoints() * sizeof(CVector3f));
  DCFlushRange(mPositions.get(), skinRules.GetNumPoints() * sizeof(CVector3f));
  memcpy(mNormals.get(), cubeModel.GetNormals(), skinRules.GetNumPoints() * sizeof(CVector3f));
  DCFlushRange(mNormals.get(), skinRules.GetNumPoints() * sizeof(CVector3f));
}

void CSwarmDisplayList::DrawFromState(const CSwarmSkinnedModelState& state) const {
  Draw(state.GetPositions(), state.GetNormals());
}

} // namespace SwarmRenderHelpers
