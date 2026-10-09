#ifndef _CBACTERIASWARM
#define _CBACTERIASWARM

#include "types.h"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "WorldFormat/CCollisionSurface.hpp"

#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CAnimData;
class CAreaCollisionCache;
class CBasicSwarmData;
class CGenDescription;
class CModelFlags;
class CPlayer;
class CRelAngle;
class CStaticRes;

// Guessed class: a swarm of bacteria crawling over surfaces. They patrol, flee from safe zones and
// pursue the player, shifting their color between the two states. Unlike CSwarmBasics it is a
// plain actor with its own boid type; the boids are rendered as particles of a shared generator.
class CBacteriaSwarm : public CActor {
public:
  class CBoid {
    friend class CBacteriaSwarm;

  public:
    CBoid(const CTransform4f& xf, uint index);
    CVector3f GetTranslation() const { return mTransform.GetTranslation(); }
    const CTransform4f& GetTransform() const { return mTransform; }

  private:
    // Guessed member names; layout is derived from the GameCube constructor/consumers.
    CTransform4f mTransform;
    CVector3f mVelocity;
    CColor mAmbientLighting;
    CBoid* mNext;
    float x44_;
    CCollisionSurface mSurface;
    float mSpeed;
    float mColorBlend; // 0 while patrolling, rises to the color change time while pursuing
    uint mIndex : 10;
    bool mActive : 1;
    bool mInFrustum : 1;
    bool mInSafeZone : 1;
    bool mTouchingSafeZone : 1;
    bool mPursuingPlayer : 1;
    bool x80_15_ : 1;
  };

  class CRepulsor {
    friend class CBacteriaSwarm;
    CVector3f mCenter;
    float mMagnitude;

  public:
    CRepulsor(CVector3f center, float magnitude) : mCenter(center), mMagnitude(magnitude) {}
  };

  CBacteriaSwarm(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                 const CVector3f& boundingBoxExtent, const CTransform4f& xf,
                 CActorParameters actorParameters, const CBasicSwarmData& data,
                 float surfaceStickPriority, float containmentPriority, float patrolTurnSpeed,
                 float avoidSafeZoneTurnSpeed, float patrolSpeed, float safeZoneEscapeSpeed,
                 float playerPursuitSpeed, float acceleration, float deceleration,
                 CAssetId particleEffect, const CColor& patrolColor, const CColor& pursuitColor,
                 float colorChangeTime, float minPatrolSoundTime, float maxPatrolSoundTime,
                 float patrolSoundWeight, float minPursuitSoundTime, float maxPursuitSoundTime,
                 float pursuitSoundWeight, ushort patrolSound, ushort pursuitSound,
                 float soundFallOff, float maxAudibleDistance, uchar minVolume, uchar maxVolume,
                 const CStaticRes& scanModel, bool spawnInstantly, bool unknownFlag);

  // CEntity
  ~CBacteriaSwarm() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;

  // CBacteriaSwarm
  virtual void CreateBoid(CStateManager& mgr, int index, bool forceParticle);
  virtual void UpdateBoid(CAreaCollisionCache& cache, CStateManager& mgr, float dt, CBoid& boid,
                          int partitionIndex);
  virtual void ApplySteeringBehaviors(CStateManager& mgr, CBoid& boid, CVector3f& ahead,
                                      const rstl::reserved_vector< CBoid*, 50 >& nearList);
  virtual void RenderBoid(const CBoid* boid, uint& drawMask, const CModelFlags& flags) const;
  virtual void UpdateSwarmAnimations(CStateManager& mgr, float dt);
  virtual void UpdateAllBoidMovement(float dt);
  virtual bool ShouldBuildAreaCollisionCacheForPartition(int partitionIndex) const;

