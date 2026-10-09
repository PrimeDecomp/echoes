#ifndef _CSPORBTOP
#define _CSPORBTOP

#include "types.h"

#include "Collision/CCollidableSphere.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

// Guessed class: the head of a Sporb, driven by commands from its CSporbBase.
class CSporbTop : public CPatterned {
public:
  // Guessed names; values the base stores to select the head's next behavior.
  enum EState {
    kS_Invalid = -1,
    kS_Close = 0,
    kS_Spit = 1,
    kS_Sleeping = 2,
    kS_Patrol = 3,
    kS_Attack = 4,
    kS_Fire = 5,
    kS_Reload = 6,
    kS_Flinch = 7,
  };

  CSporbTop(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
            const CTransform4f& xf, const CModelData& modelData,
            const CPatternedInfo& patternedInfo, const CActorParameters& actorParams);

  // CEntity
  ~CSporbTop() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;

  // CPhysicsActor
  const CCollisionPrimitive* GetCollisionPrimitive() const override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

  // CPatterned
  void Freeze(CStateManager& mgr, const CVector3f& position, CUnitVector3f direction,
              float duration, float intoFreezeDuration) override;
  void SetupStateMachine(CStateManager& mgr) override;
  bool IsScanVisorSelfRender() const override { return true; }
  CAABox GetScanVisorRenderBounds(const CStateManager& mgr) const override;
  void ScanVisorRender(const CStateManager& mgr, const CTransform4f& xf,
                       const CModelFlags& flags) const override;

  // CSporbTop
  void Sleep(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void Attack(CStateManager& mgr, EStateMsg msg, float dt);
  void WakeUp(CStateManager& mgr, EStateMsg msg, float dt);
  void GoToSleep(CStateManager& mgr, EStateMsg msg, float dt);
  void Fire(CStateManager& mgr, EStateMsg msg, float dt);
  void Reload(CStateManager& mgr, EStateMsg msg, float dt);
  void Close(CStateManager& mgr, EStateMsg msg, float dt);
  void Spit(CStateManager& mgr, EStateMsg msg, float dt);
  void Flinch(CStateManager& mgr, EStateMsg msg, float dt);

  bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldPatrol(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldFire(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldReload(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldClose(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldSpit(CStateManager& mgr, const CTriggerData& data) const;

  void StartFlinch(CStateManager& mgr);
  // Guessed name; the target body is empty.
  void ResetAttack(CStateManager& mgr, bool flag);
  CAABox GetModelBounds() const;

  void SetState(EState state) { mState = state; }
  void SetBaseId(TUniqueId id) { mBaseId = id; }
  void SetProjectileId(TUniqueId id) { mProjectileId = id; }
  void SetOrbitPosition(const CVector3f& position) { mOrbitPosition = position; }
  const CVector3f& OrbitPosition() const { return mOrbitPosition; }
  void SetFireGenerateType(pas::EGenerateType type) { mFireGenerateType = type; }
  void SetImmune(bool immune) { mImmune = immune; }
  bool IsImmune() const { return mImmune; }
  float GetFreezeDuration() const { return mFreezeDuration; }

private:
  int mState;                           // Guessed name
  CVector3f mOrbitPosition;             // Guessed name
  float mFreezeDuration;                // Guessed name
  CCollidableSphere mSphere;            // Guessed name
  pas::EGenerateType mFireGenerateType; // Guessed name
  TUniqueId mBaseId;                    // Guessed name
  TUniqueId mProjectileId;              // Guessed name
  bool mImmune : 1;                     // Guessed name
};
CHECK_SIZEOF(CSporbTop, 0x808)

CEntity* REL_LoadSporbTop(CStateManager& mgr, CInputStream& input, CEntityInfo& info);

#endif // _CSPORBTOP
