#ifndef _CSCRIPTSOUND
#define _CSCRIPTSOUND

#include "MetroidPrime/CActor.hpp"

class CScriptSound : public CActor {
public:
  CScriptSound(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, ushort soundId, float maxDist, float distComp,
               float startDelay, short minVolume, short volume, short unknown198,
               short darkVisorVolume, short priority, short pan, short surroundPan,
               short unknown1a2, bool looped, bool nonEmitter, bool playerRelativePan,
               bool autoStart, bool occlusionTest, bool acoustics, bool worldSfx,
               bool allowDuplicates, bool allAreas, bool scaleByMusicVolume, int pitch);

  // CEntity
  ~CScriptSound() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  void PlaySound(CStateManager& mgr, const CScriptMsg* msg);
  void StopSound(CStateManager& mgr);
  void SetMaxVolume(short volume);
  ushort GetSoundId() const { return mSoundId; }
  static float GetOccludedVolumeAmount(const CVector3f& pos, const CStateManager& mgr);

private:
  static bool sFirstInFrame;
  rstl::vector< TUniqueId > mPositionSources;
  float mOcclusionUpdateTimer;
  CSfxHandle mSfxHandle;
  short mMaxVolume;
  short mCurrentMaxVolume;
  short mVolumeDelta;
  CVector3f mEmitterPosition;
  float mStartDelay;
  ushort mSoundId;
  float mMaxDistance;
  float mDistanceCompensation;
  short mMinVolume;
  short mVolume;
  short x198_;
  short mDarkVisorVolume;
  short mPriority;
  short mPan;
  short mSurroundPan;
  short x1a2_;
  int mPitch;
  bool mPlayRequested : 1;
  bool mLooped : 1;
  bool mNonEmitter : 1;
  bool mAutoStart : 1;
  bool mOcclusionTest : 1;
  bool mAcoustics : 1;
  bool mWorldSfx : 1;
  bool mSelfFree : 1;
  bool mAllowDuplicates : 1;
  bool mProcessedThisFrame : 1;
  bool mPlayerRelativePan : 1;
  bool x1a9_27_ : 1;
  bool mAllAreas : 1;
  bool mScaleByMusicVolume : 1;
};
CHECK_SIZEOF(CScriptSound, 0x1b0)

#endif // _CSCRIPTSOUND
