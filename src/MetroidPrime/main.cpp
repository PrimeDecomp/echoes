#include "MetroidPrime/CMain.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"
#include "Kyoto/CPakFile.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Text/CStringTable.hpp"

#include "MetroidPrime/CGameArchitectureSupport.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"

class CCharacterFactoryBuilder;
class CGameState;
class CMemoryCard;
class CInGameTweakManager;

CResFactory* gpResourceFactory;
CSimplePool* gpSimplePool;
CCharacterFactoryBuilder* gpCharacterFactoryBuilder;
CStringTable* gpStringTable;
CMain* gpMain;
unkptr gpController;
CGameState* gpGameState;
CMemoryCard* gpMemoryCard;
CInGameTweakManager* gpTweakManager;
float sInfiniteLoopTime;

static uchar sMainSpace[sizeof(CMain)];

CMain::CMain()
// : x0_osContext(true, true)
// , x6c_unk(this)
// , x6d_memorySys(x0_osContext, CMemorySys::GetGameAllocator())
// , xe8_(0.0)
// , x118_(0.f)
// , x11c_(0.f)
// , x120_(0.f)
// , x124_(0.f)
// , x128_gameGlobalObjects(nullptr)
// , x12c_restartMode(kRM_Default)
// , x130_frameTimes(0xF4240)
// , x15c_frameTimeIdx(0)
// , x160_24_finished(false)
// , x160_25_mfGameBuilt(false)
// , x160_26_screenFading(false)
// , x160_27_(false)
// , x160_28_manageCard(false)
// , x160_29_(false)
// , x160_30_(false)
// , x160_31_cardBusy(false)
// , x161_24_gameFrameDrawn(false)
// , x164_(nullptr)
{
  gpMain = this;
}

int main(int argc, char** argv) {
  DVDSetAutoFatalMessaging(TRUE);
  SetErrorHandlers();
  CMain* main = new (&sMainSpace) CMain();
  gpMain->RsMain(argc, argv);
  main->~CMain();
  return 0;
}

CMain::~CMain() {}

void CMain::InitializeSubsystems() {}

void CMain::ShutdownSubsystems() {}

CGameGlobalObjects::CGameGlobalObjects(COsContext& osContext, CMemorySys& memorySys)
: simplePool(resFactory) {}

void CGameGlobalObjects::PostInitialize(COsContext&, CMemorySys&) {}

void CGameGlobalObjects::LoadStringTable() {
  stringTable = gpSimplePool->GetObj("STRG_Main");
  gpStringTable = **stringTable;
}

void InfiniteLoopAlarm(OSAlarm* alarm, OSContext* context) {
  if (sInfiniteLoopTime >= 10.f) {
    rs_debugger_printf("INFINITE LOOP");
  }
  sInfiniteLoopTime += alarm->period / OS_TIMER_CLOCK;
}

CGameArchitectureSupport::CGameArchitectureSupport(COsContext& osContext)
: x0_audioSys(0x30, 0x30, 0x30, 0x30, 0x5fc000)
// , x30_inputGenerator(&osContext, gpTweakPlayer->GetLeftAnalogMax(),
//                      gpTweakPlayer->GetRightAnalogMax())
// , x44_guiSys(gpResourceFactory, gpSimplePool, CGuiSys::kUM_Zero)
, x78_gameFrameCount(0)
, x7c_(0.f)
, x80_(0.f)
, x84_(0.f)
, x88_(2)
, xc8_infiniteLoopAlarmSet(false) {
  CAudioSys::SysSetVolume(0x7F, 0, 0xFF);
  CAudioSys::SetDefaultVolumeScale(0x75);
  CAudioSys::SetVolumeScale(CAudioSys::GetDefaultVolumeScale());
  // CDSPStreamManager::Initialize();
  // CStreamAudioManager::SetMusicVolume(0x7F);
  CAudioSys::TrkSetSampleRate(kTSR_One);
  gpMain->SetMaxSpeed(false);
  gpMain->ResetGameState();
  // if (!gpTweakGame->GetSplashScreensDisabled()) {
  //   x58_ioWinMgr.AddIOWin(new CSplashScreen(CSplashScreen::Nintendo), 1000, 10000);
  // }
  // x58_ioWinMgr.AddIOWin(new CMainFlow(), 0, 0);
  // x58_ioWinMgr.AddIOWin(new CConsoleOutputWindow(8, 5.f, 0.75f), 100, 0);
  // x58_ioWinMgr.AddIOWin(new CAudioStateWin(), 100, -1);
  // x58_ioWinMgr.AddIOWin(new CErrorOutputWindow(false), 10000, 100000);
  // InitializeApplicationUI(x44_guiSys);
  // CGuiSys::SetGlobalGuiSys(&x44_guiSys);
  // gpController = x30_inputGenerator.GetController();
  // gpGameState->GameOptions().EnsureOptions();
  sInfiniteLoopTime = 0.f;
  OSSetPeriodicAlarm(&xa0_infiniteLoopAlarm, OSGetTime(), (float)OS_TIMER_CLOCK, InfiniteLoopAlarm);
  xc8_infiniteLoopAlarmSet = true;
}

