#include "MetroidPrime/CMain.hpp"

#include "Kyoto/Alloc/LockedCache.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CSaveRegion.hpp"
#include "MetroidPrime/CScriptMailbox.hpp"
#include "MetroidPrime/CWorldLayerState.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"

#include "Kyoto/Audio/CDSPStreamManager.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"
#include "Kyoto/CARAMManager.hpp"
#include "Kyoto/CARAMToken.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/CPakFile.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleSpawnSystemDataFactory.hpp"
#include "Kyoto/Particles/CSortedParticleSystemDataFactory.hpp"
#include "Kyoto/Streams/CBitStreamReader.hpp"
#include "Kyoto/Streams/CBitStreamWriter.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Streams/CMemoryStreamOut.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CDecalManager.hpp"
#include "MetroidPrime/CInGameTweakManager.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CRELFileToken.hpp"
#include "MetroidPrime/CSplashScreen.hpp"
#include "MetroidPrime/Player/CWorldTransManager.hpp"
#include "dolphin/ai.h"
#include "dolphin/ar.h"
#include "dolphin/arq.h"
#include "dolphin/base/PPCArch.h"
#include "dolphin/dvd.h"
#include "dolphin/os/OSCache.h"
#include "dolphin/os/OSMemory.h"
#include "dolphin/pad.h"
#include "dolphin/vi.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "MetaRender/CCubeRenderer.hpp"
#include "MetaRender/IRenderer.hpp"

#include "MetroidPrime/CAudioStateWin.hpp"
#include "MetroidPrime/CConsoleOutputWindow.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CErrorOutputWindow.hpp"
#include "MetroidPrime/CGameArchitectureSupport.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/CMainFlow.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Player/CWorldState.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

CFactoryFnReturn FModelFactory(const SObjectTag&, const rstl::auto_ptr< uchar >&, int,
                               const CVParamTransfer&);
