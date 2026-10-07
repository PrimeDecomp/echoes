#ifndef _CSCRIPTSAFEZONECRYSTAL
#define _CSCRIPTSAFEZONECRYSTAL

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class CElementGen;
class CGenDescription;
class CScannableObjectInfo;

// Guessed name. Crystal models for the expanded, entangled, hurtful and echo states.
struct SSafeZoneCrystalModels {
  SSafeZoneCrystalModels(const CModelData& normal, const CModelData& entangled,
                         const CModelData& hurtful, const CModelData& echo);

  CModelData mNormal;
  CModelData mEntangled;
  CModelData mHurtful;
  CModelData mEcho;
};
CHECK_SIZEOF(SSafeZoneCrystalModels, 0x130)

// Guessed names throughout; the class name follows the SAFC loader and SLdrSafeZoneCrystal.
class CScriptSafeZoneCrystal : public CActor {
public:
  enum EState {
    kS_Collapsed,
    kS_Entangled,
    kS_Expanded,
    kS_Hurtful,
    kS_Echo,
  };

  CScriptSafeZoneCrystal(TUniqueId uid, const CEntityInfo& info, const rstl::string& name,
                         const CTransform4f& xf, const CActorParameters& actorParms,
                         CAssetId scanCollapsed, CAssetId scanEntangled, CAssetId scanLight,
                         CAssetId scanAnnihilator, const CModelData& normalModel,
                         const CModelData& entangledModel, const CModelData& hurtfulModel,
                         const CModelData& echoModel, CAssetId collapsedEffect,
                         CAssetId expandedEffect, CAssetId entangledEffect, CAssetId hurtfulEffect,
                         CAssetId echoEffect, CAssetId refreshEffect, float maxTimeExpanded,
                         float maxTimeEntangled, float maxTimeHurtful, float maxTimeEcho,
                         float powerBeamHP, float refreshTime, float refreshDelay, bool isLight,
                         bool initiallyEntangled, const CVector3f& hitRadius,
                         const CVector3f& hitOffset, const CVector3f& effectOffset,
                         const CMayaSpline& fadeSpline);

  // CEntity
  ~CScriptSafeZoneCrystal() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  CHealthInfo* HealthInfo() override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CScannableObjectInfo* GetScannableObjectInfo() const override;

private:
  bool UpdateEffect(float dt, CElementGen* gen, bool alive, const CVector3f& pos, EState state,
                    float alpha) const;
  void UpdateEffectsVisibility(float dt, CStateManager& mgr);
  void UpdateRefreshEffect(float dt, CStateManager& mgr);
  void UpdateRegeneration(float dt, CStateManager& mgr);
  void UpdateStateTimer(float dt, CStateManager& mgr);
  void UpdateBounds();
  void UpdateModel(CStateManager& mgr);
  void UpdateVulnerability();
  void SetState(CStateManager& mgr, EState state);
  void ResetHealth();
  void SpawnRefreshEffect();
  bool KillEffectIfInactive(CElementGen* gen, EState state) const;
  static void AddEffectToRenderer(CElementGen* gen);

  rstl::single_ptr< TCachedToken< CScannableObjectInfo > > mScanCollapsed;     // 0x158
  rstl::single_ptr< TCachedToken< CScannableObjectInfo > > mScanEntangled;     // 0x15c
  rstl::single_ptr< TCachedToken< CScannableObjectInfo > > mScanLight;         // 0x160
  rstl::single_ptr< TCachedToken< CScannableObjectInfo > > mScanAnnihilator;   // 0x164
  rstl::single_ptr< SSafeZoneCrystalModels > mModels;                          // 0x168
  rstl::single_ptr< CElementGen > mCollapsedEffect;                            // 0x16c
  rstl::single_ptr< CElementGen > mExpandedEffect;                             // 0x170
  rstl::single_ptr< CElementGen > mEntangledEffect;                            // 0x174
  rstl::single_ptr< CElementGen > mHurtfulEffect;                              // 0x178
  rstl::single_ptr< CElementGen > mEchoEffect;                                 // 0x17c
  rstl::optional_object< TLockedToken< CGenDescription > > mRefreshEffectDesc; // 0x180
  rstl::single_ptr< CElementGen > mRefreshEffect;                              // 0x190
  float mRefreshCooldown;                                                      // 0x194
  float mMaxTimeExpanded;                                                      // 0x198
  float mMaxTimeEntangled;                                                     // 0x19c
  float mMaxTimeHurtful;                                                       // 0x1a0
  float mMaxTimeEcho;                                                          // 0x1a4
  float mRefreshTime;                                                          // 0x1a8
  float mRefreshDelay;                                                         // 0x1ac
  float mStateTimer;                                                           // 0x1b0
  float mRefreshTimer;                                                         // 0x1b4
  float mFadeTimer;                                                            // 0x1b8
  bool mIsLight : 1;                                                           // 0x1bc
  bool mDamaged : 1;
  bool mFadeDone : 1;
  bool mCollapsedAlive : 1;
  bool mExpandedAlive : 1;
  bool mHurtfulAlive : 1;
  bool mEntangledAlive : 1;
  bool mEchoAlive : 1;
  bool mDisabled : 1; // 0x1bd
  bool mInitiallyEntangled : 1;
  CVector3f mHitRadius;                // 0x1c0
  CVector3f mHitOffset;                // 0x1cc
  CVector3f mEffectOffset;             // 0x1d8
  EState mState;                       // 0x1e4
  CHealthInfo mHealthInfo;             // 0x1e8
  CDamageVulnerability mVulnerability; // 0x208
  CAABox mBounds;                      // 0x238
  CMayaSpline mFadeSpline;             // 0x250
};

#endif // _CSCRIPTSAFEZONECRYSTAL