CGameArchitectureSupport::~CGameArchitectureSupport() {
  if (xc8_infiniteLoopAlarmSet) {
    OSCancelAlarm(&xa0_infiniteLoopAlarm);
    xc8_infiniteLoopAlarmSet = false;
  }
  x58_ioWinMgr.RemoveAllIOWins();
  UnloadAudio();
  // CSfxManager::Shutdown();
  // CDSPStreamManager::Shutdown();
}

bool CGameArchitectureSupport::UpdateTicks() {}

void CGameArchitectureSupport::Update() {}

void CMain::MemoryCardInitializePump() {}

void CGameGlobalObjects::AddPaksAndFactories() {}

void CMain::DrawDebugMetrics(double dt, CStopwatch& stopWatch) {}

bool CMain::CheckTerminate() {
  return false;
}

extern "C" void fn_800070A4() {}

extern "C" void fn_80007040() {}

bool CMain::CheckReset() {}

void CMain::FillInAssetIDs() {
  gpSimplePool->fn_8029c7e8(*gpResourceFactory->GetResourceIdByName("sound_lookup_ATBL"));
}

int CMain::RsMain(int argc, const char* const* argv) {}

void CMain::AsyncIdle(uint time) {
  if (time < 500) {
    uint total = 0;
    for (int i = 0; i < frameTimes.capacity(); ++i) {
      total += frameTimes[i];
    }
    if (total < 500 * frameTimes.capacity()) {
      time = 500;
    } else {
      time = 0;
    }
  }
  frameTimes[frameTimeIdx] = time;
  frameTimeIdx = frameTimeIdx + 1;
  if (frameTimeIdx >= frameTimes.capacity()) {
    frameTimeIdx = 0;
  }

  time = (time <= 5000) ? time : 5000;
  if (time < frameTimeMinimum) {
    time = frameTimeMinimum;
  }
  frameTimeMinimum = 0;
  bool flag = fn_80008A1C();
  if (flag) {
    time = 1000000;
  }

  if (time != 0) {
    gpResourceFactory->AsyncIdle(time, flag);
  }
}

void CMain::AddWorldPaks() {
  rstl::string basePath = gpTweakGame->GetPakFile();
  for (int i = 0; i < 16; ++i) {
    rstl::string pak =
        basePath + (i == 0 ? rstl::string_l("") : rstl::string(CBasics::Stringize("%d", i)));
    if (CDvdFile::FileExists((pak + rstl::string_l(".pak")).data())) {
      gpResourceFactory->GetResLoader().AddPakFileAsync(pak, false, true);
    }
  }
}

void CMain::EnsureWorldPaksReady() {
  CResLoader& resLoader = gpResourceFactory->GetResLoader();
  for (int i = 0; i < resLoader.GetPakCount(); ++i) {
    CPakFile& file = resLoader.GetPakFile(i);
    if (file.IsWorldPak()) {
      file.EnsureWorldPakReady();
    }
  }
}

void CMain::StreamNewGameState(CInputStream& in, int saveIdx) {
  // TODO
  gameGlobalObjects->GameState() = nullptr;
  gpGameState = nullptr;
  gameGlobalObjects->GameState() = new CGameState(in, saveIdx);
  gpGameState = gameGlobalObjects->GameState().get();
  // gpGameState->HintOptions().SetHintNextTime();
}
