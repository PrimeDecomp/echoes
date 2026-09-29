#include "Kyoto/Particles/CRealElement.hpp"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/math.hpp"

// Unreferenced; every nearby particle TU constructs one of these, likely from a shared header.
static const CVector3f skOneVector(1.f, 1.f, 1.f);

static inline int GetKeyframeIndex(int frame, bool loop, int loopStart, int loopEnd) {
  if (loop) {
    if (frame >= loopEnd) {
      frame -= loopStart;
      frame = frame % (loopEnd - loopStart);
      frame += loopStart;
    }
  } else {
    frame = rstl::min_val(frame, loopEnd - 1);
  }
  return frame;
}

static inline int GetKeyframeTime(float value, float start, float rate) {
  return rstl::max_val(0, CCast::ToInt32(rate * (value - start)));
}

CREConstant::CREConstant(float val) : mVal(val) {}

CREConstant::~CREConstant() {}

bool CREConstant::GetValue(int frame, float& valOut) const {
  valOut = mVal;
  return false;
}

CRESineWave::CRESineWave(CRealElement* a, CRealElement* b, CRealElement* c)
: mFrequency(b), mAmplitude(c), mPhase(a) {}

CRESineWave::~CRESineWave() {
  delete mFrequency;
  delete mAmplitude;
  delete mPhase;
}

bool CRESineWave::GetValue(int frame, float& valOut) const {
  float amp, freq, phase;
  mAmplitude->GetValue(frame, amp);
  mFrequency->GetValue(frame, freq);
  mPhase->GetValue(frame, phase);
  valOut = sine(CRelAngle::FromDegrees(frame * freq + phase)) * amp;
  return false;
}

CRETimeScale::CRETimeScale(CRealElement* a) : mA(a) {}

CRETimeScale::~CRETimeScale() { delete mA; }

bool CRETimeScale::GetValue(int frame, float& valOut) const {
  float a;
  mA->GetValue(frame, a);
  valOut = static_cast< float >(frame) * a;
  return false;
}

CREAdd::CREAdd(CRealElement* a, CRealElement* b) : mA(a), mB(b) {}

CREAdd::~CREAdd() {
  delete mA;
  delete mB;
}

bool CREAdd::GetValue(int frame, float& valOut) const {
#if NONMATCHING
  float a = 0.f, b = 0.f;
#else
  float a, b;
#endif
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  valOut = a + b;
  return false;
}

CREMultiply::CREMultiply(CRealElement* a, CRealElement* b) : mA(a), mB(b) {}

CREMultiply::~CREMultiply() {
  delete mA;
  delete mB;
}

bool CREMultiply::GetValue(int frame, float& valOut) const {
#if NONMATCHING
  float a = 0.f, b = 0.f;
#else
  float a, b;
#endif
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  valOut = a * b;
  return false;
}

CREDotProduct::CREDotProduct(CVectorElement* a, CVectorElement* b) : mA(a), mB(b) {}

CREDotProduct::~CREDotProduct() {
  delete mA;
  delete mB;
}

bool CREDotProduct::GetValue(int frame, float& valOut) const {
  CVector3f a = CVector3f::Zero();
  CVector3f b = CVector3f::Zero();
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  valOut = CVector3f::Dot(a, b);
  return false;
}

CRERandom::CRERandom(CRealElement* min, CRealElement* max) : mMin(min), mMax(max) {}

CRERandom::~CRERandom() {
  delete mMin;
  delete mMax;
}

bool CRERandom::GetValue(int frame, float& valOut) const {
  float min, max;
  mMin->GetValue(frame, min);
  mMax->GetValue(frame, max);
  valOut = (max - min) * CRandom16::GetRandomNumber()->Float() + min;
  return false;
}

CREInitialRandom::CREInitialRandom(CRealElement* min, CRealElement* max)
: mMin(min), mMax(max) {}

CREInitialRandom::~CREInitialRandom() {
  delete mMin;
  delete mMax;
}

