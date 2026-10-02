#include "MetroidPrime/Player/CPlayer.hpp"

// NonMatching scaffold. Definitions are in reverse target order for deferred inlining.

void CPlayer::fn_8022c338(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdatePlayerHints(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::ResetPlayerHintState(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::SetAreaPlayerHint(const CScriptPlayerHint& hint, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
  return false;
}

// Guessed name
bool CPlayer::FireBeamHeld(const CFinalInput& input) const {
  // TODO: Recover the remaining target behavior.
  return false;
}

bool CPlayer::FireBeamPressed(const CFinalInput& input) const {
  // TODO: Recover the remaining target behavior.
  return false;
}

bool CPlayer::fn_8022b974(const CFinalInput& input) const {
  // TODO: Recover the remaining target behavior.
  return false;
}

// Guessed name
bool CPlayer::JumpHeld(const CFinalInput& input) const {
  // TODO: Recover the remaining target behavior.
  return false;
}

// Guessed name
bool CPlayer::JumpPressed(const CFinalInput& input) const {
  // TODO: Recover the remaining target behavior.
  return false;
}

bool CPlayer::fn_8022b7f4(const CFinalInput& input) const {
  // TODO: Recover the remaining target behavior.
  return false;
}

bool CPlayer::fn_8022b7a8(const CFinalInput& input) const {
  // TODO: Recover the remaining target behavior.
  return false;
}

// Guessed name
void CPlayer::UpdateRezbitRecoveryInput(const CFinalInput& input) {
  // TODO: Recover the remaining target behavior.
}

CPlayer::ERezbitState CPlayer::GetRezbitState() const { return mRezbitState; }

// Guessed name
void CPlayer::SetRezbitState(ERezbitState state) {
  if (state == kRS_None || state == kRS_Recovered) {
    mRezbitState = state;
  }
}

void CPlayer::StartRezbitState(CStateManager& mgr, const CRezbitEffectOptions& options) {
  // TODO: Recover the remaining target behavior.
}

// Guessed name
void CPlayer::UpdateRezbitState(float dt) {
  // TODO: Recover the remaining target behavior.
}

// Guessed name
void CPlayer::BeginRezbitRecovery() {
  // TODO: Recover the remaining target behavior.
}

// Guessed name
void CPlayer::ResetRezbitState(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

// Guessed name
void CPlayer::StopRezbitState(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

TUniqueId CPlayer::fn_8022af0c(CStateManager& mgr, uint controls, TUniqueId source, float duration,
                               int breakType) {
  // TODO: Create a temporary control hint. Recover the shared break-hint enum.
  return kInvalidUniqueId;
}
