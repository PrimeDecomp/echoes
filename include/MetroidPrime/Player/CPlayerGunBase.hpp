#ifndef _CPLAYERGUNBASE
#define _CPLAYERGUNBASE

#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "rstl/single_ptr.hpp"

class CFinalInput;
class CModelFlags;
class CPlayer;
class CRainSplashGenerator;
class CWorldShadow;

// Guessed name. Shared CEntity-derived gun interface, independently constructed by Echoes.
class CPlayerGunBase : public CEntity {
public:
  enum EGunHolsterState { kGHS_Drawing, kGHS_Holstered, kGHS_Drawn, kGHS_Holstering };

  CPlayerGunBase(const rstl::string& name, TUniqueId playerId, const CVector3f& scale,
                 int maxSplashes);
  ~CPlayerGunBase() override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;
  virtual void TouchModel(const CStateManager& mgr) const = 0;
  virtual void PreRender(CStateManager& mgr, const CVector3f& cameraPosition) = 0;
  virtual void AddToRenderer(const CStateManager& mgr) const = 0;
  virtual void Render(const CStateManager& mgr, const CVector3f& cameraTranslation,
                      const CModelFlags& flags) const = 0;
  virtual void DamageRumble(const CVector3f& position, float damage) = 0;
  virtual void Reset(CStateManager& mgr);
  virtual void Update(float dt, CStateManager& mgr);
  virtual float GetBeamVelocity() const = 0;
  virtual void SetAuxTargetId(TUniqueId target) = 0;
  virtual TUniqueId GetAuxTargetId() const = 0;
  virtual TUniqueId CreatePowerBomb(CStateManager& mgr) = 0; // Guessed name
  virtual void AsyncLoadSuit(CStateManager& mgr) = 0;
  virtual void ProcessInput(const CFinalInput& input, CStateManager& mgr);
  virtual void PollStateMachine(CStateManager& mgr) = 0;
  virtual void InitializeStateMachine(CStateManager& mgr) = 0;

  TUniqueId GetPlayerUniqueId() const { return mPlayerUniqueId; }
  CPlayer* GetPlayer(CStateManager& mgr) const;
  CPlayer* GetPlayerFromAll(CStateManager& mgr) const;
  CWorldShadow* GetWorldShadow();
  const CWorldShadow* GetWorldShadow() const;
  void AddGunDrawBlock();
  void RemoveGunDrawBlock();
  void UpdateTransform(CStateManager& mgr, const CVector3f& position, const CTransform4f& rotation,
                       CTransform4f& result);
  void UpdateGunHolster(const CFinalInput& input, CStateManager& mgr);
  void DrawGun(CStateManager& mgr);
  void HolsterGun(CStateManager& mgr);
  void Holster(CStateManager& mgr);
  void CreateGunLight(CStateManager& mgr);
  void DeleteGunLight(CStateManager& mgr);

protected:
  CTransform4f mTransform;
  CTransform4f mAssistAimXf;
  CVector3f mScale;
  rstl::single_ptr< CRainSplashGenerator > mRainSplashGenerator;
  CActorLights mLights;
  TUniqueId mPlayerUniqueId;
  TUniqueId mLightId;
  rstl::single_ptr< CWorldShadow > mWorldShadow;
  float mCooldown;
  float mSecondaryCooldown;
  float mGunHolsterRemTime;
  uint mInputFlags;
  uint mLastInputFlags;
  uint mReleasedInputFlags;
  uint mPressedInputFlags;
  uint mFiredWeaponFlags;
  int mGunDrawBlockCount;
  CPlayerState::EChargeStage mChargeState;
  EGunHolsterState mGunHolsterState;
  short mSoundVolume;
  bool mUnderwater : 1;
  bool x3ae_25_ : 1; // No identified reads beyond construction/reset yet.
  bool mInBigStrike : 1;
  bool mMissileMode : 1;
  bool mInPhazonPool : 1;
};
CHECK_SIZEOF(CPlayerGunBase, 0x3b0)

#endif // _CPLAYERGUNBASE
