#ifndef _CTWEAKSLIDESHOW
#define _CTWEAKSLIDESHOW

#include "rstl/single_ptr.hpp"

struct SLdrTweakSlideShow;

class CTweakSlideShow {
public:
  explicit CTweakSlideShow(const SLdrTweakSlideShow& data) : mData(&data) {}

private:
  const SLdrTweakSlideShow* mData;
};
CHECK_SIZEOF(CTweakSlideShow, 0x4)

extern rstl::single_ptr< CTweakSlideShow > gpTweakSlideShow;

#endif // _CTWEAKSLIDESHOW