  // Guessed names for non-virtual helpers.
  CAABox GetBoundingBox() const;
  CAABox BoxForPosition(int x, int y, int z, float margin) const;
  CAreaCollisionCache GetAreaCollisionCacheForPartition(int x, int y, int z) const;
  void UpdateParticles(float dt);
  void RenderParticles() const;
  void AddParticle(const CTransform4f& xf);
  void KillBoid(CBoid& boid, CStateManager& mgr, float deathRattleChance, float deadChance);
  void PlayBoidSound(ushort sfx, const CVector3f& pos, float distanceSquared);
  void UpdateEffects(CStateManager& mgr, CAnimData& animData, int volume);
  CVector3f FindClosestCell(const CVector3f& pos) const;
  void MoveBoid(CBoid& boid, const CVector3f& offsetDelta, float dt);
  void UpdatePartition();
  CBoid* GetListAt(const CVector3f& pos);
  void BuildBoidNearList(const CBoid& boid, float radius,
                         rstl::reserved_vector< CBoid*, 50 >& nearList);
  void ApplySeparation(CBoid& boid, const rstl::reserved_vector< CBoid*, 50 >& nearList,
                       CVector3f& ahead);
  void ApplySeparation(CBoid& boid, const CVector3f& pos, float radius, float magnitude,
                       CVector3f& ahead);
  void ApplyCohesion(CBoid& boid, const rstl::reserved_vector< CBoid*, 50 >& nearList,
                     CVector3f& ahead);
  void ApplyCohesion(CBoid& boid, const CVector3f& pos, float radius, float magnitude,
                     CVector3f& ahead);
  void ApplyPlayerAttraction(CBoid& boid, const CVector3f& pos, const CPlayer& player,
                             CVector3f& ahead, float radius, float magnitude);
  void ApplyBoundsAvoidance(CBoid& boid, const rstl::reserved_vector< CBoid*, 50 >& nearList,
                            CVector3f& ahead);
  void ApplySafeZoneAvoidance(CStateManager& mgr, CBoid& boid,
                              const rstl::reserved_vector< CBoid*, 50 >& nearList,
                              CVector3f& ahead);
  TUniqueId GetWaypointForState(EScriptObjectState state, CStateManager& mgr);
  bool IsBoidVisibleForLockOn(const CStateManager& mgr, const CBoid& boid,
                              const CVector3f& cameraPos, const CVector3f& cameraForward) const;
  int GetLockOnIndex(CStateManager& mgr) const;
  int FindBestLockOnIndex(CStateManager& mgr) const;
  void UpdateLockOnBlend(int prevIndex, int newIndex, float dt);
  void AddDoorRepulsors(CStateManager& mgr);
  bool FindBestSurface(const CAreaCollisionCache& cache, CVector3f pos, float radius,
                       CCollisionSurface& out);
  CCollisionSurface FindBestCollisionInBox(CStateManager& mgr, const CVector3f& pos);
  bool PointOnSurface(const CCollisionSurface& surface, const CVector3f& pos, const CPlane& plane);
  CVector3f ProjectPointToPlane(const CVector3f& point, const CVector3f& planePoint,
                                const CVector3f& normal);
  CVector3f ProjectVectorToPlane(const CVector3f& point, const CVector3f& normal);
  static CTransform4f ShortestRotationArcWrapped(const CVector3f& a, const CVector3f& b,
                                                 const CRelAngle& angle);
  void HardwareLight(const CStateManager& mgr, const CAABox& bounds) const;
  CColor SoftwareLight(const CStateManager& mgr, const CAABox& bounds) const;

private:
  // Guessed member names, supported by native accesses and the corresponding swarm members.
  CAABox mAabox;
  int mThinkCounter;
  float mOccludedTimer;
  rstl::vector< CBoid > mBoids;
  CVector3f mBoundingBoxExtent;
  float mSeparationRadius;
  float mCohesionMagnitude;
  float mSeparationMagnitude;
  float mAttractionMagnitude;
  float mAttractionRadius;
  rstl::reserved_vector< CBoid*, 125 > mPartitionedBoidLists;
  CBoid* mOutlierBoidList;
  float mBoidGenRate;
  float mBoidGenCooldownTimer;
  float mDamageCooldownTimer;
  float mDamageCooldown;
  float mBoidRadius;
  float mTouchRadius;
  float mPlayerTouchRadius;
  CDamageInfo mDamage;
  rstl::vector< CRepulsor > mDoorRepulsors;
  int mNumBoids;
  int mMaxCreatedBoids;
  int mCreatedBoids;
  float mSurfaceProbeScale;
  float mSafeZoneAvoidance;
  float mSurfaceStickPriority; // Guessed name; the unnamed 0x4a85a2da loader property
  float mContainmentPriority;
  float mPatrolTurnSpeed; // Radians per second.
  float mAvoidSafeZoneTurnSpeed;
  float mPatrolSpeed;
  float mSafeZoneEscapeSpeed;
  float mPlayerPursuitSpeed;
  float mAcceleration;
  float mDeceleration;
  float mColorChangeTime;
  CColor mPatrolColor;
  CColor mPursuitColor;
  rstl::optional_object< TLockedToken< CGenDescription > > mDeathParticleDescription;
  rstl::single_ptr< CElementGen > mDeathParticleGenerator;
  int mNumDeathParticles;
  rstl::optional_object< TLockedToken< CGenDescription > > mBoidParticleDescription;
  rstl::single_ptr< CElementGen > mBoidParticleGenerator;
  ushort mPatrolSound;
  float mMinPatrolSoundTime;
  float mMaxPatrolSoundTime;
  float mNextPatrolSoundTime;
  float mPatrolSoundWeight;
  float mPatrolSoundTimer;
  ushort mPursuitSound;
  float mMinPursuitSoundTime;
  float mMaxPursuitSoundTime;
  float mNextPursuitSoundTime;
  float mPursuitSoundWeight;
  float mPursuitSoundTimer;
  float mSoundFallOff;
  float mMaxAudibleDistance;
  uchar mMinVolume;
  uchar mMaxVolume;
  int mLockOnIndex;
  CVector3f mLastOrbitPosition;
  CVector3f mLockOnBlendStart;
  float mLockOnBlend;
  CModelData mScanModelData;
  bool mBlendingLockOn : 1;
  bool mSpawnInstantly : 1;
  bool x504_26_ : 1;
  bool mAnyPatrolling : 1;
  bool mAnyPursuing : 1;
};
NESTED_CHECK_SIZEOF(CBacteriaSwarm, CBoid, 0x88)
NESTED_CHECK_SIZEOF(CBacteriaSwarm, CRepulsor, 0x10)
CHECK_SIZEOF(CBacteriaSwarm, 0x508)

#endif // _CBACTERIASWARM
