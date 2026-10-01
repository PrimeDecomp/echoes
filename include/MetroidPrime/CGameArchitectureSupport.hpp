#ifndef _CGAMEARCHITECTURESUPPORT
#define _CGAMEARCHITECTURESUPPORT

#include "types.h"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/TOneStatic.hpp"

#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/CIOWinManager.hpp"
#include "MetroidPrime/CInputGenerator.hpp"

class CGameArchitectureSupport : public TOneStatic< CGameArchitectureSupport > {
public:
  CGameArchitectureSupport(COsContext&);
  ~CGameArchitectureSupport();

  void PreloadAudio();
  bool UpdateTicks();
  void Update();
  void UnloadAudio();

  inline CStopwatch& GetStopwatch1() { return mTickStopwatch; }
  inline CStopwatch& GetStopwatch2() { return mDrawStopwatch; }
  inline CIOWinManager& GetIOWinManager() { return mIoWinMgr; }
  inline int& GetFramesDrawn() { return mGameFrameCount; }
  OSAlarm& GetInfiniteLoopAlarm() { return mInfiniteLoopAlarm; }
  bool IsInfiniteLoopAlarmSet() const { return mInfiniteLoopAlarmSet; }
  void SetInfiniteLoopAlarmSet(bool set) { mInfiniteLoopAlarmSet = set; }

private:
  CAudioSys mAudioSys;
  CArchitectureQueue mArchQueue;
  CStopwatch mTickStopwatch;
  CStopwatch mDrawStopwatch;
  CInputGenerator mInputGenerator;
  CIOWinManager mIoWinMgr;
  int mGameFrameCount;
  float mTickRemainder;
  float mPreviousTickRemainder2;
  float mPreviousTickRemainder;
  OSAlarm mInfiniteLoopAlarm;
  bool mInfiniteLoopAlarmSet;
};
CHECK_SIZEOF(CGameArchitectureSupport, 0xa8)

#endif // _CGAMEARCHITECTURESUPPORT
