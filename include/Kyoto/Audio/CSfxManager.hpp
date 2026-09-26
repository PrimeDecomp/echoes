#ifndef _CSFXMANAGER
#define _CSFXMANAGER

#include "types.h"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"

#include <musyx/musyx.h>

class CToken;
class CSimplePool;
struct SObjectTag;

class CSfxManager {
public:
  enum ESfxChannels { kSC_Invalid = -1, kSC_Default = 0, kSC_Game, kSC_PauseScreen };
  enum ESfxAudibility { kSA_Aud0, kSA_Aud1, kSA_Aud2, kSA_Aud3 };

  class CBaseSfxWrapper {
  public:
    CBaseSfxWrapper(bool looped, short priority, CSfxHandle handle, bool useAcoustics, int area);
    virtual ~CBaseSfxWrapper() = 0;
    virtual void SetActive(bool active);
    virtual void SetPlaying(bool playing);
    virtual void SetRank(short rank);
    virtual void SetInArea(bool inArea);
    virtual bool IsLooped() const;
    virtual bool IsPlaying() const;
    virtual bool IsActive() const;
    virtual bool IsInArea() const;
    virtual bool UseAcoustics() const;
    virtual int GetRank() const;
    virtual int GetPriority() const;
    virtual int GetArea() const;
    virtual CSfxHandle GetSfxHandle() const;
    virtual bool IsEmitter() const = 0; // Guessed name
    virtual void Play() = 0;
    virtual void Stop() = 0;
    virtual bool Ready() = 0;
    virtual short GetAudible(const CVector3f& position) = 0;
    virtual SND_VOICEID GetVoice() const = 0;
    virtual ushort GetSfxId() = 0;
    virtual void UpdateEmitterSilent() = 0;
    virtual void UpdateEmitter() = 0;
    virtual void SetReverb(char reverb) = 0;

    bool Available() const;
    void Release();
    float GetTimeRemaining();
    void SetTimeRemaining(float time);
    ushort GetPitchBend() const;
    void SetPitchBend(ushort pitch);
    bool GetIgnoreAreaLowPass() const;
    void SetIgnoreAreaLowPass(bool ignore);

  private:
    float mTimeRemaining;
    short mRank;
    short mPriority;
    ushort mPitchBend;
    CSfxHandle mHandle;
    int mArea;
    bool mActive : 1;
    bool mPlaying : 1;
    bool mLooped : 1;
    bool mInArea : 1;
    bool mReleased : 1;
    bool mUseAcoustics : 1;
    bool mIgnoreAreaLowPass : 1; // Guessed name
  };

  class CSfxEmitterWrapper : public CBaseSfxWrapper {
  public:
    CSfxEmitterWrapper(bool looped, short priority, CAudioSys::C3DEmitterParmData& emitter,
                       CSfxHandle handle, bool useAcoustics, int area);

    // CBaseSfxWrapper
    ~CSfxEmitterWrapper();
    bool IsPlaying() const override;
    bool IsEmitter() const override;
    void Play() override;
    void Stop() override;
    bool Ready() override;
    short GetAudible(const CVector3f& position) override;
    SND_VOICEID GetVoice() const override;
    ushort GetSfxId() override;
    void UpdateEmitterSilent() override;
    void UpdateEmitter() override;
    void SetReverb(char reverb) override;

    CAudioSys::C3DEmitterParmData& GetEmitter();
    const CAudioSys::C3DEmitterParmData& GetEmitter() const;
    uint GetHandle() const;
    bool IsSilent() const; // Guessed name

  private:
    friend class CSfxManager;
    SND_PARAMETER mParameters[3];
    SND_PARAMETER_INFO mParameterInfo;
    CAudioSys::C3DEmitterParmData mEmitterData;
    uint mEmitterHandle;
    uchar mCachedMaxVolume;
    bool mReady : 1;
    bool mUpdatePending : 1; // Restores the cached volume in Update.
  };

  class CSfxWrapper : public CBaseSfxWrapper {
  public:
    CSfxWrapper(bool looped, short priority, ushort sfxId, short volume, short pan,
                CSfxHandle handle, bool useAcoustics, int area);

