#ifndef _SWARMRENDERHELPERS
#define _SWARMRENDERHELPERS

#include "types.h"

#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/auto_ptr.hpp"

class CModelFlags;

namespace SwarmRenderHelpers {

class CSwarmSkinnedModelState {
public:
  explicit CSwarmSkinnedModelState(const CSkinnedModel& model);
  void StateToArrays();

  // Guessed accessor names; the swarm calculates its pose into this shared state.
  CSkinnedModelState& State() { return mState; }
  const CVector3f* GetPositions() const { return mPositions.get(); }
  const CVector3f* GetNormals() const { return mNormals.get(); }

private:
  // Guessed member names, supported by native construction, skinning and draw consumers.
  const CSkinnedModel& mSkinnedModel;
  CSkinnedModelState mState;
  rstl::auto_ptr< CVector3f > mPositions;
  rstl::auto_ptr< CVector3f > mNormals;
};
CHECK_SIZEOF(CSwarmSkinnedModelState, 0x24)

class CSwarmDisplayList {
public:
  explicit CSwarmDisplayList(const CSkinnedModel& model);
  void SetMaterialCurrent(const CModelFlags& flags) const;
  void DrawFromState(const CSwarmSkinnedModelState& state) const;

private:
  // Guessed name for the native array-binding draw helper.
  void Draw(const CVector3f* positions, const CVector3f* normals) const;

  // Guessed member names; the display list omits per-vertex skinning matrix indices.
  const CSkinnedModel& mSkinnedModel;
  rstl::auto_ptr< uchar > mDisplayList;
  uint mDisplayListSize;
};
CHECK_SIZEOF(CSwarmDisplayList, 0x10)

} // namespace SwarmRenderHelpers

#endif // _SWARMRENDERHELPERS
