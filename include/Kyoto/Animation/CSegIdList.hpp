#ifndef _CSEGIDLIST
#define _CSEGIDLIST

#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "rstl/vector.hpp"

class CInputStream;
class CSegIdList {
public:
  typedef rstl::vector< CSegId >::const_iterator const_iterator;

  CSegIdList(CInputStream& in);
  ~CSegIdList() {
    CCharAnimMemoryMetrics::SubtractFromTotalSize(mSegList.capacity(),
                                                  CCharAnimMemoryMetrics::kASS_Two);
  }

  int GetCount() const { return mSegList.size(); }

  const_iterator begin() const { return mSegList.begin(); }

  const_iterator end() const { return mSegList.end(); }

  rstl::vector< CSegId > mSegList;
};
CHECK_SIZEOF(CSegIdList, 0x10)

#endif // _CSEGIDLIST
