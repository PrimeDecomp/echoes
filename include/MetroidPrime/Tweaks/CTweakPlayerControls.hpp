#ifndef _CTWEAKPLAYERCONTROLS
#define _CTWEAKPLAYERCONTROLS

#include "types.h"

#include "MetroidPrime/CControlMapper.hpp"
#include "rstl/single_ptr.hpp"

struct SLdrTweakPlayerControls;

// Echoes keeps one control tweak per control scheme; names are inferred.
class CTweakPlayerControls {
public:
  explicit CTweakPlayerControls(const SLdrTweakPlayerControls& data) : mData(&data) {}

  CControlMapper::EFunctionList GetMapping(CControlMapper::ECommands command) const;

private:
  const SLdrTweakPlayerControls* mData;
};
CHECK_SIZEOF(CTweakPlayerControls, 0x4)

extern rstl::single_ptr< CTweakPlayerControls > gpTweakPlayerControlsA;
extern rstl::single_ptr< CTweakPlayerControls > gpTweakPlayerControlsB;

#endif // _CTWEAKPLAYERCONTROLS