bool CREInitialRandom::GetValue(int frame, float& valOut) const {
  if (frame == 0) {
    float min, max;
    mMin->GetValue(frame, min);
    mMax->GetValue(frame, max);
    valOut = (max - min) * CRandom16::GetRandomNumber()->Float() + min;
  }
  return false;
}

CRETimeChain::CRETimeChain(CRealElement* a, CRealElement* b, CIntElement* c)
: mA(a), mB(b), mSwFrame(c) {}

CRETimeChain::~CRETimeChain() {
  delete mA;
  delete mB;
  delete mSwFrame;
}

bool CRETimeChain::GetValue(int frame, float& valOut) const {
  int v;
  mSwFrame->GetValue(frame, v);
  if (frame < v) {
    return mA->GetValue(frame, valOut);
  } else {
    return mB->GetValue(frame - v, valOut);
  }
}

CREClamp::CREClamp(CRealElement* a, CRealElement* b, CRealElement* c)
: mMin(a), mMax(b), mVal(c) {}

CREClamp::~CREClamp() {
  delete mMin;
  delete mMax;
  delete mVal;
}

bool CREClamp::GetValue(int frame, float& valOut) const {
  float a, b;
  mMin->GetValue(frame, a);
  mMax->GetValue(frame, b);
  mVal->GetValue(frame, valOut);
  if (valOut > b) {
    valOut = b;
  }
  if (valOut < a) {
    valOut = a;
  }
  return false;
}

CREPulse::CREPulse(CIntElement* a, CIntElement* b, CRealElement* c, CRealElement* d)
: mADuration(a), mBDuration(b), mValA(c), mValB(d) {}

CREPulse::~CREPulse() {
  delete mADuration;
  delete mBDuration;
  delete mValA;
  delete mValB;
}

bool CREPulse::GetValue(int frame, float& valOut) const {
  int a, b;
  mADuration->GetValue(frame, a);
  mBDuration->GetValue(frame, b);
  int cv = a + b + 1;
  if (cv < 0) {
    cv = 1;
  }

  if (b >= 1) {
    // CREPulse is an outlier here, the other
    // IElement classes use > instead of >=.
    if (frame % cv >= a) {
      mValB->GetValue(frame, valOut);
    } else {
      mValA->GetValue(frame, valOut);
    }
  } else {
    mValA->GetValue(frame, valOut);
  }
  return false;
}

CRELifetimePercent::CRELifetimePercent(CRealElement* a) : mPercentVal(a) {}

CRELifetimePercent::~CRELifetimePercent() { delete mPercentVal; }

bool CRELifetimePercent::GetValue(int frame, float& valOut) const {
  float a = 0.f;
  mPercentVal->GetValue(frame, a);
  if (a < 0.f) {
    a = 0.f;
  }
  valOut = (a / 100.f) * CParticleGlobals::GetParticleLifetimeReal();
  return false;
}

CRELifetimeTween::CRELifetimeTween(CRealElement* a, CRealElement* b) : mA(a), mB(b) {}

CRELifetimeTween::~CRELifetimeTween() {
  delete mA;
  delete mB;
}

// fake but using it to test
static inline float Lerp(float a, float b, float c) { return b * c + a * (1.f - c); }

bool CRELifetimeTween::GetValue(int frame, float& valOut) const {
  float ltFac = frame / CParticleGlobals::GetParticleLifetimeReal();
  float a, b;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  valOut = Lerp(a, b, ltFac);
  return false;
}

CREKeyframeEmitter::CREKeyframeEmitter(CInputStream& in)
: mPercent(in.ReadInt32())
, mUnk1(in.ReadInt32())
, mLoop(in.ReadBool())
, mUnk2(in.ReadBool())
, mLoopEnd(in.ReadInt32())
, mLoopStart(in.ReadInt32())
, mKeys(in) {}

