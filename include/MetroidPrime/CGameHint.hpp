#ifndef _CGAMEHINT
#define _CGAMEHINT

#include "MetroidPrime/CActor.hpp"

#include <string.h>

class CHintState; // Guessed name: the separate runtime hint record, not an actor.

// Guessed name: the Wii CGameHint::EBreakHintType export suggests this common hint base.
class CGameHint : public CActor {
public:
  // Guessed enumerator names; EBreakHintType is exported by the Echoes Wii build.
  enum EBreakHintType {
    kBHT_None,
    kBHT_Fire,
    kBHT_Turn,
    kBHT_Jump,
    kBHT_TriggersAndB,
    kBHT_Unknown5
  };

  // Guessed name. Type-erased callback copied into the runtime hint state.
  struct SCallback {
    typedef void (*FInvoke)(void*, const void*, CStateManager&, CHintState&);

    SCallback() : mInvoke(nullptr), mContext(nullptr) { memset(mCallable, 0, sizeof(mCallable)); }

    template < class T >
    SCallback(T* object, void (T::*method)(CStateManager&))
    : mInvoke(&InvokeMember< T >), mContext(object) {
      memcpy(mCallable, &method, sizeof(method));
    }

    template < class T >
    SCallback(T* object, void (T::*method)())
    : mInvoke(&InvokeMemberWithoutArgs< T >), mContext(object) {
      memcpy(mCallable, &method, sizeof(method));
    }

    bool IsNull() const {
      for (int i = 0; i < sizeof(mCallable); ++i) {
        if (mCallable[i] != 0) {
          return false;
        }
      }
      return true;
    }

    void operator()(CStateManager& mgr, CHintState& state) const {
      if (!IsNull()) {
        mInvoke(mContext, mCallable, mgr, state);
      }
    }

    FInvoke mInvoke;
    void* mContext;
    char mCallable[16]; // Original callable representation remains unresolved.

  private:
    template < class T >
    static void InvokeMember(void* object, const void* callable, CStateManager& mgr, CHintState&) {
      void (T::*method)(CStateManager&);
      memcpy(&method, callable, sizeof(method));
      (static_cast< T* >(object)->*method)(mgr);
    }

    template < class T >
    static void InvokeMemberWithoutArgs(void* object, const void* callable, CStateManager&,
                                        CHintState&) {
      void (T::*method)();
      memcpy(&method, callable, sizeof(method));
      (static_cast< T* >(object)->*method)();
    }
  };

  CGameHint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
            const CTransform4f& xf, int priority, float timer, int acrossAreas,
            EBreakHintType breakType, uint deleteOnRemoval, uint requiredPresses, float unknown16c,
            SCallback onExpire, SCallback onBreak, float breakDelay);

  // CEntity
  ~CGameHint() override = 0;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  int GetPriority() const { return mPriority; }
  float GetTimer() const { return mTimer; }
  EBreakHintType GetBreakType() const { return mBreakType; }
  bool GetDeleteOnRemoval() const { return mDeleteOnRemoval == 1; }
  uint GetRequiredPresses() const { return mRequiredPresses; }
  float GetUnknown16c() const { return x16c_; }
  const SCallback& GetOnExpire() const { return mOnExpire; }
  const SCallback& GetOnBreak() const { return mOnBreak; }
  float GetBreakDelay() const { return mBreakDelay; }
  bool GetAcrossAreas() const { return mAcrossAreas == 1; }

private:
  int mPriority;
  float mTimer;
  EBreakHintType mBreakType;
  uint mDeleteOnRemoval;
  uint mRequiredPresses;
  float x16c_;
  SCallback mOnExpire;
  SCallback mOnBreak;
  float mBreakDelay;
  int mAcrossAreas;
};
CHECK_SIZEOF(CGameHint, 0x1a8)
NESTED_CHECK_SIZEOF(CGameHint, SCallback, 0x18)

#endif // _CGAMEHINT
