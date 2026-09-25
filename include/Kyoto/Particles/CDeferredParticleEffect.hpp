#ifndef _CDEFERREDPARTICLEEFFECT
#define _CDEFERREDPARTICLEEFFECT
#include "Kyoto/CDependencyGroupToken.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

// Guessed name. Owns an effect, optional dependency group and lazy generator.
class CDeferredParticleEffect {
public:
  CDeferredParticleEffect(const TToken< CGenDescription >& effect,
                          const CDependencyGroupToken& group);
  ~CDeferredParticleEffect();
  void SetParticleEmission(bool emitting);

private:
  TToken< CGenDescription > mEffect;
  SObjectTag mTag;
  rstl::optional_object< CDependencyGroupToken > mDependencies;
  rstl::single_ptr< CElementGen > mGenerator;
  float x38_;
  float x3c_;
  bool x40_24_ : 1;
  bool x40_25_ : 1;
  bool x40_26_ : 1;
};
CHECK_SIZEOF(CDeferredParticleEffect, 0x44)
#endif // _CDEFERREDPARTICLEEFFECT
