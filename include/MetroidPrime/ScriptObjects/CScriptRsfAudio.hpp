#ifndef _CSCRIPTRSFAUDIO
#define _CSCRIPTRSFAUDIO

#include "MetroidPrime/CEntity.hpp"

#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

#include "Kyoto/Audio/DolphinCRSFAudio.hpp"

// Guessed names throughout; the class name follows the RSFA loader and SLdrRsfAudio.
class CScriptRsfAudio : public CEntity {
public:
  CScriptRsfAudio(TUniqueId uid, const CEntityInfo& info, const rstl::string& name,
                  const rstl::string& fileName, int loopStart, int loopEnd, float fadeInTime,
                  float fadeOutTime, int volume);

  // CEntity
  ~CScriptRsfAudio() override {}
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

private:
  void Load(CStateManager& mgr);
  void Unload(CStateManager& mgr);
  void Play(CStateManager& mgr);
  void Stop(CStateManager& mgr);
  void StartFade(bool fadeIn);
  void SetCurrentVolume(float volume);

  rstl::string mFileName;
  float mFadeInTime;
  float mFadeOutTime;
  int mVolume;
  int mLoopStart;
  int mLoopEnd;
  float mMusicVolume;
  float mTargetMusicVolume;
  float mMusicVolumeRate;
  rstl::single_ptr< CRSFAudio > mPlayer;
  float mCurrentVolume;
  float mFadeTimeLeft;
  bool mLoadRequested : 1;
  bool mLoaded : 1;
  bool mFadingIn : 1;
  bool mFadingOut : 1;
  bool mPlaying : 1;
};
CHECK_SIZEOF(CScriptRsfAudio, 0x64)

#endif // _CSCRIPTRSFAUDIO
