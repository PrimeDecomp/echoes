#ifndef _CPLASMAPROJECTILE
#define _CPLASMAPROJECTILE

#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/Weapons/CBeamProjectile.hpp"

class CBeamInfo;
class CWeaponAssetInfo;
class CElectricDescription;
class CGenDescription;
class CTexture;

class CPlasmaProjectile : public CBeamProjectile {
public:
  enum EExpansionState { kES_Inactive, kES_Attack, kES_Sustain, kES_Release, kES_Done };

  CPlasmaProjectile(const TToken< CWeaponDescription >& description, const rstl::string& name,
                    EWeaponType type, const CBeamInfo& beamInfo, const CTransform4f& xf,
                    EMaterialTypes material, const CDamageInfo& damage, TUniqueId uid,
                    TAreaId areaId, TUniqueId owner, const CWeaponAssetInfo& resources,
                    bool drawOwnerFirst, uint attribs);

  // CEntity
  ~CPlasmaProjectile() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;

  // CBeamProjectile
  void UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr) override;
  void ResetBeam(CStateManager& mgr, bool fullReset) override;
  void Fire(const CTransform4f& xf, CStateManager& mgr, bool flag) override;

  void SetInitialDamage(float damage);
  CColor GetInnerColor() const { return mInnerColor; }
  CColor GetOuterColor() const { return mOuterColor; }
  bool IsFiring() const { return mFiring; }

private:
  static const int kMaxPlasmaLights;
  static const float kInvMaxPlasmaLights;

  float UpdateBeamState(float dt, CStateManager& mgr);
  void MakeBillboardEffect(const rstl::optional_object< TToken< CGenDescription > >& particle,
                           const rstl::optional_object< TToken< CElectricDescription > >& electric,
                           const rstl::string& name, CStateManager& mgr, uint playerMask);
  void UpdatePlayerEffects(float dt, CStateManager& mgr);
  void RenderBeam(int subdivisions, float width, const CColor& color, int flags) const;
  void RenderMotionBlur() const;
  void UpdateEnergyPulse(float dt);
  void SetLightsActive(bool active, CStateManager& mgr);
  void CreatePlasmaLights(uint sourceId, const CLight& light, CStateManager& mgr);
  void DeletePlasmaLights(CStateManager& mgr);
  void UpdateLights(float expansion, float dt, CStateManager& mgr);

  rstl::vector< TUniqueId > mLights;
  int mBeamAttributes;
  float mLifeTime;
  float mPulseSpeed;
  float mShutdownTime;
  float mExpansionSpeed;
  float mMaxLength;
  CColor mCoreColor; // Guessed name
  CColor mInnerColor;
  CColor mOuterColor;
  CDamageInfo mPhazonDamage;
  EExpansionState mExpansionState;
  float mInitialDamage; // Guessed name
  float mBeamWidth;
  float mLifeTimer;
  float mExpansionT;
  float mExpansion;
  float mBeamAngle;
  float mEnergyPulseStartY;
  float mShutdownTimer;
  float mContactPulseTimer;
  float mEnergyPulseTimer;
  float mPlayerEffectPulseTimer;
  float mPlayerDamageDuration;
  float mPlayerDamageTimer;
  TCachedToken< CTexture > mTexture;
  TCachedToken< CTexture > mGlowTexture;
  TLockedToken< CGenDescription > mPulseFxDesc;
  rstl::optional_object< TLockedToken< CGenDescription > > mContactFxDesc;
  rstl::optional_object< TLockedToken< CGenDescription > > mMuzzleFxDesc; // Guessed name
  rstl::single_ptr< CElementGen > mContactGen;
  rstl::single_ptr< CElementGen > mPulseGen;
  rstl::single_ptr< CElementGen > mWeaponGen;
  rstl::single_ptr< CElementGen > mMuzzleGen; // Guessed name
  CVector3f mMuzzleScale;                     // Guessed name
  CAssetId mFreezeSteamTxtr;
  CAssetId mFreezeIceTxtr;
  rstl::optional_object< TToken< CElectricDescription > > mVisorElectric;
  rstl::optional_object< TToken< CGenDescription > > mVisorParticle;
  TSfxId mFreezeSfx;
  TSfxId mElectricSfx;
  TUniqueId mSustainedDamagePlayerId; // Guessed name
  bool x6a6_0_ : 1;
  bool mEnableEnergyPulse : 1;
  bool mFiring : 1;
  bool mTexturesLoaded : 1;
  bool mDrawOwnerFirst : 1;
  bool mInitialDamageEnabled : 1; // Guessed name
  bool mInitialDamagePending : 1; // Guessed name
};
CHECK_SIZEOF(CPlasmaProjectile, 0x6a8)

#endif // _CPLASMAPROJECTILE
