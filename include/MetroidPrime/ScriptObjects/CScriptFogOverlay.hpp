#ifndef _CSCRIPTFOGOVERLAY
#define _CSCRIPTFOGOVERLAY

#include "MetroidPrime/CActor.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

// Guessed name: drives the state manager's dark world cloud overlay.
class CScriptFogOverlay : public CActor {
public:
  // Guessed names: the script message that last started a fade.
  enum EFadeCommand { kFC_None, kFC_FadeUp, kFC_FadeDown, kFC_SnapLow, kFC_SnapHigh };

  CScriptFogOverlay(TUniqueId uid, const CEntityInfo& info, const rstl::string& name,
                    float fullAlpha, float fadeDownTime, float fadeUpTime, bool startFadedOut,
                    float ambientSpeed, float ambientSpeedTarget, float speedFadeUpTime,
                    float speedFadeDownTime, float scaleTarget, float scaleFadeUpTime,
                    float scaleFadeDownTime, const CColor& color, const CVector2f& ambientRadius,
                    const CVector3f& scrollVelocity);

  // CEntity
  ~CScriptFogOverlay() override {}
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;

private:
  float mFullAlpha;          // Guessed name
  float mFadeDownTime;       // Guessed name
  float mFadeUpTime;         // Guessed name
  CVector2f mAmbientRadius;  // Guessed name
  float mAmbientSpeed;       // Guessed name
  float mAmbientSpeedBase;   // Guessed name
  float mAmbientSpeedTarget; // Guessed name
  float mSpeedFadeDownTime;  // Guessed name
  float mSpeedFadeUpTime;    // Guessed name
  CColor mColor;             // Guessed name
  float mAmbientAngle;       // Guessed name
  CVector3f mScrollVelocity; // Guessed name
  float mScrollScale;        // Guessed name
  float mScrollScaleTarget;  // Guessed name
  float mScaleFadeDownTime;  // Guessed name
  float mScaleFadeUpTime;    // Guessed name
  uint mAlphaCommand : 2;    // Guessed name
  uint mSpeedCommand : 2;    // Guessed name
  uint mScaleCommand : 2;    // Guessed name
};
CHECK_SIZEOF(CScriptFogOverlay, 0x1a8)

#endif // _CSCRIPTFOGOVERLAY
