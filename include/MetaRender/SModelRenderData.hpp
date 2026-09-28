#ifndef _SMODELRENDERDATA
#define _SMODELRENDERDATA

#include "types.h"

class CModel;
class CSkinnedModel;
struct SSkinningWorkspace;
class CPoseAsTransforms_Linear;

// Guessed name. Shared render input for a static or skinned model.
struct SModelRenderData {
  explicit SModelRenderData(const CModel& model)
  : mModel(&model), mSkinnedModel(nullptr), mWorkspace(nullptr), mPose(nullptr) {}

  const CModel* mModel;
  const CSkinnedModel* mSkinnedModel;
  const SSkinningWorkspace* mWorkspace;
  const CPoseAsTransforms_Linear* mPose;
};
CHECK_SIZEOF(SModelRenderData, 0x10)

#endif // _SMODELRENDERDATA
