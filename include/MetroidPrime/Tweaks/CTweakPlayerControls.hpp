#ifndef _CTWEAKPLAYERCONTROLS
#define _CTWEAKPLAYERCONTROLS

#include "types.h"

#include "MetroidPrime/CControlMapper.hpp"

// Echoes keeps one control tweak per control scheme; names are inferred.
class CTweakPlayerControls {
public:
  CControlMapper::EFunctionList GetMapping(CControlMapper::ECommands command) const;
};

extern CTweakPlayerControls* gpTweakPlayerControlsA;
extern CTweakPlayerControls* gpTweakPlayerControlsB;

#endif // _CTWEAKPLAYERCONTROLS
