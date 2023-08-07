#include "Kyoto/Basics/CStopwatch.hpp"

#include <math.h>

CStopwatch::CSWData CStopwatch::mData;
CStopwatch CStopwatch::mGlobalTimer;

bool CStopwatch::InitGlobalTimer() {
  mGlobalTimer.Reset();
  return true;
}

void CStopwatch::Wait(float wait) {
  if (wait > 1.1920929E-7f) {
    mData.Wait(wait);
  }
}

CStopwatch& CStopwatch::GetGlobalTimerObj() { return mGlobalTimer; }
