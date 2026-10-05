#ifndef _CSWARMBASICS
#define _CSWARMBASICS

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "WorldFormat/CCollisionSurface.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/vector.hpp"

class CAnimRes;
class CAreaCollisionCache;
class CBasicSwarmData;
class CElementGen;
class CGenDescription;

namespace SwarmRenderHelpers {
class CSwarmDisplayList;
class CSwarmSkinnedModelState;
} // namespace SwarmRenderHelpers

class CSwarmBasics : public CActor {
public:
  class CBoid {
    friend class CSwarmBasics;

  public:
    CBoid(const CTransform4f& xf, uint index);
    bool GetActive() const { return mActive; }
    CVector3f GetTranslation() const { return mTransform.GetTranslation(); }
    const CTransform4f& GetTransform() const { return mTransform; }

  private:
    // Guessed member names; layout is derived from the GameCube constructor/consumers.
    CTransform4f mTransform;
    CVector3f mVelocity;
    TUniqueId mTargetWaypoint;
    CPlane mSurfacePlane;
    CColor mAmbientLighting;
    CBoid* mNext;
    float mFreezeTimer;
    float x5c_;
    float mLifeTime;
    CCollisionSurface mSurface;
    float mHealth;
    int x9c_;
    float mDistanceSquaredToSoundListener;
    float xa4_;
    TUniqueId xa8_;
    TUniqueId xaa_;
    uint mFramesNotOnSurface : 8;
    uint mIndex : 10;
    uint xac_ : 14;
    signed char mPartitionIndex;
    uchar xb1_;
    bool mActive : 1;
    bool mInFrustum : 1;
    bool mLaunched : 1;
    bool xb2_3 : 1;
    bool xb2_4 : 1;
    bool mHasLoopedSound : 1;
    bool mNearPlayer : 1;
    bool xb2_7 : 1;
  };

  class CRepulsor {
    friend class CSwarmBasics;
    CVector3f mCenter;
    float mMagnitude;
  };

  // Original Wii enum type; enumerator names are guessed from native sound consumers.
  enum ELoopedSoundType { kLST_Locomotion = 0, kLST_Attack = 1 };
  typedef rstl::pair< CSfxHandle, ushort > TLoopedSound;

  CSwarmBasics(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CVector3f& boundingBoxExtent, const CTransform4f& xf, const CAnimRes& animRes,
               CActorParameters actorParameters, const CBasicSwarmData& data, bool active);

  // CEntity
  ~CSwarmBasics() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  CHealthInfo* HealthInfo() override;
  const CHealthInfo* GetHealthInfo() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;

  // CSwarmBasics
  virtual CAABox GetBoundingBox() const;
  virtual void CreateBoid(CStateManager& mgr, int index);
  virtual void ApplyRadiusDamage(CVector3f position, const CDamageInfo& info, CStateManager& mgr);
  virtual void PreRenderBoid(CBoid* boid, uint* drawMask);
  virtual void UpdateBoid(CAreaCollisionCache& cache, CStateManager& mgr, float dt, CBoid& boid,
                          int partitionIndex);
  virtual void ApplySteeringBehaviors(CStateManager& mgr, CBoid& boid, CVector3f& ahead,
                                      const rstl::reserved_vector< CBoid*, 50 >& nearList);
  virtual void KillBoid(CBoid& boid, CStateManager& mgr, const CWeaponMode& weapon);
  // Guessed names; these are virtual in Echoes, unlike Prime's swarm queries.
  virtual bool GetLockOnLocationValid(int index) const;
  virtual CVector3f GetLockOnLocation(int index) const;
  virtual void FreezeBoids(const CVector3f& position, float radius);
  virtual TUniqueId GetSeekerTargetLockedOn() const;
  virtual void RenderBoid(CBoid* boid) const;
  virtual void AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which);
  virtual void UpdateSwarmAnimations(CStateManager& mgr, float dt);
  virtual void UpdateAllBoidMovement(CStateManager& mgr, float dt);
  virtual bool ShouldBuildAreaCollisionCacheForPartition(int partitionIndex) const;
  virtual void BoidCollidedCallback(CStateManager& mgr, CBoid& boid);
  virtual void BoidCollidedWithPlayerCallback(CStateManager& mgr, CBoid& boid);
  virtual void UpdateClosestPartitionLoopedSounds(const CVector3f& listener,
                                                  rstl::vector< TLoopedSound >& sounds,
                                                  uint maxEmitters, ushort sfx,
                                                  ELoopedSoundType type);

  int GetBoidCount() const { return mBoids.size(); }
  const CVector3f& GetLastKilledOffset() const { return mLastKilledOffset; }
  int GetCurrentLockOnId() const { return mLockOnIndex; }

