#include "Kyoto/Audio/CAudioSys.hpp"

CAudioSys::C3DEmitterParmData::C3DEmitterParmData(const float maxDist, const float distComp,
                                                  const uint flags, uchar maxVol, uchar minVol)
: mPos(CVector3f::Zero())
, mDir(CVector3f::Zero())
, mMaxDist(maxDist)
, mDistComp(distComp)
, mFlags(flags)
, mSfxId(0)
, mMaxVol(maxVol)
, mMinVol(minVol)
, mImportant(false)
, mPrio(kEmitterMedPriority)
, mStudio(0) {}
