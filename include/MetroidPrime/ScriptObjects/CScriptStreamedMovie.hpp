#ifndef _CSCRIPTSTREAMEDMOVIE
#define _CSCRIPTSTREAMEDMOVIE

#include "MetroidPrime/CActor.hpp"

#include "Kyoto/Graphics/CMoviePlayer.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

// Guessed name: plays a THP movie from the disc while the script object is active.
class CScriptStreamedMovie : public CActor {
public:
  CScriptStreamedMovie(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CTransform4f& xf, const rstl::string& movieFile, bool loop,
                       bool videoFilterEnabled, float cacheLength, float fadeOutTime,
                       int whenToDraw, int volume, int volumeType);

  // CEntity
  ~CScriptStreamedMovie() override {}
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;

private:
  void LoadMovie(CStateManager& mgr);                    // Guessed name
  void PlayMovie(CStateManager& mgr);                    // Guessed name
  void StopMovie(CStateManager& mgr);                    // Guessed name
  void RewindMovie(CStateManager& mgr);                  // Guessed name
  void UnloadMovie(CStateManager& mgr);                  // Guessed name
  void UpdateVolume(CStateManager& mgr);                 // Guessed name
  void SendExternalTime(float time, CStateManager& mgr); // Guessed name

  rstl::string mMovieFile;                       // Guessed name
  int mWhenToDraw;                               // Guessed name
  int mVolume;                                   // Guessed name
  int mVolumeType;                               // Guessed name
  float mCacheLength;                            // Guessed name
  float mFadeOutTime;                            // Guessed name
  bool mLoop : 1;                                // Guessed name
  bool mVideoFilterEnabled : 1;                  // Guessed name
  bool mSkipping : 1;                            // Guessed name
  float mSkipTime;                               // Guessed name
  rstl::single_ptr< CMoviePlayer > mMoviePlayer; // Guessed name
};
CHECK_SIZEOF(CScriptStreamedMovie, 0x188)

#endif // _CSCRIPTSTREAMEDMOVIE
