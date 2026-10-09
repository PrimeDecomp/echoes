#ifndef _CSCRIPTSAFEZONE
#define _CSCRIPTSAFEZONE

#include "MetroidPrime/CDarkWorldInfo.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTriggerEllipsoid.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"

class CElementGen;
class CGenDescription;
class CLight;
struct SEchoParameters;

// Guessed name. Fog fade requested while a player camera is inside or outside the zone.
struct CSafeZoneFog {
  CSafeZoneFog(bool enabled, ERglFogMode mode, const CColor& color, const CVector2f& range,
               float colorRate, const CVector2f& rangeRate);

  bool mEnabled;
  ERglFogMode mMode;
  CColor mColor;
  CVector2f mRange;
  float mColorRate;
  CVector2f mRangeRate;
};
CHECK_SIZEOF(CSafeZoneFog, 0x20)

// Scaffold; evidence in Echoes research/CScriptSafeZone-CScriptTriggerEllipsoid-G2ME01.md.
class CScriptSafeZone : public CScriptTriggerEllipsoid {
public:
  // Guessed names.
  enum EZoneType {
    kZT_Normal,
    kZT_Hurtful,
    kZT_Echo,
  };

  CScriptSafeZone(TUniqueId uid, const rstl::string& name, CEntityInfo& info,
                  const CVector3f& scale, const CTransform4f& xf, const CDamageInfo& damage,
                  const CVector3f& forceField, float activationTime, float deactivationTime,
                  float lifetime, float randomLifetimeOffset, float insideFadeStart,
                  float insideFadeTime, float insideFadeMinAlpha, float flashTime, uint flags,
                  bool deactivateOnEnter, bool deactivateOnExit, CAssetId impactEffect,
                  const CDarkWorldInfo& normalInfo, const CDarkWorldInfo& hurtfulInfo,
                  const CDarkWorldInfo& echoInfo, const CDamageInfo& normalDamage,
                  const CDamageInfo& hurtfulDamage, bool filterSoundEffects, int lowPassFrequency,
                  bool ignoreCinematicCamera, bool mobile, bool generateMobileLight,
                  const CVector3f& mobileLightOffset, EShapeType shape,
                  const CSafeZoneFog& insideFog, const CSafeZoneFog& outsideFog,
                  const SEchoParameters& echoParameters, float flashBrightness, ushort flashSound,
                  CColor insideFilterColor, float insideFilterTime);

  // CEntity
  ~CScriptSafeZone() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;
  void SetActive(const bool active) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;

  // CScriptTrigger
  void InhabitantAdded(CActor& actor, CStateManager& mgr) override;
  void InhabitantIdle(CActor& actor, CStateManager& mgr, float dt) override;
  void InhabitantExited(CActor& actor, CStateManager& mgr) override;
  bool ShouldSendScriptMsgs(CActor& actor, CStateManager& mgr) const override;

  // CScriptSafeZone; reconstructed names/qualification for the native extra slots.
  virtual bool IsHurtful() const;
  virtual void SpawnImpactEffect(const CVector3f& position, float scale);

  void ApplyRenderEffect(CStateManager& mgr); // Guessed name.
  void DamageActor(CStateManager& mgr, TUniqueId id, float dt); // Name from the Wii SEL export.

  // Shell appearance used while teleporting; WorldTeleporter reads the first entry.
  const CDarkWorldInfo& GetDarkWorldInfo() const { return mNormalInfo; }
  EZoneType GetZoneType() const { return mZoneType; }

private:
  // Guessed names.
  void SetZoneType(EZoneType type);
  void UpdateObstruction(CStateManager& mgr, bool enable);
  void ModifyObstruction(CStateManager& mgr, int delta, int type);
  void RenderDarkVisorSpot(const CStateManager& mgr) const;
  void UpdatePlayerInside(CActor& actor, bool inside, CStateManager& mgr);
  void HandleProjectile(CActor& actor, CStateManager& mgr);
  void ApplyFog(CStateManager& mgr, const CSafeZoneFog& fog);
  void ApplyInsideFilter(CStateManager& mgr);
  void UpdateSafeZoneManager(CStateManager& mgr);
  void UpdateEchoEmitter(float dt, CStateManager& mgr);
  void UpdateInsideAlpha(float dt, CStateManager& mgr);
  void UpdateFlash(float dt, CStateManager& mgr);
  void UpdateLoopSound(float dt, CStateManager& mgr);
  void SetLowPassFilter(bool enabled);
  void PlayActivateSound(CStateManager& mgr);
  void PlayDeactivateSound(CStateManager& mgr);
  void PlaySound(CStateManager& mgr, ushort sfx, uint flags);
  CLight BuildLight() const;

  float mActivationTime;       // 0x200
  float mDeactivationTime;     // 0x204
  float mActivation;           // 0x208
  float mLifetime;             // 0x20c
  float mRandomLifetimeOffset; // 0x210
  float mLifeTimer;            // 0x214
  float mShellPulse;           // 0x218
  float mLoopSoundDelay;       // 0x21c
  float x220_;
  float mMinRadius;          // 0x224
  float mInsideTime;         // 0x228
  float mInsideAlpha;        // 0x22c
  float mInsideFadeStart;    // 0x230
  float mInsideFadeTime;     // 0x234
  float mInsideFadeMinAlpha; // 0x238
  float mFlashTimer;         // 0x23c
  float mFlashTime;          // 0x240
  float mFlashBrightness;    // 0x244
  ushort mFlashSound;        // 0x248
  bool mCameraInside : 1;    // 0x24a
  bool mPrevCameraInside : 1;
  bool mIgnoreCinematicCamera : 1;
  bool mFogDirty : 1;
  bool mMobile : 1;
  bool mGenerateMobileLight : 1;
  bool mFilterSoundEffects : 1;
  int mLowPassFrequency;                                   // 0x24c
  int mLowPassFilterId;                                    // 0x250
  CVector3f mMobileLightOffset;                            // 0x254
  TUniqueId mLightId;                                      // 0x260
  TUniqueId x262_;                                         // 0x262
  CDarkWorldInfo mNormalInfo;                              // 0x264
  CDarkWorldInfo mHurtfulInfo;                             // 0x2d4
  CDarkWorldInfo mEchoInfo;                                // 0x344
  CDamageInfo mNormalDamage;                               // 0x3b4
  CDamageInfo mHurtfulDamage;                              // 0x3d0
  CDarkWorldInfo* mCurrentInfo;                            // 0x3ec
  EZoneType mZoneType;                                     // 0x3f0
  int mObstructionType;                                    // 0x3f4
  CVector3f mObstructionPos;                               // 0x3f8
  float mObstructionRadius;                                // 0x404
  CSafeZoneFog mInsideFog;                                 // 0x408
  CSafeZoneFog mOutsideFog;                                // 0x428
  CColor mInsideFilterColor;                               // 0x448
  float mInsideFilterTime;                                 // 0x44c
  TToken< CGenDescription > mImpactEffect;                 // 0x450
  rstl::list< rstl::auto_ptr< CElementGen > > mImpactGens; // 0x458
  rstl::list< TUniqueId > mProjectiles;                    // 0x470
};
CHECK_SIZEOF(CScriptSafeZone, 0x488)

// Existing DOL forwarder into the loaded SafeZone REL callback.
void SafeZone_ApplyRenderEffect(CEntity& entity, CStateManager& mgr); // Guessed name.

#endif // _CSCRIPTSAFEZONE
