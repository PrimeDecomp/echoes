#ifndef _CSCRIPTMIDI
#define _CSCRIPTMIDI

#include "MetroidPrime/CEntity.hpp"

#include "Kyoto/Audio/CMidiManager.hpp"
#include "Kyoto/TToken.hpp"

// Guessed name, correlated with Prime's MIDI script object and the Echoes loader.
class CScriptMidi : public CEntity {
public:
  CScriptMidi(TUniqueId id, const CEntityInfo& info, const rstl::string& name, CAssetId song,
              float fadeIn, float fadeOut, int volume);

  // CEntity
  ~CScriptMidi() override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  void Play(CStateManager& mgr, float fadeTime);
  void Stop(CStateManager& mgr, float fadeTime);

private:
  TToken< CMidiManager::CMidiData > mSong;
  CSfxHandle mHandle;
  float mFadeInTime;
  float mFadeOutTime;
  short mVolume;

  void StopInternal(float fadeTime);
};
CHECK_SIZEOF(CScriptMidi, 0x3c)

#endif // _CSCRIPTMIDI