CFactoryFnReturn FTextureFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FSkinRulesFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn AnimSourceFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FCharLayoutInfo(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FAnimCharacterSet(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FCollisionResponseDataFactory(const SObjectTag&, CInputStream&,
                                               const CVParamTransfer&);
CFactoryFnReturn FParticleSwooshDataFactory(const SObjectTag&, CInputStream&,
                                            const CVParamTransfer&);
CFactoryFnReturn FParticleFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FParticleElectricDataFactory(const SObjectTag&, CInputStream&,
                                              const CVParamTransfer&);
CFactoryFnReturn FProjectileWeaponDataFactory(const SObjectTag&, CInputStream&,
                                              const CVParamTransfer&);
CFactoryFnReturn RGuiFrameFactoryInGame(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FRasterFontFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FScannableObjectInfoFactory(const SObjectTag&, CInputStream&,
                                             const CVParamTransfer&);
CFactoryFnReturn FAiFiniteStateMachineFactory(const SObjectTag&, CInputStream&,
                                              const CVParamTransfer&);
CFactoryFnReturn FAiStateMachine2Factory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FAudioGroupSetLocDataFactory(const SObjectTag&, const rstl::auto_ptr< uchar >&,
                                              int, const CVParamTransfer&);
CFactoryFnReturn FCollidableOBBTreeGroupFactory(const SObjectTag&, CInputStream&,
                                                const CVParamTransfer&);
CFactoryFnReturn FDecalDataFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FAudioTranslationTableFactory(const SObjectTag&, CInputStream&,
                                               const CVParamTransfer&);
CFactoryFnReturn FPathFindAreaFactory(const SObjectTag&, const rstl::auto_ptr< uchar >&, int,
                                      const CVParamTransfer&);
CFactoryFnReturn FMapWorldFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FMapAreaFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FMapUniverseFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FMidiDataFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FSaveWorldFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FHintFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FSpatialPrimitivesFactory(const SObjectTag&, CInputStream&,
                                           const CVParamTransfer&);
CFactoryFnReturn FPortalAreaDataFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FStringListFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
CFactoryFnReturn FEditorGeometryToStaticGeometryFactory(const SObjectTag&, CInputStream&,
                                                        const CVParamTransfer&);

class CCharacterFactoryBuilder;
class CGameState;
class CMemoryCard;
class CInGameTweakManager;

extern "C" BOOL __PADDisableRecalibration(BOOL);
extern "C" void OSSetSaveRegion(void*, void*);
extern "C" void sndQuit();
extern "C" BOOL DVDCheckDisk();
IRenderer* AllocateRenderer(IObjectStore& store, COsContext& context, CMemorySys& memorySys,
                            IFactory& factory);

CResFactory* gpResourceFactory;
CSimplePool* gpSimplePool;
CCharacterFactoryBuilder* gpCharacterFactoryBuilder;
CStringTable* gpStringTable;
CMain* gpMain;
CGameState* gpGameState;
CMemoryCard* gpMemoryCard;
CInGameTweakManager* gpTweakManager;
float sInfiniteLoopTime;

// Guessed name. Four audio buffers, each decoded from 0x8f00 ADPCM bytes.
uint gARAMAllocationSize = (0x8f00 * 28 / 8) * 4;
CIOWinManager* gpIOWinManager;
CRELFileManager* gpRelFileManager;
extern bool sProgressiveModePrompt; // Prime-correlated name; shared with CSplashScreen.

#define UNUSED_STACK_VAL 0x7337D00D

static uchar sMainSpace[sizeof(CMain)];
static u32 sARAMMemArray[3];

bool CMain::IsMaxSpeed() { return mIsMaxSpeed; }

void CMain::SetMaxSpeed(const bool enabled) {
  if (enabled && !mIsMaxSpeed) {
    CFrameDelayedKiller::StallAndFlushAllAllocations();
  }
  mMaxSpeedDrawTimer = 1.f;
  mIsMaxSpeed = enabled;
}

void CMain::SetThirtyFps(bool enabled) { mThirtyFps = enabled; }

CMain::CMain(COsContext* context, CSaveRegion* saveRegion, CMemorySys* memorySys,
             CDvdRequestSys* dvdRequestSys)
: mOsContext(context)
, mSaveRegion(saveRegion)
, mMemorySys(memorySys)
, mDvdRequestSys(dvdRequestSys)
, x10_(0.0)
, mAverageTickTime(0.f)
, mAverageDrawTime(0.f)
, mSoftResetHoldTime(0.f)
, mResetInputDelay(0.f)
, mGameGlobalObjects(nullptr)
, mRestartMode(kRM_Default)
, mMaxSpeedDrawTimer(1.0f)
, mFrameTimes(0xF4240)
, mFrameTimeIdx(0)
, mFinished(false)
, mMfGameBuilt(false)
, mIsMaxSpeed(false)
, mResetButtonHeld(false)
, mManageCard(false)
, mResetRequested(false)
, mGameExitReset(false)
, mGameFrameDrawn(false)
, mArchSupport(nullptr) {
  gpMain = this;
}

extern "C" void InvokeCMain(int argc, char** argv, COsContext* context, CSaveRegion* saveRegion,
                            CMemorySys* memorySys, CDvdRequestSys* dvdRequestSys) {
  CMain* main = new (&sMainSpace) CMain(context, saveRegion, memorySys, dvdRequestSys);
  main->RsMain(argc, argv);
  main->~CMain();
}

CMain::~CMain() {}

void CMain::InitializeSubsystems() {
  ARInit(sARAMMemArray, 3);
  ARAlloc(gARAMAllocationSize);
  CARAMManager::PreInitializeAlloc(gARAMAllocationSize);
  ARQInit();

  OSThread* thread = OSGetCurrentThread();
  printf("Protecting stack...  ");
  uchar* stackEnd =
      reinterpret_cast< uchar* >((reinterpret_cast< uint >(thread->stackEnd) + 0x3ff) & ~0x3ff);
  uchar* stackBase = thread->stackBase;
  OSProtectRange(OS_PROTECT_CHAN3, stackEnd, 0x400, OS_PROTECT_CONTROL_NONE);
  for (uchar* ptr = stackEnd + 0x400; ptr < stackBase - 0x2000; ptr += sizeof(uint)) {
    *reinterpret_cast< uint* >(ptr) = UNUSED_STACK_VAL;
  }
  DCFlushRange(stackEnd + 0x400, stackBase - 0x2000 - (stackEnd + 0x400));
  printf("Stack: 0x%8.8x down to 0x%8.8x\n", thread->stackBase, thread->stackEnd);

  CElementGen::Initialize();
  CAnimData::InitializeCache();
  CARAMManager::Initialize(0x800, 0x600000, 0x1000);
  CDecalManager::Initialize();
  CDamageVulnerability::Initialize();
  CFrameDelayedKiller::Initialize();
}

void CMain::ShutdownSubsystems() {
  CFrameDelayedKiller::ShutDown();
  CDecalManager::ShutDown();
  CElementGen::ShutDown();
  CAnimData::FreeCache();
  {
    CRELFileToken tweaks(rstl::string_l("Tweaks.rel"), 1);
    tweaks.Load();
    while (!tweaks.IsLoaded()) {
      gpRelFileManager->Update();
    }
    FreeTweaks();
    tweaks.Unload();
  }
  gpRelFileManager->WaitForAllFiles();
  CDamageVulnerability::Shutdown();

  OSThread* thread = OSGetCurrentThread();
  uchar* stackEnd =
      reinterpret_cast< uchar* >((reinterpret_cast< uint >(thread->stackEnd) + 0x3ff) & ~0x3ff);
  uchar* ptr = stackEnd + 0x400;
  for (; ptr < thread->stackBase - 0x2000; ptr += sizeof(uint)) {
    if (*reinterpret_cast< uint* >(ptr) != UNUSED_STACK_VAL) {
      break;
    }
  }
  const int used = thread->stackBase - ptr;
  OSReport("Stack usage: %d bytes (%dk)\n", used, static_cast< uint >(used) / 1024);
}

CGameGlobalObjects::CGameGlobalObjects(COsContext& context, CMemorySys& memorySys)
: mSimplePool(mResFactory)
, mGameState(rs_new CGameState())
, mInGameTweakManager(rs_new CInGameTweakManager()) {
  gpResourceFactory = &mResFactory;
  gpSimplePool = &mSimplePool;
  gpCharacterFactoryBuilder = &mCharacterFactoryBuilder;
  gpGameState = mGameState.get();
  gpTweakManager = mInGameTweakManager.get();
  gpRelFileManager = &mRelFileManager;
}

void CGameGlobalObjects::PostInitialize(COsContext& context, CMemorySys& memorySys) {
  AddPaksAndFactories(context);
  LoadStringTable();
  printf("Initializing renderer...\n");
  mRenderer = AllocateRenderer(mSimplePool, context, memorySys, mResFactory);
  gpRender = static_cast< CCubeRenderer* >(mRenderer.get());
  CEnvFxManager::Initialize();
}

void CGameGlobalObjects::LoadStringTable() {
  mStringTable = gpSimplePool->GetObj("STRG_Main");
  gpStringTable = **mStringTable;
}

void InfiniteLoopAlarm(OSAlarm* alarm, OSContext* context) {
  if (sInfiniteLoopTime >= 10.f) {
    OSCancelAlarm(alarm);
    rs_debugger_printf("INFINITE LOOP");
  }
  sInfiniteLoopTime += alarm->period / OS_TIMER_CLOCK;
}

CGameArchitectureSupport::CGameArchitectureSupport(COsContext& osContext)
: mAudioSys(0x30, 0x30, 0x30, 0x30, gARAMAllocationSize)
, mInputGenerator(&osContext, gpTweakPlayerA->GetLeftAnalogMax(),
                  gpTweakPlayerA->GetRightAnalogMax())
, mGameFrameCount(0)
, mTickRemainder(0.f)
, mPreviousTickRemainder2(0.f)
, mPreviousTickRemainder(0.f)
, mInfiniteLoopAlarmSet(false) {
  CAudioSys::SysSetVolume(0x7F, 0, 0xFF);
  CAudioSys::SetDefaultVolumeScale(0x75);
  CAudioSys::SetVolumeScale(CAudioSys::GetDefaultVolumeScale());
  CSfxManager::Initialize();
  CDSPStreamManager::Initialize();
  CStreamAudioManager::SetMusicVolume(0x7F);
  CAudioSys::TrkSetSampleRate(kTSR_One);
  gpMain->SetMaxSpeed(false);
  gpMain->ResetGameState();
  gpController = mInputGenerator.GetController();
  gpIOWinManager = &mIoWinMgr;
  mIoWinMgr.AddIOWin(rs_new CMainFlow(), 0, 0);
  mIoWinMgr.AddIOWin(rs_new CConsoleOutputWindow(8, 5.f, 0.75f), 100, 0);
  mIoWinMgr.AddIOWin(rs_new CAudioStateWin(), 100, -1);
  mIoWinMgr.AddIOWin(rs_new CErrorOutputWindow(CErrorOutputWindow::kF_Zero), 10000, 100000);
  gpGameState->GameOptions().EnsureOptions();
  sInfiniteLoopTime = 0.f;
  OSSetPeriodicAlarm(&mInfiniteLoopAlarm, OSGetTime(), static_cast< float >(OS_TIMER_CLOCK),
                     InfiniteLoopAlarm);
  mInfiniteLoopAlarmSet = true;
}

CGameArchitectureSupport::~CGameArchitectureSupport() {
  if (mInfiniteLoopAlarmSet) {
    OSCancelAlarm(&mInfiniteLoopAlarm);
    mInfiniteLoopAlarmSet = false;
  }
  mIoWinMgr.RemoveAllIOWins();
  gpIOWinManager = nullptr;
  CSfxManager::Shutdown();
  CDSPStreamManager::Shutdown();
}

bool CGameArchitectureSupport::UpdateTicks() {
  bool result = false;
  const BOOL interrupts = OSDisableInterrupts();
  float stopwatchTime = mTickStopwatch.GetElapsedTime();
  mTickStopwatch.Reset();
  OSRestoreInterrupts(interrupts);
  sInfiniteLoopTime = 0.0f;
  mTickRemainder += stopwatchTime;
  if (gpMain->GetThirtyFps()) {
    mTickRemainder = 1.f / 30.f;
  }
  const bool maxSpeed = gpMain->IsMaxSpeed();
  if (maxSpeed || stopwatchTime > 0.035f) {
    gpMain->DecrementMaxSpeedDrawTimer(stopwatchTime);
    mTickRemainder = 1.f / 60.f;
  }
  bool keepLooping = true;
  mArchQueue.Push(MakeMsg::CreateFrameBegin(kAMT_Game, mGameFrameCount));

  while (keepLooping || mTickRemainder >= 1.f / 60.f) {
    keepLooping = false;
    if (!mInputGenerator.Update(1.f / 60.f, mArchQueue)) {
      result = true;
    }
    mArchQueue.Push(MakeMsg::CreateTimerTick(kAMT_Game, 1.f / 60.f));
    mTickRemainder -= 1.f / 60.f;
    mIoWinMgr.PumpMessages(mArchQueue);
  }

  if (close_enough((mPreviousTickRemainder2 - mPreviousTickRemainder) +
                       (mPreviousTickRemainder - mTickRemainder),
                   0.f, 0.00005f)) {
    mTickRemainder = 0.0f;
  }

  mPreviousTickRemainder2 = mPreviousTickRemainder;
  mPreviousTickRemainder = mTickRemainder;
  mIoWinMgr.PumpMessages(mArchQueue);
  return !result;
}

void CGameArchitectureSupport::Update() {
  gpGameState->WorldTransitionManager()->TouchModels();
  mArchQueue.Push(MakeMsg::CreateFrameEnd(kAMT_Game, mGameFrameCount));
  mIoWinMgr.PumpMessages(mArchQueue);
}

void CMain::MemoryCardInitializePump() {
  if (gpMemoryCard != nullptr) {
    return;
  }
  rstl::single_ptr< CMemoryCard >& card = mGameGlobalObjects->MemoryCard();
  if (card.get() == nullptr) {
    card = rs_new CMemoryCard();
  }
  if (card->InitializePump()) {
    gpMemoryCard = card.get();
    gpGameState->SystemOptions().InitializeMemoryState();
    gpGameState->InitializeMemoryStates();
  }
}

void CGameGlobalObjects::AddPaksAndFactories(COsContext& context) {
  CResFactory& factory = *gpResourceFactory;
  CResLoader& loader = factory.GetResLoader();
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  if (CDvdFile::FileExists("Strings.pak")) {
    loader.AddPakFileAsync(rstl::string_l("aram:Strings"), false, false);
  }

  CDvdFile tweakFile("Standard.NTWK");
  rstl::auto_ptr< uchar > tweakData(
      static_cast< uchar* >(CMemory::Alloc(tweakFile.Length(), IAllocator::kHI_RoundUpLen)));
  rstl::single_ptr< CDvdRequest > request(tweakFile.SyncRead(tweakData.get(), tweakFile.Length()));
  CRELFileToken tweaks(rstl::string_l("Tweaks.rel"), 1);
  tweaks.Load();
  loader.AddPakFileAsync(rstl::string_l("NoARAM"), false, false);
  loader.AddPakFileAsync(rstl::string_l("AudioGrp"), false, false);
  loader.AddPakFileAsync(rstl::string_l("aram:MiscData"), false, false);
  loader.AddPakFileAsync(rstl::string_l("aram:TestAnim"), true, false);
  loader.AddPakFileAsync(rstl::string_l("aram:MidiData"), false, false);
  loader.AddPakFileAsync(rstl::string_l("aram:GGuiSys"), false, false);
  if (CDvdFile::FileExists("FrontEnd.pak")) {
    loader.AddPakFileAsync(rstl::string_l("FrontEnd"), false, true);
  }

  CErrorOutputWindow errors(CErrorOutputWindow::kF_One);
  CGraphics::SetIsBeginSceneClearFb(true);
  CGraphics::SetViewport(0, 0, CGraphics::GetViewport().mWidth, CGraphics::GetViewport().mHeight);
  rstl::single_ptr< IController > controller(IController::Create(context));
  gpController = controller.get();
  while (!loader.AreAllPaksLoaded() || !request->IsComplete() || !tweaks.IsLoaded()) {
    gpResourceFactory->GetResLoader().AsyncIdlePakLoading();
    gpRelFileManager->Update();
    errors.Update();
    CGraphics::BeginScene();
    errors.ShowMessage();
    CGraphics::EndScene();
    if (controller.get() != nullptr) {
      controller->Poll();
    }
    gpMain->CheckReset();
  }
  gpController = nullptr;
  {
    CMemoryInStream stream(tweakData.get(), tweakFile.Length(), CMemoryInStream::kOS_NotOwned);
    LoadTweaks(stream);
    CreateTweakGlobals();
    tweaks.Unload();
  }

  CFactoryMgr& factories = factory.GetFactoryMgr();
  factories.AddFactory('STRG', FStringTableFactory);
  factories.AddFactory('CMDL', FModelFactory);
  factories.AddFactory('TXTR', FTextureFactory);
  factories.AddFactory('CSKR', FSkinRulesFactory);
  factories.AddFactory('ANIM', AnimSourceFactory);
  factories.AddFactory('CINF', FCharLayoutInfo);
  factories.AddFactory('ANCS', FAnimCharacterSet);
  factories.AddFactory('CRSC', FCollisionResponseDataFactory);
  factories.AddFactory('SWHC', FParticleSwooshDataFactory);
  factories.AddFactory('PART', FParticleFactory);
  factories.AddFactory('ELSC', FParticleElectricDataFactory);
  factories.AddFactory('SPSC', FSpawnParticleSystemDataFactory);
  factories.AddFactory('SRSC', FSortedParticleSystemDataFactory);
  factories.AddFactory('WPSC', FProjectileWeaponDataFactory);
  factories.AddFactory('FRME', RGuiFrameFactoryInGame);
  factories.AddFactory('FONT', FRasterFontFactory);
  factories.AddFactory('SCAN', FScannableObjectInfoFactory);
  factories.AddFactory('AFSM', FAiFiniteStateMachineFactory);
  factories.AddFactory('FSM2', FAiStateMachine2Factory);
  factories.AddFactory('AGSC', FAudioGroupSetLocDataFactory);
  factories.AddFactory('DCLN', FCollidableOBBTreeGroupFactory);
  factories.AddFactory('DPSC', FDecalDataFactory);
  factories.AddFactory('ATBL', FAudioTranslationTableFactory);
  factories.AddFactory('PATH', FPathFindAreaFactory);
  factories.AddFactory('MAPW', FMapWorldFactory);
  factories.AddFactory('MAPA', FMapAreaFactory);
  factories.AddFactory('MAPU', FMapUniverseFactory);
  factories.AddFactory('CSNG', FMidiDataFactory);
  factories.AddFactory('DGRP', FDependencyGroupFactory);
  factories.AddFactory('SAVW', FSaveWorldFactory);
  factories.AddFactory('HINT', FHintFactory);
  factories.AddFactory('CSPP', FSpatialPrimitivesFactory);
  factories.AddFactory('PTLA', FPortalAreaDataFactory);
  factories.AddFactory('STLC', FStringListFactory);
  factories.AddFactory('EGMC', FEditorGeometryToStaticGeometryFactory);
  factories.AddFactory('RULE', FRuleSetFactory);
}

void CMain::DrawDebugMetrics(double dt, CStopwatch& stopWatch) {
  static uint frames = 0;
  if (++frames == 1800) {
    frames = 0;
  }
  CMemory::GetMetrics(frames == 0, false);
}

bool CMain::CheckTerminate() { return false; }

bool CMain::CheckReset() {
  const bool resetPressed = OSGetResetButtonState() != 0;
  const CControllerGamepadData& pad = gpController->GetGamepadData(0);
  bool resetChord = true;
  for (int i = 0; i < kBU_MAX && resetChord; ++i) {
    const bool expected = i == kBU_B || i == kBU_X || i == kBU_Start;
    if (pad.GetButton(static_cast< EButton >(i)).GetIsPressed() != expected) {
      resetChord = false;
    }
  }
  if (resetChord) {
    if (mResetInputDelay >= 0.5f) {
      mSoftResetHoldTime += 1.f / 60.f;
      if (mSoftResetHoldTime > 0.5f) {
        mResetButtonHeld = true;
      }
    }
  } else {
    if (mResetInputDelay < 0.5f) {
      mResetInputDelay += 1.f / 60.f;
    }
    mSoftResetHoldTime = 0.f;
  }
  if (!resetPressed && mResetButtonHeld) {
    mResetRequested = true;
  }
  if (CMemoryCardSys::mIsCardBusy || !(mResetRequested || mManageCard || mGameExitReset)) {
    mResetButtonHeld = resetPressed;
    return false;
  }

  if (mArchSupport != nullptr && mArchSupport->IsInfiniteLoopAlarmSet()) {
    OSCancelAlarm(&mArchSupport->GetInfiniteLoopAlarm());
    mArchSupport->SetInfiniteLoopAlarmSet(false);
  }
  GXDrawDone();
  GXAbortFrame();
  if (!mGameExitReset) {
    gpGameState->GameOptions() = CGameOptions();
    gpGameState->PreviousGameResults() = CGameState::SPreviousGameResults();
    __PADDisableRecalibration(false);
  } else {
    CGameOptions& options = gpGameState->GameOptions();
    options.SetScreenBrightness(4, false);
    options.SetScreenPositionX(0, false);
    options.SetScreenPositionY(0, false);
    options.SetScreenStretch(0, false);
    __PADDisableRecalibration(true);
  }
  {
    CMemoryStreamOut stream(CSaveRegion::GetSaveBuffer(), CSaveRegion::kSaveBufferSize);
    CBitStreamWriter writer(stream);
    writer.WriteBits(CGraphics::GetProgressiveMode(), 1);
    gpGameState->GameOptions().PutTo(writer);
    gpGameState->PreviousGameResults().PutTo(writer);
    writer.WriteBits(sProgressiveModePrompt, 1);
    writer.FlushAll();
    if (writer.GetOutputStream().GetWrittenBytes() < CSaveRegion::kSaveBufferSize) {
      OSReport("Wrote: %d", writer.GetOutputStream().GetWrittenBytes());
    } else {
      rs_debugger_printf("Reset failed! Tried %d", stream.GetWrittenBytes());
    }
  }

  gpGameState->GameOptions().EnsureOptions();
  VISetBlack(true);
  VIFlush();
  VIWaitForRetrace();
  if (mManageCard) {
    OSResetSystem(OS_RESET_HOTRESET, 0, true);
  } else if (DVDCheckDisk()) {
    AISetStreamPlayState(0);
    if (CAudioSys::mInitialized) {
      sndQuit();
    }
    void* savedOptions = CSaveRegion::GetSaveRegionStart();
    memcpy(savedOptions, CSaveRegion::GetSaveBuffer(), CSaveRegion::kSaveBufferSize);
    DCFlushRange(savedOptions, CSaveRegion::kSaveBufferSize);
    OSSetSaveRegion(savedOptions, CSaveRegion::GetSaveRegionEnd());
    OSResetSystem(OS_RESET_RESTART, 0, false);
  } else {
    OSResetSystem(OS_RESET_HOTRESET, 0, false);
  }
  mResetButtonHeld = false;
  mResetRequested = false;
  mGameExitReset = false;
  mManageCard = false;
  return true;
}

void CMain::FillInAssetIDs() {
  gpSimplePool->fn_8029c7e8(*gpResourceFactory->GetResourceIdByName("sound_lookup_ATBL"));
}

CGameGlobalObjects::~CGameGlobalObjects() {}

int CMain::RsMain(int argc, const char* const* argv) {
  PPCSetFpIEEEMode();
  CStopwatch startupTimer;
  if (GetLockedCacheAllocationBase() == LCGetBase()) {
    LCEnable();
  }
  rstl::single_ptr< CGameGlobalObjects > globalObjects(
      rs_new CGameGlobalObjects(*mOsContext, *mMemorySys));
  mGameGlobalObjects = globalObjects.get();
  CStringTable::SetLanguage(GetLanguage());
  for (int i = 0; i < 4; ++i) {
    mTickTimes.AddValue(0.3f);
    mDrawTimes.AddValue(0.2f);
  }
  mAverageTickTime = 0.3f;
  mAverageDrawTime = 0.2f;
  InitializeSubsystems();
  globalObjects->PostInitialize(*mOsContext, *mMemorySys);
  AddWorldPaks();

  {
    rstl::string audioTweaksStatus;
    if (gpTweakManager->ReadFromMemoryCard(rstl::string_l("AudioTweaks"))) {
      audioTweaksStatus = rstl::string_l("Loaded audio tweaks from memory card\n");
    } else {
      audioTweaksStatus = rstl::string_l("FAILED to load audio tweaks from memory card\n");
    }
    FillInAssetIDs();
    rstl::single_ptr< CGameArchitectureSupport > architecture(
        rs_new CGameArchitectureSupport(*mOsContext));
    mArchSupport = architecture.get();
    srand(startupTimer.GetElapsedMicros());
    if (CSaveRegion::GetNonVolatileSettingsBuffer() != nullptr) {
      CMemoryInStream stream(CSaveRegion::GetNonVolatileSettingsBuffer(),
                             CSaveRegion::kSaveBufferSize);
      CBitStreamReader reader(stream);
      reader.ReadBits(1);
      gpGameState->GameOptions() = CGameOptions(reader);
      gpGameState->PreviousGameResults() = CGameState::SPreviousGameResults(reader);
      gpGameState->GameOptions().EnsureOptions();
      sProgressiveModePrompt = reader.ReadPackedBool();
    }
    const int gameMode = gpGameState->GetGameModeType();
    if (gameMode != 'COIN' && gameMode != 'DTHM') {
      architecture->GetIOWinManager().AddIOWin(
          rs_new CSplashScreen(CSplashScreen::kSplashScreen_ProgressiveCheck), 1000, 10000);
    }
    CDvdFile::FileExists("Strings.pak");

    while (!mFinished) {
      CStopwatch& drawTimer = architecture->GetStopwatch2();
      drawTimer.Reset();
      gpResourceFactory->GetResLoader().AsyncIdlePakLoading();
      gpRelFileManager->Update();
      if (gpMemoryCard == nullptr && gpResourceFactory->GetResLoader().AreAllPaksLoaded()) {
        MemoryCardInitializePump();
      }
      CARAMManager::CollectGarbage();
      CARAMToken::UpdateAllDMAs();
      if (!architecture->UpdateTicks()) {
        mFinished = true;
      }
      const double tickTime = drawTimer.GetElapsedTime();
      mTickTimes.AddValue(tickTime / (1.f / 60.f));
      mAverageTickTime = *mTickTimes.GetAverage();
      drawTimer.Reset();

      bool drawFrame = true;
      if (IsMaxSpeed()) {
        AsyncIdle(1000000);
        if (mMaxSpeedDrawTimer > 0.f) {
          drawFrame = false;
          CFrameDelayedKiller::FlushAllocationsForFrame();
          CFrameDelayedKiller::FlushAllocationsForFrame();
        } else {
          mMaxSpeedDrawTimer = 1.f;
        }
      }
      if (drawFrame) {
        gpRender->BeginScene();
        architecture->GetIOWinManager().Draw();
        DrawDebugMetrics(tickTime, drawTimer);
        const double drawTime = drawTimer.GetElapsedTime();
        mDrawTimes.AddValue(drawTime / (1.f / 60.f));
        mAverageDrawTime = *mDrawTimes.GetAverage();
        gpRelFileManager->Update();
        const double idleTime = (1.f / 60.f - (tickTime + drawTimer.GetElapsedTime())) - 0.00075;
        AsyncIdle(idleTime <= 0.0 ? 0 : static_cast< uint >(idleTime * 1000000.0));
        if (gpMain->GetThirtyFps()) {
          const float waitTime =
              1.f / 30.f - static_cast< float >(tickTime + drawTimer.GetElapsedTime());
          if (waitTime > 0.f) {
            CStopwatch::Wait(waitTime);
          }
        }
        gpRender->EndScene();
        if (mGameFrameDrawn) {
          ++architecture->GetFramesDrawn();
          mGameFrameDrawn = false;
        }
      } else {
        gpResourceFactory->AsyncIdle(1000000, false);
      }
      architecture->Update();
      CSfxManager::Update(1.f / 60.f);
      UpdateStreamedAudio();
      if (CheckTerminate()) {
        gpGameState->ClearAudioGroups();
        break;
      }
      if (architecture->GetIOWinManager().IsEmpty() || CheckReset()) {
        mRestartMode = kRM_Default;
        CStreamAudioManager::StopAll();
        PADRecalibrate(0xf0000000);
        CGraphics::SetIsBeginSceneClearFb(true);
        CGraphics::BeginScene();
        CGraphics::EndScene();
        CFrameDelayedKiller::StallAndFlushAllAllocations();
        architecture = nullptr;
        architecture = rs_new CGameArchitectureSupport(*mOsContext);
        mArchSupport = architecture.get();
      }
    }
  }

  ShutdownSubsystems();
  globalObjects = nullptr;
  CARAMManager::Shutdown();
  return 0;
}

void CMain::SetFrameTimeMinimum(uint time) { mFrameTimeMinimum = time; }

void CMain::AsyncIdle(uint time) {
  if (time < 500) {
    uint total = 0;
    for (int i = 0; i < mFrameTimes.capacity(); ++i) {
      total += mFrameTimes[i];
    }
    if (total < 500 * mFrameTimes.capacity()) {
      time = 500;
    } else {
      time = 0;
    }
  }
  mFrameTimes[mFrameTimeIdx] = time;
  mFrameTimeIdx = mFrameTimeIdx + 1;
  if (mFrameTimeIdx >= mFrameTimes.capacity()) {
    mFrameTimeIdx = 0;
  }

  time = (time <= 5000) ? time : 5000;
  if (time < mFrameTimeMinimum) {
    time = mFrameTimeMinimum;
  }
  mFrameTimeMinimum = 0;
  bool flag = IsMaxSpeed();
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

void CMain::EnsureWorldPakReady(CAssetId id) {
  CResLoader& loader = gpResourceFactory->GetResLoader();
  for (int i = 0; i < loader.GetPakCount(); ++i) {
    bool otherWorld = true;
    CPakFile& pak = *loader.GetPakFile(i);
    if (!pak.IsWorldPak()) {
      continue;
    }
    const rstl::vector< rstl::pair< rstl::string, SObjectTag > > names =
        pak.GetStringToObjectList();
    for (rstl::vector< rstl::pair< rstl::string, SObjectTag > >::const_iterator it = names.begin();
         it != names.end(); ++it) {
      if (it->second.GetId() == id) {
        otherWorld = false;
      }
    }
    if (otherWorld) {
      pak.sub_80323554();
    } else {
      pak.EnsureWorldPakReady();
    }
  }
}

void CMain::EnsureWorldPaksReady() {
  CResLoader& resLoader = gpResourceFactory->GetResLoader();
  for (int i = 0; i < resLoader.GetPakCount(); ++i) {
    CPakFile& file = *resLoader.GetPakFile(i);
    if (file.IsWorldPak()) {
      file.EnsureWorldPakReady();
    }
  }
}

void CMain::StreamNewGameState(bool forceReloadSave) {
  const CPersistentOptions systemOptions = gpGameState->SystemOptions();
  const u64 cardSerial = gpGameState->GetCardSerial();
  const rstl::reserved_vector< rstl::vector< uchar >, 3 > states =
      gpGameState->GetCompressedGameStates();
  const rstl::vector< uchar > checkpoint = gpGameState->GetCheckpointGameState();
  const bool useCheckpoint = !checkpoint.empty() && !forceReloadSave;
  const rstl::reserved_vector< rstl::vector< uchar >, 3 > options =
      gpGameState->GetCompressedGameOptions();
  const rstl::vector< uchar > multiplayerOptions = gpGameState->GetCompressedMultiplayerOptions();
  const CGameOptions gameOptions = gpGameState->GameOptions();

  mGameGlobalObjects->GameState() = nullptr;
  gpGameState = nullptr;
  {
    const rstl::vector< uchar >& savedState =
        useCheckpoint ? checkpoint : states[systemOptions.GetSaveIdx()];
    CMemoryInStream stream(savedState.data(), savedState.size());
    CBitStreamReader reader(stream);
    mGameGlobalObjects->GameState() = rs_new CGameState(reader);
  }
  gpGameState = mGameGlobalObjects->GameState().get();
  gpGameState->SetSystemOptions(systemOptions);
  gpGameState->SetCompressedGameStates(states);
  gpGameState->SetCompressedGameOptions(options);
  gpGameState->SetCompressedMultiplayerOptions(multiplayerOptions);
  gpGameState->GameOptions() = gameOptions;
  gpGameState->GameOptions().EnsureOptions();
  gpGameState->SetCardSerial(cardSerial);
  if (useCheckpoint) {
    gpGameState->ClearCheckpoint();
  }
}

CPlayerState::~CPlayerState() {}

CPlayerState::SPersistentState::~SPersistentState() {}

CStaticInterference::~CStaticInterference() {}

CGameOptions::~CGameOptions() {}

CWorldState::~CWorldState() {}

CGameState::~CGameState() {}

void CMain::ResetGameState() {
  CPersistentOptions systemOptions = gpGameState->SystemOptions();
  CGameOptions gameOptions = gpGameState->GameOptions();
  rstl::reserved_vector< rstl::vector< uchar >, 3 > compressedGameOptions =
      gpGameState->GetCompressedGameOptions();
  rstl::vector< uchar > compressedMultiplayerOptions =
      gpGameState->GetCompressedMultiplayerOptions();
  CGameState::SPreviousGameResults previousResults = gpGameState->PreviousGameResults();
  mGameGlobalObjects->GameState() = nullptr;
  gpGameState = nullptr;
  mGameGlobalObjects->GameState() = rs_new CGameState();
  gpGameState = mGameGlobalObjects->GameState().get();
  gpGameState->SystemOptions() = systemOptions;
  gpGameState->GameOptions() = gameOptions;
  gpGameState->GameOptions().EnsureOptions();
  gpGameState->SetCompressedGameOptions(compressedGameOptions);
  gpGameState->SetCompressedMultiplayerOptions(compressedMultiplayerOptions);
  gpGameState->PreviousGameResults() = previousResults;
}

int CMain::GetLanguage() const {
  int language = mOsContext->GetLanguage();
  if (language == 5) {
    language = 0;
  }
  return language;
}

void CMain::UpdateStreamedAudio() { CStreamAudioManager::Update(1.f / 60.f); }