    // CBaseSfxWrapper
    ~CSfxWrapper();
    bool IsPlaying() const override;
    bool IsEmitter() const override;
    void Play() override;
    void Stop() override;
    bool Ready() override;
    short GetAudible(const CVector3f& position) override;
    SND_VOICEID GetVoice() const override;
    ushort GetSfxId() override;
    void UpdateEmitterSilent() override;
    void UpdateEmitter() override;
    void SetReverb(char reverb) override;

    void SetVolume(short volume);

  private:
    ushort mSfxId;
    SND_VOICEID mVoiceHandle;
    short mVolume;
    short mPan;
    bool mReady;
  };

  class CSfxListener {
  public:
    CSfxListener(CVector3f position = CVector3f::Zero(), CVector3f direction = CVector3f::Zero(),
                 CVector3f heading = CVector3f::Zero(), CVector3f up = CVector3f::Zero(),
                 float frontSur = 0.f, float backSur = 0.f, float soundSpeed = 0.f, uint flags = 0,
                 uchar maxVolume = 0);

    CVector3f mPosition;
    CVector3f mDirection;
    CVector3f mHeading;
    CVector3f mUp;
    float mFrontSur;
    float mBackSur;
    float mSoundSpeed;
    uint mFlags;
    uchar mMaxVolume;
  };

  struct SListener { // Guessed name
    SListener() : mActive(false) {}
    CSfxListener mListener;
    bool mActive;
  };

  class CSfxChannel {
  public:
    CSfxChannel();
    rstl::reserved_vector< SListener, 4 > mListeners;
    rstl::reserved_vector< CBaseSfxWrapper*, 72 > mSounds;
  };

  struct SLowPassFilter { // Guessed name
    SLowPassFilter(int frequency, float duration, int id)
    : mFrequency(frequency), mTimeRemaining(duration), mId(id), mTimed(duration > 0.f) {}
    int mFrequency;
    float mTimeRemaining;
    int mId;
    bool mTimed : 1;
  };

  struct SAreaVolume { // Guessed name
    SAreaVolume() : mArea(-1), mVolume(0) {}
    int mArea;
    uchar mVolume;
  };

  static const short kMaxPriority;
  static const short kMedPriority;
  static const ushort kInternalInvalidSfxId;
  static const int kAllAreas;

