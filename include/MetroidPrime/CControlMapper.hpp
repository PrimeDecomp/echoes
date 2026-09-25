#ifndef _CCONTROLMAPPER
#define _CCONTROLMAPPER

#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "types.h"

class CControlMapper {
public:
  // Command identities beyond the established input interface remain under research.
  enum ECommands { kC_Count = 76 };
  enum EFilterType { kFT_Filtered, kFT_Unfiltered };

  explicit CControlMapper(int controlScheme);

private:
  rstl::reserved_vector< bool, kC_Count > mCommandEnabled;
  rstl::reserved_vector< bool, kC_Count > mCommandOverridden;
  rstl::reserved_vector< rstl::pair< ECommands, int >, 8 > mCommandOverrides;
  int mControlScheme;
};
CHECK_SIZEOF(CControlMapper, 0xe8)

#endif // _CCONTROLMAPPER
