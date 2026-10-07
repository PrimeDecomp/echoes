#ifndef _CFLUIDPLANE
#define _CFLUIDPLANE

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CFluidUVMotion.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/optional_object.hpp"

class CTexture;
class CStateManager;
class CAABox;
class CTransform4f;
class CVector2f;

class CFluidPlane {
public:
  CFluidPlane(CAssetId colorMap, CAssetId colorWarpMap, CAssetId glossMap, float alpha,
              int fluidType, float viscosity, const CFluidUVMotion& motion);
  virtual ~CFluidPlane();
  // TODO: recover the name and parameters of this empty, inherited virtual slot.
  virtual void UnkVtable0C();
  virtual void Render(const CStateManager& mgr, float alpha, const CAABox& bounds,
                      const CTransform4f& xf, const CTransform4f& areaXf, TUniqueId waterId,
                      const char* gridFlags, int gridDimX, int gridDimY) const;
  virtual void PreRender(const CStateManager& mgr, const CVector2f& extent) = 0; // Guessed name

  float GetAlpha() const { return mAlpha; }
  int GetFluidType() const { return mFluidType; }
  float GetViscosity() const { return mViscosity; }
  const CFluidUVMotion& GetUVMotion() const { return mUvMotion; }

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

extern const float gkWaterGridRayMargin; // Guessed name
extern const bool gkWaterEnable;
extern const bool gkWaterFog;

#endif // _CFLUIDPLANE
