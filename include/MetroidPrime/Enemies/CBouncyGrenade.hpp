#ifndef _CBOUNCYGRENADE
#define _CBOUNCYGRENADE

#include "Collision/CMaterialList.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

struct SGrenadeVelocityInfo {
private:
  float mMass;
  float mSpeed;

public:
  SGrenadeVelocityInfo(float mass, float speed) : mMass(mass), mSpeed(speed) {}
  explicit SGrenadeVelocityInfo(CInputStream& in) : mMass(in.ReadFloat()), mSpeed(in.ReadFloat()) {}

  float GetMass() const { return mMass; }
  float GetSpeed() const { return mSpeed; }
};
CHECK_SIZEOF(SGrenadeVelocityInfo, 0x8)

class CBouncyGrenadeData {
  SGrenadeVelocityInfo mVelocityInfo;
  CDamageInfo mDamageInfo;
  uint mNumBounces;
  CAssetId mElementGenId1;
  CAssetId mElementGenId2;
  CAssetId mElementGenId3;
  CAssetId mElementGenId4;
  ushort mBounceSfx;
  ushort mExplodeSfx;
  float mExplodeSfxFalloff; // Guessed name.
  float mExplodeSfxMaxDist; // Guessed name.
  float x44_;
  float x48_;
  bool x4c_ : 1;

public:
  CBouncyGrenadeData(const SGrenadeVelocityInfo& velocityInfo, CDamageInfo damageInfo,
                     uint numBounces, CAssetId elementGenId1, CAssetId elementGenId2,
                     CAssetId elementGenId3, CAssetId elementGenId4, ushort bounceSfx,
                     ushort explodeSfx, float explodeSfxFalloff, float explodeSfxMaxDist, float x44,
                     float x48, bool x4c)
  : mVelocityInfo(velocityInfo)
  , mDamageInfo(damageInfo)
  , mNumBounces(numBounces)
  , mElementGenId1(elementGenId1)
  , mElementGenId2(elementGenId2)
  , mElementGenId3(elementGenId3)
  , mElementGenId4(elementGenId4)
  , mBounceSfx(bounceSfx)
  , mExplodeSfx(explodeSfx)
  , mExplodeSfxFalloff(explodeSfxFalloff)
  , mExplodeSfxMaxDist(explodeSfxMaxDist)
  , x44_(x44)
  , x48_(x48)
  , x4c_(x4c) {}

  const SGrenadeVelocityInfo& GetVelocityInfo() const { return mVelocityInfo; }
  const CDamageInfo& GetDamageInfo() const { return mDamageInfo; }
  uint GetNumBounces() const { return mNumBounces; }
  CAssetId GetElementGenId1() const { return mElementGenId1; }
  CAssetId GetElementGenId2() const { return mElementGenId2; }
  CAssetId GetElementGenId3() const { return mElementGenId3; }
  CAssetId GetElementGenId4() const { return mElementGenId4; }
  ushort GetBounceSfx() const { return mBounceSfx; }
  ushort GetExplodeSfx() const { return mExplodeSfx; }
  float GetExplodeSfxFalloff() const { return mExplodeSfxFalloff; }
  float GetExplodeSfxMaxDist() const { return mExplodeSfxMaxDist; }
  bool GetX4c() const { return x4c_; }
};
CHECK_SIZEOF(CBouncyGrenadeData, 0x50)

class CBouncyGrenade : public CPhysicsActor {
public:
  CBouncyGrenade(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                 const CTransform4f& xf, const CModelData& mData, const CActorParameters& actParams,
                 TUniqueId parentId, float velocity, const CBouncyGrenadeData& data,
                 float explodePlayerDistance, const CAABox& bounds, TUniqueId targetId,
                 float maxHomingAngle, uint flags, const CMaterialList* extraMaterials,
                 const CHealthInfo* healthInfo);

  // CEntity
  ~CBouncyGrenade() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  CHealthInfo* HealthInfo() override { return mHealthInfo.valid() ? &*mHealthInfo : nullptr; }
  const CHealthInfo* GetHealthInfo() const override {
    return mHealthInfo.valid() ? &*mHealthInfo : nullptr;
  }
  uint GetFlags() const { return mFlags; }
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& act, CStateManager& mgr) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

private:
  void UpdateGrenadeFX(float dt, CStateManager& mgr);
  void Explode(CStateManager& mgr, TUniqueId uid);
  void UpdateExplodeChecks(float dt, CStateManager& mgr);
  void Bounce(CStateManager& mgr, const CVector3f& normal, bool consumeBounce, bool home);
  bool IsArmed() const;

  CBouncyGrenadeData mData;
  uint mNumBounces;
  TUniqueId mParentId;
  TUniqueId mTargetId;
  float mMaxHomingAngle;
  CVector3f mLastPosition;
  float mStationaryTime;
  float mElapsedTime;
  rstl::single_ptr< CElementGen > mElementGenExplodeCombat;
  rstl::single_ptr< CElementGen > mElementGenExplodeXRay;
  rstl::single_ptr< CElementGen > mElementGenTrailCombat;
  rstl::single_ptr< CElementGen > mElementGenTrailXRay;
  float mExplodePlayerDistance;
  rstl::optional_object< CHealthInfo > mHealthInfo;
  rstl::optional_object< CMaterialList > mMaterialsToRemove;
  uint mFlags;
  bool mExploded : 1;
  bool mHasRenderBounds : 1;
};
CHECK_SIZEOF(CBouncyGrenade, 0x390)

namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CMaterialList)
}

#endif // _CBOUNCYGRENADE
