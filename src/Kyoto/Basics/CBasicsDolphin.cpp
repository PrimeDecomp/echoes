#include "Kyoto/Basics/CBasics.hpp"

#include "Kyoto/Basics/CStopwatch.hpp"

#include "dolphin/dvd.h"
#include "dolphin/os.h"

static bool gInitialized = false;
bool CBasics::Init() {
  if (!gInitialized) {
    OSInit();
    OSInitFastCast();
    DVDInit();

    CStopwatch::InitGlobalTimer();
    gInitialized = true;
  }
  return gInitialized;
}
