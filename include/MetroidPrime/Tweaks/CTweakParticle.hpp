#ifndef _CTWEAKPARTICLE
#define _CTWEAKPARTICLE

#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

struct SLdrTweakParticle;

class CTweakParticle {
public:
  explicit CTweakParticle(const SLdrTweakParticle& data) : mData(&data) {}

private:
  const SLdrTweakParticle* mData;
  rstl::string x4_;
  rstl::string x14_;
  rstl::string x24_;
};
CHECK_SIZEOF(CTweakParticle, 0x34)

extern rstl::single_ptr< CTweakParticle > gpTweakParticle;

#endif // _CTWEAKPARTICLE
