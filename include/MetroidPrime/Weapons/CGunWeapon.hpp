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
void DrawClipCube(const CAABox& bounds);
class CPlayer;
class CSkinnedModel;
struct SSkinningWorkspace;

enum EFrozenFxType {
  kFFT_None,
  kFFT_Frozen,
  kFFT_Thawed,
};

class CVelocityInfo {
public:
  ~CVelocityInfo() {}

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
  CGunWeapon(EWeaponType type, TUniqueId playerId, const CVector3f& scale, int flags);
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
  virtual void PreRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {}
  virtual void PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf);
  virtual void UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                           const CTransform4f& xf);
  virtual void Fire(const TToken< CWeaponDescription >& projectile, bool underwater, float dt,
                    CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                    CStateManager& mgr, TUniqueId homingTarget, uint projectileAttributes,
                    ushort soundId, TUniqueId* projectileId, CSfxHandle* soundHandle,
                    float chargeFactor1, float chargeFactor2);
  virtual void EnableFx(bool enable) {}
  virtual void EnableSecondaryFx(ESecondaryFxType type) { mEnabledSecondaryEffect = type; }
  virtual void Draw(bool drawSuitArm, int playerIndex, const CStateManager& mgr,
                    const CTransform4f& xf, const CModelFlags& flags,
                    const CActorLights* lights) const;
  virtual void DrawMuzzleFx(const CStateManager& mgr) const;
  virtual void Update(float dt, CStateManager& mgr);

  virtual void UpdateMuzzleFx(float dt, const CVector3f& scale, const CVector3f& pos,
                              bool emitting);
  virtual void ActivateCharge(bool enable, bool resetEffect);
  virtual void OnChargeReset() {} // Guessed name; default charge-reset hook.
  virtual void InitializeResources(CStateManager& mgr); // Guessed name

  virtual void Load(CStateManager& mgr, bool subtypeBasePose);
  virtual void Unload(CStateManager& mgr);
  virtual bool IsLoaded() const;

  virtual void ReleaseResources(CStateManager& mgr); // Guessed name
  // Guessed name
  virtual void SetModelTouchEnabled(bool enabled) { mModelTouchEnabled = enabled; }

  const CVelocityInfo& GetVelocityInfo() const { return mVelInfo; }
  rstl::optional_object< CModelData >& SolidModelData() { return mSolidModelData; }
  const CModelData& GetSolidModelData() const { return mSolidModelData.data(); }

  EWeaponType GetType() const { return mWeaponType; }
  const TCachedToken< CWeaponDescription >&
  GetProjectileToken(CPlayerState::EChargeStage stage) const { return mWeapons[stage]; }
  TUniqueId GetPlayerId() const { return mPlayerId; }
  EMaterialTypes GetPlayerMaterial() const { return mPlayerMaterial; }

  CAABox GetBounds() const;
  CAABox GetBounds(const CTransform4f& xf) const;
  const SWeaponInfo& GetWeaponInfo() const;
  void EnterComboFire(CStateManager& mgr); // Guessed name
  bool IsChargeAnimOver() const;
  CElementGen* GetMuzzleFx(int index) const; // Guessed name
  void DrawHologram(const CStateManager& mgr, const CTransform4f& xf,
                    const CModelFlags& flags) const;
  void ReturnToDefault(CStateManager& mgr, bool reset);
  void EnterFidget(CStateManager& mgr, SamusGun::EFidgetType type, int animSet);
  void Touch(const CStateManager& mgr);
  void TouchHolo(const CStateManager& mgr);
  void AsyncLoadSuitArm();
  void AsyncLoadFidget(CStateManager& mgr, SamusGun::EFidgetType type, int animSet);
  void UnLoadFidget();
  bool IsFidgetLoaded();
  void EnableFrozenEffect(EFrozenFxType type);

  CDamageInfo GetDamageInfo(CStateManager& mgr, CPlayerState::EChargeStage chargeState,
                            float chargeFactor);
  float GetAnimDuration(NWeaponTypes::EGunAnimType type) const; // Guessed name
  CPlayer* GetPlayer(CStateManager& mgr) const;
  CPlayer* GetPlayerFromAll(CStateManager& mgr) const; // Guessed name
  const CVector3f& GetRainSplashPosition() const { return mRainSplashPosition; }
  void SetRainSplashGenerator(CRainSplashGenerator* generator) { mRainSplashGenerator = generator; }
  void SetSpeedUpAnimation(bool enabled) { mSpeedUpAnimation = enabled; }
  bool GetSpeedUpAnimation() const { return mSpeedUpAnimation; }
  bool IsSpecialAnimationPlaying() const { return mSpecialAnimationPlaying; }
  static const char* GetMuzzleLocatorName() { return skMuzzleLocator; }
  void SetSpecialAnimationPlaying(bool playing) { mSpecialAnimationPlaying = playing; }
  void SetEnableCharge(bool enabled) { mEnableCharge = enabled; }
  void SetSoundVolume(short volume) { mSoundVolume = volume; }
  TCachedToken< CGenDescription >& GetTransferEffect() { return mXferEffect; }
  static void FillTokenVector(const rstl::vector< SObjectTag >& tags,
                              rstl::vector< CToken >& objects, bool includeTxtr);

