#ifndef _CAUDIOSYS
#define _CAUDIOSYS

#include "types.h"

#include "rstl/map.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

#include "Kyoto/Math/CVector3f.hpp"

class CAudioGroupSet;
struct SND_PLAYPARA;

enum ETRKSampleRate {
  kTSR_Zero,
  kTSR_One,
  // TODO
};

enum ETRKRepeatMode {
  // TODO
};

class CAudioSys {
  static const rstl::string mpDefaultInvalidString;

public:
  enum ESurroundModes { kSM_Mono, kSM_Stereo, kSM_Surround };
  class CEmitterData {
  public:
    CEmitterData() : _50(0), _51(0), _52(kEmitterMedPriority){};

  private:
    char data[0x50];
    char _50;
    char _51;
    uchar _52;
  };

  struct C3DEmitterParmData {
    // ????
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

    C3DEmitterParmData(const CVector3f& pos, const CVector3f& dir, float maxDist, float distComp,
                       uint flags, ushort sfxId, float maxVol, float minVol, bool important,
                       uchar prio)
    : mPos(pos)
    , mDir(dir)
    , mMaxDist(maxDist)
    , mDistComp(distComp)
    , mFlags(flags)
    , mSfxId(sfxId)
    , mMaxVol(maxVol)
    , mMinVol(minVol)
    , mImportant(important)
    , mPrio(prio) {}

    CVector3f mPos;
    CVector3f mDir;
    float mMaxDist;
    float mDistComp;
    uint mFlags;
    ushort mSfxId;
    float mMaxVol;
    float mMinVol;
    bool mImportant; // Can't be allocated over, regardless of priority
    uchar mPrio;
  };

  class CTrkData {};

  CAudioSys(char, char, char, char, uint);
  ~CAudioSys();

  static void SysSetVolume(uchar, uint, uchar);
  static void SysSetSfxVolume(uchar, ushort, uchar, uchar);

  static void SetDefaultVolumeScale(short);
  static void SetVolumeScale(short);
  static void SetSurroundMode(ESurroundModes);
  static ESurroundModes GetSurroundMode();
  static void EnableAICallback(bool enable);
  static bool IsAICallbackEnabled();
  static void TrkSetSampleRate(ETRKSampleRate);

  static uint SeqPlayEx(const ushort gid, const ushort sid, void* arrfile, SND_PLAYPARA* para,
                        const uchar studio);
  static void SeqStop(uint seqId);
  static void SeqVolume(uchar, ushort, uint, uchar);

  static short GetScaledVolume(int vol) { return kVolumeTable[vol]; }
  static const ushort kVolumeTable[];
  static short GetDefaultVolumeScale();
  static bool GetVerbose();

  static bool mInitialized;
  static bool mIsListenerActive;
  static bool mVerbose;
  static uchar mMaxNumEmitters;
  static rstl::map< rstl::string, rstl::ncrc_ptr< CAudioGroupSet > >* mpGroupSetDB;
  static rstl::map< uint, rstl::string >* mpGroupSetResNameDB;
  static rstl::map< rstl::string, rstl::ncrc_ptr< CTrkData > >* mpDVDTrackDB;
  static rstl::vector< CEmitterData >* mpEmitterDB;
  static unkptr mpListener;

  /* TODO: Remaining globals */

  static ESurroundModes mSurroundMode;
  static uint mMaxAramUsage;
  static uint mCurrentAramUsage;
  static bool mProLogic2;
  static const uchar kMaxVolume;
  static uchar kEmitterMedPriority;
};

#endif // _CAUDIOSYS
