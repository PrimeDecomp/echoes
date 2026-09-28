#ifndef _CWORLDSHADOW
#define _CWORLDSHADOW
#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/single_ptr.hpp"
class CTexture;
class CStateManager;
class CAABox;

class CWorldShadow {
public:
  CWorldShadow(uint width, uint height, bool rgba8);
  ~CWorldShadow();
  // Guessed name: Echoes disables these shadows in multiplayer, Dark Aether, and other visors.
  static bool CanRender(const CStateManager& mgr);
  void BuildLightShadowTexture(const CStateManager& mgr, TAreaId area, uint lightIndex,
                               const CAABox& bounds, bool motionBlur, bool lighten);
  void EnableModelProjectedShadow(const CTransform4f& transform, uint lightIndex,
                                  float scale) const;
  void DisableModelProjectedShadow() const;
  void ResetBlur();

private:
  rstl::single_ptr< CTexture > mTexture;
  CTransform4f mView;
  CTransform4f mModel;
  float mObjectHalfExtent;
  CVector3f mObjectPosition;
  CVector3f mLightPosition;
  TAreaId mArea;
  int mLightIndex;
  bool mBlurReset;
};
CHECK_SIZEOF(CWorldShadow, 0x8c)
#endif // _CWORLDSHADOW
