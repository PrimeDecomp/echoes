#ifndef _CSCRIPTWATER
#define _CSCRIPTWATER

#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CFluidPlaneCPU;
class CGenDescription;
class CFluidUVMotion;
class CPlane;

class CScriptWater : public CScriptTrigger {
public:
  CScriptWater(CStateManager& mgr, TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CVector3f& position, const CAABox& bounds, const CDamageInfo& damage,
               const CVector3f& forceField, uint triggerFlags, float alphaInTime,
               float alphaOutTime, float morphInTime, float morphOutTime, int fluidType,
               CAssetId lightMap, CAssetId colorMap, const CColor& baseColor, CAssetId colorWarpMap,
               CAssetId glossMap, CAssetId envMap, float envMapSize, CAssetId texture,
               CAssetId foamMap, CAssetId alphaMap, float alpha, float glossFlat, float unknown1,
               float unknown2, float unknown3, const CFluidUVMotion& uvMotion,
               const CColor& splashColor, const CColor& insideFogColor, CAssetId splashParticle1,
               CAssetId splashParticle2, CAssetId splashParticle3, CAssetId visorRunoffParticle,
               CAssetId unmorphVisorRunoffParticle, TSfxId visorRunoffSfx,
               TSfxId unmorphVisorRunoffSfx, TSfxId splashSfx1, TSfxId splashSfx2,
               TSfxId splashSfx3, const CColor& fogColor, float fogBias, float fogMagnitude,
               float fogSpeed, float viscosity, bool displaySurface, float unknownScale,
               const CVector2f& uvScale, const CVector2f& uvOffset, const CVector2f& surfaceScale,
               bool useDynamicLights, float unknown4, float unknown5, float unknown6,
               float unknown7, bool occlusion, bool filterSoundEffects, int unknown8);

  // CEntity
  ~CScriptWater() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& position,
                                                         const CVector3f& direction,
                                                         const CWeaponMode& mode,
                                                         int attrib) const override;
  CAABox GetSortingBounds(const CStateManager& mgr) const override;

  // CScriptTrigger
  void InhabitantAdded(CActor& actor, CStateManager& mgr) override;
  void InhabitantIdle(CActor& actor, CStateManager& mgr) override;
  void InhabitantExited(CActor& actor, CStateManager& mgr) override;

  bool CanRippleAtPoint(const CVector3f& point) const;
  void SetupGrid(bool recomputeClipping);
  void SetupGridClipping(CStateManager& mgr, int computeVerts);
  void SetMorphing(bool morphing);
  float GetSplashEffectScale(float scale) const;
  TSfxId GetSplashSound(float scale) const;
  const rstl::optional_object< TLockedToken< CGenDescription > >&
  GetSplashEffect(float scale) const;
  int GetSplashIndex(float scale) const;
  const CColor& GetSplashColor() const { return mSplashColor; }
  CVector2f GetFluidUVExtent(const CAABox& bounds) const; // Guessed name
  void CalculateRenderBounds();
  void ClearSplashInhabitants(); // Guessed name
  void UpdateSplashInhabitants(CStateManager& mgr);
  const CScriptWater* GetNextConnectedWater(const CStateManager& mgr) const;
  CPlane GetWRSurfacePlane() const;

private:
  static const float kSplashScales[6];

  rstl::single_ptr< CFluidPlaneCPU > mFluidPlane;
  CVector3f mPositionMorphed;
  CVector3f mExtentMorphed;
  float mMorphInTime;
  CVector3f mPositionOrig;
  CVector3f mExtentOrig;
  float mDamageOrig;
  float mDamageMorphed;
  float mMorphOutTime;
  float mMorphFactor;
  CAABox mSurfaceBounds; // Guessed name
  rstl::list< rstl::pair< TUniqueId, bool > > mWaterInhabitants;
  float mFogBias;
  float mFogMagnitude;
  float mOrigFogBias;
  float mOrigFogMagnitude;
  float mFogSpeed;
  CColor mFogColor;
  CAssetId mSplashParticle1Id;
  CAssetId mSplashParticle2Id;
  CAssetId mSplashParticle3Id;
  CAssetId mVisorRunoffParticleId;
  rstl::optional_object< TLockedToken< CGenDescription > > mVisorRunoffEffect;
  CAssetId mUnmorphVisorRunoffParticleId;
  rstl::optional_object< TLockedToken< CGenDescription > > mUnmorphVisorRunoffEffect;
  TSfxId mVisorRunoffSfx;
  TSfxId mUnmorphVisorRunoffSfx;
  rstl::reserved_vector< rstl::optional_object< TLockedToken< CGenDescription > >, 3 >
      mSplashEffects;
  rstl::reserved_vector< TSfxId, 3 > mSplashSounds;
  CColor mSplashColor;
  CColor mInsideFogColor;
  float mAlphaInTime;
  float mAlphaOutTime;
  float mAlphaInRecip;
  float mAlphaOutRecip;
  float mAlpha;
  int mGridDimX;
  int mGridDimY;
  int mGridCellCount;
  int mPatchDimX;
  int mPatchDimY;
  rstl::single_ptr< char > mTileIntersects;
  rstl::single_ptr< bool > mVertIntersects;
  rstl::single_ptr< char > mPatchIntersects;
  int mComputedGridCellCount;
  float x310_;
  float x314_;
  float x318_;
  float x31c_;
  CVector2f mSurfaceScale; // Guessed name
  uint x328_;
  bool mMorphIn : 1;
  bool mMorphing : 1;
  bool mAllowRender : 1;
  bool mRecomputeClipping : 1;
  bool mAlphaIn : 1;
  bool mAlphaOut : 1;
  bool x32c_6_ : 1;
  bool x32c_7_ : 1;
};
CHECK_SIZEOF(CScriptWater, 0x330)

#endif // _CSCRIPTWATER
