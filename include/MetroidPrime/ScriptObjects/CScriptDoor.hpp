#ifndef _CSCRIPTDOOR
#define _CSCRIPTDOOR

#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"

class CPASAnimParmData;

class CScriptDoor : public CPhysicsActor {
public:
  // Guessed names; Echoes has separate door, shield and lock state machines.
  enum EDoorState {
    kDS_Closed,
    kDS_WaitingForArea,
    kDS_Opening,
    kDS_Open,
    kDS_CloseDelay,
    kDS_Closing,
  };
  enum EShieldState {
    kSS_Visible,
    kSS_FadingOut,
    kSS_Hidden,
    kSS_FadingIn,
  };
  enum ELockState {
    kLS_Unlocked,
    kLS_Pending,
    kLS_Locking,
    kLS_Locked,
    kLS_Unlocking,
  };
  enum EDoorAnimType {
    kDAT_Open,
    kDAT_Closing,
    kDAT_Opening,
    kDAT_Closed,
  };

  CScriptDoor(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
              const CTransform4f& xf, const CModelData& model,
              const rstl::optional_object< CModelData >& shellModel,
              const rstl::optional_object< CModelData >& blueShellModel,
              const rstl::optional_object< TLockedToken< CTexture > >& burnTexture,
              const CColor& shellColor, const CHealthInfo& health,
              const CDamageVulnerability& vulnerability, const CActorParameters& parameters,
              CAssetId alternateScan, const CVector3f& orbitOffset, const CAABox& bounds, bool open,
              bool locked, float openTime, float closeTime, float closeDelay,
              float shieldFadeOutTime, float shieldFadeInTime, bool ballDoor, bool horizontal);

  // CEntity
  ~CScriptDoor() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  CHealthInfo* HealthInfo() override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& point,
                                                         const CVector3f& normal,
                                                         const CWeaponMode& mode,
                                                         int attribs) const override;
  CScannableObjectInfo* GetScannableObjectInfo() const override;

  bool IsOpen() const { return mIsOpen; }
  bool IsHorizontal() const { return mHorizontal; }
  bool IsBallDoor() const { return mBallDoor; }
  TUniqueId GetConnectedDockID() const { return mDockId; }
  bool IsConnectedToArea(const CStateManager& mgr, TAreaId area) const;
  void ForceClosed(CStateManager& mgr);
  void ResetDoor(CStateManager& mgr);
  void SetDoorAnimation(EDoorAnimType animation);

  // Guessed names.
  void SetBurnOrigin(const CVector3f& position);

private:
  // Guessed names.
  int FindAnimation(const CPASAnimParmData& parms) const;
  void SetDoorState(CStateManager& mgr, EDoorState state);
  void UpdateShield(float dt, CStateManager& mgr);
  void SetShieldAlpha(float alpha, CStateManager& mgr);
  void UpdateLock(float dt, CStateManager& mgr);
  void SetLockState(CStateManager& mgr, ELockState state);
  void SetLockAnimation(CStateManager& mgr, int animation);
  void UpdateShellColor(float dt);
  void ResetBurnOrigin();

  // Target-derived field names; original spellings are not known.
  EDoorState mDoorState;
  float mOpenTime;
  float mCloseTime;
  float mCloseDelay;
  float mCloseTimer;
  EShieldState mShieldState;
  float mShieldAlpha;
  float mShieldFadeOutTime;
  float mShieldFadeInTime;
  rstl::optional_object< CModelData > mShellModel;
  rstl::optional_object< CModelData > mBlueShellModel;
  rstl::optional_object< TLockedToken< CTexture > > mBurnTexture;
  ELockState mLockState;
  TUniqueId mLockActorId;
  float mLockTimer;
  int mAnimationId;
  CAABox mBounds;
  CHealthInfo mCurrentHealth;
  CHealthInfo mInitialHealth;
  CDamageVulnerability mBaseVulnerability;
  CDamageVulnerability mCurrentVulnerability;
  TUniqueId mPartnerDoorId;
  TUniqueId mOpeningSenderDoorId;
  TUniqueId mDockId;
  CVector3f mOrbitOffset;
  int mOpenRequestCount;
  CVector3f mBurnOrigin;
  CColor mCurrentShellColor;
  CColor mShellColor;
  int mClosedAnimation;
  int mOpeningAnimation;
  int mClosingAnimation;
  int mOpenAnimation;
  rstl::single_ptr< TCachedToken< CScannableObjectInfo > > mAlternateScan;
  bool mWasOpen : 1;
  bool mIsOpen : 1;
  bool mBallDoor : 1;
  bool mInitiallyLocked : 1;
  bool mColorDirty : 1;
  bool mResetPending : 1;
  bool mHasReset : 1;
  bool mHorizontal : 1;
};
CHECK_SIZEOF(CScriptDoor, 0x4b0)

#endif // _CSCRIPTDOOR
