#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"

#include "MetroidPrime/CHintManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"

CScriptPlayerHint::CScriptPlayerHint(TUniqueId uid, const rstl::string& name,
                                     const CEntityInfo& info, const CTransform4f& xf, int priority,
                                     float timer, uint overrideFlags, int acrossAreas,
                                     float controlInterpDur)
: CGameHint(uid, name, info, xf, priority, timer, acrossAreas, kBHT_None, 0, 0, 0.f, SCallback(),
            SCallback(), 0.f)
, mOverrideFlags(overrideFlags)
, mActorId(kInvalidUniqueId)
, mControlInterpDur(controlInterpDur) {}

void CScriptPlayerHint::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId sender = msg.GetUnk();
  switch (msg.GetMessage()) {
  case kSM_Deactivate:
  case kSM_XDelete: {
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(msg.GetOriginator()));
    if (!player) {
      player = mgr.GetPlayer(0);
    }
    player->GetPlayerHintManager()->ForceRemoveHint(GetUniqueId(), mgr, kInvalidUniqueId);
    break;
  }
  case kSM_Decrement: {
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(msg.GetOriginator()));
    if (!player) {
      player = mgr.GetPlayer(0);
    }
    player->GetPlayerHintManager()->RemoveHint(GetUniqueId(), sender, mgr);
    break;
  }
  case kSM_Increment:
    if (GetActive()) {
      CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(msg.GetOriginator()));
      if (!player) {
        player = mgr.GetPlayer(0);
      }
      player->GetPlayerHintManager()->AddHint(GetUniqueId(), sender, mgr);
    }
    break;
  case kSM_XALD:
    mActorId = CheckConnectedObject(mgr, kSS_Connect, kSM_Attach);
    break;
  default:
    break;
  }
  CGameHint::AcceptScriptMsg(mgr, msg);
}

CScriptPlayerHint::~CScriptPlayerHint() {}
