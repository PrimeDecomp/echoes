#include "MetroidPrime/Player/CPlayerBodyController.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

CPlayerBodyController::SKnockBackState::SKnockBackState() {}

void CPlayerBodyController::SKnockBackState::Start(CStateManager& mgr,
                                               CPlayerBodyController& controller) {
  const CPBCKnockBackCmd* command =
      static_cast< const CPBCKnockBackCmd* >(controller.CommandMgr().GetCmd(kPBSC_KnockBack));
  if (command) {
    const CVector3f direction =
        controller.GetPlayer().GetTransform().TransposeRotate(command->GetDirection());
    const CAbsAngle angle = CAbsAngle::FromRadians(atan2(direction.GetY(), direction.GetX()));
    const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_KnockBack),
                                CPASAnimParm::FromReal32(angle.AsDegrees()));
    controller.SelectAnimation(parms, *mgr.Random());
  }
}

bool CPlayerBodyController::SKnockBackState::Update(CStateManager& mgr,
                                                CPlayerBodyController& controller) {
  return !controller.IsAnimationOver();
}

void CPlayerBodyController::SKnockBackState::Shutdown(CPlayerBodyController& controller) {}
