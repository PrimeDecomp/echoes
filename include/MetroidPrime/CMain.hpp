#ifndef _CMAIN
#define _CMAIN

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TReservedAverage.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/reserved_vector.hpp"

class CStopwatch;
class CGameGlobalObjects;
class CMemorySys;
class CDvdRequestSys;
class CSaveRegion;
class COsContext;
class CGameArchitectureSupport;

class CMain {
public:
  enum ERestartMode {
    // Echoes values; 1-5 end the game (names inferred from CMainFlow's handling)
    kRM_None,
    kRM_Credits1,
    kRM_Credits2,
    kRM_EndMovie1,
    kRM_EndAutoSave,
    kRM_EndMovie2,
    kRM_Default,
    kRM_StateSetter,
  };

  CMain(COsContext* context, CSaveRegion* saveRegion, CMemorySys* memorySys,
        CDvdRequestSys* dvdRequestSys);
  ~CMain();

  bool LoadAudio();
  void UpdateStreamedAudio(); // Name inferred from Prime's stream-audio update method.
  void RegisterResourceTweaks();
  void ResetGameState();
  void StreamNewGameState(bool);
  int GetLanguage() const; // Guessed name
  void RefreshGameState();
  void AddWorldPaks();
  void AsyncIdle(uint time);
  int RsMain(int argc, const char* const* argv);
  void InitializeSubsystems();
  void FillInAssetIDs();
  void ShutdownSubsystems();
  void MemoryCardInitializePump();
  void DoPredrawMetrics();
  void DrawDebugMetrics(double dt, CStopwatch& stopWatch);
  bool CheckTerminate();
  bool CheckReset();
  void OpenWindow();
  void SetRestartMode(ERestartMode mode) { mRestartMode = mode; }
  ERestartMode GetRestartMode() const { return mRestartMode; }

  void SetMaxSpeed(bool enabled);

  bool IsMaxSpeed();

  void SetGameExitReset(bool reset) { mGameExitReset = reset; }
  void SetManageCard(bool manage) { mManageCard = manage; }
  void SetGameFrameDrawn(bool drawn) { mGameFrameDrawn = drawn; }
  void SetGameFlowBuilt(bool built) { mMfGameBuilt = built; }
  // Guessed names; the native flag forces two ticks and a 30-FPS frame wait.
  void SetThirtyFps(bool enabled);
  bool GetThirtyFps() const { return mThirtyFps; }
  // Guessed name; minimum asynchronous resource budget for the next frame.
  void SetFrameTimeMinimum(uint time);

  static void EnsureWorldPaksReady();
  static void EnsureWorldPakReady(CAssetId id);

  void DecrementMaxSpeedDrawTimer(float dt) { mMaxSpeedDrawTimer -= dt; }
  bool GetFinished() const { return mFinished; }
  float GetAverageTickTime() const { return mAverageTickTime; }
  float GetAverageDrawTime() const { return mAverageDrawTime; }

private:
  COsContext* mOsContext;
  CSaveRegion* mSaveRegion;
  CMemorySys* mMemorySys;
  CDvdRequestSys* mDvdRequestSys;
  double x10_;
  TReservedAverage< float, 4 > mTickTimes;
  TReservedAverage< float, 4 > mDrawTimes;
  float mAverageTickTime;
  float mAverageDrawTime;
  uint mFrameTimeMinimum;
  float mSoftResetHoldTime;
  float mResetInputDelay;
  CGameGlobalObjects* mGameGlobalObjects;
  ERestartMode mRestartMode;
  float mMaxSpeedDrawTimer; // Guessed name.
  rstl::reserved_vector< uint, 10 > mFrameTimes;
  int mFrameTimeIdx;
  bool mFinished : 1;
  bool mMfGameBuilt : 1; // Inherited name; no semantic use identified in this TU.
  bool mIsMaxSpeed : 1;  // Guessed name: cinematic-skip fast-forward.
  bool mResetButtonHeld : 1;
  bool mManageCard : 1;
  bool mResetRequested : 1;
  bool mGameExitReset : 1;
  bool mGameFrameDrawn : 1;
  bool mThirtyFps : 1;
  CGameArchitectureSupport* mArchSupport;
};
CHECK_SIZEOF(CMain, 0x98)

extern CMain* gpMain;

#endif // _CMAIN