CREKeyframeEmitter::~CREKeyframeEmitter() {}

bool CREKeyframeEmitter::GetValue(int frame, float& valOut) const {
  if (mPercent == 0) {
    int emitterTime =
        GetKeyframeIndex(CParticleGlobals::GetEmitterTime(), mLoop, mLoopStart, mLoopEnd);
    valOut = mKeys[emitterTime];
    return false;
  }

  if (CParticleGlobals::GetParticleLifetimePercentage() == 100) {
    valOut = mKeys[CParticleGlobals::GetParticleLifetimePercentage()];
  } else {
    valOut = (1.f - CParticleGlobals::GetParticleLifetimePercentageRemainder()) *
                 mKeys[CParticleGlobals::GetParticleLifetimePercentage()] +
             CParticleGlobals::GetParticleLifetimePercentageRemainder() *
                 mKeys[CParticleGlobals::GetParticleLifetimePercentage() + 1];
  }
  return false;
}

CREKeyframeInput::CREKeyframeInput(CInputStream& in)
: mPercent(in.ReadInt32())
, mUnk1(in.ReadInt32())
, mLoop(in.ReadBool())
, mUnk2(in.ReadBool())
, mLoopEnd(in.ReadInt32())
, mLoopStart(in.ReadInt32())
, x18_(in.ReadFloat())
, x1c_(in.ReadFloat())
, mKeys(in)
, x30_(CParticleDataFactory::GetRealElement(in)) {}

CREKeyframeInput::~CREKeyframeInput() { delete x30_; }

bool CREKeyframeInput::GetValue(int frame, float& valOut) const {
  if (mPercent == 2) {
    float in = 0.0f;
    x30_->GetValue(frame, in);
    int idx = GetKeyframeIndex(GetKeyframeTime(in, x18_, x1c_), mLoop, GetLoopStart(), GetLoopEnd());
    bool lerp = idx > 0 && idx < mLoopEnd - 1;
    if (lerp) {
      float t = CMath::Clamp(0.0f, (in - x18_) - static_cast< float >(idx) / x1c_, 1.0f);
      valOut = (1.0f - t) * mKeys[idx] + t * mKeys[idx + 1];
    } else {
      valOut = mKeys[idx];
    }
  }
  return false;
}

bool CREParticleAccessParameter1::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[0];
  return false;
}

bool CREParticleAccessParameter2::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[1];
  return false;
}

bool CREParticleAccessParameter3::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[2];
  return false;
}

bool CREParticleAccessParameter4::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[3];
  return false;
}

bool CREParticleAccessParameter5::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[4];
  return false;
}

bool CREParticleAccessParameter6::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[5];
  return false;
}

bool CREParticleAccessParameter7::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[6];
  return false;
}

bool CREParticleAccessParameter8::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[7];
  return false;
}

bool CREParticleAccessParameter9::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[8];
  return false;
}

bool CREParticleSizeOrLineLength::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticle()->mLineLengthOrSize;
  return false;
}

bool CREParticleRotationOrLineWidth::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticle()->mLineWidthOrRota;
  return false;
}

CREVectorXToReal::CREVectorXToReal(CVectorElement* a) : mA(a) {}

CREVectorXToReal::~CREVectorXToReal() { delete mA; }

bool CREVectorXToReal::GetValue(int frame, float& valOut) const {
  CVector3f a = CVector3f::Zero();
  mA->GetValue(frame, a);
  valOut = a[0];
  return false;
}

CREVectorYToReal::CREVectorYToReal(CVectorElement* a) : mA(a) {}

CREVectorYToReal::~CREVectorYToReal() { delete mA; }

bool CREVectorYToReal::GetValue(int frame, float& valOut) const {
  CVector3f a = CVector3f::Zero();
  mA->GetValue(frame, a);
  valOut = a[1];
  return false;
}

CREVectorZToReal::CREVectorZToReal(CVectorElement* a) : mA(a) {}

