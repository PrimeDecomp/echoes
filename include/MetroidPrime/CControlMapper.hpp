#ifndef _CCONTROLMAPPER
#define _CCONTROLMAPPER

#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "types.h"

class CFinalInput;

class CControlMapper {
public:
  // Command identities beyond the established input interface remain under research.
  enum ECommands {
    kC_MapCircleUp = 43,
    kC_MapCircleDown,
    kC_MapCircleLeft,
    kC_MapCircleRight,
    kC_MapMoveForward,
    kC_MapMoveBack,
    kC_MapMoveLeft,
    kC_MapMoveRight,
    kC_MapZoomIn,
    kC_MapZoomOut,
    kC_Count = 76
  };
  enum EFilterType { kFT_Filtered, kFT_Unfiltered };

  explicit CControlMapper(int controlScheme);

  float GetAnalogInput(ECommands command, const CFinalInput& input,
                       EFilterType filter = kFT_Filtered) const;
  bool GetDigitalInput(ECommands command, const CFinalInput& input,
                       EFilterType filter = kFT_Filtered) const;

private:
  rstl::reserved_vector< bool, kC_Count > mCommandEnabled;
  rstl::reserved_vector< bool, kC_Count > mCommandOverridden;
  rstl::reserved_vector< rstl::pair< ECommands, int >, 8 > mCommandOverrides;
  int mControlScheme;
};
CHECK_SIZEOF(CControlMapper, 0xe8)

#endif // _CCONTROLMAPPER
