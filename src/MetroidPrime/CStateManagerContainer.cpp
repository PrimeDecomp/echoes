#include "MetroidPrime/CStateManagerContainer.hpp"

#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

namespace {
// Guessed enum spelling for the native four-player index domain.
enum EPlayerIndex { kPI_Player1 = 0, kPI_Player2 = 1, kPI_Player3 = 2, kPI_Player4 = 3 };
} // namespace

CStateManagerContainer::CStateManagerContainer()
: mCameraManager0(kInvalidUniqueId, kPI_Player1)
, mCameraManager1(kInvalidUniqueId, kPI_Player2)
, mCameraManager2(kInvalidUniqueId, kPI_Player3)
, mCameraManager3(kInvalidUniqueId, kPI_Player4)
, mRumbleManager0(kPI_Player1, static_cast< EIOPort >(
                                   gpGameState->GetPlayerState(kPI_Player1)->GetPlayerSelection()))
, mRumbleManager1(kPI_Player2, static_cast< EIOPort >(
                                   gpGameState->GetPlayerState(kPI_Player2)->GetPlayerSelection()))
, mRumbleManager2(kPI_Player3, static_cast< EIOPort >(
                                   gpGameState->GetPlayerState(kPI_Player3)->GetPlayerSelection()))
, mRumbleManager3(
      kPI_Player4,
      static_cast< EIOPort >(gpGameState->GetPlayerState(kPI_Player4)->GetPlayerSelection())) {}
