#ifndef _CSCRIPTACTORKEYFRAME
#define _CSCRIPTACTORKEYFRAME

#include "MetroidPrime/CEntity.hpp"

class CScriptActorKeyframe : public CEntity {
public:
  CScriptActorKeyframe(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       int animationId, bool looping, float lifetime, bool passive, uint modeFlags,
                       float playbackRate);

  // CEntity
  ~CScriptActorKeyframe() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  void UpdateEntity(TUniqueId uid, CStateManager& mgr);
  bool IsPassive() const { return mPassive; }
  void SetIsPassive(bool passive) { mPassive = passive; }

private:
  int mAnimationId;
  float mInitialLifetime;
  float mPlaybackRate;
  float mLifetime;
  bool mLooping : 1;
  bool mPassive : 1;
  bool mFadeOut : 1;
  bool mTimedLoop : 1;
  bool mPlaying : 1;
  bool mUseOriginator : 1;
};
CHECK_SIZEOF(CScriptActorKeyframe, 0x38)

#endif // _CSCRIPTACTORKEYFRAME
