#ifndef _CELECTRICDESCRIPTION
#define _CELECTRICDESCRIPTION

#include "types.h"

#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"

class CColorElement;
class CEmitterElement;
class CGenDescription;
class CIntElement;
class CRealElement;
class CSwooshDescription;
class CUVElement;

class CElectricDescription {
public:
  CElectricDescription();
  ~CElectricDescription();

  CIntElement* x0_LIFE;
  CIntElement* x4_SLIF;
  CRealElement* x8_GRAT;
  CIntElement* xc_SCNT;
  CIntElement* x10_SSEG;
  CColorElement* x14_COLR;
  CEmitterElement* x18_IEMT;
  CEmitterElement* x1c_FEMT;
  CRealElement* x20_AMPL;
  CRealElement* x24_AMPD;
  CRealElement* x28_LWD1;
  CRealElement* x2c_LWD2;
  CRealElement* x30_LWD3;
  CColorElement* x34_LCL1;
  CColorElement* x38_LCL2;
  CColorElement* x3c_LCL3;
  CUVElement* x40_TEXR;
  int x44_DFLG;
  rstl::optional_object< TCachedToken< CSwooshDescription > > x48_SSWH;
  rstl::optional_object< TCachedToken< CGenDescription > > x58_GPSM;
  rstl::optional_object< TCachedToken< CGenDescription > > x68_EPSM;
  bool x78_ZERY;
};
CHECK_SIZEOF(CElectricDescription, 0x7c)

#endif // _CELECTRICDESCRIPTION
