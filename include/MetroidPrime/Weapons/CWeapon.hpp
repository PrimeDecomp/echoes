#ifndef _CWEAPON
#define _CWEAPON

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"

class CWeapon : public CActor {
public:
  enum EProjectileAttrib {
    kPA_None = 0,
    kPA_PartialCharge = 1 << 0,
    kPA_Charged = 1 << 2,
    kPA_Phazon = 1 << 6,
    kPA_ComboShot = 1 << 7,
    kPA_PowerBombs = 1 << 9, // Guessed name, based on CPowerBomb construction.
    kPA_BigProjectile = 1 << 10,
    kPA_DamageFalloff = 1 << 13,
    kPA_PlayerUnFreeze = 1 << 15,
    kPA_ParticleOPTS = 1 << 16,
    kPA_Dark = 1 << 18,
    kPA_Light = 1 << 19,
    kPA_Annihilator = 1 << 20,
    kPA_Bombs = 1 << 25, // Guessed name, based on CBomb construction.
  };

  CWeapon(TUniqueId uid, TAreaId areaId, bool active, TUniqueId owner, EWeaponType type,
          const rstl::string& name, const CTransform4f& xf, const CMaterialFilter& filter,
          const CMaterialList& materials, const CDamageInfo& damageInfo, int attribs,
          const CModelData& modelData);

  // CEntity
  ~CWeapon() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& position,
                                                         const CVector3f& direction,
                                                         const CWeaponMode& mode,
                                                         int attribs) const override;
  void FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) override;

  void SetDamageFalloffSpeed(float speed);
  int GetAttribField() const { return mProjectileAttribs; }
  bool HasAttrib(EProjectileAttrib attrib) const { return (mProjectileAttribs & attrib) == attrib; }
  TUniqueId GetOwnerId() const { return mOwnerId; }
  EWeaponType GetType() const { return mWeaponType; }
  CMaterialFilter GetFilter() const { return mFilter; }
  const CDamageInfo& GetCurrentDamageInfo() const { return mCurDamageInfo; }

protected:
  int mProjectileAttribs;
  TUniqueId mOwnerId;
  EWeaponType mWeaponType;
  CMaterialFilter mFilter;
  CDamageInfo mOrigDamageInfo;
  CDamageInfo mCurDamageInfo;
  float mCurTime;
  float mDamageFalloffSpeed;
  float mDamageDuration;
  float mInterferenceDuration;
};
CHECK_SIZEOF(CWeapon, 0x1c8)

#endif // _CWEAPON
