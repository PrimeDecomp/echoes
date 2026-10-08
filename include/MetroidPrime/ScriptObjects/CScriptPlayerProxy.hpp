#ifndef _CSCRIPTPLAYERPROXY
#define _CSCRIPTPLAYERPROXY

#include "types.h"

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/Player/CPlayerListener.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"

class CModelData;
class CPlayer;

// Guessed class: a multiplayer script object that follows, watches or rewards the players it
// listens to. The loader names it PlayerController; one of eleven behaviours is chosen by the
// proxy type.
class CScriptPlayerProxy : public CActor {
public:
  // Guessed names; the behaviours are inferred from the Think, message and game-event handlers.
  enum EProxyType {
    kPT_EventRelay,
    kPT_CoinCollector,
    kPT_ArchenemyMarker,
    kPT_PlayerFollower,
    kPT_PersistentCounterReset,
    kPT_CombatVisorWatcher,
    kPT_ScanVisorWatcher,
    kPT_DarkVisorWatcher,
    kPT_EchoVisorWatcher,
    kPT_PlayerMessageRelay,
    kPT_DamageOverTime,
  };

  CScriptPlayerProxy(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CModelData& model, CStateManager& mgr, uint playerMask, int proxyType,
                     const CVector3f& playerOffset, int intParameter1, int intParameter2,
                     float floatParameter1, float floatParameter2, float floatParameter3,
                     const CVector3f& vectorParameter1, const rstl::string& stringParameter1);

  // CEntity
  ~CScriptPlayerProxy() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;

  // CScriptPlayerProxy
  void OnGameEvent(CStateManager& mgr, uint sourceIndex, uint targetIndex, uint event,
                   const void* value); // Guessed name

private:
  // Guessed name; forwards the multiplayer game events to the owning proxy.
  class CListener : public CPlayerListener {
  public:
    CListener(CStateManager& mgr, uint playerMask, CScriptPlayerProxy* owner)
    : CPlayerListener(mgr, playerMask), mOwner(owner) {}
    ~CListener() override {}
    void OnGameEvent(CStateManager& mgr, uint sourceIndex, uint targetIndex, uint event,
                     const void* value) override;

  private:
    CScriptPlayerProxy* mOwner; // Guessed name
  };

  // Guessed name; one pending damage-over-time effect.
  struct SDamageOverTime {
    SDamageOverTime(float timeRemaining, float damagePerSecond, TUniqueId attacker)
    : mTimeRemaining(timeRemaining), mDamagePerSecond(damagePerSecond), mAttacker(attacker) {}
    float mTimeRemaining;
    float mDamagePerSecond;
    TUniqueId mAttacker;
  };

  static bool NotifyProxies(CStateManager& mgr, CPlayer& player,
                            EScriptObjectState state);          // Guessed name
  uint GetPlayerMask() const;                                   // Guessed name
  uint GetArchenemyIndex(const CStateManager& mgr) const;       // Guessed name
  void SetArchenemyIndex(CStateManager& mgr, uint playerIndex); // Guessed name
  void UpdateTransform(CStateManager& mgr, uint playerIndex);   // Guessed name
  void SendStateToPlayer(EScriptObjectState state, CStateManager& mgr,
                         uint playerIndex); // Guessed name
  void CheckVisors(CStateManager& mgr);     // Guessed name
  void InitVisors(CStateManager& mgr);      // Guessed name
  void DropCoins(CStateManager& mgr, int amount, uint playerIndex, bool amplified,
                 int denominationMode);                                             // Guessed name
  void UpdateDamageOverTime(float dt, CStateManager& mgr);                          // Guessed name
  void AddDamageOverTime(uint sourceIndex, TUniqueId attacker, CStateManager& mgr); // Guessed name
  void ClearDamageOverTime(CStateManager& mgr);                                     // Guessed name
  void SetVisorFromProxyType();                                                     // Guessed name
  void HandleMessage(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr,
                     uint playerIndex); // Guessed name
  void HandleRespawnMessage(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr,
                            uint playerIndex); // Guessed name
  void HandleFollowerMessage(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr,
                             uint playerIndex); // Guessed name
  void HandleArchenemyMessage(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr,
                              uint playerIndex); // Guessed name
  void HandleVisorMessage(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr,
                          uint playerIndex); // Guessed name
  void HandleDamageOverTimeMessage(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr,
                                   uint playerIndex); // Guessed name

  CListener mListener;            // Guessed name
  int mProxyType;                 // Guessed name; SLdrPlayerController::proxyType
  CVector3f mPlayerOffset;        // Guessed name; SLdrPlayerController::playerOffset
  int mIntParameter1;             // Guessed name; SLdrPlayerController::intParameter1
  int mIntParameter2;             // Guessed name; SLdrPlayerController::intParameter2
  float mFloatParameter1;         // Guessed name; SLdrPlayerController::floatParameter1
  float mFloatParameter2;         // Guessed name; SLdrPlayerController::floatParameter2
  float mFloatParameter3;         // Guessed name; SLdrPlayerController::floatParameter3
  CVector3f mVectorParameter1;    // Guessed name; SLdrPlayerController::vectorParameter1
  rstl::string mStringParameter1; // Guessed name; SLdrPlayerController::stringParameter1
  float mAccumulatedDamage;       // Guessed name
  int mActorCount;                // Guessed name
  int mCoinsToDrop;               // Guessed name
  int mPendingCoins;              // Guessed name
  int mCoinDropCooldown;          // Guessed name
  float mTime;                    // Guessed name
  rstl::reserved_vector< SDamageOverTime, 8 > mDamageOverTime; // Guessed name
  float mCurrentDuration;                                      // Guessed name
  float mCurrentMinDamage;                                     // Guessed name
  float mCurrentMaxDamage;                                     // Guessed name
  int mCurrentWeaponType;                                      // Guessed name
  bool mDamageActive : 1;                                      // Guessed name
  bool mFollowing : 1;                                         // Guessed name
};
CHECK_SIZEOF(CScriptPlayerProxy, 0x238)

#endif // _CSCRIPTPLAYERPROXY
