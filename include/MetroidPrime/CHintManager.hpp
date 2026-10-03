#ifndef _CHINTMANAGER
#define _CHINTMANAGER

#include "MetroidPrime/CHintState.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

class CStateManager;

// Guessed interface names, recovered from the shared hint runtime and its specializations.
class CHintManager {
public:
  CHintManager(int playerIndex, const rstl::string& name);
  virtual ~CHintManager();
  virtual void RefreshHint(CStateManager& mgr);
  virtual void Reset(CStateManager& mgr);
  virtual bool SetHint(CHintState* hint, CStateManager& mgr, bool areaChanged, bool force);
  virtual void ClearHint(CStateManager& mgr, bool areaChanged);
  virtual bool SelectHintFromStack(CHintState* hint, CStateManager& mgr, bool areaChanged);
  virtual void OnHintRemoved(CStateManager& mgr);

  // Guessed names. These queue changes for the manager's next update.
  void AddHint(TUniqueId hint, TUniqueId sender, CStateManager& mgr);
  void RemoveHint(TUniqueId hint, TUniqueId sender, CStateManager& mgr);
  void ForceRemoveHint(TUniqueId hint, CStateManager& mgr, TUniqueId sender);
  void Update(float dt, CStateManager& mgr);
  bool HasHint(const CStateManager& mgr) const;
  const CGameHint* GetCurrentHint(const CStateManager& mgr) const;
  CHintState* GetHintState(TUniqueId hint);
  const CHintState* GetHintState(TUniqueId hint) const;
  CHintState* GetBestHintState();

protected:
  int GetPlayerIndex() const { return mPlayerIndex; }
  int GetCurrentPriority() const { return mPriority; }
  void ClearCurrentHint(int priority) {
    mCurrentHintId = kInvalidUniqueId;
    mPriority = priority;
  }

  // Guessed name. Sorting compares the priority only, not the record's other fields.
  struct SHint {
    SHint(const int& priority, const CHintState& state) : mPriority(priority), mState(state) {}
    bool operator<(const SHint& other) const { return mPriority < other.mPriority; }

    int mPriority;
    CHintState mState;
  };
  const rstl::vector< SHint >& GetHints() const { return mHints; }
  rstl::vector< SHint >& GetHints() { return mHints; }

private:
  typedef rstl::pair< TUniqueId, TUniqueId > THintSender;

  static bool ContainsHint(const rstl::vector< SHint >& hints, TUniqueId hint);
  static bool ContainsHint(const rstl::vector< THintSender >& hints, TUniqueId hint);

  bool ProcessAddedHints(CStateManager& mgr);
  bool ProcessRemovedHints(CStateManager& mgr);
  bool AddHintState(int priority, const CHintState& hint);

  int mPlayerIndex;
  TAreaId mAreaId;
  TUniqueId mCurrentHintId;
  int mPriority;
  rstl::vector< SHint > mHints;
  rstl::vector< THintSender > mRemovedHints;
  rstl::vector< THintSender > mAddedHints;
};
CHECK_SIZEOF(CHintManager, 0x44)

#endif // _CHINTMANAGER
