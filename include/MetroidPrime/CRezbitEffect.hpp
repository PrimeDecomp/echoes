#ifndef _CREZBITEFFECT
#define _CREZBITEFFECT

#include "MetroidPrime/CActor.hpp"

#include "rstl/rc_ptr.hpp"
#include "rstl/single_ptr.hpp"

class CIOWin;
class CElementGen;

// Original options class name; member names are reconstructed from consumers.
class CRezbitEffectOptions {
public:
  CRezbitEffectOptions(CAssetId particleEffect, ushort virusSound, ushort rebootSound,
                      float duration, float interferenceTimeLowerBound,
                      float interferenceEndTime, bool waitForRecovery);

  CAssetId GetParticleEffect() const { return mParticleEffect; }
  ushort GetVirusSound() const { return mVirusSound; }
  ushort GetRebootSound() const { return mRebootSound; }
  float GetDuration() const { return mDuration; }
  float GetInterferenceTimeLowerBound() const { return mInterferenceTimeLowerBound; }
  float GetInterferenceEndTime() const { return mInterferenceEndTime; }
  bool GetWaitForRecovery() const { return mWaitForRecovery; }

private:
  CAssetId mParticleEffect;
  ushort mVirusSound;
  ushort mRebootSound;
  float mDuration;
  float mInterferenceTimeLowerBound;
  float mInterferenceEndTime;
  bool mWaitForRecovery : 1;
};
CHECK_SIZEOF(CRezbitEffectOptions, 0x18)

// Guessed actor name, supported by the effect's native label and player consumers.
class CRezbitEffect : public CActor {
public:
  CRezbitEffect(TUniqueId uid, const CEntityInfo& info, const CRezbitEffectOptions& options);

  // CEntity
  ~CRezbitEffect() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;

private:
  void FinishEffect();
  static CElementGen* CreateParticleEffect(CAssetId asset);

  float mDuration;
  float mInterferenceTimeLowerBound;
  float mInterferenceEndTime;
  float mElapsedTime;
  int mInterferenceCooldown;
  rstl::ncrc_ptr< CIOWin > mIOWin;
  rstl::single_ptr< CElementGen > mParticleEffect;
  CRezbitEffectOptions mOptions;
  CSfxHandle mSound;
  bool mAwaitingRecovery : 1;
};
CHECK_SIZEOF(CRezbitEffect, 0x198)

#endif // _CREZBITEFFECT
