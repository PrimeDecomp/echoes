#ifndef _CAUXWEAPON
#define _CAUXWEAPON

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CElementGen;
class CPlayer;
class CScriptMsg;
class CStateManager;
class CTransform4f;
class CVector3f;
class CWeaponDescription;

class CAuxWeapon {
public:
  explicit CAuxWeapon(TUniqueId playerId);
  ~CAuxWeapon();

  void Fire(bool underwater, int currentBeam, CPlayerState::EChargeStage chargeState,
            const CTransform4f& xf, CStateManager& mgr, EWeaponType type, TUniqueId homingId,
            uint attributes, ushort soundId);
  bool IsComboFxActive(const CStateManager& mgr) const;
  bool UpdateComboFx(float dt, const CVector3f& scale, const CVector3f& firePos,
                     const CTransform4f& targetXf, CStateManager& mgr);
  void StopComboFx(CStateManager& mgr, bool deactivate);
  void RenderMuzzleFx() const;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg);
  void Load(int beam, CStateManager& mgr);
  void LoadIdle();
  bool HasChargeCombo(int beam, CStateManager& mgr) const;
  void SetTargetId(TUniqueId target);
  TUniqueId GetTargetId() const;
  bool IsLoaded() const { return mIsLoaded; }

private:
  CPlayer* GetPlayerFromAll(CStateManager& mgr) const;
  CPlayer* FindPlayer(CStateManager& mgr) const;
  void FreeComboVoiceId();
  void FireLightCombo(bool underwater, int comboId, uint attributes, const CTransform4f& xf,
                      TUniqueId homingId, CStateManager& mgr); // Guessed name.
  void FireProjectile(EWeaponType type, bool underwater, bool isCombo, bool adjustSpawn,
                      int comboId, uint attributes, const CTransform4f& xf, TUniqueId homingId,
                      ushort soundId, CStateManager& mgr);
  void InitComboData();

  TLockedToken< CWeaponDescription > mMissile;
  rstl::single_ptr< CElementGen > mMuzzleFxGen; // Prime-derived type; unused in observed ctor.
  rstl::reserved_vector< TCachedToken< CWeaponDescription >, 4 > mCombos;
  TUniqueId mPlayerId;
  CPlayerState::EBeamId mFiringBeamId;
  int mLoadBeamId;
  CSfxHandle mComboSfx;
  short mSoundVolume;
  bool mIsLoaded : 1;
};
CHECK_SIZEOF(CAuxWeapon, 0x58)

#endif // _CAUXWEAPON
