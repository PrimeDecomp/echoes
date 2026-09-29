#ifndef _CVECTORELEMENT
#define _CVECTORELEMENT

#include "Kyoto/Streams/CInputStream.hpp"
#include "types.h"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/IElement.hpp"

#include "rstl/single_ptr.hpp"

class CVEFastConstant : public CVectorElement {
public:
  CVEFastConstant(float x, float y, float z);
  ~CVEFastConstant() override;
  bool GetValue(int frame, CVector3f& valOut) const override;
  bool IsFastConstant() const override { return true; }

private:
  CVector3f mVal;
};

class CVEParticleLocation : public CVectorElement {
public:
  ~CVEParticleLocation() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEParticlePreviousLocation : public CVectorElement {
public:
  ~CVEParticlePreviousLocation() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEParticleVelocity : public CVectorElement {
public:
  ~CVEParticleVelocity() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEParticleSystemOrientationFront : public CVectorElement {
public:
  ~CVEParticleSystemOrientationFront() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEParticleSystemOrientationUp : public CVectorElement {
public:
  ~CVEParticleSystemOrientationUp() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEParticleSystemOrientationRight : public CVectorElement {
public:
  ~CVEParticleSystemOrientationRight() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEParticleSystemTranslation : public CVectorElement {
public:
  ~CVEParticleSystemTranslation() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEConstant : public CVectorElement {
public:
  CVEConstant(CRealElement* x, CRealElement* y, CRealElement* z);
  ~CVEConstant();
  bool GetValue(int frame, CVector3f& valOut) const override;

public:
  CRealElement* mX;
  CRealElement* mY;
  CRealElement* mZ;
};

class CVECircleCluster : public CVectorElement {
public:
  CVECircleCluster(CVectorElement* circleOffset, CVectorElement* circleNormal,
                   CIntElement* cycleFrames, CRealElement* randomFactor);
  ~CVECircleCluster() override;

  bool GetValue(int frame, CVector3f& valOut) const override;

private:
  CVectorElement* mCircleOffset;
  CVector3f mXVec;
  CVector3f mYVec;
  float mRadius;
  CRealElement* mRandomFactor;
};

class CVECone : public CVectorElement {
  CVectorElement* mDirection;
  CRealElement* mMagnitude;
  CVector3f mXVec;
  CVector3f mYVec;

public:
  CVECone(CVectorElement* direction, CRealElement* magnitude);
  ~CVECone();
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEAngleCone : public CVectorElement {
public:
  CVEAngleCone(CRealElement* angleXConstant, CRealElement* angleYConstant,
               CRealElement* angleXRange, CRealElement* angleYRange, CRealElement* magnitude);
  ~CVEAngleCone();
  bool GetValue(int frame, CVector3f& valOut) const override;

private:
  CRealElement* mAngleXConstant;
  CRealElement* mAngleYConstant;
  CRealElement* mAngleXRange;
  CRealElement* mAngleYRange;
  CRealElement* mMagnitude;
};

class CVECircle : public CVectorElement {
public:
  CVECircle(CVectorElement* circleOffset, CVectorElement* circleNormal, CRealElement* angleConstant,
            CRealElement* angleLinear, CRealElement* radius);
  ~CVECircle() override;

  bool GetValue(int frame, CVector3f& valOut) const override;

private:
  CVectorElement* mCircleOffset;
  CVector3f mXVec;
  CVector3f mYVec;
  CRealElement* mAngleConstant;
  CRealElement* mAngleLinear;
  CRealElement* mRadius;
};

class CVEKeyframeEmitter : public CVectorElement {
public:
  CVEKeyframeEmitter(CInputStream& in);
  ~CVEKeyframeEmitter() override;

  bool GetValue(int frame, CVector3f& valOut) const override;

private:
  int mPercent;
  int mUnk1;
  bool mLoop;
  bool mUnk2;
  int mLoopEnd;
  int mLoopStart;
  rstl::vector< CVector3f > mKeys;
};

class CVEAdd : public CVectorElement {
public:
  CVEAdd(CVectorElement* a, CVectorElement* b);
  ~CVEAdd() override;

  bool GetValue(int frame, CVector3f& valOut) const override;

private:
  CVectorElement* mA;
  CVectorElement* mB;
};

class CVEMultiply : public CVectorElement {
public:
  CVEMultiply(CVectorElement* a, CVectorElement* b);
  ~CVEMultiply() override;

  bool GetValue(int frame, CVector3f& valOut) const override;

private:
  CVectorElement* mA;
  CVectorElement* mB;
};

class CVETimeChain : public CVectorElement {
public:
  CVETimeChain(CVectorElement* a, CVectorElement* b, CIntElement* mSwitchFrame);
  ~CVETimeChain() override;

  bool GetValue(int frame, CVector3f& valOut) const override;

private:
  CVectorElement* mA;
  CVectorElement* mB;
  CIntElement* mSwitchFrame;
};

class CVEPulse : public CVectorElement {
public:
  CVEPulse(CIntElement* durationA, CIntElement* durationB, CVectorElement* a, CVectorElement* b);
  ~CVEPulse() override;

  bool GetValue(int frame, CVector3f& valOut) const override;

private:
  CIntElement* mDurationA;
  CIntElement* mDurationB;
  CVectorElement* mA;
  CVectorElement* mB;
};
class CVERealToVector : public CVectorElement {
public:
  CVERealToVector(CRealElement* value);
  ~CVERealToVector() override;

  bool GetValue(int frame, CVector3f& valOut) const override;

private:
  CRealElement* mValue;
};

class CVESubtract : public CVectorElement {
public:
  CVESubtract(CVectorElement* a, CVectorElement* b);
  ~CVESubtract() override;

  bool GetValue(int frame, CVector3f& valOut) const override;

private:
  CVectorElement* mA;
  CVectorElement* mB;
};

class CVEColorToVector : public CVectorElement {
public:
  CVEColorToVector(CColorElement* value);
  ~CVEColorToVector() override;

  bool GetValue(int frame, CVector3f& valOut) const override;

private:
  CColorElement* mValue;
};

// Echoes additions. FourCC-based class names are placeholders; descriptive names follow
// Prime/MP3 analogues. Member offsets come from the constructors.

class CVEInitialSwitch : public CVectorElement {
  CVectorElement* x4_;
  CVectorElement* x8_;

public:
  CVEInitialSwitch(CVectorElement* a, CVectorElement* b);
  ~CVEInitialSwitch() override;
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEKeyframeInput : public CVectorElement {
  int mPercent;
  int mUnk1;
  bool mLoop;
  bool mUnk2;
  int mLoopEnd;
  int mLoopStart;
  float x18_;
  float x1c_;
  rstl::vector< CVector3f > mKeys;
  CRealElement* x30_;

public:
  CVEKeyframeInput(CInputStream& in);
  ~CVEKeyframeInput() override;
  bool GetValue(int frame, CVector3f& valOut) const override;
  
  const int GetLoopStart() const { return mLoopStart; }
  const int GetLoopEnd() const { return mLoopEnd; }
};

class CVEKeepInitial : public CVectorElement {
  CVectorElement* x4_;

public:
  CVEKeepInitial(CVectorElement* a);
  ~CVEKeepInitial() override;
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVENormalize : public CVectorElement {
  CVectorElement* x4_;

public:
  CVENormalize(CVectorElement* a) : x4_(a) {}
  ~CVENormalize() override;
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEParticleEndNormalizedVelocity : public CVectorElement {
public:
  ~CVEParticleEndNormalizedVelocity() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEParticleEndTranslation : public CVectorElement {
public:
  ~CVEParticleEndTranslation() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEParticleEndVelocity : public CVectorElement {
public:
  ~CVEParticleEndVelocity() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEParticleInitialNormalizedVelocity : public CVectorElement {
public:
  ~CVEParticleInitialNormalizedVelocity() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEParticleInitialTranslation : public CVectorElement {
public:
  ~CVEParticleInitialTranslation() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEParticleInitialVelocity : public CVectorElement {
public:
  ~CVEParticleInitialVelocity() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVENormalizedCompensatedVelocity : public CVectorElement {
public:
  ~CVENormalizedCompensatedVelocity() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEParticleAccessParameter1 : public CVectorElement {
public:
  ~CVEParticleAccessParameter1() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEParticleAccessParameter2 : public CVectorElement {
public:
  ~CVEParticleAccessParameter2() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVEParticleAccessParameter3 : public CVectorElement {
public:
  ~CVEParticleAccessParameter3() override {}
  bool GetValue(int frame, CVector3f& valOut) const override;
};

class CVERandomVector : public CVectorElement {
  CRealElement* x4_;

public:
  CVERandomVector(CRealElement* a);
  ~CVERandomVector() override;
  bool GetValue(int frame, CVector3f& valOut) const override;
};

#endif // _CVECTORELEMENT
