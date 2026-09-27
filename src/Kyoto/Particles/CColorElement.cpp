#include "Kyoto/Particles/CColorElement.hpp"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
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

CCEConstant::CCEConstant(CRealElement* r, CRealElement* g, CRealElement* b, CRealElement* a)
: mR(r), mG(g), mB(b), mA(a) {}

CCEConstant::~CCEConstant() {
  delete mR;
  delete mG;
  delete mB;
  delete mA;
}

bool CCEConstant::GetValue(int frame, CColor& colorOut) const {
  float r, g, b, a;
  mR->GetValue(frame, r);
  r = CMath::Clamp(0.f, r, 1.f);
  mG->GetValue(frame, g);
  g = CMath::Clamp(0.f, g, 1.f);
  mB->GetValue(frame, b);
  b = CMath::Clamp(0.f, b, 1.f);
  mA->GetValue(frame, a);
  a = CMath::Clamp(0.f, a, 1.f);
  colorOut.Set(r, g, b, a);
  return false;
}

CCEFastConstant::CCEFastConstant(const float r, const float g, const float b, const float a) {
   float cr = CMath::Clamp(0.f, r, 1.f);
   float cg = CMath::Clamp(0.f, g, 1.f);
   float cb = CMath::Clamp(0.f, b, 1.f);
   float ca = CMath::Clamp(0.f, a, 1.f);
   mVal.Set(cr, cg, cb, ca);
}

CCEFastConstant::~CCEFastConstant() {}

bool CCEFastConstant::GetValue(int frame, CColor& colorOut) const {
  colorOut = mVal;
  return false;
}

CCEFade::CCEFade(CColorElement* a, CColorElement* b, CRealElement* end)
: mA(a), mB(b), mEndFrame(end) {}

CCEFade::~CCEFade() {
  delete mA;
  delete mB;
  delete mEndFrame;
}

bool CCEFade::GetValue(int frame, CColor& colorOut) const {
  float c;
  mEndFrame->GetValue(frame, c);

  float t = static_cast< float >(frame) * (1.f / c);
  if (t >= 1.f) {
    mB->GetValue(frame, colorOut);
  } else {
    CColor colA;
    CColor colB;
    mA->GetValue(frame, colA);
    mB->GetValue(frame, colB);

    float ar, ag, ab, aa;
    float br, bg, bb, ba;
    colA.Get(ar, ag, ab, aa);
    colB.Get(br, bg, bb, ba);

    float nt = 1.f - t;
    colorOut = CColor(ar * nt + br * t, ag * nt + bg * t, ab * nt + bb * t, aa * nt + ba * t);
  }
  return false;
}

CCEFadeEnd::CCEFadeEnd(CColorElement* a, CColorElement* b, CRealElement* start, CRealElement* end)
: mA(a), mB(b), mStartFrame(start), mEndFrame(end) {}

CCEFadeEnd::~CCEFadeEnd() {
  delete mA;
  delete mB;
  delete mStartFrame;
  delete mEndFrame;
}

bool CCEFadeEnd::GetValue(int frame, CColor& colorOut) const {
  float start;
  mStartFrame->GetValue(frame, start);

  float frameF = static_cast< float >(frame);
  if (frameF < start) {
    mA->GetValue(frame, colorOut);
    return false;
  }

  float end;
  mEndFrame->GetValue(frame, end);

  CColor colA;
  CColor colB;
  mA->GetValue(frame, colA);
  mB->GetValue(frame, colB);

  float ar, ag, ab, aa;
  float br, bg, bb, ba;
  colA.Get(ar, ag, ab, aa);
  colB.Get(br, bg, bb, ba);

  float t = rstl::max_val(0.f, rstl::min_val((frameF - start) / (end - start), 1.f));
  float nt = 1.f - t;
  colorOut = CColor(ar * nt + br * t, ag * nt + bg * t, ab * nt + bb * t, aa * nt + ba * t);
  return false;
}

CCETimeChain::CCETimeChain(CColorElement* a, CColorElement* b, CIntElement* c)
: mA(a), mB(b), mSwFrame(c) {}

CCETimeChain::~CCETimeChain() {
  delete mA;
  delete mB;
  delete mSwFrame;
}

bool CCETimeChain::GetValue(int frame, CColor& colorOut) const {
  int v;
  mSwFrame->GetValue(frame, v);
  if (frame < v) {
    return mA->GetValue(frame, colorOut);
  } else {
    return mB->GetValue(frame - v, colorOut);
  }
}

CCEPulse::CCEPulse(CIntElement* a, CIntElement* b, CColorElement* c, CColorElement* d)
: mADuration(a), mBDuration(b), mAVal(c), mBVal(d) {}

CCEPulse::~CCEPulse() {
  delete mADuration;
  delete mBDuration;
  delete mAVal;
  delete mBVal;
}

bool CCEPulse::GetValue(int frame, CColor& colorOut) const {
  int a, b;
  mADuration->GetValue(frame, a);
  mBDuration->GetValue(frame, b);
  int cv = a + b + 1;
  if (cv < 0) {
    cv = 1;
  }

  if (b >= 1) {
    if (frame % cv > a) {
      mBVal->GetValue(frame, colorOut);
    } else {
      mAVal->GetValue(frame, colorOut);
    }
  } else {
    mAVal->GetValue(frame, colorOut);
  }
  return false;
}

