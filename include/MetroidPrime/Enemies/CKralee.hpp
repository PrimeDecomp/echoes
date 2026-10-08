#ifndef _CKRALEE
#define _CKRALEE

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CWallCrawler.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"

// Guessed class: a wall-crawling enemy that warps in and out of sight.
class CKralee : public CWallCrawler {
public:
  enum EWarpState { kWS_Visible, kWS_Invisible, kWS_WarpIn, kWS_WarpOut }; // Guessed names

  CKralee(TUniqueId uid, const rstl::string& name, CEntityInfo& info, const CTransform4f& xf,
          const CModelData& modelData, const CPatternedInfo& patternedInfo,
          const CActorParameters& actorParams, float stickyReach, float floorTurnSpeed,
          float waypointApproachDistance, float visibleDistance, float projectileBoundsMultiplier,
          float collisionLookAhead, float warpInTime, float warpOutTime, bool initiallyPaused,
          float warpAttackRadius, float warpAttackKnockback, float warpAttackDamage,
          float animSpeedScalar, float maxAudibleDistance, CAssetId warpInParticleEffect,
          CAssetId warpOutParticleEffect, ushort warpInSound, ushort warpOutSound,
          bool initiallyInvisible, float visibleTime, float visibleTimeRandomOffset,
          float invisibleTime, float invisibleTimeRandomOffset);

  // CEntity
  ~CKralee() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;

  // CKralee
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void WarpOut(CStateManager& mgr, EStateMsg msg, float dt);
  void WarpOutExit(CStateManager& mgr, float dt);
  bool ShouldPatrol(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldWarpOut(CStateManager& mgr, const CTriggerData& data) const;
  bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;

private:
  void WarpIn(CStateManager& mgr);           // Guessed name
  EWarpState mWarpState;                     // Guessed name
  float mCollisionLookAhead;                 // Guessed name
  TUniqueId mCurrentWaypointId;              // Guessed name
  float mWarpInTime;                         // Guessed name
  float mWarpOutTime;                        // Guessed name
  CDamageInfo mWarpAttackDamage;             // Guessed name
  float mAnimSpeedScalar;                    // Guessed name
  float mMaxAudibleDistance;                 // Guessed name
  float mWarpStateTimer;                     // Guessed name
  CAssetId mWarpInParticleEffect;            // Guessed name
  CAssetId mWarpOutParticleEffect;           // Guessed name
  ushort mWarpInSound;                       // Guessed name
  ushort mWarpOutSound;                      // Guessed name
  uchar mEffectIndex;                        // Guessed name
  CDamageVulnerability mDamageVulnerability; // Guessed name
  bool mWantsWarpOut;                        // Guessed name
  float mVisibleTime;                        // Guessed name
  float mVisibleTimeRandomOffset;            // Guessed name
  float mInvisibleTime;                      // Guessed name
  float mInvisibleTimeRandomOffset;          // Guessed name
  float mStateElapsedTime;                   // Guessed name
  float mVisibleDuration;                    // Guessed name
  float mInvisibleDuration;                  // Guessed name
  CTransform4f mWarpEffectTransform;         // Guessed name
};
CHECK_SIZEOF(CKralee, 0x938)

#endif // _CKRALEE
