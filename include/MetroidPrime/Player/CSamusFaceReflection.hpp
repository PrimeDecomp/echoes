#ifndef _CSAMUSFACEREFLECTION
#define _CSAMUSFACEREFLECTION

#include "types.h"

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/single_ptr.hpp"

class CActorLights;
class CDependencyGroupToken;
class CModelData;
class CRandom16;
class CStateManager;

// Echoes builds the face model once its dependency group has loaded and renders it per player.
class CSamusFaceReflection {
public:
  CSamusFaceReflection(const CStateManager& mgr, int playerIndex);
  ~CSamusFaceReflection();
  void PreDraw(const CStateManager& mgr);
  void Draw(const CStateManager& mgr) const;
  void Update(float dt, const CStateManager& mgr, CRandom16& rand);

private:
  uint mPlayerIndex;
  rstl::single_ptr< CDependencyGroupToken > mDependencies;
  rstl::single_ptr< CModelData > mModelData;
  rstl::single_ptr< CActorLights > mLights;
  CQuaternion mLookRot;
  CVector3f mLookDir;
  int x2c_;
  bool mHidden;
};
CHECK_SIZEOF(CSamusFaceReflection, 0x34)

#endif // _CSAMUSFACEREFLECTION