CREVectorZToReal::~CREVectorZToReal() { delete mA; }

bool CREVectorZToReal::GetValue(int frame, float& valOut) const {
  CVector3f a = CVector3f::Zero();
  mA->GetValue(frame, a);
  valOut = a[2];
  return false;
}

CREVectorMagnitude::CREVectorMagnitude(CVectorElement* a) : mA(a) {}

CREVectorMagnitude::~CREVectorMagnitude() { delete mA; }

bool CREVectorMagnitude::GetValue(int frame, float& valOut) const {
  CVector3f a = CVector3f::Zero();
  mA->GetValue(frame, a);
  valOut = a.Magnitude();
  return false;
}

CREInitialSwitch::CREInitialSwitch(CRealElement* a, CRealElement* b) : mA(a), mB(b) {}

CREInitialSwitch::~CREInitialSwitch() {
  delete mA;
  delete mB;
}

bool CREInitialSwitch::GetValue(int frame, float& valOut) const {
  if (frame == 0) {
    mA->GetValue(0, valOut);
  } else {
    mB->GetValue(frame - 1, valOut);
  }
  return false;
}

CRECompareLessThan::CRECompareLessThan(CRealElement* a, CRealElement* b, CRealElement* c,
                                       CRealElement* d)
: mA(a), mB(b), mC(c), mD(d) {}

CRECompareLessThan::~CRECompareLessThan() {
  delete mA;
  delete mB;
  delete mC;
  delete mD;
}

bool CRECompareLessThan::GetValue(int frame, float& valOut) const {
  float a, b;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  if (a < b) {
    mC->GetValue(frame, valOut);
  } else {
    mD->GetValue(frame, valOut);
  }
  return false;
}

CRECompareEqual::CRECompareEqual(CRealElement* a, CRealElement* b, CRealElement* c, CRealElement* d)
: mA(a), mB(b), mC(c), mD(d) {}

CRECompareEqual::~CRECompareEqual() {
  delete mA;
  delete mB;
  delete mC;
  delete mD;
}

bool CRECompareEqual::GetValue(int frame, float& valOut) const {
  float a, b;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  if (close_enough(a, b)) {
    mC->GetValue(frame, valOut);
  } else {
    mD->GetValue(frame, valOut);
  }
  return false;
}

CREConstantRange::CREConstantRange(CRealElement* a, CRealElement* b, CRealElement* c,
                                   CRealElement* d, CRealElement* e)
: mVal(a), mMin(b), mMax(c), mInRange(d), mOutOfRange(e) {}

CREConstantRange::~CREConstantRange() {
  delete mVal;
  delete mMin;
  delete mMax;
  delete mInRange;
  delete mOutOfRange;
}

bool CREConstantRange::GetValue(int frame, float& valOut) const {
  float val, min, max;
  mVal->GetValue(frame, val);
  mMin->GetValue(frame, min);
  mMax->GetValue(frame, max);
  if (val > min && val < max) {
    mInRange->GetValue(frame, valOut);
  } else {
    mOutOfRange->GetValue(frame, valOut);
  }
  return false;
}

CREExternalVar::CREExternalVar(CIntElement* a) : mA(a) {}

CREExternalVar::~CREExternalVar() { delete mA; }

bool CREExternalVar::GetValue(int frame, float& valOut) const {
  int a = 0;
  mA->GetValue(frame, a);
  a = rstl::max_val(0, a);
  a %= 16;
  valOut = static_cast< CElementGen* >(CParticleGlobals::GetCurrentParticleSystem()->mSystem)
               ->GetExternalVar(a);
  return false;
}

CRESubtract::CRESubtract(CRealElement* a, CRealElement* b) : mA(a), mB(b) {}

CRESubtract::~CRESubtract() {
  delete mA;
  delete mB;
}

bool CRESubtract::GetValue(int frame, float& valOut) const {
  float a = 0.f, b = 0.f;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  valOut = a - b;
  return false;
}

