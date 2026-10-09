#ifndef _CCIRCULARBUFFER
#define _CCIRCULARBUFFER

#include <rstl/auto_ptr.hpp>
#include <types.h>

class CCircularBuffer {
public:
  enum EOwnership { kOS_Owned, kOS_NotOwned };

  CCircularBuffer(void* buf, int len, EOwnership owned = kOS_NotOwned);
  bool IsWrappedMemory(int offset, int len);
  void* Alloc(int len);
  void Free(void* ptr, int len);
  int GetAllocatedAmount() const;
  void* GetOffsettedMemory(const int offset) { return mPtr.get() + offset; }

private:
  rstl::auto_ptr< char > mPtr;
  int mMemorySize;
  int xc_;
  int mNextFreeAddr;
  int x14_;
};

#endif // _CCIRCULARBUFFER
