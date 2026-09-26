#include "Kyoto/Audio/CAudioGroupSet.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"
#include "Kyoto/CARAMManager.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"

#include "rstl/StringExtras.hpp"
#include "rstl/string.hpp"

#include "dolphin/os.h"
#include "musyx/musyx.h"

#include <string.h>

extern "C" {
u32 sndStackGetSize(void);
u32 sndStackAdd(void* buffer, const void* aramAddr, u32 size);
bool sndStackSetCurrent(u32 id);
void sndStackRemove(u32 id);
}

static uchar* sUploadBuffer;
bool CAudioGrpSetLoc::sInSinglePlayer;

static void* UploadCallback(u32 offset, u32 bytes) {
  memcpy(sUploadBuffer, reinterpret_cast< void* >(offset), bytes);
  DCFlushRange(sUploadBuffer, bytes);
  return sUploadBuffer;
}

const CFactoryFnReturn FAudioGroupSetLocDataFactory(const SObjectTag& tag,
                                                    const rstl::auto_ptr< uchar >& data, int length,
                                                    const CVParamTransfer& xfer) {
  return rs_new CAudioGrpSetLoc(data, length);
}

CAudioGrpSetLoc::CAudioGrpSetLoc(const rstl::auto_ptr< uchar >& data, int length)
: mStackId(-2), mAramAlloc(CARAMManager::GetInvalidAlloc()) {
  CMemoryInStream in(data.get(), length);
  if (in.ReadInt32() <= 1) {
    rstl::string name(in);
    const uint groupId = in.ReadUint16();
    const int poolSize = in.ReadInt32();
    const int projectSize = in.ReadInt32();
    const int sampleDirSize = in.ReadInt32();
    int sampleSize = in.ReadInt32();

    mPool = rs_new uchar[poolSize];
    in.ReadBytes(mPool.get(), poolSize);
    mProject = rs_new uchar[projectSize];
    in.ReadBytes(mProject.get(), projectSize);
    mSampleDir = rs_new uchar[sampleDirSize];
    in.ReadBytes(mSampleDir.get(), sampleDirSize);

    uchar* samples = data.get() + in.GetReadPosition();
    sampleSize = (sampleSize + 31) & ~31;
    mAramAlloc = CARAMManager::Alloc(sampleSize, 1);
    if (mAramAlloc == CARAMManager::GetInvalidAlloc()) {
      return;
    }

    mStackBuffer = rs_new uchar[sndStackGetSize()];
    mStackId = sndStackAdd(mStackBuffer.get(), mAramAlloc, sampleSize);
    if (mStackId == -1) {
      CARAMManager::Free(mAramAlloc, 1);
      mAramAlloc = CARAMManager::GetInvalidAlloc();
      return;
    }

    uchar uploadBuffer[0x1000 + 31];
    sUploadBuffer = reinterpret_cast< uchar* >(reinterpret_cast< uint >(uploadBuffer + 31) & ~31);
    sndSetSampleDataUploadCallback(UploadCallback, 0x1000);
    if (!sndStackSetCurrent(mStackId)) {
      CARAMManager::Free(mAramAlloc, 1);
      mAramAlloc = CARAMManager::GetInvalidAlloc();
      return;
    }

    if (!sndPushGroup(mProject.get(), groupId, samples, mSampleDir.get(), mPool.get())) {
      CARAMManager::Free(mAramAlloc, 1);
      mAramAlloc = CARAMManager::GetInvalidAlloc();
      sndStackRemove(mStackId);
      sndStackSetCurrent(-2);
      mStackId = -2;
      return;
    }

    sndSetSampleDataUploadCallback(nullptr, 0);
    sndStackSetCurrent(-2);

    if (sInSinglePlayer) {
      if (CStringExtras::IndexOfSubstring(name, rstl::string_l("_MP")) != -1 ||
          CStringExtras::IndexOfSubstring(name, rstl::string_l("Multiplayer")) != -1) {
        char* str = static_cast< char* >(__alloca(name.size() + 1));
        strcpy(str, name.data());
        rs_debugger_printf("Multi sound %s in single", str);
      }
    }
  }
}

CAudioGrpSetLoc::~CAudioGrpSetLoc() {
  if (mStackId != -2 && mStackId != -1) {
    sndStackSetCurrent(mStackId);
    sndPopGroup();
    sndStackRemove(mStackId);
    sndStackSetCurrent(-2);
  }
  if (mAramAlloc != CARAMManager::GetInvalidAlloc()) {
    CARAMManager::Free(mAramAlloc, 1);
  }
}
