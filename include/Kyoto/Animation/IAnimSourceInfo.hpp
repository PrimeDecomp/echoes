#ifndef _IANIMSOURCEINFO
#define _IANIMSOURCEINFO

#include "Kyoto/Animation/CCharAnimTime.hpp"

class IAnimSourceInfo {
public:
  virtual CCharAnimTime GetAnimationDuration() const = 0;
  // Guessed name: tests whether the source has scale keys.
  virtual bool HasScaleData() const = 0;
  virtual ~IAnimSourceInfo() {}
};
CHECK_SIZEOF(IAnimSourceInfo, 0x4)

#endif // _IANIMSOURCEINFO
