#include "GuiSys/CRepeatState.hpp"

CRepeatState::CRepeatState() : mTimer(0.f) {}

const bool CRepeatState::Update(float dt, bool pressed) {
  bool repeat = false;
  if (mTimer == 0.f) {
    if (pressed) {
      mTimer = 0.6f;
      repeat = true;
    }
  } else {
    if (pressed) {
      mTimer -= dt;
      if (mTimer <= 0.f) {
        mTimer = 0.05f;
        repeat = true;
      }
    } else {
      mTimer = 0.f;
    }
  }

  return repeat;
}
