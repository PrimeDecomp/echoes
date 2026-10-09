#ifndef _CSCRIPTAIHINT
#define _CSCRIPTAIHINT

#include "MetroidPrime/CActor.hpp"

class CScriptAIHint : public CActor {
public:
  // Guessed domain names; other serialized values remain unidentified.
  enum EHintType {
    kHT_Unknown0 = 0,
    kHT_Hop = 1,         // Guessed name
    kHT_SunlightHop = 2, // Guessed name
    kHT_BloggHint = 16,   // Guessed name
    kHT_SplinterPad = 18, // Guessed name
    kHT_ShadowDashPoint = 19, // Guessed name; DarkCommando shadow dash destinations.
    kHT_GrenadeLauncherRaisedAim = 23,
    kHT_SplinterAttackBlock = 24, // Guessed name
    kHT_SplinterHide = 25,        // Guessed name
  };

  CScriptAIHint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, EHintType hintType, float radius, float valueParm,
               float valueParm2, float valueParm3);

  // CEntity
  ~CScriptAIHint() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;

  EHintType GetHintType() const { return mHintType; }
  float GetRadius() const { return mRadius; }
  float GetValueParm() const;
  float GetValueParm2() const;
  void SetInUse(bool inUse);
  void SetTimeRemaining(float time) { mTimeRemaining = time; } // Guessed name
  bool GetInUse(TUniqueId uid) const;
  bool GetInUseIgnoreLock(TUniqueId uid) const;

private:
  EHintType mHintType;
  float mRadius;
  float mValueParm;
  float mValueParm2;
  float mValueParm3;
  bool mInUse : 1;
  TUniqueId mOccupant;
  float mTimeRemaining;
};
CHECK_SIZEOF(CScriptAIHint, 0x178)

#endif // _CSCRIPTAIHINT
