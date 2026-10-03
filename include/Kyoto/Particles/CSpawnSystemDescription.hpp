#ifndef _CSPAWNSYSTEMDESCRIPTION
#define _CSPAWNSYSTEMDESCRIPTION

#include "types.h"

#include "Kyoto/Particles/IElement.hpp"

class CSpawnSystemKeyframeData;

// Property identities and layout recovered from the G2ME01 SPSC reader.
class CSpawnSystemDescription {
public:
  CSpawnSystemDescription();
  ~CSpawnSystemDescription();

  CIntElement* mPSLT;
  CVectorElement* mIVEC;
  CRealElement* mVBLN;
  CModVectorElement* mVLM1;
  CModVectorElement* mVLM2;
  CIntElement* mGIVL;
  bool mIGGT : 1;
  bool mIGLT : 1;
  bool mVMD1 : 1;
  bool mVMD2 : 1;
  bool mDEOL : 1;
  bool mFRCO : 1;
  CColorElement* mPCOL;
  CVectorElement* mSCLE;
  CVectorElement* mLSCL;
  CVectorElement* mTRNL;
  CVectorElement* mORNT;
  CVectorElement* mGTRN;
  CVectorElement* mGORN;
  CVectorElement* mFROV;
  CSpawnSystemKeyframeData* mSPWN;
};
CHECK_SIZEOF(CSpawnSystemDescription, 0x40)

#endif // _CSPAWNSYSTEMDESCRIPTION
