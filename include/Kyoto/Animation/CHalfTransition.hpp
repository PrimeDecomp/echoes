#ifndef _CHALFTRANSITION
#define _CHALFTRANSITION

#include "rstl/rc_ptr.hpp"

class IMetaTrans;
class CInputStream;
class CHalfTransition {
public:
  explicit CHalfTransition(CInputStream& in);

private:
  uint mId;
  rstl::rc_ptr<IMetaTrans> mTrans;
};

#endif // _CHALFTRANSITION
