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
  void ResetBlur();

private:
  rstl::single_ptr< CTexture > x0_texture;
  CTransform4f x4_view;
  CTransform4f x34_model;
  float x64_objectHalfExtent;
  CVector3f x68_objectPosition;
  CVector3f x74_lightPosition;
  TAreaId x80_area;
  int x84_lightIndex;
  bool x88_blurReset;
};
CHECK_SIZEOF(CWorldShadow, 0x8c)
#endif // _CWORLDSHADOW
