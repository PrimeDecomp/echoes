#ifndef _CSCRIPTSOUNDMODIFIER
#define _CSCRIPTSOUNDMODIFIER

#include "Kyoto/Math/CMayaSpline.hpp"
#include "MetroidPrime/CEntity.hpp"

// Guessed name; modulates connected sounds using four serialized splines.
class CScriptSoundModifier : public CEntity {
public:
  CScriptSoundModifier(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       float duration, bool autoReset, bool autoStart, const CMayaSpline& volume,
                       const CMayaSpline& pan, const CMayaSpline& surroundPan,
                       const CMayaSpline& pitch);

  // CEntity
  ~CScriptSoundModifier() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // Guessed names.
  uchar GetCurrentVolume();
  ushort GetCurrentPitch();
  uchar GetCurrentPan();
  uchar GetCurrentSurroundPan();

private:
  rstl::vector< TUniqueId > mSounds;
  CMayaSpline mVolume;
  CMayaSpline mPan;
  CMayaSpline mSurroundPan;
  CMayaSpline mPitch;
  float mElapsedTime;
  float mDuration;
  bool mAutoReset : 1;
  bool mAutoStart : 1;
  bool mRunning : 1;
};
CHECK_SIZEOF(CScriptSoundModifier, 0x150)

#endif // _CSCRIPTSOUNDMODIFIER
