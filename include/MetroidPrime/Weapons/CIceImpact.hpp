#ifndef _CICEIMPACT
#define _CICEIMPACT

#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CEffect.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CMarkerGrid {
public:
  explicit CMarkerGrid(const CAABox& bounds);

  uint GetValue(uint x, uint y, uint z) const;
  void SetValue(uint x, uint y, uint z, uint value);
  bool GetCoords(const CVector3f& point, uint& x, uint& y, uint& z) const;
  bool AABoxTouchesData(const CAABox& bounds, uint value) const;
  void MarkCells(const CSphere& sphere, uint value);
  CVector3f GetWorldPositionForCell(uint x, uint y, uint z) const;

  const CAABox& GetBounds() const { return mBounds; }

private:
  CAABox mBounds;
  CVector3f mGridUnits;
  rstl::reserved_vector< uchar, 1024 > mGridState;
};
CHECK_SIZEOF(CMarkerGrid, 0x428)

class CElementGen;
class CGenDescription;
class COBBTree;

class CIceImpact : public CEffect {
public:
  CIceImpact(const TLockedToken< CGenDescription >& particle, TUniqueId uid, TAreaId aid,
             TUniqueId ownerId, bool active, const rstl::string& name, const CTransform4f& xf,
             uint flags, const CVector3f& scale, float boundScale);

  // CEntity
  ~CIceImpact() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

private:
  struct SImpactSphere {
    SImpactSphere(const CVector3f& pos, float maxRadius, float radiusStep, float radius,
                  float previousRadius)
    : mPos(pos)
    , mMaxRadius(maxRadius)
    , mRadiusStep(radiusStep)
    , mRadius(radius)
    , mPreviousRadius(previousRadius) {}

    CVector3f mPos;
    float mMaxRadius;
    float mRadiusStep;
    float mRadius;
    float mPreviousRadius;
  };

  rstl::optional_object< SImpactSphere > GenerateNewSphere();
  bool GenerateParticlesAgainstWorld(CStateManager& mgr,
                                     const CMetroidAreaCollider::COctreeLeafCache& cache,
                                     const CSphere& outer, const CSphere& inner);
  bool GenerateParticlesAgainstActors(CStateManager& mgr, const CAABox& bounds,
                                      const CSphere& outer, const CSphere& inner);
  bool GenerateParticlesAgainstAABox(CStateManager& mgr, const CAABox& bounds, const CSphere& outer,
                                     const CSphere& inner);
  bool GenerateParticlesAgainstOBBTree(CStateManager& mgr, const COBBTree& tree,
                                       const CTransform4f& xf, const CSphere& outer,
                                       const CSphere& inner);
  bool SubdivideAndGenerateParticles(CStateManager& mgr, const CVector3f& a, const CVector3f& b,
                                     const CVector3f& c, const CSphere& outer,
                                     const CSphere& inner);

  rstl::single_ptr< CElementGen > mElementGen;
  TUniqueId mLightId;
  CAssetId mGenAssetId;
  TUniqueId mOwnerId;
  float mLifeTimer;
  float mLatestDamageTime;
  uint mSearchDirection;
  float mHalfBounds;
  float mParticleRemainder;
  CSphere mSphereGenRange;
  CMarkerGrid mGrid;
  rstl::reserved_vector< SImpactSphere, 3 > mImpactSpheres;
  bool mFollowPlayerArea : 1;
  bool mHasRenderBounds : 1;
};
CHECK_SIZEOF(CIceImpact, 0x610)

#endif // _CICEIMPACT
