#ifndef _CSCRIPTSTREAMEDMUSIC
#define _CSCRIPTSTREAMEDMUSIC

#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Streams/CFilePreload.hpp"
#include "MetroidPrime/CEntity.hpp"

#include "rstl/optional_object.hpp"

class CScriptStreamedMusic : public CEntity {
public:
  CScriptStreamedMusic(TUniqueId id, const CEntityInfo& info, const rstl::string& name,
                       const rstl::string& fileName, bool noStopOnDeactivate, float fadeIn,
                       float fadeOut, uint volume, bool loop, bool music);
  ~CScriptStreamedMusic() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;
  void PlayAudio();

  static bool IsDSPFile(const rstl::string& fileName);

private:
  static CStreamAudioManager::ESoftwareChannel IsOneShot(bool loop);
  void StartStream();
  void StopStream();
  void StopNonDsp();
  void PlayNonDsp();
  void Play(CStateManager& mgr);
  void Stop();
  void TweakOverride(CStateManager& mgr);
  void PreloadMemoryAudio();
  void SetStereoPair();

  rstl::string mFileName;
  bool mNoStopOnDeactivate : 1;
  bool mFileIsDsp : 1;
  bool mLoop : 1;
  bool mMusic : 1;
  bool mPreloadPending : 1;
  float mFadeIn;
  float mFadeOut;
  uint mVolume;
  rstl::optional_object< CFilePreload > mPreload;
};
CHECK_SIZEOF(CScriptStreamedMusic, 0x4C)

#endif // _CSCRIPTSTREAMEDMUSIC