  static void Initialize(); // Guessed name
  static void Shutdown();
  static void StopAndRemoveAllEmitters();
  static void Update(float dt);
  static CSfxHandle SfxStart(ushort id, short volume, short pan, int area = kAllAreas,
                             bool useAcoustics = false, bool looped = false,
                             short priority = kMedPriority);
  static CSfxHandle AddEmitter(ushort id, const CVector3f& position, int area = kAllAreas,
                               bool useAcoustics = false, bool looped = false,
                               short priority = kMedPriority);
  static CSfxHandle AddEmitter(ushort id, const CVector3f& position, uchar volume, int area,
                               bool useAcoustics, bool looped, short priority);
  static CSfxHandle AddEmitter(CAudioSys::C3DEmitterParmData& params, int area = kAllAreas,
                               bool useAcoustics = false, bool looped = false,
                               short priority = kMedPriority);
  static void RemoveEmitter(CSfxHandle handle);
  static void UpdateEmitter(CSfxHandle handle, const CVector3f& position,
                            const CVector3f& direction, uchar maxVolume);
  static void SfxStop(CSfxHandle handle);
  static void SfxStop(ESfxChannels channel, CSfxHandle handle);
  static void StopSound(ESfxChannels channel, CSfxHandle handle);
  static void SfxVolume(CSfxHandle handle, uchar volume);
  static void SfxPan(CSfxHandle handle, uchar pan);
  static void SfxSpan(CSfxHandle handle, uchar span);
  static void PitchBend(CSfxHandle handle, int pitch);
  static void SetDuration(CSfxHandle handle, float duration);
  static bool IsPlaying(CSfxHandle handle);
  static bool IsQueued(CSfxHandle handle);
  static void SetMuted(bool muted);
  static void SetChannel(ESfxChannels channel);
  static ESfxChannels GetChannel();
  static void KillAll(ESfxChannels channel);
  static void TurnOnChannel(ESfxChannels channel);
  static void TurnOffChannel(ESfxChannels channel);
  static void AddListener(ESfxChannels channel, const CVector3f& position,
                          const CVector3f& direction, const CVector3f& heading, const CVector3f& up,
                          float frontSur, float backSur, float soundSpeed, uint flags,
                          uchar maxVolume, int listener);
  static void UpdateListener(const CVector3f& position, const CVector3f& direction,
                             const CVector3f& heading, const CVector3f& up, uchar maxVolume,
                             int listener);
  static void SetActiveAreas(const rstl::reserved_vector< int, 10 >& areas, int currentArea);
  static ushort TranslateSFXID(ushort id);
  static bool LoadTranslationTable(CSimplePool* pool, const SObjectTag* tag);
  static CSfxHandle LocateHandle();
  static int GetRank(CBaseSfxWrapper* sound);
  static CSfxWrapper* AllocateCSfxWrapper(const CSfxWrapper& sound);
  static CSfxEmitterWrapper* AllocateCSfxEmitterWrapper(const CSfxEmitterWrapper& sound);
  static short GetReverbAmount();
  static uchar GetStudio(int area);                                 // Guessed name
  static void SetAreaVolume(int area, uchar volume);                // Guessed name
  static uchar GetAreaVolume(int area);                             // Guessed name
  static void SetIgnoreAreaLowPass(CSfxHandle handle, bool ignore); // Guessed name
  static int AddLowPassAreaFilter(int frequency, float duration);
  static void RemoveLowPassAreaFilter(int id);
  static void UpdateLowPassAreaFilters(float dt);             // Guessed name
  static int GetLowPassAreaFrequency();                       // Guessed name
  static bool IsLowPassAreaFilterEnabled();                   // Guessed name
  static int AddLowPassFilter(int frequency, float duration); // Guessed name
  static void RemoveLowPassFilter(int id);                    // Guessed name
  static void UpdateLowPassFilters(float dt);                 // Guessed name
  static int GetLowPassFrequency();                           // Guessed name
  static bool IsLowPassEnabled();                             // Guessed name
  static bool ShouldApplyLowPass(CBaseSfxWrapper* sound);     // Guessed name
  static int GetLowPassFrequency(CBaseSfxWrapper* sound);     // Guessed name

  // TODO: identify the shared pitch-ramp and auxiliary-effect types before declaring
  // their registration/update methods and the eight effect-parameter overloads.

private:
  static CSfxChannel mChannels[4];
  static ESfxChannels mCurrentChannel;
  static bool mDoUpdate;
  static bool mMuted;
  static rstl::vector< short >* mpTranslationTable;
  static rstl::auto_ptr< CToken > mpTranslationTableToken;
  static rstl::reserved_vector< CSfxEmitterWrapper, 64 > mEmitterWrapperPool;
  static rstl::reserved_vector< CSfxWrapper, 64 > mWrapperPool;
  static rstl::reserved_vector< SLowPassFilter, 8 > mAreaLowPassFilters;
  static rstl::reserved_vector< SLowPassFilter, 8 > mLowPassFilters;
  static int mNextAreaFilterId;
  static int mNextFilterId;
  static int mAreaLowPassFrequency;
  static int mLowPassFrequency;
  static rstl::reserved_vector< SAreaVolume, 10 > mAreaVolumes;
  static int mCurrentArea;
  static bool mCurrentStudio;
};

inline CSfxManager::CBaseSfxWrapper::~CBaseSfxWrapper() {}

NESTED_CHECK_SIZEOF(CSfxManager, CBaseSfxWrapper, 0x1c)
NESTED_CHECK_SIZEOF(CSfxManager, CSfxWrapper, 0x2c)
NESTED_CHECK_SIZEOF(CSfxManager, CSfxEmitterWrapper, 0x64)
NESTED_CHECK_SIZEOF(CSfxManager, CSfxListener, 0x44)
NESTED_CHECK_SIZEOF(CSfxManager, SListener, 0x48)
NESTED_CHECK_SIZEOF(CSfxManager, CSfxChannel, 0x248)
NESTED_CHECK_SIZEOF(CSfxManager, SLowPassFilter, 0x10)

#endif // _CSFXMANAGER
