#ifndef _CSCRIPTHINT
#define _CSCRIPTHINT

#include "MetroidPrime/CActor.hpp"

#include <string.h>

class CGameHint;

// Guessed name: the common base of Echoes's camera, player and control hints.
class CScriptHint : public CActor {
public:
  // Guessed name. Type-erased callback copied into the runtime CGameHint.
  struct SCallback {
    typedef void (*FInvoke)(void*, const void*, CStateManager&, CGameHint&);

    SCallback() : mInvoke(nullptr), mContext(nullptr) { memset(mCallable, 0, sizeof(mCallable)); }

    FInvoke mInvoke;
    void* mContext;
    uchar mCallable[16]; // Original callable representation remains unresolved.
  };

  CScriptHint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
              const CTransform4f& xf, int priority, float timer, int acrossAreas, int breakType,
              uint deleteOnRemoval, uint requiredPresses, float unknown16c,
              const SCallback& onExpire, const SCallback& onBreak, float breakDelay);

  // CEntity
  ~CScriptHint() override = 0;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  int GetPriority() const { return mPriority; }

private:
  int mPriority;
  float mTimer;
  int mBreakType; // CGameHint::EBreakHintType in the Wii export; GC enum scope unverified.
  uint mDeleteOnRemoval;
  uint mRequiredPresses;
  float x16c_;
  SCallback mOnExpire;
  SCallback mOnBreak;
  float mBreakDelay;
  int mAcrossAreas;
};
CHECK_SIZEOF(CScriptHint, 0x1a8)
NESTED_CHECK_SIZEOF(CScriptHint, SCallback, 0x18)

#endif // _CSCRIPTHINT