CCEKeyframeEmitter::CCEKeyframeEmitter(CInputStream& in)
: mPercent(in.ReadInt32())
, mUnk1(in.ReadInt32())
, mLoop(in.ReadBool())
, mUnk2(in.ReadBool())
, mLoopEnd(in.ReadInt32())
, mLoopStart(in.ReadInt32())
, mKeys(in) {
  if (mLoopStart >= mLoopEnd) {
    mLoopStart = 0;
  }
}

CCEKeyframeEmitter::~CCEKeyframeEmitter() {}

bool CCEKeyframeEmitter::GetValue(int frame, CColor& valOut) const {
  if (mPercent == 0) {
    int emitterTime =
        GetKeyframeIndex(CParticleGlobals::GetEmitterTime(), mLoop, mLoopStart, mLoopEnd);
    valOut = mKeys[emitterTime];
    return false;
  }

  if (CParticleGlobals::GetParticleLifetimePercentage() == 100) {
    valOut = mKeys[CParticleGlobals::GetParticleLifetimePercentage()];
  } else {
    valOut = CColor::Lerp(mKeys[CParticleGlobals::GetParticleLifetimePercentage()],
                          mKeys[CParticleGlobals::GetParticleLifetimePercentage() + 1],
                          CParticleGlobals::GetParticleLifetimePercentageRemainder());
  }
  return false;
}

CCEKEYF::CCEKEYF(CInputStream& in)
: mPercent(in.ReadInt32())
, mUnk1(in.ReadInt32())
, mLoop(in.ReadBool())
, mUnk2(in.ReadBool())
, mLoopEnd(in.ReadInt32())
, mLoopStart(in.ReadInt32())
, x18_(in.ReadFloat())
, x1c_(in.ReadFloat())
, mKeys(in)
, x30_(CParticleDataFactory::GetRealElement(in)) {
  if (mLoopStart >= mLoopEnd) {
    mLoopStart = 0;
  }
}

CCEKEYF::~CCEKEYF() { delete x30_; }

bool CCEKEYF::GetValue(int frame, CColor& valOut) const {
  if (mPercent == 2) {
    float in = 0.0f;
    x30_->GetValue(frame, in);
    int idx = GetKeyframeIndex(GetKeyframeTime(in, x18_, x1c_), mLoop, mLoopStart, mLoopEnd);
    bool lerp = idx > 0 && idx < mLoopEnd - 1;
    if (lerp) {
      float t = CMath::Clamp(0.0f, (in - x18_) - static_cast< float >(idx) / x1c_, 1.0f);
      valOut = CColor::Lerp(mKeys[idx], mKeys[idx + 1], t);
    } else {
      valOut = mKeys[idx];
    }
  }
  return false;
}

bool CCEParticleColor::GetValue(int frame, CColor& colorOut) const {
  colorOut = CParticleGlobals::GetCurrentParticle()->mColor;
  return false;
}

CCEMultiply::CCEMultiply(CColorElement* a, CColorElement* b) : x4_(a), x8_(b) {}

CCEMultiply::~CCEMultiply() {
  delete x4_;
  delete x8_;
}

bool CCEMultiply::GetValue(int frame, CColor& valOut) const {
  CColor a;
  CColor b;
  x4_->GetValue(frame, a);
  x8_->GetValue(frame, b);
  valOut = CColor::Modulate(a, b);
  return false;
}

CCEVRTC::CCEVRTC(CVectorElement* a, CRealElement* b) : x4_(a), x8_(b) {}

CCEVRTC::~CCEVRTC() {
  delete x4_;
  delete x8_;
}

bool CCEVRTC::GetValue(int frame, CColor& valOut) const {
  CVector3f rgb = CVector3f::Zero();
  float a = 1.f;
  x4_->GetValue(frame, rgb);
  x8_->GetValue(frame, a);
  valOut.Set(CMath::Clamp(0.f, rgb.GetX(), 1.f), CMath::Clamp(0.f, rgb.GetY(), 1.f),
             CMath::Clamp(0.f, rgb.GetZ(), 1.f), CMath::Clamp(0.f, a, 1.f));
  return false;
}

CCEInitialSwitch::CCEInitialSwitch(CColorElement* a, CColorElement* b) : x4_(a), x8_(b) {}

CCEInitialSwitch::~CCEInitialSwitch() {
  delete x4_;
  delete x8_;
}

bool CCEInitialSwitch::GetValue(int frame, CColor& valOut) const {
  if (frame == 0) {
    x4_->GetValue(0, valOut);
  } else {
    x8_->GetValue(frame - 1, valOut);
  }
  return false;
}

CCEKPIN::CCEKPIN(CColorElement* a) : x4_(a) {}

CCEKPIN::~CCEKPIN() { delete x4_; }

bool CCEKPIN::GetValue(int frame, CColor& valOut) const {
  if (frame == 0) {
    x4_->GetValue(0, valOut);
  }
  return false;
}

CCEMDAO::CCEMDAO(CColorElement* a, CRealElement* b) : x4_(a), x8_(b) {}

CCEMDAO::~CCEMDAO() {
  delete x4_;
  delete x8_;
}

bool CCEMDAO::GetValue(int frame, CColor& valOut) const {
  x4_->GetValue(frame, valOut);
  float a = 1.f;
  x8_->GetValue(frame, a);
  valOut.SetAlpha(CMath::Clamp(0.f, a, 1.f));
  return false;
}
