#ifndef _CCAMERASHAKERMANAGER
#define _CCAMERASHAKERMANAGER

#include "types.h"

#include "MetroidPrime/Cameras/CCameraShakerData.hpp"
#include "rstl/reserved_vector.hpp"

class CStateManager;

class CCameraShakerManager {
public:
  explicit CCameraShakerManager(int playerIndex);
  virtual ~CCameraShakerManager();

  int AddCameraShaker(const CCameraShakerData& data, CStateManager& mgr, bool playSound,
                      bool useThresholdTimes);
  void UpdateCameraShaker(int id, const CCameraShakerData& data);
  void RemoveCameraShaker(int id);

  // Reconstructed names for the remaining native operations.
  void Reset();
  void Update(float dt, CStateManager& mgr);
  CVector3f GetTranslation(const CStateManager& mgr) const;

private:
  // Reconstructed record and member names; no original exported type name is known.
  struct SShaker {
    SShaker(int id, int playerIndex, const CCameraShakerData& data, bool playSound,
            bool useThresholdTimes);

    void SetData(const CCameraShakerData& data);
    float GetDistanceAttenuation(const CStateManager& mgr) const;
    CVector3f GetTranslation(const CStateManager& mgr);

    float mTime;
    int mId;
    int mPlayerIndex;
    CCameraShakerData mData;
    bool mPlaySound : 1;
    bool mSoundStarted : 1;
    bool mUseThresholdTimes : 1;
  };

  void StartSound(SShaker& shaker);

  rstl::reserved_vector< SShaker, 8 > mShakers;
  CVector3f mTranslation;
  int mNextId;
  int mPlayerIndex;
  bool mPendingRumble : 1;
  bool mRumbling : 1;
  short x83e_; // Only constructor-zero is established; runtime role remains unknown.
  float mRumbleCooldown;
};
CHECK_SIZEOF(CCameraShakerManager, 0x844)

#endif // _CCAMERASHAKERMANAGER
