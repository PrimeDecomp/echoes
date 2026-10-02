#ifndef _CSCRIPTSTEAM
#define _CSCRIPTSTEAM

#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"

// Guessed name, correlated with Prime and the native STEM loader.
class CScriptSteam : public CScriptTrigger {
public:
  CScriptSteam(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CVector3f& position, const CAABox& bounds, const CDamageInfo& damage,
               const CVector3f& forceField, uint flags, CAssetId texture, float strength,
               float fadeInRate, float fadeOutRate, float radius, bool splash);

  // CEntity
  ~CScriptSteam() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

private:
  float GetStrength() const { return mStrength; }

  bool mEnableSplash; // Guessed name: writes the environment splash rate.
  CAssetId mTexture;
  float mStrength;
  float mAlphaInDuration;
  float mAlphaOutDuration;
  float mMaxDistance;
  float mInverseMaxDistance;
};
CHECK_SIZEOF(CScriptSteam, 0x1e8)

#endif // _CSCRIPTSTEAM
