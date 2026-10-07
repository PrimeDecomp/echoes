#ifndef _CSCRIPTGUISLIDER
#define _CSCRIPTGUISLIDER

#include "MetroidPrime/ScriptObjects/CScriptGuiWidget.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CScriptGuiSlider : public CScriptGuiWidget {
public:
  // Guessed names.
  enum ESlideState {
    kSS_Idle,
    kSS_Decreasing,
    kSS_Increasing,
  };

  CScriptGuiSlider(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, int controller,
                   const rstl::string& label, bool locked, float minValue, float maxValue,
                   float increment, float slideSpeed, ushort slideSfx, int slideSfxVolume);
  ~CScriptGuiSlider() override;

  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // Guessed names.
  void UpdatePlatforms(CStateManager& mgr);
  void FindPlatforms(CStateManager& mgr);
  void SetSecondaryValue(CStateManager& mgr, float value, float rangeMin, float rangeMax);
  void SetValue(CStateManager& mgr, float value, float rangeMin, float rangeMax);
  float GetValue(float rangeMin, float rangeMax) const;
  int GetRoundedValue(float rangeMin, float rangeMax) const;
  void StartDecrease(CStateManager& mgr);
  void StartIncrease(CStateManager& mgr);

private:
  // Guessed names.
  float mMinValue;
  float mMaxValue;
  float mIncrement;
  float mSlideSpeed;
  float mTargetValue;
  float mValue;
  float mSecondaryValue;
  TUniqueId mPrimaryPlatform;
  TUniqueId mSecondaryPlatform;
  int mSlideState;
  bool mSlideRequested : 1;
  CSfxHandle mSlideSfxHandle;
  ushort mSlideSfx;
  int mSlideSfxVolume;
};
CHECK_SIZEOF(CScriptGuiSlider, 0xa0)

#endif // _CSCRIPTGUISLIDER
