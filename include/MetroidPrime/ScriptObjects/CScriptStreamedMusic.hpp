#ifndef _CSCRIPTSTREAMEDMUSIC
#define _CSCRIPTSTREAMEDMUSIC

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
  static int IsOneShot(bool loop);
  void StartStream();
  void StopStream();
  void StopNonDsp();
  void PlayNonDsp();
  void Play(CStateManager& mgr);
  void Stop();
  void TweakOverride(CStateManager& mgr);
  void PreloadMemoryAudio();
  void SetStereoPair();

  rstl::string x24_fileName;
  bool x34_noStopOnDeactivate : 1;
  bool x34_fileIsDsp : 1;
  bool x34_loop : 1;
  bool x34_music : 1;
  bool x34_preloadPending : 1;
  float x38_fadeIn;
  float x3c_fadeOut;
  uint x40_volume;
  rstl::optional_object< CFilePreload > x44_preload;
};
CHECK_SIZEOF(CScriptStreamedMusic, 0x4C)

#endif // _CSCRIPTSTREAMEDMUSIC
