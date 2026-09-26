#ifndef _CPROJECTILEWEAPON
#define _CPROJECTILEWEAPON

#include "Weapons/CWeaponDescription.hpp"
#include "Weapons/IWeaponProjectile.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/ActorCommon.hpp"

class CDecalDescription;
class CElementGen;
class CParticleSwoosh;

class CProjectileWeapon : public IWeaponProjectile {
public:
  CProjectileWeapon(const TToken< CWeaponDescription >& description, const CVector3f& worldOffset,
                    const CTransform4f& localToWorld, const CVector3f& scale, int flags);

  // IWeaponProjectile
  ~CProjectileWeapon() override;
  bool Update(float dt) override;
  void AddToRenderer() const override;
  void Render() const override;
  const CVector3f GetTranslation() const override;
  const CTransform4f GetTransform() const override;

  void UpdateParticleFX();
  void SetRelativeOrientation(const CTransform4f& orientation);
  void SetWorldSpaceOrientation(const CTransform4f& orientation);
  void UpdatePSTranslationAndOrientation();
  void UpdateChildParticleSystems(float dt, bool translationOnly);
  const bool IsSystemDeletable() const;
  void RenderParticles() const;
  void DeactivateProjectile();
  rstl::optional_object< TLockedToken< CGenDescription > >
  CollisionOccured(EWeaponCollisionResponseTypes type, bool deflected, bool useTarget,
                   bool keepActive, const CVector3f& position, const CVector3f& normal,
                   const CVector3f& target);
  uint GetSoundIdForCollision(EWeaponCollisionResponseTypes type) const;
  rstl::optional_object< TLockedToken< CDecalDescription > >
  GetDecalForCollision(EWeaponCollisionResponseTypes type) const;
  float GetAudibleRange() const;
  float GetAudibleFallOff() const;
  float GetMaxTurnRate() const;
  void SetVelocity(const CVector3f& velocity);
  const CVector3f& GetVelocity() const;
  void SetGravity(const CVector3f& gravity);
  const CVector3f& GetGravity() const;
  rstl::optional_object< CAABox > GetBounds() const;
  static float GetTickTime();
  static void SetGlobalSeed(uint seed);
  void SetParticleTranslationOffset(const CVector3f& offset); // Guessed name

private:
  // Guessed names: Echoes-only billboard/trail evaluation and rendering.
  void UpdateBillboardEffects();
  void RenderBillboardEffects() const;

  static uint skGlobalSeed;
  static bool sDisableAlphaUpdates; // Guessed name
  TLockedToken< CWeaponDescription > mWeaponDesc;
  CRandom16 mRandom;
  mutable CRandom16 mRenderRandom; // Guessed name
  CTransform4f mLocalToWorldXf;
  CTransform4f mLocalXf;
  CVector3f mWorldOffset;
  CVector3f mPreviousLocalOffset;
  CVector3f mLocalOffset;
  CVector3f mInterpolationOffset;
  CVector3f mProjOffset;
  CVector3f mScale;
  CVector3f mLocalOffset2;
  CVector3f mGlobalScale; // Guessed name
  CVector3f mVelocity;
  CVector3f mGravity;
  // Guessed names, derived from B1PO/B2PO/TLPO and their render consumers.
  CVector3f mBillboard1Offset;
  CVector3f mBillboard2Offset;
  CVector3f mTrailOffset;
  CVector3f mParticleTranslationOffset; // Guessed name
  CColor mAmbientLightColor;
  // Guessed names, derived from B1SE/B2SE/TSZE/TLEN.
  float mBillboard1Size;
  float mBillboard2Size;
  float mTrailSize;
  float mTrailLength;
  double mCurTime;
  double mRemainderTime;
  float mMaxTurnRate;
  int mFlags;
  int mLifetime;
  int mChildSystemUpdateRate;
  int mCurFrame;
  int mLastParticleFrame;
  int x160_; // Collision responses are suppressed while mCurFrame is below this value.
  CElementGen* mAPSMGen;
  CElementGen* mAPS2Gen;
  CElementGen* x16c_;
  rstl::optional_object< TLockedToken< CModel > > mModel;
  CParticleSwoosh* mSwoosh1;
  CParticleSwoosh* mSwoosh2;
  CParticleSwoosh* mSwoosh3;
  bool mActive : 1;
  bool mAPSO : 1;
  bool mAP11 : 1;
  bool mAP21 : 1;
  bool mAS11 : 1;
  bool mAS12 : 1;
  bool mAS13 : 1;
  bool mVMD2 : 1;
  bool mHasBillboardEffects : 1;          // Guessed name
  bool mUseParticleTranslationOffset : 1; // Guessed name
};
CHECK_SIZEOF(CProjectileWeapon, 0x190)

#endif // _CPROJECTILEWEAPON
