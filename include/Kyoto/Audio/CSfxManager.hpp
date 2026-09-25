#ifndef _CSFXMANAGER
#define _CSFXMANAGER

#include "types.h"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"

class CVector3f;
class CSfxManager {
public:
  enum ESfxChannels { kSC_Invalid = -1, kSC_Default = 0, kSC_Game, kSC_PauseScreen };

  static void Update(float dt);
  static void RemoveEmitter(CSfxHandle handle);
  static void UpdateEmitter(CSfxHandle handle, const CVector3f& pos, const CVector3f& dir,
                            uchar maxVol);
  static void UpdateListener(const CVector3f& pos, const CVector3f& dir, const CVector3f&,
                             const CVector3f&, const uchar);

  static const short kMaxPriority;           // 0xFF
  static const short kMedPriority;           // 0x7F
  static const ushort kInternalInvalidSfxId; // 0xFFFF
  static const int kAllAreas;                // 0xFFFFFFFF
  static CSfxHandle AddEmitter(const ushort id, const CVector3f& pos, const CVector3f& dir,
                               const bool useAcoustics, const bool looped, const short prio = kMedPriority,
                               const int areaId = kAllAreas);
  static CSfxHandle AddEmitter(const ushort id, const CVector3f& pos, const CVector3f& dir, uchar vol,
                               const bool useAcoustics, const bool looped, const short prio = kMedPriority,
                               const int areaId = kAllAreas);
  static CSfxHandle AddEmitter(CAudioSys::C3DEmitterParmData& parmData, bool useAcoustics,
                               short prio, bool looped, int areaId = kAllAreas);

  static void AddListener(ESfxChannels channel, const CVector3f& pos, const CVector3f& dir,
                          const CVector3f& vec1, const CVector3f& vec2, float f1, float f2,
                          float f3, uint w1, const uchar maxVolume);
  static void Shutdown();
  static ushort TranslateSFXID(ushort);

  static void PitchBend(CSfxHandle handle, int pitch);

  static CSfxHandle SfxStart(ushort id, short vol, short pan, bool useAcoustics,
                             short prio = kMaxPriority, bool looped = false,
                             int areaId = kAllAreas);
  static void SfxSpan(CSfxHandle, uchar);
  static bool IsPlaying(const CSfxHandle& handle);
  static void SfxStop(CSfxHandle handle);

  static void SetChannel(ESfxChannels);
  static void KillAll(ESfxChannels);
  static void TurnOnChannel(ESfxChannels);
  static void TurnOffChannel(ESfxChannels);

  static void DisableAuxCallback();
  static void EnableAuxCallback();
  static void PrepareDelayCallback(const SND_AUX_DELAY& info);
  static void PrepareReverbStdCallback(const SND_AUX_REVERBSTD& info);
  static void PrepareChorusCallback(const SND_AUX_CHORUS& info);
  static void PrepareReverbHiCallback(const SND_AUX_REVERBHI& info);
  static void DisableAuxProcessing();
};

#endif // _CSFXMANAGER
