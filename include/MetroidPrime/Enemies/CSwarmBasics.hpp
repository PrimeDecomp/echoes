#ifndef _CSWARMBASICS
#define _CSWARMBASICS

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "WorldFormat/CCollisionSurface.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/vector.hpp"

class CAdvancementDeltas;
class CAnimData;
class CAnimRes;
class CRelAngle;
class CMarkerGrid;
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
    float GetDistanceSquaredToSoundListener() const { return mDistanceSquaredToSoundListener; }
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
    float mTimeToExplode;
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
    uint mRemainingLaunchNotOnSurfaceFrames : 8;
    uint xac_ : 6;
    int mPartitionIndex : 8;
    uint xb1_ : 8;
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

  public:
    CRepulsor(CVector3f center, float magnitude) : mCenter(center), mMagnitude(magnitude) {}
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

  // Guessed names for non-virtual helpers.
  CAABox BoxForPosition(int x, int y, int z, float margin) const;
  CAreaCollisionCache GetAreaCollisionCacheForPartition(int x, int y, int z) const;
  void UpdateParticles(float dt);
  void RenderParticles() const;
  void CachePose(CModelData& modelData, SwarmRenderHelpers::CSwarmSkinnedModelState& state) const;
  void DrawBoidSkinnedModel(const CBoid* boid,
                            const SwarmRenderHelpers::CSwarmSkinnedModelState& state) const;
  void UpdateSeekerTargets(CStateManager& mgr);
  void AssignSeekerBoids(CStateManager& mgr, const rstl::vector< uint >& taken, uint numNeeded,
                         rstl::vector< uint >& out);
  void StopLoopedSound(CBoid& boid, rstl::vector< TLoopedSound >& sounds);
  void AddParticle(const CTransform4f& xf);
  void FreezeCollision(const CMarkerGrid& grid);
  int EvaluateActiveBoidCount() const;
  CVector3f FindClosestCell(const CVector3f& pos) const;
  CBoid* GetClosestPartitionList(const CVector3f& pos) const;
  uint UpdateLoopedSounds(uint maxEmitters, int partitionIndex,
                          rstl::vector< TLoopedSound >& sounds);
  bool AddLoopedSoundToHandlesList(CBoid& boid, rstl::vector< TLoopedSound >& sounds, ushort sfx);
  void StartLoopedSound(CBoid& boid, rstl::vector< TLoopedSound >& sounds, ushort sfx, uint slot);
  void UpdateLoopedSoundPositions(const rstl::vector< TLoopedSound >& sounds) const;
  bool CanStartLoopedSound(const CBoid& boid, ELoopedSoundType type) const;
  CSfxHandle AddLoopedEmitter(const CVector3f& pos, ushort sfx);
  void UpdateEffects(CStateManager& mgr, CAnimData& animData, int volume);
  void MoveBoid(CStateManager& mgr, CBoid& boid, const CVector3f& offsetDelta, float dt);
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
  void ApplyAttraction(CBoid& boid, const CVector3f& pos, float radius, float magnitude,
                       CVector3f& ahead);
  void ApplyAlignment(CBoid& boid, const rstl::reserved_vector< CBoid*, 50 >& nearList,
                      CVector3f& ahead);
  void ApplyBoundsAvoidance(CBoid& boid, const rstl::reserved_vector< CBoid*, 50 >& nearList,
                            CVector3f& ahead);
  void MoveToWayPoint(CBoid& boid, CStateManager& mgr, CVector3f& ahead);
  TUniqueId GetWaypointForState(EScriptObjectState state, CStateManager& mgr);
  void SetExplodeTimers(const CVector3f& pos, float radius, float minTime, float maxTime);
  bool IsBoidVisibleForLockOn(const CStateManager& mgr, const CBoid& boid,
                              const CVector3f& cameraPos, const CVector3f& cameraForward) const;
  int GetLockOnIndex(CStateManager& mgr) const;
  int FindBestLockOnIndex(CStateManager& mgr) const;
  void UpdateLockOnBlend(int prevIndex, int newIndex, float dt);
  void AddDoorRepulsors(CStateManager& mgr);
  void UpdateLightComboBeam(CBoid& boid, CStateManager& mgr);
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
  void FinishConstruction();                   // Guessed name; empty in the base class.
  void StopLocomotionSounds();                 // Guessed name.
  void QueueDeathMessage(CStateManager& mgr);  // Guessed name.
  void FlushDeathMessages(CStateManager& mgr); // Guessed name.

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
  float mTurnRate; // Degrees per second.
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
  rstl::optional_object< TLockedToken< CGenDescription > > mParticleDescription;
  rstl::single_ptr< CElementGen > mParticleGenerator;
  int mNumDeathParticles;
  int mNumBoids;
  int mMaxCreatedBoids;
  int mCreatedBoids;
  bool x4f0_24_ : 1;
  bool x4f0_25_ : 1;
  bool x4f0_26_ : 1;
  bool x4f0_27_ : 1;
  bool x4f0_28_ : 1;
  bool x4f0_29_ : 1;
  bool x4f0_30_ : 1;
  bool x4f0_31_ : 1;
  bool x4f1_24_ : 1;
  bool x4f1_25_ : 1;
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
  int x52c_; // Boids still to spawn immediately.
  int x530_; // Death messages sent this frame.
  int x534_; // Death messages deferred to later frames.
  float mFreezeDuration;
  rstl::auto_ptr< CUnknownBuffer > x53c_;
  uint x544_;
  float mLifeTime;
  bool x54c_24_ : 1;
  bool x54c_25_ : 1;
  bool x54c_26_ : 1;
  bool x54c_27_ : 1;
  bool x54c_28_ : 1;
  bool x54c_29_ : 1;
  bool x54c_30_ : 1;
  bool x54c_31_ : 1;
  CVector3f x550_;
  float x55c_;
  int x560_;
  rstl::vector< TUniqueId > mSeekerTargets;
  rstl::vector< int > mSeekerBoidIndices; // Boid index per entry of mSeekerTargets.
  rstl::vector< uint > mActiveBoidIndices;
};
NESTED_CHECK_SIZEOF(CSwarmBasics, CBoid, 0xb8)
NESTED_CHECK_SIZEOF(CSwarmBasics, CRepulsor, 0x10)
NESTED_CHECK_SIZEOF(CSwarmBasics, TLoopedSound, 0x8)
CHECK_SIZEOF(CSwarmBasics, 0x598)

#endif // _CSWARMBASICS