CREGetComponentRed::CREGetComponentRed(CColorElement* a) : mA(a) {}

CREGetComponentRed::~CREGetComponentRed() { delete mA; }

bool CREGetComponentRed::GetValue(int frame, float& valOut) const {
  CColor color = CColor::Black();
  mA->GetValue(frame, color);
  valOut = color.GetRed();
  return false;
}

CREGetComponentGreen::CREGetComponentGreen(CColorElement* a) : mA(a) {}

CREGetComponentGreen::~CREGetComponentGreen() { delete mA; }

bool CREGetComponentGreen::GetValue(int frame, float& valOut) const {
  CColor color = CColor::Black();
  mA->GetValue(frame, color);
  valOut = color.GetGreen();
  return false;
}

CREGetComponentBlue::CREGetComponentBlue(CColorElement* a) : mA(a) {}

CREGetComponentBlue::~CREGetComponentBlue() { delete mA; }

bool CREGetComponentBlue::GetValue(int frame, float& valOut) const {
  CColor color = CColor::Black();
  mA->GetValue(frame, color);
  valOut = color.GetBlue();
  return false;
}

CREGetComponentAlpha::CREGetComponentAlpha(CColorElement* a) : mA(a) {}

CREGetComponentAlpha::~CREGetComponentAlpha() { delete mA; }

bool CREGetComponentAlpha::GetValue(int frame, float& valOut) const {
  CColor color = CColor::Black();
  mA->GetValue(frame, color);
  valOut = color.GetAlpha();
  return false;
}

CREIntTimesReal::CREIntTimesReal(CIntElement* a, CRealElement* b) : mA(a), mB(b) {}

CREIntTimesReal::~CREIntTimesReal() {
  delete mA;
  delete mB;
}

bool CREIntTimesReal::GetValue(int frame, float& valOut) const {
  int a = 0;
  float b = 1.f;
  mB->GetValue(frame, b);
  mA->GetValue(frame, a);
  valOut = b * static_cast< float >(a);
  return false;
}

bool CREGetCumulativeParticleCount::GetValue(int frame, float& valOut) const {
  CParticleGlobals::SParticleSystem* system = CParticleGlobals::GetCurrentParticleSystem();
  FourCC type = system == nullptr ? 'NULL' : system->mType;
  switch (type) {
  case 'PART':
    valOut = static_cast< CElementGen* >(system->mSystem)->GetCumulativeParticleCount();
    break;
  case 'ELSC':
    valOut = static_cast< CParticleElectric* >(system->mSystem)->GetCumulativeParticleCount();
    break;
  default:
    valOut = 0.f;
    break;
  }
  return false;
}

CREKeepInitial::CREKeepInitial(CRealElement* a) : x4_(a) {}

CREKeepInitial::~CREKeepInitial() { delete x4_; }

bool CREKeepInitial::GetValue(int frame, float& valOut) const {
  if (frame == 0) {
    x4_->GetValue(0, valOut);
  }
  return false;
}

CREOscillatingSweep::CREOscillatingSweep(CIntElement* a) : x4_(a) {}

CREOscillatingSweep::~CREOscillatingSweep() { delete x4_; }

bool CREOscillatingSweep::GetValue(int frame, float& valOut) const {
  int count = static_cast< CElementGen* >(CParticleGlobals::GetCurrentParticleSystem()->mSystem)
                  ->GetCumulativeParticleCount();
  int period;
  x4_->GetValue(frame, period);
  int t = count % (period * 2);
  if (t < period) {
    valOut = t;
  } else {
    valOut = period - t % period - 1;
  }
  return false;
}

CRETimeOscillatingSweep::CRETimeOscillatingSweep(const bool a, CIntElement* b, CIntElement* c, CIntElement* d)
: x4_(b), x8_(c), xc_(d), x10_(a), x14_(0), x18_(0), x1c_(0), x20_(0), x24_(0), x28_(-1) {
  if (x4_) {
    x4_->GetValue(0, x14_);
  }
  if (x8_) {
    x8_->GetValue(0, x18_);
  }
  if (x1c_) {
    xc_->GetValue(0, x1c_);
  }
}

