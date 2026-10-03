#include "Kyoto/Graphics/CGXTransientBuffer.hpp"

#include "Kyoto/Alloc/CCircularBuffer.hpp"

#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"

#include "dolphin/gx.h"
#include "dolphin/os.h"

namespace {
// Guessed record name; pointer, byte count, and submitted draw-sync token are target-derived.
struct SAllocation {
  void* mPointer;
  int mSize;
  ushort mToken;

  SAllocation(void* pointer, int size, ushort token)
  : mPointer(pointer), mSize(size), mToken(token) {}
};
CHECK_SIZEOF(SAllocation, 0xc)

static void* sBufferBase;
static ushort sCurrentToken;
static bool sInitialized;
static bool sDumpedSpinLockMessage;
static bool sAllocationActive;
// Guessed name: the only known native use of this word is SetBuffer resetting it to zero.
static int sBufferResetState;
static int sBufferSize = 0x80000;
static rstl::optional_object< CCircularBuffer > sBuffer;
static rstl::list< SAllocation > sAllocations;
} // namespace

void CGXTransientBuffer::SetBuffer(void* buffer, uint size) {
  sBufferSize = size;
  sAllocations.clear();
  sBufferBase = buffer;
  sBufferResetState = 0;
  if (buffer != nullptr) {
    sBuffer = CCircularBuffer(buffer, sBufferSize, CCircularBuffer::kOS_NotOwned);
  }
}

void* CGXTransientBuffer::EnsureAllocation(int size) {
  if (!sInitialized) {
    GXSetDrawSync(0xffff);
    while (GXReadDrawSync() != 0xffff) {}
    sCurrentToken = 1;
    sInitialized = true;
  }

  sAllocationActive = true;
  TickAllocations();
  int alignedSize = (size + 31) & ~31u;
  void* data = sBuffer->Alloc(alignedSize);
  if (data == nullptr && !sDumpedSpinLockMessage) {
    sDumpedSpinLockMessage = true;
  }

  s32 startTick = OSGetTick();
  while (data == nullptr) {
    TickAllocations();
    data = sBuffer->Alloc(alignedSize);
    if (data == nullptr) {
      s32 currentTick = OSGetTick();
      if (OSTicksToMilliseconds(static_cast< uint >(currentTick - startTick)) > 60) {
        ushort token = GXReadDrawSync();
        for (rstl::list< SAllocation >::iterator it = sAllocations.begin();
             it != sAllocations.end(); ++it) {}
        sCurrentToken = token;
        startTick = currentTick;
        GXSetDrawSync(sCurrentToken);
        sAllocations.clear();
      }
    }
  }

  sAllocations.push_back(SAllocation(data, alignedSize, sCurrentToken));
  if (data == sBufferBase) {
    GXInvalidateVtxCache();
  }
  return data;
}

void CGXTransientBuffer::ReleaseAllocation() {
  sAllocationActive = false;
  GXSetDrawSync(sCurrentToken);
  ++sCurrentToken;
}

void CGXTransientBuffer::TickAllocations() {
  do {
    int syncVal = GXReadDrawSync();
    if (syncVal > static_cast< int >(sCurrentToken)) {
      syncVal -= 0x10000;
    }
    while (sAllocations.size() != 0) {
      SAllocation& front = sAllocations.front();
      int tokenVal = static_cast< int >(front.mToken);
      if (tokenVal > static_cast< int >(sCurrentToken)) {
        tokenVal -= 0x10000;
      }
      if (syncVal < tokenVal) {
        break;
      }
      sBuffer->Free(front.mPointer, front.mSize);
      sAllocations.pop_front();
    }
  } while (sAllocations.size() > 150);
}
