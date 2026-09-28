#ifndef _CSCRIPTDOCK
#define _CSCRIPTDOCK

#include "MetroidPrime/CPhysicsActor.hpp"

class CPlane;

class CScriptDock : public CPhysicsActor {
public:
  CScriptDock(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
              const CVector3f& position, const CVector3f& extent, int dock, TAreaId area,
              int dockReferenceCount, bool loadConnected, bool isVirtual, bool showSoftTransition);

  // CEntity
  ~CScriptDock() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  int GetDockId() const { return mDock; }
  TAreaId GetAreaId() const { return mArea; }
  int GetDockReference(const CStateManager& mgr) const;
  TUniqueId GetConnectedScriptDockId(const CStateManager& mgr) const;
  TAreaId GetCurrentConnectedAreaId(const CStateManager& mgr) const;
  CPlane GetPlane(const CStateManager& mgr) const;
  bool HasPointCrossedDock(const CStateManager& mgr, const CVector3f& point) const;
  void UpdateAreaActivateFlags(CStateManager& mgr);
  bool IsVirtual() const { return mIsVirtual; }
  bool GetLoadConnected() const { return mLoadConnected; }
  void SetLoadConnected(bool load) { mLoadConnected = load; }
  void SetLoadConnected(CStateManager& mgr, bool loadConnected, bool pauseValidation);
  void InitializeConnectedArea(CStateManager& mgr); // Guessed name.
  void AreaUnloaded(CStateManager& mgr);

private:
  // Guessed names, following the Prime dock state machine.
  enum EDockState { kDS_InSourceRoom, kDS_PlayerTouched, kDS_EnterNextArea, kDS_InNextRoom };

  int mDockReferenceCount;
  int mDock;
  TAreaId mArea;
  EDockState mDockState;
  bool mDockReferenced : 1;
  bool mLoadConnected : 1;
  bool mAreaPostConstructed : 1;
  bool mIsVirtual : 1;
  bool mShowSoftTransition : 1;
};
CHECK_SIZEOF(CScriptDock, 0x2e8)

#endif // _CSCRIPTDOCK
