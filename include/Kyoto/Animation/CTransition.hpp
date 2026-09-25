#ifndef _CTRANSITION
#define _CTRANSITION

#include "rstl/rc_ptr.hpp"
#include "rstl/pair.hpp"


class IMetaTrans;
class CInputStream;
class CTransition {
  uint mId;
  uint mAnimA;
  uint mAnimB;
  rstl::rc_ptr<IMetaTrans> mTrans;

public:
  explicit CTransition(CInputStream& in);
  u32 GetAnimA() const { return mAnimA; }
  u32 GetAnimB() const { return mAnimB; }
  rstl::pair<uint, uint> GetAnimPair() const { return rstl::pair<uint, uint>(mAnimA, mAnimB); }
  const rstl::rc_ptr<IMetaTrans>& GetMetaTrans() const { return mTrans; }
};

#endif // _CTRANSITION
