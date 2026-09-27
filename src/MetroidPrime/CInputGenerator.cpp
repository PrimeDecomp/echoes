#include "MetroidPrime/CInputGenerator.hpp"

#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/Decode.hpp"

#include "Kyoto/Basics/COsContext.hpp"

CInputGenerator::CInputGenerator(COsContext* ctx, float leftDiv, float rightDiv)
: mContext(ctx)
, mController(IController::Create(*ctx))
, mLeftDiv(leftDiv)
, mRightDiv(rightDiv) {
  for (uint i = 0; i <= kIOP_Player4; ++i) {
    mConnectedControllers[i] = false;
  }
}

bool CInputGenerator::Update(float dt, CArchitectureQueue& queue) {
  if (!mContext->Update()) {
    return false;
  }

  if (!mController.null()) {
    const int count = mController->GetDeviceCount();
    mController->Poll();
    for (int i = 0; i < count; ++i) {
      const CControllerGamepadData& cont = mController->GetGamepadData(i);
      if (cont.DeviceIsPresent() || cont.DeviceJustDisconnected()) {
        const CFinalInput input(i, dt, cont, mLeftDiv, mRightDiv);
        const CArchitectureMessage msg = MakeMsg::CreateUserInput(kAMT_Game, input);
        queue.Push(msg);
      }

      const bool connected = cont.DeviceIsPresent();
      if (mConnectedControllers[i] != connected) {
        const CArchitectureMessage msg = MakeMsg::CreateControllerStatus(kAMT_Game, i, connected);
        queue.Push(msg);
        mConnectedControllers[i] = connected;
      }
    }
  }

  return true;
}
