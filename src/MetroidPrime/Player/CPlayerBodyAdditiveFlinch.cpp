#include "MetroidPrime/Player/CPlayerBodyController.hpp"

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

CPlayerBodyController::SAdditiveFlinchState::SAdditiveFlinchState() : mAnimationId(-1) {}

void CPlayerBodyController::SAdditiveFlinchState::Start(CStateManager& mgr,
                                                     CPlayerBodyController& controller) {
  const CPBCFlinchCmd* command =
      static_cast< const CPBCFlinchCmd* >(controller.CommandMgr().GetCmd(kPBSC_Flinch));
  if (command) {
    const CVector3f direction =
        controller.GetPlayer().GetTransform().TransposeRotate(command->GetDirection());
    const CAbsAngle angle = CAbsAngle::FromRadians(atan2(direction.GetY(), direction.GetX()));
    const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_AdditiveFlinch),
                                CPASAnimParm::FromReal32(angle.AsDegrees()));
    const rstl::pair< float, int > best =
        controller.GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
    mAnimationId = best.second;
    if (mAnimationId != -1) {
      controller.GetPlayer().AnimationData()->AddAdditiveAnimation(mAnimationId, 1.f, false, true);
    }
  } else {
    mAnimationId = -1;
  }
}

bool CPlayerBodyController::SAdditiveFlinchState::Update(CStateManager& mgr,
                                                      CPlayerBodyController& controller) {
  if (mAnimationId != -1) {
    const rstl::rc_ptr< CAnimTreeNode > tree =
        controller.GetPlayer().AnimationData()->GetAdditiveAnimationTree(mAnimationId);
    return tree && !close_enough(tree->VGetTimeRemaining().GetSeconds(), 0.f);
  }
  return false;
}

void CPlayerBodyController::SAdditiveFlinchState::Shutdown(CPlayerBodyController& controller) {
  mAnimationId = -1;
}