private:
  // Opaque owned allocation: cleanup is established, but its element type is unresolved.
  class CUnknownBuffer; // Guessed name

  // Guessed member names, supported by native accesses and the corresponding Prime swarm.
  CAABox mAabox;
  int mThinkCounter;
  float mOccludedTimer;
  rstl::vector< CBoid > mBoids;
  CVector3f mBoundingBoxExtent;
  mutable CVector3f mLastOrbitPosition;
  CVector3f mLastKilledOffset;
  float mSeparationRadius;
  float mCohesionMagnitude;
  float mAlignmentWeight;
  float mSeparationMagnitude;
  float mMoveToWaypointWeight;
  float mAttractionMagnitude;
  float mAttractionRadius;
  float x1c8_;
  float mAnimPlaybackSpeed;
  float mWaypointGoalRadius;
  rstl::reserved_vector< CBoid*, 125 > mPartitionedBoidLists;
  CBoid* mOutlierBoidList;
  float mBoidGenRate;
  float mBoidGenCooldownTimer;
  float mDamageCooldownTimer;
  float mDamageCooldown;
  float mBoidRadius;
  float mTouchRadius;
  float mSafeZoneAvoidancePriority;
  float mPlayerTouchRadius;
  CDamageInfo mDamage;
  CDamageInfo mRadiusDamage;
  CHealthInfo mHealthInfo;
  CDamageVulnerability mDamageVulnerability;
  int mLockOnIndex;
  rstl::vector< SwarmRenderHelpers::CSwarmSkinnedModelState > mSkinnedModelStates;
  rstl::vector< CModelData > mModelDatas;
  rstl::vector< CAdvancementDeltas > mAdvancementDeltas;
  rstl::single_ptr< CModelData > mModelData;
  rstl::single_ptr< SwarmRenderHelpers::CSwarmDisplayList > mDisplayList;
  rstl::single_ptr< SwarmRenderHelpers::CSwarmSkinnedModelState > mSkinnedModelState;
  CModelData::EWhichModel mWhichModel;
  rstl::vector< CRepulsor > mDoorRepulsors;
  rstl::optional_object< TCachedToken< CGenDescription > > mParticleDescription;
  rstl::single_ptr< CElementGen > mParticleGenerator;
  int mAttackerCount;
  int mNumBoids;
  int mMaxCreatedBoids;
  int mCreatedBoids;
  ushort x4f0_;
  float x4f4_;
  ushort mLocomotionLoopedSound;
  ushort mAttackLoopedSound;
  rstl::vector< TLoopedSound > mLocomotionSounds;
  rstl::vector< TLoopedSound > mAttackSounds;
  float mSoundFallOff;
  float mMaxAudibleDistance;
  uchar mMinVolume;
  uchar mMaxVolume;
  uchar mMaxLocomotionEmitters;
  uchar mMaxAttackEmitters;
  int x528_;
  int x52c_;
  int x530_;
  int x534_;
  float mFreezeDuration;
  rstl::auto_ptr< CUnknownBuffer > x53c_;
  uint x544_;
  float mLifeTime;
  uint x54c_;
  CVector3f x550_;
  float x55c_;
  int x560_;
  rstl::vector< TUniqueId > mSeekerTargets;
  rstl::vector< int > x574_;
  rstl::vector< int > mActiveBoidIndices;
};
NESTED_CHECK_SIZEOF(CSwarmBasics, CBoid, 0xb8)
NESTED_CHECK_SIZEOF(CSwarmBasics, CRepulsor, 0x10)
NESTED_CHECK_SIZEOF(CSwarmBasics, TLoopedSound, 0x8)
CHECK_SIZEOF(CSwarmBasics, 0x598)

#endif // _CSWARMBASICS
