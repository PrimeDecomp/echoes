#ifndef _CFLUIDPLANE
#define _CFLUIDPLANE

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CFluidUVMotion.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/optional_object.hpp"

class CTexture;

// Ownership/layout declaration. The remaining fluid virtual interface is not yet recovered.
class CFluidPlane {
public:
  virtual ~CFluidPlane();

protected:
  CAssetId mColorMapId;
  CAssetId mColorWarpMapId;
  CAssetId mGlossMapId;
  rstl::optional_object< TLockedToken< CTexture > > mColorMap;
  rstl::optional_object< TLockedToken< CTexture > > mColorWarpMap;
  rstl::optional_object< TLockedToken< CTexture > > mGlossMap;
  float mAlpha;
  int mFluidType; // TODO: recover the Echoes fluid enum.
  float mViscosity;
  CFluidUVMotion mUvMotion;
};
CHECK_SIZEOF(CFluidPlane, 0xd0)

#endif // _CFLUIDPLANE
