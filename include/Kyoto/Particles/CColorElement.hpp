#ifndef _CCOLORELEMENT
#define _CCOLORELEMENT

#include "types.h"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Particles/IElement.hpp"

class CCEConstant : public CColorElement {
  CRealElement* mR;
  CRealElement* mG;
  CRealElement* mB;
  CRealElement* mA;

public:
  CCEConstant(CRealElement* r, CRealElement* g, CRealElement* b, CRealElement* a);
  ~CCEConstant() override;
  bool GetValue(int frame, CColor& colorOut) const override;
};

class CCEFastConstant : public CColorElement {
  CColor mVal;

public:
  CCEFastConstant(float r, float g, float b, float a);
  ~CCEFastConstant() override;
  bool GetValue(int frame, CColor& colorOut) const override;
};

class CCEFade : public CColorElement {
  CColorElement* mA;
  CColorElement* mB;
  CRealElement* mEndFrame;

public:
  CCEFade(CColorElement* a, CColorElement* b, CRealElement* end);
  ~CCEFade() override;
  bool GetValue(int frame, CColor& colorOut) const override;
};

class CCEFadeEnd : public CColorElement {
  CColorElement* mA;
  CColorElement* mB;
  CRealElement* mStartFrame;
  CRealElement* mEndFrame;

public:
  CCEFadeEnd(CColorElement* a, CColorElement* b, CRealElement* start, CRealElement* end);
  ~CCEFadeEnd() override;
  bool GetValue(int frame, CColor& colorOut) const override;
};

class CCETimeChain : public CColorElement {
  CColorElement* mA;
  CColorElement* mB;
  CIntElement* mSwFrame;

public:
  CCETimeChain(CColorElement* a, CColorElement* b, CIntElement* c);
  ~CCETimeChain() override;
  bool GetValue(int frame, CColor& colorOut) const override;
};

class CCEPulse : public CColorElement {
  CIntElement* mADuration;
  CIntElement* mBDuration;
  CColorElement* mAVal;
  CColorElement* mBVal;

public:
  CCEPulse(CIntElement* a, CIntElement* b, CColorElement* c, CColorElement* d);
  ~CCEPulse() override;
  bool GetValue(int frame, CColor& colorOut) const override;
};

class CCEKeyframeEmitter : public CColorElement {
  int mPercent;
  int mUnk1;
  bool mLoop;
  bool mUnk2;
  int mLoopEnd;
  int mLoopStart;
  rstl::vector< CColor > mKeys;

public:
  CCEKeyframeEmitter(CInputStream& in);
  ~CCEKeyframeEmitter() override;
  bool GetValue(int frame, CColor& valOut) const override;
};

class CCEParticleColor : public CColorElement {
public:
  ~CCEParticleColor() override {}
  bool GetValue(int frame, CColor& colorOut) const override;
};

// Echoes additions. FourCC-based class names are placeholders; descriptive names follow
// Prime/MP3 analogues. Member offsets come from the constructors.

class CCEInitialSwitch : public CColorElement {
  CColorElement* x4_;
  CColorElement* x8_;

public:
  CCEInitialSwitch(CColorElement* a, CColorElement* b);
  ~CCEInitialSwitch() override;
  bool GetValue(int frame, CColor& valOut) const override;
};

class CCEKEYF : public CColorElement {
  int mPercent;
  int mUnk1;
  bool mLoop;
  bool mUnk2;
  int mLoopEnd;
  int mLoopStart;
  float x18_;
  float x1c_;
  rstl::vector< CColor > mKeys;
  CRealElement* x30_;

public:
  CCEKEYF(CInputStream& in);
  ~CCEKEYF() override;
  bool GetValue(int frame, CColor& valOut) const override;
};

class CCEKPIN : public CColorElement {
  CColorElement* x4_;

public:
  CCEKPIN(CColorElement* a);
  ~CCEKPIN() override;
  bool GetValue(int frame, CColor& valOut) const override;
};

class CCEMDAO : public CColorElement {
  CColorElement* x4_;
  CRealElement* x8_;

public:
  CCEMDAO(CColorElement* a, CRealElement* b);
  ~CCEMDAO() override;
  bool GetValue(int frame, CColor& valOut) const override;
};

class CCEMultiply : public CColorElement {
  CColorElement* x4_;
  CColorElement* x8_;

public:
  CCEMultiply(CColorElement* a, CColorElement* b);
  ~CCEMultiply() override;
  bool GetValue(int frame, CColor& valOut) const override;
};

class CCEVRTC : public CColorElement {
  CVectorElement* x4_;
  CRealElement* x8_;

public:
  CCEVRTC(CVectorElement* a, CRealElement* b);
  ~CCEVRTC() override;
  bool GetValue(int frame, CColor& valOut) const override;
};

#endif // _CCOLORELEMENT
