#ifndef _CGAMEARCHITECTURESUPPORT
#define _CGAMEARCHITECTURESUPPORT

#include "types.h"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/TOneStatic.hpp"
#include "CGuiSys/CGuiSys.hpp"

#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/CIOWinManager.hpp"
#include "MetroidPrime/CInputGenerator.hpp"

#include "rstl/vector.hpp"

class CToken;

class CGameArchitectureSupport : public TOneStatic< CGameArchitectureSupport > {
public:
  CGameArchitectureSupport(COsContext&);
  ~CGameArchitectureSupport();

  void PreloadAudio();
  bool UpdateTicks();
  void Update();
  void UnloadAudio();

  inline CStopwatch& GetStopwatch1() { return stopwatch1; }
  inline CStopwatch& GetStopwatch2() { return stopwatch2; }
  inline CIOWinManager& GetIOWinManager() { return ioWinMgr; }
  inline int& GetFramesDrawn() { return gameFrameCount; }

private:
  CAudioSys audioSys;
  CArchitectureQueue archQueue;
  CStopwatch stopwatch1;
  CStopwatch stopwatch2;
  CInputGenerator inputGenerator;
  CIOWinManager ioWinMgr;
  // CGuiSys guiSys;
  int gameFrameCount;
  float x68_;
  float x6c_;
  float x70_;
  uint x74_;
  // rstl::vector< CToken > x90_;
  OSAlarm infiniteLoopAlarm;
  bool infiniteLoopAlarmSet;
};
// CHECK_SIZEOF(CGameArchitectureSupport, 0xd0)

#endif // _CGAMEARCHITECTURESUPPORT