CRETimeOscillatingSweep::~CRETimeOscillatingSweep() {
  delete x4_;
  delete x8_;
  delete xc_;
}

bool CRETimeOscillatingSweep::GetValue(int frame, float& valOut) const {
  if (x28_ != frame) {
    x28_ = frame;
    if (x20_ > 0) {
      --x20_;
      return false;
    }

    int cycle = x24_ / x14_;
    int t = x24_ % x14_;
    if (t == 0) {
      bool odd = cycle & 1;
      CIntElement* elem = odd ? xc_ : x8_;
      x20_ = odd ? x1c_ : x18_;
      elem->GetValue(x24_, x20_);
      x20_ = rstl::max_val(x20_ - 1, 0);
    } else if (x10_) {
      int m = x24_ % (x14_ * 2);
      if (m < x14_) {
        valOut = m;
      } else {
        valOut = x14_ - m % x14_;
      }
    } else {
      valOut = t;
    }
    ++x24_;
  }
  return false;
}

CREPerlinNoise1d::CREPerlinNoise1d(CRealElement* a) : x4_(a) {}

CREPerlinNoise1d::~CREPerlinNoise1d() { delete x4_; }

bool CREPerlinNoise1d::GetValue(int frame, float& valOut) const {
  float x = 0.f;
  x4_->GetValue(frame, x);
  valOut = CMath::Noise1d(x);
  return false;
}

CREPerlinNoise2d::CREPerlinNoise2d(CRealElement* a, CRealElement* b) : x4_(a), x8_(b) {}

CREPerlinNoise2d::~CREPerlinNoise2d() {
  delete x4_;
  delete x8_;
}

bool CREPerlinNoise2d::GetValue(int frame, float& valOut) const {
  float x = 0.f;
  x4_->GetValue(frame, x);
  float y = 0.f;
  x8_->GetValue(frame, y);
  valOut = CMath::Noise2d(x, y);
  return false;
}

CREPerlinNoise3d::CREPerlinNoise3d(CVectorElement* a) : x4_(a) {}

CREPerlinNoise3d::~CREPerlinNoise3d() { delete x4_; }

bool CREPerlinNoise3d::GetValue(int frame, float& valOut) const {
  CVector3f v = CVector3f::Zero();
  x4_->GetValue(frame, v);
  valOut = CMath::Noise3d(v.GetX(), v.GetY(), v.GetZ());
  return false;
}

CREPerlinNoise4d::CREPerlinNoise4d(CVectorElement* a, CRealElement* b) : x4_(a), x8_(b) {}

CREPerlinNoise4d::~CREPerlinNoise4d() {
  delete x4_;
  delete x8_;
}

bool CREPerlinNoise4d::GetValue(int frame, float& valOut) const {
  CVector3f v = CVector3f::Zero();
  x4_->GetValue(frame, v);
  float w = 0.f;
  x8_->GetValue(frame, w);
  valOut = CMath::Noise4d(v.GetX(), v.GetY(), v.GetZ(), w);
  return false;
}

CREPerlinNoiseOctave1d::CREPerlinNoiseOctave1d(CRealElement* a, CRealElement* b, CRealElement* c, CIntElement* d)
: x4_(a), x8_(b), xc_(c), x10_(d) {}

CREPerlinNoiseOctave1d::~CREPerlinNoiseOctave1d() {
  delete x4_;
  delete x8_;
  delete xc_;
  delete x10_;
}

