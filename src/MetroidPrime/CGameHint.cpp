#include "MetroidPrime/CGameHint.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"

CGameHint::CGameHint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, int priority, float timer, int acrossAreas,
                     int breakType, uint deleteOnRemoval, uint requiredPresses, float unknown16c,
                     const SCallback& onExpire, const SCallback& onBreak, float breakDelay)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters(), kInvalidUniqueId)
, mPriority(priority)
, mTimer(timer)
, mBreakType(breakType)
, mDeleteOnRemoval(deleteOnRemoval)
, mRequiredPresses(requiredPresses)
, x16c_(unknown16c)
, mOnExpire(onExpire)
, mOnBreak(onBreak)
, mBreakDelay(breakDelay)
, mAcrossAreas(acrossAreas) {}

CGameHint::~CGameHint() {}

void CGameHint::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  if (!mgr.IsMultiplayer()) {
    CActor::AcceptScriptMsg(mgr, msg);
    return;
  }

  CScriptMsg forwarded = msg;
  switch (msg.GetMessage()) {
  case kSM_Deactivate:
  case kSM_Decrement:
  case kSM_Increment:
    if (CGameCamera* camera = TCastToPtr< CGameCamera >(mgr.ObjectById(msg.GetOriginator()))) {
      forwarded = CScriptMsg(msg.GetUnk(), camera->Player(mgr).GetUniqueId(), msg.GetId(),
                             msg.GetMessage(), msg.GetState());
    }
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(mgr, forwarded);
}
