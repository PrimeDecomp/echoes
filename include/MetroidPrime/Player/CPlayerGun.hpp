#ifndef _CPLAYERGUN
#define _CPLAYERGUN

#include "MetroidPrime/CEntity.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

class CWorldTransManager;
class CStateManager;
class CPlayer;
class CWorldShadow;
class CGunWeapon;

class CPlayerGun : public CEntity {
public:
  enum EChargePhase {
    kCP_NotCharging,
    kCP_Phase_1,
    kCP_Phase_2,
    kCP_Phase_3,
    kCP_AnimAndSfx,
    kCP_Phase_5,
  };
  enum ESeekerChargeState {
    kSCS_NotCharging,
    kSCS_State_1,
    kSCS_State_2,
    kSCS_State_3,
    kSCS_FullyCharged,
    // Ghidra has State 9 too
  };

  CPlayerGun(TUniqueId, int);
  ~CPlayerGun();

  TUniqueId GetPlayerUniqueId() const { return m_playerUniqueId; }
  CPlayer* GetPlayer(CStateManager& mgr) const;
  CPlayer* GetPlayerFromAll(CStateManager& mgr) const;

  void PlayAnim(CStateManager&, int animType, int loop);
  void fn_801cdca0(CStateManager&, CPlayer*, bool);
  TUniqueId GetTargetId(CStateManager&);

  bool IsOutOfAmmoToShoot(CStateManager&) const;
  bool GetBeamAmmoTypeAndCosts(bool chargeCombo, CStateManager&,
                               CPlayerState::EItemType& beamAmmoTypeA,
                               CPlayerState::EItemType& beamAmmoTypeB, int& outBeamAmmoCost) const;

  void ResetCharge(CStateManager&, bool);
  void StopChargeSound(CStateManager&, bool);
  void EnableChargeFx(CStateManager&, bool);

  void UpdateNormalShotCycle(float dt, CStateManager& mgr);
  void UpdateChargeState(float dt, CStateManager& mgr);

  void Charging(CStateManager&, int);

private:
  CTransform4f m_0x24;
  CTransform4f m_0x54;
  float m_0x84;
  float m_0x88;
  float m_0x8c;
  CWorldTransManager* m_worldTransManager;
  CActorLights m_lights;
  TUniqueId m_playerUniqueId;
  TUniqueId m_lightId;
  CWorldShadow* m_worldShadow;
  float m_cooldown;
  float m_0x384;
  float m_gunHolsterRemTime;
  char m_pad0[24];
  CPlayerState::EChargeStage m_chargeState; // 0x3a4
  int m_gunHolsterState;      // 0x3a8 EGunHolsterState
  short m_0x3ac;
  bool m_isUnderwater : 1;
  bool m_0x3ae_b1 : 1;
  bool m_0x3ae_b2 : 1;
  bool m_0x3ae_b3 : 1;
  bool m_0x3ae_b4 : 1;
  bool m_0x3ae_b5 : 1;
  bool m_0x3ae_b6 : 1;
  bool m_0x3ae_b7 : 1;
  CTransform4f m_gunWorldXf;              // 0x3b0
  char m_pad1[0x264];                     // 0x3e0
  CGunWeapon* m_currentBeam;              // 0x644
  CGunWeapon* m_outgoingBeam;             // 0x648
  CGunWeapon* m_loadingBeam;              // 0x64c
  CGunWeapon* m_nextBeam;                 // 0x650
  char m_pad1b[12];                       // 0x654
  float m_0x660[13];                      // 0x660
  EChargePhase m_chargePhase;             // 0x694
  ESeekerChargeState m_seekerChargeState; // 0x698
  float m_timerRelatedToSeekers;          // 0x69c
  char m_pad2[0xD0];                      // 0x6A0
  int m_0x770;                            // 0x770
  CPlayerState::EBeamId m_currentBeamId;  // 0x774
  CPlayerState::EBeamId m_nextBeamId;     // 0x778
  int m_0x77c;                            // 0x77c
  char m_pad3[0x20];                      // 0x780
  CSfxHandle m_chargeSfx;                 // 0x7a0
  CSfxHandle m_sfxForShoot;               // 0x7a4
  short m_chargeRumbleHandle;             // 0x7a8
  float m_maybeChargeAnim;                // 0x7ac
  char m_pad4[0x4C];                      // 0x7B0
  int m_absorbedPhazonShots;              // 0x7fc
  char m_pad5[0x10];                      // 0x800
  uint m_0x810_b0 : 1;
  uint m_0x810_b1 : 1;
  uint m_0x810_b2 : 1;
  uint m_0x810_b3 : 1;
  uint m_0x810_b4 : 1;
  uint m_0x810_b5 : 1;
};
CHECK_SIZEOF(CPlayerGun, 0x814)

#endif // _CPLAYERGUN