bool CREPerlinNoiseOctave1d::GetValue(int frame, float& valOut) const {
  float x = 0.f;
  x4_->GetValue(frame, x);
  float freq = 1.f;
  x8_->GetValue(frame, freq);
  float persistence = 1.f;
  xc_->GetValue(frame, persistence);
  int octaves = 1;
  x10_->GetValue(frame, octaves);

  valOut = 0.f;
  float amp = 1.f;
  for (int i = 0; i < octaves; ++i) {
    valOut += amp * CMath::Noise1d(x * freq);
    freq *= 2.f;
    amp *= persistence;
  }
  return false;
}

CREPerlinNoiseOctave2d::CREPerlinNoiseOctave2d(CRealElement* a, CRealElement* b, CRealElement* c, CRealElement* d,
                 CIntElement* e)
: x4_(a), x8_(b), xc_(c), x10_(d), x14_(e) {}

CREPerlinNoiseOctave2d::~CREPerlinNoiseOctave2d() {
  delete x4_;
  delete x8_;
  delete xc_;
  delete x10_;
  delete x14_;
}

bool CREPerlinNoiseOctave2d::GetValue(int frame, float& valOut) const {
  float x = 0.f;
  x4_->GetValue(frame, x);
  float y = 0.f;
  x8_->GetValue(frame, y);
  float freq = 1.f;
  xc_->GetValue(frame, freq);
  float persistence = 1.f;
  x10_->GetValue(frame, persistence);
  int octaves = 1;
  x14_->GetValue(frame, octaves);

  valOut = 0.f;
  float amp = 1.f;
  for (int i = 0; i < octaves; ++i) {
    valOut += amp * CMath::Noise2d(x * freq, y * freq);
    freq *= 2.f;
    amp *= persistence;
  }
  return false;
}

CREPerlinNoiseOctave3d::CREPerlinNoiseOctave3d(CVectorElement* a, CRealElement* b, CRealElement* c, CIntElement* d)
: x4_(a), x8_(b), xc_(c), x10_(d) {}

CREPerlinNoiseOctave3d::~CREPerlinNoiseOctave3d() {
  delete x4_;
  delete x8_;
  delete xc_;
  delete x10_;
}

bool CREPerlinNoiseOctave3d::GetValue(int frame, float& valOut) const {
  CVector3f v = CVector3f::Zero();
  x4_->GetValue(frame, v);
  float freq = 1.f;
  x8_->GetValue(frame, freq);
  float persistence = 1.f;
  xc_->GetValue(frame, persistence);
  int octaves = 1;
  x10_->GetValue(frame, octaves);

  valOut = 0.f;
  float amp = 1.f;
  for (int i = 0; i < octaves; ++i) {
    valOut += amp * CMath::Noise3d(freq * v.GetX(), freq * v.GetY(), freq * v.GetZ());
    freq *= 2.f;
    amp *= persistence;
  }
  return false;
}

CREPerlinNoiseOctave4d::CREPerlinNoiseOctave4d(CVectorElement* a, CRealElement* b, CRealElement* c, CRealElement* d,
                 CIntElement* e)
: x4_(a), x8_(b), xc_(c), x10_(d), x14_(e) {}

CREPerlinNoiseOctave4d::~CREPerlinNoiseOctave4d() {
  delete x4_;
  delete x8_;
  delete xc_;
  delete x10_;
  delete x14_;
}

bool CREPerlinNoiseOctave4d::GetValue(int frame, float& valOut) const {
  CVector3f v = CVector3f::Zero();
  x4_->GetValue(frame, v);
  float w = 0.f;
  x8_->GetValue(frame, w);
  float freq = 1.f;
  xc_->GetValue(frame, freq);
  float persistence = 1.f;
  x10_->GetValue(frame, persistence);
  int octaves = 1;
  x14_->GetValue(frame, octaves);

  valOut = 0.f;
  float amp = 1.f;
  for (int i = 0; i < octaves; ++i) {
    valOut += amp * CMath::Noise4d(freq * v.GetX(), freq * v.GetY(), freq * v.GetZ(), w);
    freq *= 2.f;
    amp *= persistence;
  }
  return false;
}
