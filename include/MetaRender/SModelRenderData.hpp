#ifndef _SMODELRENDERDATA
#define _SMODELRENDERDATA

#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "types.h"

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
  const CAABox& GetAABB() const;
  void DrawFlat(const CModelFlags& flags, bool unsorted, bool sorted) const;

  const CModel* mModel;
  const CSkinnedModel* mSkinnedModel;
  const SSkinningWorkspace* mWorkspace;
  const CPoseAsTransforms_Linear* mPose;
};
CHECK_SIZEOF(SModelRenderData, 0x10)

#endif // _SMODELRENDERDATA
