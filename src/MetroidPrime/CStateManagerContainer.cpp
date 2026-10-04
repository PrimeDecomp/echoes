#include "MetroidPrime/CStateManagerContainer.hpp"

#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

CStateManagerContainer::CStateManagerContainer()
: mCameraManager0(kInvalidUniqueId, 0)
, mCameraManager1(kInvalidUniqueId, 1)
, mCameraManager2(kInvalidUniqueId, 2)
, mCameraManager3(kInvalidUniqueId, 3)
, mRumbleManager0(0, static_cast< EIOPort >(gpGameState->GetPlayerState(0)->GetPlayerSelection()))
, mRumbleManager1(1, static_cast< EIOPort >(gpGameState->GetPlayerState(1)->GetPlayerSelection()))
, mRumbleManager2(2, static_cast< EIOPort >(gpGameState->GetPlayerState(2)->GetPlayerSelection()))
, mRumbleManager3(3, static_cast< EIOPort >(gpGameState->GetPlayerState(3)->GetPlayerSelection())) {
}
