#ifndef _CSCRIPTGUIPLAYERJOINMANAGER
#define _CSCRIPTGUIPLAYERJOINMANAGER

#include "MetroidPrime/CEntity.hpp"

#include "rstl/reserved_vector.hpp"

class CScriptGuiPlayerJoinManager : public CEntity {
public:
  // Guessed names for the per-controller join states.
  enum EJoinState {
    kJS_Disconnected,
    kJS_Connected,
    kJS_Joined,
    kJS_Ready,
    kJS_ForcedJoin,
  };

  CScriptGuiPlayerJoinManager(TUniqueId uid, const rstl::string& name, const CEntityInfo& info);
  ~CScriptGuiPlayerJoinManager() override;

  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // Guessed names.
  bool HasEnoughPlayers() const;
  bool NoPlayersJoined() const;
  void UpdatePlayerCount(CStateManager& mgr);
  void SendControllerStates(CStateManager& mgr);
  void RestorePreviousPlayers(CStateManager& mgr);

private:
  rstl::reserved_vector< EJoinState, 4 > mJoinStates;
  int mPlayerCountState;
};
CHECK_SIZEOF(CScriptGuiPlayerJoinManager, 0x3c)

#endif // _CSCRIPTGUIPLAYERJOINMANAGER
