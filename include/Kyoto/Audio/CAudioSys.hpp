#ifndef _CAUDIOSYS
#define _CAUDIOSYS

#include "types.h"

#include "rstl/map.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

#include "Kyoto/Math/CVector3f.hpp"

#include "dolphin/dtk.h"
#include "musyx/musyx.h"

enum ETRKSampleRate {
  kTSR_Zero,
  kTSR_One,
};

enum ETRKRepeatMode {
  kTRM_NoRepeat,
  kTRM_RepeatOne,
  kTRM_RepeatAll,
};

enum ETRKPlayState {
  kTPS_Stopped,
  kTPS_Playing,
  kTPS_Paused,
  kTPS_Busy,
};

class CAudioSys {
  static const rstl::string mpDefaultInvalidString;

public:
  enum ESurroundModes { kSM_Mono, kSM_Stereo, kSM_Surround };

  class CEmitterData {
  public:
    CEmitterData() : mUsed(false), mImportant(false), mPriority(kEmitterMedPriority) {}

    SND_EMITTER mEmitter;
    bool mUsed;
    bool mImportant;
    uchar mPriority;
  };

  struct C3DEmitterParmData {
    C3DEmitterParmData(const float maxDist = 150.f, const float distComp = 0.1f,
                       const uint flags = 1, const uchar maxVol = 127, const uchar minVol = 0)
    : mMaxDist(maxDist)
    , mDistComp(distComp)
    , mFlags(flags)
    , mSfxId(0)
    , mMaxVol(maxVol)
    , mMinVol(minVol)
    , mImportant(false)
    , mPrio(127) {}

    CVector3f mPos;
    CVector3f mDir;
    float mMaxDist;
    float mDistComp;
    uint mFlags;
    ushort mSfxId;
    uchar mMaxVol;
    uchar mMinVol;
    bool mImportant; // Can't be allocated over, regardless of priority
    uchar mPrio;
    uchar mStudio;
  };

  class CTrkData {
  public:
    CTrkData(const rstl::string& name) : mInUse(true), mName(name) {}

    bool GetIsTrackInUse() const { return mInUse; }
    void SetIsTrackInUse(bool v) { mInUse = v; }
    DTKTrack* GetTrack() { return &mTrack; }
    char* GetFileName() { return const_cast< char* >(mName.data()); }

    DTKTrack mTrack;
    bool mInUse;
    rstl::string mName;
  };

  CAudioSys(uchar numVoices, uchar numMusic, uchar numSfx, uchar maxNumEmitters, uint aramSize);
  ~CAudioSys();

  static void SysSetVolume(uchar volume, uint time, uchar group);
  static void SysSetSfxVolume(uchar volume, ushort time, uchar music, uchar fx);

  static SND_VOICEID SfxStart(ushort sfxId, uchar vol, uchar pan, uchar prio);
  static void SfxStop(SND_VOICEID handle);
  static SND_VOICEID SfxCheck(SND_VOICEID handle);
  static void SfxPan(SND_VOICEID handle, uchar pan);
  static void SfxSpan(SND_VOICEID handle, uchar span);
  static void SfxVolume(SND_VOICEID handle, uchar vol);
  static void SfxPitchBend(SND_VOICEID handle, ushort pitch);
  static void SfxCtrl(SND_VOICEID handle, uchar ctrl, uchar val);
  static bool SfxSetFilter(SND_VOICEID handle, uint filter, uint param);

  static int TrkQueueTrack(const rstl::string& name, void (*callback)(u32), uint eventMask);
  static void TrkRemoveTrack(const rstl::string& name);
  static void TrkFlushTracks();
  static void TrkSetSampleRate(ETRKSampleRate rate);
  static void TrkSetRepeatMode(ETRKRepeatMode mode);
  static void TrkSetState(ETRKPlayState state);
  static ETRKPlayState TrkGetState();
  static void TrkSetVolume(uchar left, uchar right);
  static void TrkNextTrack();
  static rstl::ncrc_ptr< CTrkData > FindTrack(const rstl::string& name);

  static void S3dAddListener(const CVector3f& pos, const CVector3f& dir, const CVector3f& heading,
                             const CVector3f& up, float frontSur, float backSur,
                             float soundSpeed, uint flags, uchar volume);
  static bool S3dUpdateListener(const CVector3f& pos, const CVector3f& dir,
                                const CVector3f& heading, const CVector3f& up, uchar volume);
  static bool S3dRemoveListener();
  static uint S3dAddEmitterParaEx(const C3DEmitterParmData& params, ushort groupId,
                                  SND_PARAMETER_INFO* paraInfo);
  static bool S3dUpdateEmitter(uint handle, const CVector3f& pos, const CVector3f& dir,
                               uchar maxVol);
  static bool S3dRemoveEmitter(uint handle);
  static void S3dFlushAllEmitters();
  static void S3dFlushUnusedEmitters();
  static bool S3dCheckEmitter(uint handle);
  static uint S3dEmitterVoiceID(uint handle);
  static uint S3dFindUnusedHandle();
  static uint S3dFindLowerPriorityHandle(uint prio);

  static SND_SEQID SeqPlayEx(ushort gid, ushort sid, void* arrfile, SND_PLAYPARA* para,
                             uchar studio);
  static void SeqStop(SND_SEQID seqId);
  static void SeqVolume(uchar volume, ushort time, SND_SEQID seqId, uchar mode);

  static void SetStereoMode(bool mode);
  static ESurroundModes GetSurroundMode();
  static void SetSurroundMode(ESurroundModes mode);
  static void SetVolumeScale(short scale);
  static void SetDefaultVolumeScale(short scale);
  static short GetDefaultVolumeScale();
  static void EnableAICallback(bool enable);
  static bool IsAICallbackEnabled();

  static short GetScaledVolume(int vol) { return kVolumeTable[vol]; }
  static const ushort kVolumeTable[];

  static bool mInitialized;
  static bool mIsListenerActive;
  static uchar mMaxNumEmitters;
  static rstl::map< rstl::string, rstl::ncrc_ptr< CTrkData > >* mpDVDTrackDB;
  static rstl::vector< CEmitterData >* mpEmitterDB;
  static SND_LISTENER* mpListener;
  static uint mUnusedEmitterHandle;
  static ESurroundModes mSurroundMode;
  static uint mMaxAramUsage;
  static void* mAICallback;
  static bool mProLogic2;
  static short mVolumeScale;
  static short mDefaultVolumeScale;
  static bool mAICallbackEnabled;
  static const uchar kMaxVolume;
  static const uchar kEmitterMedPriority;
};

NESTED_CHECK_SIZEOF(CAudioSys, C3DEmitterParmData, 0x2c)

#endif // _CAUDIOSYS