protected:
  // x0 is vtable
  CVector3f mScale;
  mutable rstl::optional_object< CAABox > mBounds;
  rstl::optional_object< CModelData > mSolidModelData;
  rstl::optional_object< CModelData > mHoloModelData;
  rstl::optional_object< CModelData > mSuitArmModelData;
  CPlayerState::EPlayerSuit mCurrentPlayerSuit;
  rstl::single_ptr< CGunController > mGunController;
  rstl::optional_object< TToken< CAnimCharacterSet > > mGunCharacter;
  rstl::vector< CToken > mAnims;
  rstl::vector< CToken > x140_;
  rstl::vector< CToken > mDeps;
  rstl::vector< int > mAnimIds;
  rstl::vector< int > mShootAnimIds;

  TToken< CModel > mArmModel;
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
  ESecondaryFxType mEnabledSecondaryEffect;
  CVelocityInfo mVelInfo;
  CPlayerState::EBeamId mBeamId;
  EFrozenFxType mFrozenEffect;
  uint mMuzzleEffectIdx;
  uint mShaderIdx;
  // 0x1: load request, 0x2: muzzle fx, 0x4: projectile data, 0x8: anims, 0x10: everything else
  int mLoadFlags;
  CAssetId mAncsId;
  short mSoundVolume;
  float mAnimationTimer;         // Guessed name
  CVector3f mRainSplashPosition; // Guessed name
  bool x270_24 : 1;
  bool mEnableCharge : 1;
  bool mLoaded : 1;
  // Initialize in selected beam's pose, rather than power beam's pose
  bool mSubtypeBasePose : 1;
  bool mSuitArmLocked : 1;
  bool mDrawHologram : 1;
  bool mResourcesAllocated : 1;      // Guessed name
  bool mSpecialAnimationPlaying : 1; // Guessed name
  bool mSpeedUpAnimation : 1;        // Guessed name
  bool mModelTouchEnabled : 1;       // Guessed name
  bool x271_26 : 1;

  static const char* const skMuzzleLocator;
  static const char* const skElbowLocator;

  void AllocResPools(CPlayerState::EBeamId beam);
  void FreeResPools();
  void BuildDependencyList(CPlayerState::EBeamId beam);
  void LoadSuitArm();
  void LoadGunModels();
  void BuildAnimationIdList(const CAnimData& animData); // Guessed name
  void LoadAnimations();
  bool IsAnimsLoaded() const;
  void LoadMuzzleFx(float dt);
  void LoadProjectileData(CStateManager& mgr);
  void LoadFxIdle(float dt, CStateManager& mgr);
  void LockTokens();
  void UnlockTokens();

  static void PointGenerator(const CSkinnedModel& model, const SSkinningWorkspace& workspace,
                             void* context);
};
CHECK_SIZEOF(CGunWeapon, 0x274)

#endif // _CGUNWEAPON
