#ifndef _CPOSEASTRANSFORMS
#define _CPOSEASTRANSFORMS

#include "types.h"

#include "Kyoto/Math/CTransform4f.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CPoseAsTransforms {
public:
private:
  CSegId mNextId;
  CSegId mCount;
  // TODO TSegIdMapVariableSize<CPoseAsTransforms::CElementType>
  rstl::reserved_vector< rstl::pair< CSegId, CSegId >, 100 > mLinks;
  rstl::single_ptr< CTransform4f > mTransformArr;
  CSegId mLastInserted;
};
CHECK_SIZEOF(CPoseAsTransforms, 0xd8)

#endif // _CPOSEASTRANSFORMS
