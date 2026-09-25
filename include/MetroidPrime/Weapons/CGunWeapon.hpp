#ifndef _CGUNWEAPON
#define _CGUNWEAPON

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Weapons/GunController/CGunMotion.hpp"
#include "MetroidPrime/Weapons/WeaponCommon.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#include "Collision/CMaterialList.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CGenDescription;
class CGunController;
class CAnimCharacterSet;
class CWeaponDescription;
class CElementGen;
class CRainSplashGenerator;
class CTransform4f;
class CModelFlags;
class CActorLights;
class SWeaponInfo;
class SShotParam;

enum EFrozenFxType {
  kFFT_None,
  kFFT_Frozen,
  kFFT_Thawed,
};

class CVelocityInfo {
public:
  ~CVelocityInfo();

  CVector3f& Velocity(int i) { return mVel[i]; }
  const CVector3f& GetVelocity(int i) const { return mVel[i]; }
  bool GetTargetHoming(int i) const { return mTargetHoming[i]; }

  void Clear();

  void AddVelocity(const CVector3f& vel) { mVel.push_back(vel); }
  void AddTargetHoming(const bool& homing) { mTargetHoming.push_back(homing); }
  void AddTrat(const float& trat) { mTrat.push_back(trat); }

private:
  rstl::reserved_vector< CVector3f, 2 > mVel;
  rstl::reserved_vector< bool, 2 > mTargetHoming;
  rstl::reserved_vector< float, 2 > mTrat;
};

class CGunWeapon {
public:
  CGunWeapon(EWeaponType type, TUniqueId playerId, const CVector3f& scale, int);
  virtual ~CGunWeapon();

  enum ESecondaryFxType {
    kSFT_None,
    kSFT_Charge,
    kSFT_ToCombo,
    kSFT_CancelCharge,
  };

  // Virtual Methods
  virtual void Reset(CStateManager& mgr);
  virtual void PlayAnim(NWeaponTypes::EGunAnimType type, bool loop);
  virtual void PreRenderGunFx(const CStateManager& mgr, const CTransform4f& xf);
  virtual void PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf);
  virtual void UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                           const CTransform4f& xf);
  virtual void Fire(CToken& token, bool underwater, float dt, CPlayerState::EChargeStage chargeState,
                      const CTransform4f& xf, CStateManager& mgr, TUniqueId homingTarget,
                      int unk1, ushort unk2, TUniqueId id, CSfxHandle sfx, float chargeFactor1, float chargeFactor2);
  virtual void EnableFx(bool enable);
  virtual void EnableSecondaryFx(ESecondaryFxType type);
  virtual void Draw(bool drawSuitArm, const CStateManager& mgr, const CTransform4f& xf,
                    const CModelFlags& flags, const CActorLights* lights) const;
  virtual void DrawMuzzleFx(const CStateManager& mgr) const;
  virtual void Update(float dt, CStateManager& mgr);
  
  virtual void Unk7();
  virtual void ActivateCharge();
  virtual void Unk8();
  virtual void Unk9(CStateManager& mgr);

  virtual void Load(CStateManager& mgr, bool subtypeBasePose);
  virtual void Unload(CStateManager& mgr);
  virtual bool IsLoaded() const;

  virtual void Unk10();
  virtual void Unk11(CStateManager& mgr);

  const CVelocityInfo& GetVelocityInfo() const { return mVelInfo; }
  rstl::optional_object< CModelData >& SolidModelData() { return mSolidModelData; }
  const CModelData& GetSolidModelData() const { return mSolidModelData.data(); }

  EWeaponType GetType() const { return mWeaponType; }
  TUniqueId GetPlayerId() const { return mPlayerId; }
  EMaterialTypes GetPlayerMaterial() const { return mPlayerMaterial; }

  CAABox GetBounds() const;
  CAABox GetBounds(const CTransform4f& xf) const;
  const SWeaponInfo& GetWeaponInfo() const;
  void ActivateCharge(bool enable, bool resetEffect);
  bool PlayPasAnim(SamusGun::EAnimationState state, CStateManager& mgr, float angle);
  bool IsChargeAnimOver() const;
  void UpdateMuzzleFx(float dt, const CVector3f& scale, const CVector3f& pos, bool emitting);
  CElementGen* GetChargeMuzzleFx() const;
  void DrawHologram(const CStateManager& mgr, const CTransform4f& xf,
                    const CModelFlags& flags) const;
  void ReturnToDefault(CStateManager& mgr);
  bool ComboFireOver() const;
  void EnterFidget(CStateManager& mgr, SamusGun::EFidgetType type, int parm2);
  void Touch(const CStateManager& mgr);
  void TouchHolo(const CStateManager& mgr);
  void AsyncLoadSuitArm(CStateManager& mgr);
  void AsyncLoadFidget(CStateManager& mgr, SamusGun::EFidgetType type, int animSet);
  void UnLoadFidget();
  bool IsFidgetLoaded();
  void EnableFrozenEffect(EFrozenFxType type);

  CDamageInfo GetDamageInfo(CStateManager& mgr, CPlayerState::EChargeStage chargeState,
                            float chargeFactor);
  CDamageInfo GetShotDamageInfo(const SShotParam& shotParam, CStateManager& mgr) const;

