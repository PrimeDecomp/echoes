#ifndef _CSAVEREGION
#define _CSAVEREGION

#include "types.h"

class COsContext;

// Prime-correlated name; the native constructor allocates and restores the save buffer.
class CSaveRegion {
public:
  enum { kSaveBufferSize = 128 };

  explicit CSaveRegion(COsContext& context);

  static void* GetSaveBuffer() { return mSaveBuffer; }

  static const void* GetNonVolatileSettingsBuffer() { return mNonVolatileSettingsBuf; }

  // Guessed names. Echoes reserves the final 128 bytes of its 18-MiB arena for restart.
  static uchar* GetSaveRegionEnd() { return reinterpret_cast< uchar* >(0x81200000); }

  static uchar* GetSaveRegionStart() { return GetSaveRegionEnd() - kSaveBufferSize; }

private:
  static void* mSaveBuffer;
  static const void* mNonVolatileSettingsBuf;
};

#endif // _CSAVEREGION
