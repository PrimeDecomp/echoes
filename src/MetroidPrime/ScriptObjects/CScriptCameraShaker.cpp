#include "MetroidPrime/ScriptObjects/CScriptCameraShaker.hpp"

CScriptCameraShaker::CScriptCameraShaker(TUniqueId uid, const rstl::string& name,
                                         const CEntityInfo& info,
                                         const CCameraShakerData& shakeData)
: CEntity(uid, info, name, 0), mShakeData(shakeData) {
  for (int i = 0; i < 4; ++i) {
    mPlayerShakeIds[i] = -1;
  }
}

void CScriptCameraShaker::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  // TODO: Action updates the originator position when flag 0x80 is set, then starts
  // shakes in unoccluded areas. Stop removes them. Flag 2 selects all players;
  // otherwise the message's originator player selects the camera-shaker manager.
  CEntity::AcceptScriptMsg(mgr, msg);
}

CScriptCameraShaker::~CScriptCameraShaker() {}
