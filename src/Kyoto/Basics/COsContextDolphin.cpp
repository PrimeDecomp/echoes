#include "Kyoto/Basics/COsContext.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "dolphin/os.h"

COsContext::COsContext(bool, bool)
: mLanguage(OSGetLanguage())
, mArenaLo1(nullptr)
, mArenaHi(nullptr)
, mArenaLo2(nullptr)
, mArenaBlock(nullptr)
, x28_(nullptr)
, mArenaBlockSize(0) {
  CBasics::Init();

  switch (OSGetConsoleType()) {
  case OS_CONSOLE_RETAIL1:
    mConsoleType = kCT_Retail;
    break;
  case OS_CONSOLE_DEVHW1:
    mConsoleType = kCT_Development1;
    break;
  case OS_CONSOLE_DEVHW2:
  case OS_CONSOLE_DEVHW3:
    mConsoleType = kCT_Development2Or3;
    break;
  case OS_CONSOLE_EMULATOR:
    mConsoleType = kCT_Emulator;
    break;
  }

  mArenaBlockSize = 0x1fe000;
  mArenaBlock = AllocFromArena(mArenaBlockSize);
}

COsContext::~COsContext() {}

bool COsContext::Update() { return true; }

void* COsContext::AllocFromArena(size_t sz) {
  void* ret = OSAllocFromArenaLo(sz, 32);

  mArenaLo2 = OSGetArenaLo();
  mArenaLo1 = OSGetArenaLo();
  mArenaHi = OSGetArenaHi();
  return ret;
}

int COsContext::GetLanguage() const { return mLanguage; }
