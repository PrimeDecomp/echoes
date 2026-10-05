#ifndef _SMODELRENDERDATA
#define _SMODELRENDERDATA

#include "types.h"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Graphics/CModel.hpp"

class CModel;
class CSkinnedModel;
struct SSkinningWorkspace;
class CPoseAsTransforms_Linear;

// Guessed name. Shared render input for a static or skinned model.
struct SModelRenderData {
  explicit SModelRenderData(const CModel& model)
  : mModel(&model), mSkinnedModel(nullptr), mWorkspace(nullptr), mPose(nullptr) {}
  SModelRenderData(const CSkinnedModel& model, const CPoseAsTransforms_Linear& pose)
  : mModel(nullptr), mSkinnedModel(&model), mWorkspace(nullptr), mPose(&pose) {}

  // Guessed method names; native helpers dispatch through these four fields.
  const CAABox& GetAABB() const {
    return mModel ? mModel->GetAABB() : mSkinnedModel->GetModel()->GetAABB();
  }

  void DrawFlat(const CModelFlags& flags, bool unsorted, bool sorted) const {
    if (!unsorted && !sorted) {
      return;
    }
    if (mModel) {
      const CModel::EDrawFlatFlags selection =
          unsorted && sorted ? CModel::kDF_All : unsorted ? CModel::kDF_Unsorted : CModel::kDF_Sorted;
      mModel->PreDrawModel(flags);
      mModel->DolphinDrawFlat(selection);
    } else if (mSkinnedModel) {
      uint drawFlags = CSkinnedModel::kDF_Flat;
      if (unsorted) {
        drawFlags |= CSkinnedModel::kDF_Unsorted;
      }
      if (sorted) {
        drawFlags |= CSkinnedModel::kDF_Sorted;
      }
      if (mPose) {
        mSkinnedModel->DolphinDrawWithFlags(mPose, drawFlags, flags);
      } else if (mWorkspace) {
        mSkinnedModel->DolphinDrawFromWorkspace(*mWorkspace, drawFlags, flags);
      }
    }
  }

  const CModel* mModel;
  const CSkinnedModel* mSkinnedModel;
  const SSkinningWorkspace* mWorkspace;
  const CPoseAsTransforms_Linear* mPose;
};
CHECK_SIZEOF(SModelRenderData, 0x10)

#endif // _SMODELRENDERDATA