protected:
  // x0 is vtable
  CVector3f mScale;
  rstl::optional_object< CAABox > caabox;
  rstl::optional_object< CModelData > mSolidModelData;
  rstl::optional_object< CModelData > mHoloModelData;
  rstl::optional_object< CModelData > mSuitArmModelData;
  CPlayerState::EPlayerSuit currentPlayerSuit;
  rstl::single_ptr< CGunController > mGunController;
  TToken< CAnimCharacterSet > mGunCharacter;
  rstl::vector< CToken > mAnims;
  rstl::vector< int > mUnk;
  rstl::vector< CToken > mDeps;

  TToken< CAnimCharacterSet > mArmCharacter;
  rstl::reserved_vector< TCachedToken< CWeaponDescription >, 2 > mWeapons;
  TCachedToken< CGenDescription > mXferEffect;
  rstl::reserved_vector< TCachedToken< CGenDescription >, 2 > mMuzzleEffects;
  rstl::reserved_vector< TCachedToken< CGenDescription >, 2 > mFrozenEffects;
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 2 > mMuzzleGenerators;
  rstl::single_ptr< CElementGen > mFrozenGenerator;
  CRainSplashGenerator* mRainSplashGenerator;
  EWeaponType mWeaponType;
  TUniqueId mPlayerId;
  EMaterialTypes mPlayerMaterial;
  ESecondaryFxType mEnabledSecondaryEffect;  // TODO: at 0x210
  CVelocityInfo mVelInfo;
  CPlayerState::EBeamId mBeamId;
  EFrozenFxType mFrozenEffect;
  uint mMuzzleEffectIdx;
  uint mShaderIdx;
  // 0x1: load request, 0x2: muzzle fx, 0x4: projectile data, 0x8: anims, 0x10: everything else
  int mLoadFlags;
  CAssetId mAncsId;
  bool x218_24 : 1;
  bool mEnableCharge : 1;
  bool mLoaded : 1;
  // Initialize in selected beam's pose, rather than power beam's pose
  bool mSubtypeBasePose : 1;
  bool mSuitArmLocked : 1;
  bool mDrawHologram : 1;

  char _filler[56];

  static const char* skMuzzleLocator;
  static const char* skElbowLocator;
  static const int skShootAnim[2];

  void AllocResPools(CPlayerState::EBeamId beam);
  void FreeResPools();
  static void FillTokenVector(const rstl::vector< SObjectTag >& tags,
                              rstl::vector< CToken >& objects, bool includeTxtr);
  void BuildDependencyList(CPlayerState::EBeamId beam);
  void LoadSuitArm(CStateManager& mgr);
  void LoadGunModels(CStateManager& mgr);
  void LoadAnimations();
  bool IsAnimsLoaded() const;
  void LoadMuzzleFx(float dt);
  void LoadProjectileData(CStateManager& mgr);
  void LoadFxIdle(float dt, CStateManager& mgr);
  void LockTokens(CStateManager& mgr);
  void UnlockTokens();

  static void PointGenerator(void*, const CVector3f*, const CVector3f*, int);
};
CHECK_SIZEOF(CGunWeapon, 0x274)

#endif // _CGUNWEAPON
