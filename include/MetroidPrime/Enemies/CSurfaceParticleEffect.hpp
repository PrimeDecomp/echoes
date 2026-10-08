#ifndef _CSURFACEPARTICLEEFFECT
#define _CSURFACEPARTICLEEFFECT

#include "types.h"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CEffect.hpp"

#include "WorldFormat/CCollisionSurface.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

namespace rstl {
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(CCollisionSurface)
RSTL_DECLARE_BITWISE_CONSTRUCTION(CCollisionSurface)
} // namespace rstl

class CElementGen;
class CGenDescription;
class CStateManager;

// Guessed class: a particle effect whose particles are snapped onto nearby collision surfaces.
// It is registered by the GeomBlobV2 REL (module 25) through SSurfaceParticleEffect_FuncPtrs.
class CSurfaceParticleEffect : public CEffect {
public:
  CSurfaceParticleEffect(const TLockedToken< CGenDescription >& desc, TUniqueId uid, TAreaId area,
                         bool active, const rstl::string& name, const CTransform4f& xf,
                         TUniqueId ignoredCollisionId, int flags);

  // CEntity
  ~CSurfaceParticleEffect() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  // CSurfaceParticleEffect
  CElementGen* GetParticleGen(); // Guessed name
  const CElementGen* GetParticleGen() const;
  float GetGeneratorRate() const; // Guessed name
  void SetGeneratorRate(float rate);
  void SetFlag(bool value); // Guessed name

private:
  // Guessed name: collects the world and actor triangles that overlap the box.
  void GatherCollisionSurfaces(CStateManager& mgr, const CAABox& bounds,
                               rstl::reserved_vector< CCollisionSurface, 256 >& surfaces,
                               rstl::reserved_vector< uint, 256 >& surfaceIds);

  CRandom16 mRandom;
  rstl::single_ptr< CElementGen > mParticleGen;
  TUniqueId mLightId;
  CAssetId mParticleAssetId;
  float mGeneratorRate;
  float mSpawnRemainder;
  uint mFrameCounter;
  TUniqueId mIgnoredCollisionId;
  bool mHasRenderBounds : 1;
  bool mFlag : 1; // Guessed name
};
CHECK_SIZEOF(CSurfaceParticleEffect, 0x178)

void SetSurfaceParticleEffectFuncPtrs();
void ClearSurfaceParticleEffectFuncPtrs();

#endif // _CSURFACEPARTICLEEFFECT
