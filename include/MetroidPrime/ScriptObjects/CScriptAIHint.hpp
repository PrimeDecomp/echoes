#ifndef _CSCRIPTAIHINT
#define _CSCRIPTAIHINT

#include "MetroidPrime/CActor.hpp"

// Partial layout: only the AIHT loader's hint type and radius are known so far. The Wii SEL
// exports CScriptAIHint (GetInUse, SetInUse, GetValueParm, GetValueParm2).
class CScriptAIHint : public CActor {
public:
  int GetHintType() const { return mHintType; }
  float GetRadius() const { return mRadius; }

private:
  int mHintType;
  float mRadius;
};

#endif // _CSCRIPTAIHINT
