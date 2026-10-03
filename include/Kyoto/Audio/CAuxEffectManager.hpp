#ifndef _CAUXEFFECTMANAGER
#define _CAUXEFFECTMANAGER

#include "Kyoto/Audio/CAuxEffect.hpp"
#include "rstl/reserved_vector.hpp"

// Guessed names, supported by the three-bus callback, mixing and priority behavior.
class CAuxEffectManager {
public:
  enum ECategory { kEC_Parallel, kEC_Serial };
  enum EState {
    kES_Parallel,
    kES_Serial,
    kES_Free,
    kES_ParallelFadeIn,
    kES_ParallelFadeOut,
    kES_SerialFadeIn,
    kES_SerialFadeOut,
    kES_SerialBypassFadeOut,
    kES_PendingCleanup,
  };

  CAuxEffectManager();
  void Initialize();
  void Shutdown();
  void Cleanup();
  int AddEffect(int bus, const CAuxEffect& effect, ECategory category, bool replace);
  void RemoveEffect(int id);
  void FadeOut(int bus, ECategory category);
  void SetHighestPrioritySerialState(int bus, EState state);
  static void AuxCallback(uchar reason, SND_AUX_INFO* info, void* user);
  static void NoEffectCallback(uchar reason, SND_AUX_INFO* info, void* user);

private:
  struct SEffectSlot {
    SEffectSlot(float fade, EState state, int id, const CAuxEffect& effect);
    int GetPriority() const;
    EState GetState() const;
    void SetState(EState state);
    float GetFade() const;
    void SetFade(float fade);
    int GetId() const;
    void SetId(int id);
    void SetEffect(const CAuxEffect& effect);
    void Prepare();
    void Shutdown();
    void Process(uchar reason, SND_AUX_INFO* info);

    float mFade;
    EState mState;
    int mId;
    CAuxEffect mEffect;
  };
  struct SCallbackContext {
    SCallbackContext(CAuxEffectManager* manager, int bus);
    CAuxEffectManager* GetManager() const;
    int GetBusIndex() const;

    CAuxEffectManager* mManager;
    int mBusIndex;
  };
  typedef rstl::reserved_vector< SEffectSlot, 4 > TBus;
  rstl::reserved_vector< TBus, 3 > mBuses;
  rstl::reserved_vector< SCallbackContext, 3 > mContexts;
  int mNextId;
  bool mCallbackInstalled[3];
};
CHECK_SIZEOF(CAuxEffectManager, 0x1834)

#endif // _CAUXEFFECTMANAGER
