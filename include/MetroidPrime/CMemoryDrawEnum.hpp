#ifndef _CMEMORYDRAWENUM
#define _CMEMORYDRAWENUM

#include "types.h"

class CMemoryDrawEnum {
public:
  static void AddWorldMemory(uint size) { mWorldMemory += size; }
  static void SubtractWorldMemory(uint size) { mWorldMemory -= size; }

private:
  static uint mWorldMemory;
};

#endif // _CMEMORYDRAWENUM
