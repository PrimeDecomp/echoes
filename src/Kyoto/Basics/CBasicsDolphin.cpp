#include "Kyoto/Basics/CBasics.hpp"

#include "Kyoto/Basics/CStopwatch.hpp"

#include "dolphin/dvd.h"
#include "dolphin/os.h"
#include <string.h>

static bool gInitialized = false;
bool CBasics::Init() {
  if (!gInitialized) {
    OSInit();
    OSInitFastCast();
    DVDInit();

    CStopwatch::InitGlobalTimer();
    gInitialized = true;
  }
  return gInitialized;
}

void CBasics::CopyMemory(void* dst, const void* src, uint size) {
  uchar* out = static_cast< uchar* >(dst);
  const uchar* in = static_cast< const uchar* >(src);
  if ((reinterpret_cast< uint >(out) & 31) != (reinterpret_cast< uint >(in) & 31) || size < 32) {
    memcpy(dst, src, size);
  } else if (((reinterpret_cast< uint >(out) | size) & 31) == 0) {
    while (size != 0) {
      __dcbz(out, 0);
      size -= 32;
      double a = reinterpret_cast< const double* >(in)[0];
      double b = reinterpret_cast< const double* >(in)[1];
      double c = reinterpret_cast< const double* >(in)[2];
      double d = reinterpret_cast< const double* >(in)[3];
      in += 32;
      reinterpret_cast< double* >(out)[0] = a;
      reinterpret_cast< double* >(out)[1] = b;
      reinterpret_cast< double* >(out)[2] = c;
      reinterpret_cast< double* >(out)[3] = d;
      out += 32;
    }
  } else {
    uchar* alignedDst = reinterpret_cast< uchar* >((reinterpret_cast< uint >(out) + 31) & ~31);
    uint head = alignedDst - out;
    const uchar* alignedSrc =
        reinterpret_cast< const uchar* >((reinterpret_cast< uint >(in) + 31) & ~31);
    if (head != 0) {
      memcpy(out, in, head);
    }
    uint blocks = (size - head) & ~31;
    if (blocks != 0) {
      uint n = blocks;
      uchar* blockDst = alignedDst;
      const uchar* blockSrc = alignedSrc;
      while (n != 0) {
        __dcbz(blockDst, 0);
        n -= 32;
        double a = reinterpret_cast< const double* >(blockSrc)[0];
        double b = reinterpret_cast< const double* >(blockSrc)[1];
        double c = reinterpret_cast< const double* >(blockSrc)[2];
        double d = reinterpret_cast< const double* >(blockSrc)[3];
        blockSrc += 32;
        reinterpret_cast< double* >(blockDst)[0] = a;
        reinterpret_cast< double* >(blockDst)[1] = b;
        reinterpret_cast< double* >(blockDst)[2] = c;
        reinterpret_cast< double* >(blockDst)[3] = d;
        blockDst += 32;
      }
    }
    uint tail = size - head - blocks;
    if (tail != 0) {
      alignedSrc += blocks;
      alignedDst += blocks;
      memcpy(alignedDst, alignedSrc, tail);
    }
  }
}

void CBasics::ZeroMemory(void* dst, uint size) {
  uchar* out = static_cast< uchar* >(dst);
  if (size < 32) {
    memset(out, 0, size);
  } else if (((reinterpret_cast< uint >(out) | size) & 31) == 0) {
    while (size != 0) {
      __dcbz(out, 0);
      size -= 32;
      out += 32;
    }
  } else {
    uchar* aligned = reinterpret_cast< uchar* >((reinterpret_cast< uint >(out) + 31) & ~31);
    uint head = aligned - out;
    if (head != 0) {
      memset(out, 0, head);
    }
    uint blocks = (size - head) & ~31;
    if (blocks != 0) {
      uint n = blocks;
      uchar* block = aligned;
      while (n != 0) {
        __dcbz(block, 0);
        n -= 32;
        block += 32;
      }
    }
    uint tail = size - blocks - head;
    if (tail != 0) {
      aligned += blocks;
      memset(aligned, 0, tail);
    }
  }
}
