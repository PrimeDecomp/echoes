#ifndef _CSEGID
#define _CSEGID

#include <types.h>

#include "Kyoto/Streams/CInputStream.hpp"

class CSegId {
public:
  CSegId() : mId(-1) {}
  CSegId(CInputStream& in);
private:
  uchar mId;
};

#endif // _CSEGID
