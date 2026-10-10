#ifndef _CSCRIPTPLAYERTURRET
#define _CSCRIPTPLAYERTURRET

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"

#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/TToken.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include "rstl/optional_object.hpp"

class CFinalInput;

// Guessed class: a turret the player rides. The base actor carries the animation, and the hull
// actor is the collision object that sits on it. Guessed member names.
class CScriptPlayerTurret : public CActor {
public:
  CScriptPlayerTurret(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                      const CTransform4f& xf, uint flags, float horizontalRotationLeft,
                      float horizontalRotationRight, float verticalElevationUp,
                      float verticalElevationDown, float damageAngle, float horizontalSpeed,
                      float verticalSpeed, float fireRate, const CDamageInfo& weaponDamage,
                      CAssetId weaponEffect, CAssetId weaponEffectMultiPlayer, ushort sfxRotation,
                      ushort sfxSinglePlayerImpact, ushort sfxMultiPlayerImpact,
                      ushort sfxSinglePlayerProjectile, ushort sfxMultiPlayerProjectile);

  // CEntity
  ~CScriptPlayerTurret() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;

  // Member-pointer bridge entries (SPlayerTurret_FuncPtrs). Guessed names.
  CTransform4f GetCameraTransform(CStateManager& mgr);             // Guessed name
  CTransform4f GetTurretTransform(CStateManager& mgr);             // Guessed name
  void ExitTurret(CStateManager& mgr);                             // Guessed name
  void ProcessInput(const CFinalInput& input, CStateManager& mgr); // Guessed name
  TUniqueId GetHullActorId();                                      // Guessed name

  float GetMaxAimAngle() const { return mMaxAimAngle; }
  void SetTargetPosition(const CVector3f& position) { mTargetPosition = position; }

private:
  void AttachToActors(CStateManager& mgr);                   // Guessed name
  CTransform4f GetMuzzleTransform(CStateManager& mgr) const; // Guessed name
  void Fire(CStateManager& mgr);                             // Guessed name
  void EnableHullCollision(CStateManager& mgr);              // Guessed name
  void DisableHullCollision(CStateManager& mgr);             // Guessed name

  CAABox mBounds;                            // Guessed name
  uint mFlags;                               // Guessed name
  float mHorizRotationLeft;                  // Guessed name
  float mHorizRotationRight;                 // Guessed name
  float mVertElevationUp;                    // Guessed name
  float mVertElevationDown;                  // Guessed name
  float mMaxAimAngle;                        // 0x184, guessed name
  TUniqueId mPlayerId;                       // Guessed name
  float mHorizSpeed;                         // Guessed name
  float mVertSpeed;                          // Guessed name
  float mFireRate;                           // Guessed name
  TUniqueId mBaseId;                         // 0x198, guessed name
  TUniqueId mHullId;                         // 0x19A, guessed name
  int mAimUpAnim;                            // Guessed name
  int mAimDownAnim;                          // Guessed name
  int mFireAnim;                             // Guessed name
  int mDeathAnim;                            // Guessed name
  int mIdleAnim;                             // Guessed name
  float mTargetElevation;                    // Guessed name
  float mElevation;                          // Guessed name
  float mFireTimer;                          // Guessed name
  float mRotationSfxTimer;                   // Guessed name
  float mAlignment;                          // Guessed name
  CVector3f mLastForward;                    // Guessed name
  float mHorizInput;                         // Guessed name
  CModelFlags mBaseModelFlags;               // Guessed name
  CAssetId mWeaponEffect;                    // Guessed name
  CAssetId mWeaponEffectMultiPlayer;         // Guessed name
  TToken< CWeaponDescription > mWeaponToken; // Guessed name
  CDamageInfo mDamageInfo;                   // Guessed name
  float mDamageFlashTimer;                   // Guessed name
  CVector3f mTargetPosition;                 // 0x210
  CSfxHandle mRotationSfxHandle;             // Guessed name
  TUniqueId mDamagerId;                      // Guessed name
  ushort mSfxRotation;                       // Guessed name
  ushort mSfxSinglePlayerImpact;             // Guessed name
  ushort mSfxMultiPlayerImpact;              // Guessed name
  ushort mSfxSinglePlayerProjectile;         // Guessed name
  ushort mSfxMultiPlayerProjectile;          // Guessed name
  bool mPlayerInTurret : 1;                  // Guessed name
  bool x22c_25_ : 1;
};
CHECK_SIZEOF(CScriptPlayerTurret, 0x230)

#endif // _CSCRIPTPLAYERTURRET
