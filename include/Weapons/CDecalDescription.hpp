#ifndef _CDECALDESCRIPTION
#define _CDECALDESCRIPTION

#include "Kyoto/Particles/IElement.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class CModel;

class CDecalDescription {
public:
  struct SQuadDescr {
    SQuadDescr();

    rstl::single_ptr< CIntElement > mLFT;
    rstl::single_ptr< CRealElement > mSZE;
    rstl::single_ptr< CRealElement > mROT;
    rstl::single_ptr< CVectorElement > mOFF;
    rstl::single_ptr< CColorElement > mCLR;
    rstl::single_ptr< CUVElement > mTEX;
    bool mADD;
  };

  CDecalDescription();

  SQuadDescr mQuad1;
  SQuadDescr mQuad2;
  rstl::optional_object< TLockedToken< CModel > > mDMDL;
  rstl::single_ptr< CIntElement > mDLFT;
  rstl::single_ptr< CVectorElement > mDMOP;
  rstl::single_ptr< CVectorElement > mDMRT;
  rstl::single_ptr< CVectorElement > mDMSC;
  rstl::single_ptr< CColorElement > mDMCL;
  bool mDMAB : 1;
  bool mDMOO : 1;
};
NESTED_CHECK_SIZEOF(CDecalDescription, SQuadDescr, 0x1c)
CHECK_SIZEOF(CDecalDescription, 0x60)

#endif // _CDECALDESCRIPTION
