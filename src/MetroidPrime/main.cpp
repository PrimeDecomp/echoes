#include "MetroidPrime/CMain.hpp"

#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"
#include "Kyoto/CPakFile.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "dolphin/ar.h"

#include "MetaRender/IRenderer.hpp"

#include "MetroidPrime/CAudioStateWin.hpp"
#include "MetroidPrime/CConsoleOutputWindow.hpp"
#include "MetroidPrime/CErrorOutputWindow.hpp"
#include "MetroidPrime/CGameArchitectureSupport.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/CMainFlow.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

class CCharacterFactoryBuilder;
class CGameState;
class CMemoryCard;
class CInGameTweakManager;

extern "C" void fn_8029EFCC();
extern "C" void fn_8033CEE8();
extern "C" char* fn_8034C0D8(char *, ...);
IRenderer* AllocateRenderer(IObjectStore& store, COsContext& osContext, CMemorySys& memorySys, IFactory& resFactory);

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

CMain::CMain(COsContext* context, void* unk1, CMemorySys* memorySys, void* unk2)
: osContext(context)
, x4_unk1(unk1)
, memorySys(memorySys)
, xc_unk2(unk2)
// , xe8_(0.0)
// , x118_(0.f)
// , x11c_(0.f)
// , x120_(0.f)
// , x124_(0.f)
, frameTimeMinimum(0)
, x4c(0.0f)
, gameGlobalObjects(nullptr)
, restartMode(kRM_StateSetter)  // value must be 6, TODO if the correct enum
, x5c(1.0f)
, frameTimes(0xF4240)
, frameTimeIdx(0)
, finished(false)
, mfGameBuilt(false)
, screenFading(false)
, x90_27_(false)
, x90_28_manageCard(false)
, x90_29_(false)
, x90_30_(false)
, x90_31_cardBusy(false)
{
  gpMain = this;
}

extern "C" void InvokeCMain(int argc, char** argv, COsContext* context, void* unk1,
                            CMemorySys* memorySys, void* unk2) {
  CMain* main = new (&sMainSpace) CMain(context, unk1, memorySys, unk2);
  main->RsMain(argc, argv);
  main->~CMain();
}

CMain::~CMain() {}

void CMain::InitializeSubsystems() {
  ARInit((u32*) 0x803c5ab8, 3);  // (u32*)(&sMainSpace + 0x98)
  // TODO
}

void CMain::ShutdownSubsystems() {}

CGameGlobalObjects::CGameGlobalObjects(COsContext& osContext, CMemorySys& memorySys)
: simplePool(resFactory) {}

void CGameGlobalObjects::PostInitialize(COsContext& osContext, CMemorySys& memorySys) {
  AddPaksAndFactories();
  LoadStringTable();
  fn_8034C0D8("Initializing renderer...\n");
  renderer = AllocateRenderer(simplePool, osContext, memorySys, resFactory);                            
  CEnvFxManager::Initialize();
}

void CGameGlobalObjects::LoadStringTable() {
  stringTable = gpSimplePool->GetObj("STRG_Main");
  gpStringTable = **stringTable;
}

void InfiniteLoopAlarm(OSAlarm* alarm, OSContext* context) {
  if (sInfiniteLoopTime >= 10.f) {
    OSCancelAlarm(alarm);
    rs_debugger_printf("INFINITE LOOP");
  }
  sInfiniteLoopTime += alarm->period / OS_TIMER_CLOCK;
}

CGameArchitectureSupport::CGameArchitectureSupport(COsContext& osContext)
: audioSys(0x30, 0x30, 0x30, 0x30, 0x5fc000)
, inputGenerator(&osContext, gpTweakPlayerA->GetLeftAnalogMax(),
                 gpTweakPlayerA->GetRightAnalogMax())
, gameFrameCount(0)
, x68_(0.f)
, x6c_(0.f)
, x70_(0.f)
// , x74_(2)
, infiniteLoopAlarmSet(false) {
  CAudioSys::SysSetVolume(0x7F, 0, 0xFF);
  CAudioSys::SetDefaultVolumeScale(0x75);
  CAudioSys::SetVolumeScale(CAudioSys::GetDefaultVolumeScale());
  // CDSPStreamManager::Initialize();
  fn_8029EFCC();
  fn_8033CEE8();
  CStreamAudioManager::SetMusicVolume(0x7F);
  CAudioSys::TrkSetSampleRate(kTSR_One);
  gpMain->SetMaxSpeed(false);
  gpMain->ResetGameState();
  ioWinMgr.AddIOWin(new CMainFlow(), 0, 0);
  ioWinMgr.AddIOWin(new CConsoleOutputWindow(8, 5.f, 0.75f), 100, 0);
  ioWinMgr.AddIOWin(new CAudioStateWin(), 100, -1);
  ioWinMgr.AddIOWin(new CErrorOutputWindow(false), 10000, 100000);
  gpGameState->GameOptions().EnsureOptions();
  sInfiniteLoopTime = 0.f;
  OSSetPeriodicAlarm(&infiniteLoopAlarm, OSGetTime(), (float)OS_TIMER_CLOCK, InfiniteLoopAlarm);
  infiniteLoopAlarmSet = true;
}

CGameArchitectureSupport::~CGameArchitectureSupport() {
  if (infiniteLoopAlarmSet) {
    OSCancelAlarm(&infiniteLoopAlarm);
    infiniteLoopAlarmSet = false;
  }
  ioWinMgr.RemoveAllIOWins();
  UnloadAudio();
  // CSfxManager::Shutdown();
  // CDSPStreamManager::Shutdown();
}

bool CGameArchitectureSupport::UpdateTicks() {
  bool result = false;
  OSDisableInterrupts();
  float stopwatchTime = stopwatch1.GetElapsedTime();
  stopwatch1.Reset();
  OSRestoreInterrupts(1);
  sInfiniteLoopTime = 0.0f;
  x68_ += stopwatchTime;
  if (gpMain->GetFinished()) {
    x68_ = 0.033333335f;
  }
  bool flag = gpMain->fn_80008A1C();
  if (flag || 0.035 < stopwatchTime) {
    gpMain->Increment_x5c(-stopwatchTime);
    x68_ = 0.016666668f;
  }
  archQueue.Push(MakeMsg::CreateFrameBegin(kAMT_Game, gameFrameCount));

  bool keepLooping = true;
  while (keepLooping || x68_ > 0.016666668f) {
    keepLooping = false;
    if (!inputGenerator.Update(0.016666668f, archQueue)) {
      result = true;
    }
    archQueue.Push(MakeMsg::CreateTimerTick(kAMT_Game, 0.016666668f));
    x68_ -= 0.016666668f;
    ioWinMgr.PumpMessages(archQueue);
  }

  if (close_enough((x6c_ - x70_) + (x70_ - x68_), 0.0f)) {
    x68_ = 0.0f;
  }

  x6c_ = x70_;
  x70_ = x68_;
  ioWinMgr.PumpMessages(archQueue);
  return result;
}

void CGameArchitectureSupport::Update() {}

void CMain::MemoryCardInitializePump() {}

void CGameGlobalObjects::AddPaksAndFactories() {}

void CMain::DrawDebugMetrics(double dt, CStopwatch& stopWatch) {}

bool CMain::CheckTerminate() { return false; }

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
