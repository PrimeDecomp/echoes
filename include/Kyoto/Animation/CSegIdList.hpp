#ifndef _CSEGIDLIST
#define _CSEGIDLIST

#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "rstl/vector.hpp"

class CInputStream;
class CSegIdList {
public:
  CSegIdList(CInputStream& in);
  ~CSegIdList() {
    CCharAnimMemoryMetrics::SubtractFromTotalSize(mSegList.capacity(),
                                                  CCharAnimMemoryMetrics::kASS_Two);
  }

  int GetCount() const { return mSegList.size(); }

  rstl::vector< CSegId > mSegList;
};
CHECK_SIZEOF(CSegIdList, 0x10)

#endif // _CSEGIDLIST
