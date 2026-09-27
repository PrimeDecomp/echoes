#ifndef _CHALFTRANSITION
#define _CHALFTRANSITION

#include "rstl/rc_ptr.hpp"

class IMetaTrans;
class CInputStream;
class CHalfTransition {
public:
  explicit CHalfTransition(CInputStream& in);
  uint GetPrimitiveIndex() const { return mId; }
  const rstl::rc_ptr< IMetaTrans >& GetMetaTrans() const { return mTrans; }

private:
  uint mId;
  rstl::rc_ptr< IMetaTrans > mTrans;
};

CHECK_SIZEOF(CHalfTransition, 0xc)

#endif // _CHALFTRANSITION
