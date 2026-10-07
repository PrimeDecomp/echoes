#ifndef _CHINTSTATE
#define _CHINTSTATE

#include "MetroidPrime/CGameHint.hpp"
#include "rstl/reserved_vector.hpp"

class CControlMapper;
class CFinalInput;

// Guessed name: live hint state is separate from the script actor in Echoes.
class CHintState {
public:
  CHintState(TUniqueId hint, int priority, float timer, CGameHint::EBreakHintType breakType,
             uint requiredPresses, float unknown30, CGameHint::SCallback onExpire,
             CGameHint::SCallback onBreak, float breakDelay);

  // Guessed method names.
  void AddSender(TUniqueId sender);
  void RemoveSender(TUniqueId sender, CStateManager& mgr);
  TUniqueId GetFirstSender() const;
  const bool ProcessInput(const CFinalInput& input, const CControlMapper& mapper,
                          CStateManager& mgr);
  void OnExpire(CStateManager& mgr);

  TUniqueId GetHintId() const { return mHintId; }
  int GetPriority() const { return mPriority; }
  float GetTimer() const { return mTimer; }
  void SetTimer(float timer) { mTimer = timer; }
  bool HasSenders() const { return mSenders.size() != 0u; }
  bool GetForceRemoval() const { return mForceRemoval; }
  void SetForceRemoval(bool force) { mForceRemoval = force; }

private:
  TUniqueId mHintId;
  int mPriority;
  float mTimer;
  rstl::reserved_vector< TUniqueId, 8 > mSenders;
  CGameHint::EBreakHintType mBreakType;
  uint mPressCount;
  uint mRequiredPresses;
  float x2c_;
  float x30_;
  CGameHint::SCallback mOnExpire;
  CGameHint::SCallback mOnBreak;
  float mBreakDelay;
  bool mForceRemoval;
};
CHECK_SIZEOF(CHintState, 0x6c)

#endif // _CHINTSTATE
