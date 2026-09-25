#ifndef _CGUIWIDGETIDDB
#define _CGUIWIDGETIDDB
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
class CGuiWidgetIdDB {
public:
  CGuiWidgetIdDB();

private:
  rstl::vector< rstl::string > mNames;
  short mLastPoolId;
  rstl::string mInvalidWidgetName;
};
CHECK_SIZEOF(CGuiWidgetIdDB, 0x24)
#endif // _CGUIWIDGETIDDB
