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

  bool GetFreeLookTurnsPlayer() const;            // Guessed name.
  bool GetMoveDuringFreeLook() const;             // Guessed name.
  bool GetHoldButtonsForFreeLook() const;         // Guessed name.
  bool GetTwoButtonsForFreeLook() const;          // Guessed name.
  bool GetAimWhenOrbitingPoint() const;           // Guessed name.
  bool GetStayInFreeLookWhileFiring() const;      // Guessed name.
  bool GetOrbitFixedOffset() const;               // Guessed name.
  bool GetGunButtonTogglesHolster() const;        // Guessed name.
  bool GetGunNotFiringHolstersGun() const;        // Guessed name.
  bool GetFallingDoubleJump() const;              // Guessed name.
  bool GetImpulseDoubleJump() const;              // Guessed name.
  bool GetFiringCancelsCameraPitch() const;       // Guessed name.
  bool GetAssistedAimingIgnoreHorizontal() const; // Guessed name.
  bool GetAssistedAimingIgnoreVertical() const;   // Guessed name.

private:
  const SLdrTweakPlayerControls* mData;
};
CHECK_SIZEOF(CTweakPlayerControls, 0x4)

extern rstl::single_ptr< CTweakPlayerControls > gpTweakPlayerControlsA;
extern rstl::single_ptr< CTweakPlayerControls > gpTweakPlayerControlsB;

#endif // _CTWEAKPLAYERCONTROLS
