#ifndef _CSTONETOAD
#define _CSTONETOAD

#include "types.h"

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

// Guessed class: a statue-like toad that wakes up when hit, retreats from safe zones and returns to
// its starting spot afterwards.
class CStoneToad : public CPatterned {
public:
  // Guessed names.
  enum EToadState {
    kTS_Sleep,
    kTS_WakeUp,
    kTS_MoveAway,
    kTS_Wait,
    kTS_MoveBack,
    kTS_GoToSleep,
    kTS_ReactToSafeZone,
    kTS_ReactToWeapon
  };
  // Guessed names.
  enum EProvoker { kP_None = -1, kP_SafeZone, kP_Weapon };
  // Guessed names.
  enum EMoveType { kMV_Away, kMV_Back };

  CStoneToad(TUniqueId uid, const rstl::string& name, CEntityInfo& info, const CTransform4f& xf,
             const CModelData& modelData, const CPatternedInfo& patternedInfo,
             const CActorParameters& actorParams);

  // CEntity
  ~CStoneToad() override {}
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;
  CEntity* TypesMatch(int typeId) const override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& position,
                                                         const CVector3f& direction,
                                                         const CWeaponMode& mode,
                                                         int attributes) const override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;

  // CStoneToad
  void Sleep(CStateManager& mgr, EStateMsg msg, float dt);
  void WakeUp(CStateManager& mgr, EStateMsg msg, float dt);
  void MoveAway(CStateManager& mgr, EStateMsg msg, float dt);
  void Wait(CStateManager& mgr, EStateMsg msg, float dt);
  void MoveBack(CStateManager& mgr, EStateMsg msg, float dt);
  void GoToSleep(CStateManager& mgr, EStateMsg msg, float dt);
  void ReactToSafeZone(CStateManager& mgr, EStateMsg msg, float dt);
  void ReactToWeapon(CStateManager& mgr, EStateMsg msg, float dt);
  bool ShouldPatrol(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldMoveAway(CStateManager& mgr, const CTriggerData& data) const;
  bool MoveAwayFinished(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldMoveBack(CStateManager& mgr, const CTriggerData& data) const;
  bool MoveBackFinished(CStateManager& mgr, const CTriggerData& data) const;
  bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;
  bool HitBySafeZone(CStateManager& mgr, const CTriggerData& data) const;
  bool HitByWeapon(CStateManager& mgr, const CTriggerData& data) const;

private:
  void UpdateLookAt(CStateManager& mgr);            // Guessed name
  CVector3f GetMoveDirection(EMoveType type) const; // Guessed name
  bool IsMovingBackToStart() const;                 // Guessed name
  bool IsMovingAwayFromStart() const;               // Guessed name

  EToadState mState;                                                 // Guessed name
  CDamageInfo mContactDamageCopy;                                    // Guessed name
  rstl::vector< TUniqueId > mWaypoints;                              // Guessed name
  CTransform4f mInitialTransform;                                    // Guessed name
  CVector3f mAwayPosition;                                           // Guessed name
  CVector3f mMoveDirection;                                          // Guessed name
  float mAwayDelay;                                                  // Guessed name
  uint mLookUpAnim;                                                  // Guessed name
  uint mLookLeftAnim;                                                // Guessed name
  uint mLookRightAnim;                                               // Guessed name
  uint mLookDownAnim;                                                // Guessed name
  float mLookUpWeight;                                               // Guessed name
  float mLookDownWeight;                                             // Guessed name
  float mLookLeftWeight;                                             // Guessed name
  float mLookRightWeight;                                            // Guessed name
  rstl::single_ptr< CCollisionActorManager > mCollisionActorManager; // Guessed name
  bool mProvoked : 1;                                                // Guessed name
  bool mReturnRequested : 1;                                         // Guessed name
  EProvoker mProvoker;                                               // Guessed name
};
CHECK_SIZEOF(CStoneToad, 0x868)

#endif // _CSTONETOAD
