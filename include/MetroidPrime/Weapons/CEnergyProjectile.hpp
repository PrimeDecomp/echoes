#ifndef _CENERGYPROJECTILE
#define _CENERGYPROJECTILE

#include "MetroidPrime/Cameras/CCameraShakerData.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "rstl/list.hpp"
#include "rstl/pair.hpp"

class CEnergyProjectile : public CGameProjectile {
public:
  CEnergyProjectile(bool active, const TToken< CWeaponDescription >& description, EWeaponType type,
                    const CTransform4f& xf, EMaterialTypes excludeMaterial,
                    const CDamageInfo& damage, TUniqueId uid, TAreaId areaId, TUniqueId owner,
                    TUniqueId homingTarget, uint attribs, bool underwater, const CVector3f& scale,
                    const CImpactVisorEffect& visorEffect, bool unused, bool playImpactSound,
                    bool orientImpactToOwner, float chargeFactor, float closeImpactDistance,
                    float impactScaleDistance);

  // CEntity
  ~CEnergyProjectile() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  CAABox GetSortingBounds(const CStateManager& mgr) const override;

  // CGameProjectile
  void StopProjectile(CStateManager& mgr) override;
  void ResolveCollisionWithActor(const CRayCastResult& result, CActor& actor,
                                 CStateManager& mgr) override;

  // CEnergyProjectile
  virtual bool Explode(const CVector3f& position, const CVector3f& normal,
                       EWeaponCollisionResponseTypes type, CStateManager& mgr,
                       const CDamageVulnerability& vulnerability, TUniqueId hitActor);
  virtual void ResolveCollisionWithWorld(const CRayCastResult& result, CStateManager& mgr);
  // Guessed names: optional particle override and the forced-explosion normal.
  virtual rstl::optional_object< TLockedToken< CGenDescription > >
  GetImpactParticle(CStateManager& mgr) {
    return rstl::optional_object_null();
  }

  virtual CVector3f GetExplosionNormal() const { return CVector3f::Up(); }

  void SetEchoVisorMaxVolume(uchar volume);
  void SetCombatVisorMaxVolume(uchar volume);
  void SetCameraShakerData(const CCameraShakerData& data);
  void PlayImpactSound(const CVector3f& position, EWeaponCollisionResponseTypes type); // Guessed name
  void InitializeMuzzleOffset(float duration, CStateManager& mgr); // Guessed name
  void SetExplodePending(bool pending) { mExplodePending = pending; }
  bool HasExploded() const { return mHasExploded; }

private:
  // Guessed name; the original owns a sorted list of IDs and expiry times.
  class CCollisionCooldowns {
  public:
    explicit CCollisionCooldowns(float duration);
    bool Contains(TUniqueId id) const;
    void Add(TUniqueId id);
    void Add(TUniqueId id, float duration);
    void Update(float dt);

  private:
    rstl::list< rstl::pair< TUniqueId, float > > mEntries;
    float mDefaultDuration;
  };

  static const CMaterialList kCheckMaterial;

  CSfxHandle mSfx;
  CVector3f mInitialDirection;
  float mInitialDirectionMagnitude;
  float mLifetime;
  float mChargeFactor; // Guessed name: scales the charged Dark impact.
  CCameraShakerData mCameraShaker;
  CCollisionCooldowns mCollisionCooldowns;
  CVector3f mMuzzleOffset; // Guessed names: remote-player visual correction.
  float mMuzzleOffsetDuration;
  float mMuzzleOffsetTime;
  float mCloseImpactDistance; // Guessed names: camera distance thresholds for impact effects.
  float mImpactScaleDistance;
  uint mCombatVisorMaxVolume : 8;
  uint mEchoVisorMaxVolume : 8;
  // Guessed names for flags absent from Prime; masks verified against Echoes.
  uint mUseCombatVisorVolume : 1;
  uint mDead : 1;
  uint mHasExploded : 1;
  uint mExplodePending : 1;
  uint mCameraShakerDirty : 1;
  uint mSuppressDecal : 1;
  uint mPlayImpactSound : 1;
  uint mHasMuzzleOffset : 1;
  uint mMuzzleOffsetApplied : 1;
  uint mOrientImpactToOwner : 1;
  int mLastVisibleFrame; // Guessed name: used to age out off-screen impact effects.
};
CHECK_SIZEOF(CEnergyProjectile, 0x568)

#endif // _CENERGYPROJECTILE
