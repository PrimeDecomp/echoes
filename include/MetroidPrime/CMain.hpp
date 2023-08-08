#ifndef _CMAIN
#define _CMAIN

#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/reserved_vector.hpp"

class CStopwatch;
class CGameGlobalObjects;
class CMemorySys;

class CMain {
public:
  enum ERestartMode {
    kRM_None,
    kRM_WinBad,
    kRM_WinGood,
    kRM_WinBest,
    kRM_LoseGame,
    kRM_Default,
    kRM_StateSetter,
    kRM_PreFrontEnd,
    kRM_FrontEnd,
    kRM_Game,
    kRM_GameExit,
  };

  CMain();
  ~CMain();

  bool LoadAudio();
  void UpdateStreamedAudio();
  void RegisterResourceTweaks();
  void ResetGameState();
  void StreamNewGameState(CInputStream& in, int saveIdx);
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
  void SetRestartMode(ERestartMode s) { restartMode = s; }
  ERestartMode GetRestartMode() const { return restartMode; }
  // void SetCardBusy(bool v) { x160_31_cardBusy = v; }

  void SetMaxSpeed(bool v); // {x160_26_screenFading = v; }

  bool fn_80008A1C();

  // void SetX30(bool v) { x160_30_ = v; }

  static void EnsureWorldPaksReady();
  static void EnsureWorldPakReady(CAssetId id);

  void Increment_x5c(float f) { x5c + f; }
  bool GetFinished() const { return finished; }

  // // TODO
  // COsContext& InitOsContext() {
  //   OpenWindow();
  //   return x0_osContext;
  // }

private:
  COsContext* x0_osContext;
  float x4;
  CMemorySys* x8_memorySys;
  char xc_pad[0x3C];
  int frameTimeMinimum;
  char pad2[8];
  CGameGlobalObjects* gameGlobalObjects;
  ERestartMode restartMode;
  float x5c;
  rstl::reserved_vector< uint, 10 > frameTimes;
  int frameTimeIdx;
  bool finished : 1;
  bool mfGameBuilt : 1;
  bool screenFading : 1;
  // bool x160_27_ : 1;
  // bool x160_28_manageCard : 1;
  // bool x160_29_ : 1;
  // bool x160_30_ : 1;
  // bool x160_31_cardBusy : 1;
  // bool x161_24_gameFrameDrawn : 1;
  // CGameArchitectureSupport* x164_;
};

extern CMain* gpMain;

#endif // _CMAIN
