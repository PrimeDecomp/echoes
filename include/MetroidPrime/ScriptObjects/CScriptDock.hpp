#ifndef _CSCRIPTDOCK
#define _CSCRIPTDOCK

#include "MetroidPrime/CPhysicsActor.hpp"

// Partial interface recovered for CGameArea's dock management.
class CScriptDock : public CPhysicsActor {
public:
  int GetDockId() const { return mDock; }
  bool IsVirtual() const { return mIsVirtual; }
  bool GetLoadConnected() const { return mLoadConnected; }
  void SetLoadConnected(bool load) { mLoadConnected = load; }
  void InitializeConnectedArea(CStateManager& mgr); // Guessed name.

private:
  int mDockReferenceCount;
  int mDock;
  TAreaId mArea;
  int x2dc_;
  bool x2e0_0_ : 1;
  bool mLoadConnected : 1;
  bool x2e0_2_ : 1;
  bool mIsVirtual : 1;
  bool mShowSoftTransition : 1;
};
CHECK_SIZEOF(CScriptDock, 0x2e8)

#endif // _CSCRIPTDOCK
