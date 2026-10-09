#ifndef _COCTAPEDESEGMENT
#define _COCTAPEDESEGMENT

#include "types.h"

#include "Kyoto/Particles/CParticleElectric.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Enemies/CWallCrawler.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/vector.hpp"

// Wii SEL class name (OctapedeSegment REL). One link of the Octapede: the head crawls along its
// waypoints, the other segments trail the one in front, and when killed a segment breaks apart,
// bounces around and explodes.
class COctapedeSegment : public CWallCrawler {
public:
  enum ESegmentState {
    kSegS_None = -1,    // Guessed name
    kSegS_Attached = 0, // Guessed name
    kSegS_Bouncing = 1, // Guessed name
    kSegS_Landed = 2    // Guessed name
  };

  COctapedeSegment(TUniqueId uid, const rstl::string& name, CEntityInfo& info,
                   const CTransform4f& xf, const CModelData& modelData,
                   const CPatternedInfo& patternedInfo, const CActorParameters& actorParams,
                   float stickyReach, float floorTurnSpeed, float waypointApproachDistance,
                   float visibleDistance, float projectileBoundsMultiplier,
                   float collisionLookAhead, float animSpeedScalar, float maxAudibleDistance,
                   bool initiallyPaused, float segmentSpacing, CAssetId betweenSegmentsEffect,
                   float minBreakApartSpeed, float maxBreakApartSpeed, float minBreakApartAngle,
                   float maxBreakApartAngle, float minBreakApartSpinSpeed,
                   float maxBreakApartSpinSpeed, float minRunAroundTime, float maxRunAroundTime,
                   float minTurnInterval, float maxTurnInterval, int minBounces, int maxBounces,
                   float bounciness, const CDamageInfo& explosionDamage, ushort walkSound,
                   ushort idleSound, ushort separateSound, ushort bounceSound, ushort explodeSound,
                   float soundFalloff);

  // CEntity
  ~COctapedeSegment() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

  // CPatterned
  void Freeze(CStateManager& mgr, const CVector3f& position, CUnitVector3f direction,
              float duration, float intoFreezeDuration) override;
  void ThinkAboutMove(float dt) override;
  void Burn(CStateManager& mgr, float duration, float damage) override;
  void SetupStateMachine(CStateManager& mgr) override;

  // COctapedeSegment
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);

private:
  void Explode(CStateManager& mgr);                                      // Guessed name
  void Bounce(CStateManager& mgr, const CVector3f& normal, bool counts); // Guessed name

  TUniqueId mCurrentWaypointId;                  // Guessed name
  float mAnimSpeedScalar;                        // Guessed name
  float mMaxAudibleDistance;                     // Guessed name
  float mCollisionLookAhead;                     // Guessed name
  ESegmentState mSegmentState;                   // Guessed name
  TUniqueId mFollowedSegmentId;                  // Guessed name
  TUniqueId mHeadSegmentId;                      // Guessed name
  float mSegmentSpacing;                         // Guessed name
  rstl::auto_ptr< CParticleElectric > mElectric; // Guessed name
  float mMinBreakApartSpeed;                     // Guessed name
  float mMaxBreakApartSpeed;                     // Guessed name
  float mMinBreakApartAngle;                     // Guessed name
  float mMaxBreakApartAngle;                     // Guessed name
  float mMinRunAroundTime;                       // Guessed name
  float mMaxRunAroundTime;                       // Guessed name
  float mMinTurnInterval;                        // Guessed name
  float mMaxTurnInterval;                        // Guessed name
  CVector3f mBounceDirection;                    // Guessed name
  float mBreakApartSpeed;                        // Guessed name
  float mMinBreakApartSpinSpeed;                 // Guessed name
  float mMaxBreakApartSpinSpeed;                 // Guessed name
  float mRunAroundTimer;                         // Guessed name
  int mMinBounces;                               // Guessed name
  int mMaxBounces;                               // Guessed name
  float mBounciness;                             // Guessed name
  int mBouncesRemaining;                         // Guessed name
  float mTurnTimer;                              // Guessed name
  CVector3f mRunDirection;                       // Guessed name
  CDamageInfo mExplosionDamage;                  // Guessed name
  ushort mWalkSound;                             // Guessed name
  ushort mIdleSound;                             // Guessed name
  ushort mSeparateSound;                         // Guessed name
  ushort mBounceSound;                           // Guessed name
  ushort mExplodeSound;                          // Guessed name
  float mSoundFalloff;                           // Guessed name
  float mSharedFreezeDuration;                   // Guessed name
  float mSharedBurnDuration;                     // Guessed name
  float mSharedBurnDamage;                       // Guessed name
  rstl::vector< TUniqueId > mChildSegmentIds;    // Guessed name
  bool mBrokenApart : 1;                         // Guessed name
  bool mBounceLatch : 1;                         // Guessed name
};
CHECK_SIZEOF(COctapedeSegment, 0x938)

#endif // _COCTAPEDESEGMENT
