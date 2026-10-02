#ifndef _CSEGSTATEMENTSET
#define _CSEGSTATEMENTSET

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CSegStatement.hpp"

class CSegStatementSet {
public:
  virtual ~CSegStatementSet() = 0;
  explicit CSegStatementSet(void* storage);

  CSegStatement& operator[](const CSegId& seg) { return mSegData[seg.val()]; }
  const CSegStatement& operator[](const CSegId& seg) const { return mSegData[seg.val()]; }

protected:
  CSegStatement* mSegData;
};
CHECK_SIZEOF(CSegStatementSet, 0x8)

inline CSegStatementSet::~CSegStatementSet() {}

class CStackSegStatementSet : public CSegStatementSet {
public:
  CStackSegStatementSet();

  // CSegStatementSet
  ~CStackSegStatementSet() override;
};
CHECK_SIZEOF(CStackSegStatementSet, 0x8)

#endif // _CSEGSTATEMENTSET
