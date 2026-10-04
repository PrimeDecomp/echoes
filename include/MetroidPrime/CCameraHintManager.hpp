#ifndef _CCAMERAHINTMANAGER
#define _CCAMERAHINTMANAGER

#include "MetroidPrime/CHintManager.hpp"

class CScriptCameraHint;

// Guessed name: the camera specialization of the shared hint manager.
class CCameraHintManager : public CHintManager {
public:
  CCameraHintManager(int playerIndex, const rstl::string& name);

  // CHintManager
  ~CCameraHintManager() override;
  void RefreshHint(CStateManager& mgr) override;
  bool SetHint(CHintState* hint, CStateManager& mgr, bool areaChanged, bool force) override;
  void ClearHint(CStateManager& mgr, bool areaChanged) override;
  bool SelectHintFromStack(CHintState* hint, CStateManager& mgr, bool areaChanged) override;
  void OnHintRemoved(CStateManager& mgr) override;

  bool HasBallCameraInitialPositionHint(const CStateManager& mgr) const;

private:
  // Guessed name. An initialize-position hint can be null.
  void TeleportInitialPosition(const CScriptCameraHint* hint, CStateManager& mgr);
};
CHECK_SIZEOF(CCameraHintManager, 0x44)

#endif // _CCAMERAHINTMANAGER
