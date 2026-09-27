#ifndef _CANIMATION
#define _CANIMATION

#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"

class IMetaAnim;
class CInputStream;

class CAnimation {
public:
  CAnimation(CInputStream& in);
  const rstl::rc_ptr< IMetaAnim >& GetMetaAnim() const { return mAnim; }

private:
  rstl::string mName;
  rstl::rc_ptr< IMetaAnim > mAnim;
};
CHECK_SIZEOF(CAnimation, 0x18)

#endif // _CANIMATION
