#ifndef _CSCRIPTTRIGGER
#define _CSCRIPTTRIGGER

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "rstl/list.hpp"

enum ETriggerFlags {
  kTFL_None = 0,
  kTFL_DetectPlayer = 0x1,
  kTFL_DetectMorphedPlayer = 0x2,
  kTFL_DetectUnmorphedPlayer = 0x4,
  kTFL_DetectAI = 0x8000,
  kTFL_KillOnEnter = 0x10000,
  kTFL_UseCollisionImpulses = 0x20000,
  kTFL_UseBooleanIntersection = 0x40000,
  kTFL_DetectCamera = 0x80000,
  kTFL_BlockEnvironmentalEffects = 0x100000,
  kTFL_DetectProjectiles = 0x200000,
  kTFL_DetectBombs = 0x400000,
  kTFL_DetectPowerBombs = 0x800000,
  kTFL_DetectScrewAttack = 0x10000000,
};

class CScriptTrigger : public CActor {
public:
  class CObjectTracker {
  public:
    CObjectTracker(TUniqueId id, TUniqueId triggerId);
    TUniqueId GetObjectId() const { return mId; }
    void SetObjectId(TUniqueId id) { mId = id; }
    const rstl::list< TUniqueId >& GetTriggers() const { return mTriggers; }

  private:
    TUniqueId mId;
    rstl::list< TUniqueId > mTriggers;
  };

  CScriptTrigger(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                 const CVector3f& position, const CAABox& bounds, const CDamageInfo& damage,
                 const CVector3f& forceField, uint flags, bool deactivateOnEntered,
                 bool deactivateOnExited);

  // CEntity
  ~CScriptTrigger() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  // CScriptTrigger
  virtual void InhabitantAdded(CActor& actor, CStateManager& mgr);
  virtual void InhabitantIdle(CActor& actor, CStateManager& mgr);
  virtual void InhabitantExited(CActor& actor, CStateManager& mgr);
  virtual void InhabitantRejected(CActor& actor, CStateManager& mgr);
  virtual bool ShouldSendScriptMsgs(CActor& actor, CStateManager& mgr) const; // Guessed name
  virtual bool BoundsOverlap(const CAABox& bounds) const;                     // Guessed name

  bool RemoveInhabitant(TUniqueId id, CStateManager& mgr);                      // Guessed name
  bool RemoveInhabitantIfOutside(TUniqueId id, CStateManager& mgr);             // Guessed name
  bool ReplaceInhabitant(TUniqueId oldId, TUniqueId newId, CStateManager& mgr); // Guessed name
  bool IsAI(CStateManager& mgr, CActor& actor) const;                           // Guessed name
  bool GetPlayerInside(int playerIndex) const;
  bool HasInhabitant(TUniqueId id) const; // Guessed name
  void UpdateInhabitants(float dt, CStateManager& mgr);
  void SetPlayerInside(CStateManager& mgr, bool inside, int playerIndex); // Guessed name
  void UpdateCameraInhabitant(TUniqueId id, CStateManager& mgr);          // Guessed name
  void NotifyInhabitantIdle(CActor& actor, CStateManager& mgr);           // Guessed name
  void NotifyInhabitantAdded(CActor& actor, CStateManager& mgr);          // Guessed name
  void NotifyInhabitantExited(CActor& actor, CStateManager& mgr);         // Guessed name
  void ClearInhabitants(CStateManager& mgr);                              // Guessed name
  CAABox GetTriggerBoundsWR() const;
  void AddInhabitant(CStateManager& mgr, int playerIndex, TUniqueId id,
                     TUniqueId triggerId); // Guessed name

  const CAABox& GetTriggerBounds() const { return mBounds; }
  uint GetTriggerFlags() const { return mFlags; }
  const CDamageInfo& GetDamageInfo() const { return mDamageInfo; }
  const rstl::list< CObjectTracker >& GetInhabitants() const { return mInhabitants; }

protected:
  rstl::list< CObjectTracker > mInhabitants;
  TUniqueId mAttachedTrigger; // Guessed name
  CDamageInfo mDamageInfo;
  CVector3f mForceField;
  float mForceMagnitude;
  uint mFlags;
  CAABox mBounds;
  uint mDeactivateOnEntered : 1;
  uint mDeactivateOnExited : 1;
  uint x1bc_2_ : 30; // Remaining flag-word bits are unresolved.
  bool mPlayerInside[4];
  bool mPlayerEnvironmentDamage[4]; // Guessed name
};
CHECK_SIZEOF(CScriptTrigger, 0x1c8)

#endif // _CSCRIPTTRIGGER
