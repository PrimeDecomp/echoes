#ifndef _CDSPSTREAMMANAGER
#define _CDSPSTREAMMANAGER

#include "types.h"

#include "rstl/string.hpp"

class CDSPStreamManager {
public:
  static void Initialize();
  static void Shutdown();

  static int StartStreaming(const rstl::string& fileName, int volume, int oneshot);
  static void StopStreaming(int handle);
  static void UpdateVolume(int handle, int volume);
  static bool IsStreamAvailable(int handle);
  static bool CanStop(int handle);
  static int GetStreamState(int handle);
};

#endif // _CDSPSTREAMMANAGER
