#ifndef _CFLUIDPLANEMANAGER
#define _CFLUIDPLANEMANAGER

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/reserved_vector.hpp"

class CScriptWater;
class CStateManager;

class CFluidPlaneManager {
public:
  class CFluidProfile {
  public:
    void Clear();

  private:
    float x0_;
    float x4_;
    float x8_;
    float xc_;
    float x10_;
  };

  class CSplashRecord {
  public:
    explicit CSplashRecord(TUniqueId id) : mTime(0.f), mId(id) {}

    void SetTime(float time) { mTime = time; }

    float GetTime() const { return mTime; }

    TUniqueId GetUniqueId() const { return mId; }

  private:
    float mTime;
    TUniqueId mId;
  };

  CFluidPlaneManager();
  void Update(float dt);
  void StartFrame(bool enabled) const;
  void EndFrame() const;
  float GetLastSplashDeltaTime(TUniqueId splasher) const;
  void CreateSplash(TUniqueId splasher, CStateManager& mgr, const CScriptWater& water,
                    const CVector3f& pos, float factor, bool sfx);
  float GetUVTime() const { return mUvTime; }

  static CFluidProfile sProfile;

private:
  rstl::reserved_vector< CSplashRecord, 32 > mSplashes;
  CVector3f mLastSplashPosition;
  float mSplashCooldown;
  float mUvTime;
  mutable bool x118_;
  mutable bool mFrameActive; // Guessed name.
};
CHECK_SIZEOF(CFluidPlaneManager, 0x11c)
NESTED_CHECK_SIZEOF(CFluidPlaneManager, CSplashRecord, 0x8)
NESTED_CHECK_SIZEOF(CFluidPlaneManager, CFluidProfile, 0x14)

#endif // _CFLUIDPLANEMANAGER
