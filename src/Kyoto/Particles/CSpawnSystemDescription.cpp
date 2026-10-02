#include "Kyoto/Particles/CSpawnSystemDescription.hpp"

#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"

CSpawnSystemDescription::CSpawnSystemDescription()
: mPSLT(nullptr),
  mIVEC(nullptr),
  mVBLN(nullptr),
  mVLM1(nullptr),
  mVLM2(nullptr),
  mGIVL(nullptr),
  mIGGT(false),
  mIGLT(false),
  mVMD1(false),
  mVMD2(false),
  mDEOL(false),
  mFRCO(false),
  mPCOL(nullptr),
  mSCLE(nullptr),
  mLSCL(nullptr),
  mTRNL(nullptr),
  mORNT(nullptr),
  mGTRN(nullptr),
  mGORN(nullptr),
  mFROV(nullptr),
  mSPWN(nullptr) {}

CSpawnSystemDescription::~CSpawnSystemDescription() {
  delete mPSLT;
  delete mIVEC;
  delete mVBLN;
  delete mVLM1;
  delete mVLM2;
  delete mGIVL;
  delete mPCOL;
  delete mSCLE;
  delete mLSCL;
  delete mTRNL;
  delete mORNT;
  delete mGTRN;
  delete mGORN;
  delete mFROV;
  delete mSPWN;
}

