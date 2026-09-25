#ifndef _CAUDIOGROUPSET
#define _CAUDIOGROUPSET

#include "types.h"

#include "rstl/auto_ptr.hpp"
#include "rstl/single_ptr.hpp"

#include "Kyoto/CFactoryMgr.hpp"

class CVParamTransfer;

// Echoes AGSC: the group is pushed straight into ARAM through a MusyX sound stack.
class CAudioGrpSetLoc {
public:
  CAudioGrpSetLoc(const rstl::auto_ptr< uchar >& data, int length);
  ~CAudioGrpSetLoc();

  static bool sInSinglePlayer;

private:
  uint mStackId;
  const void* mAramAlloc;
  rstl::single_ptr< uchar > mStackBuffer;
  rstl::single_ptr< uchar > mPool;
  rstl::single_ptr< uchar > mProject;
  rstl::single_ptr< uchar > mSampleDir;
};
CHECK_SIZEOF(CAudioGrpSetLoc, 0x18)

const CFactoryFnReturn FAudioGroupSetLocDataFactory(const SObjectTag& tag,
                                                    const rstl::auto_ptr< uchar >& data, int length,
                                                    const CVParamTransfer& xfer);

#endif // _CAUDIOGROUPSET
