#ifndef _CPLAYERGUN
#define _CPLAYERGUN

#include "MetroidPrime/CEntity.hpp"

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"

#include "MetroidPrime/CActorLights.hpp"

class CWorldTransManager;
class CStateManager;
class CPlayer;
class CWorldShadow;

enum EChargeState {
  kCS_Normal,
  kCS_Charged,
};

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

  bool IsOutOfAmmoToShoot(CStateManager&) const;
  void ResetCharge(CStateManager&, bool);
  void StopChargeSound(CStateManager&, bool);
  void EnableChargeFx(CStateManager&, bool);

  void UpdateChargeState(float dt, CStateManager& mgr);

  void Charging(CStateManager&, int);

private:
  CTransform4f m_0x24;
  CTransform4f m_0x54;
  float m_0x84;
  float m_0x88;
  float m_0x8c;
  CWorldTransManager *m_worldTransManager;
  CActorLights m_lights;
  TUniqueId m_playerUniqueId;
  TUniqueId m_lightId;
  CWorldShadow* m_worldShadow;
  float m_cooldown;
  float m_0x384;
  float m_gunHolsterRemTime;
  char m_pad0[24];
  EChargeState m_chargeState; // 0x3a4
  int m_gunHolsterState; // 0x3a8 EGunHolsterState
  short m_0x3ac;
  bool m_0x3ae_b0 : 1;
  bool m_0x3ae_b1 : 1;
  bool m_0x3ae_b2 : 1;
  bool m_0x3ae_b3 : 1;
  bool m_0x3ae_b4 : 1;
  bool m_0x3ae_b5 : 1;
  bool m_0x3ae_b6 : 1;
  bool m_0x3ae_b7 : 1;
  CTransform4f m_gunWorldXf;  // 0x3b0
  char m_pad1[0x2B4];         // 0x3e0
  EChargePhase m_chargePhase; // 0x694
  ESeekerChargeState m_seekerChargeState; // 0x698
  float m_timerRelatedToSeekers; // 0x69c
  char m_pad2[0xD0];          // 0x6A0
  int m_0x770;                // 0x770
  char m_pad4[0x2b];          // 0x774
  unsigned char m_0x70f : 1;  // 0x70f
  CSfxHandle m_chargeSfx;     // 0x7a0
  CSfxHandle m_sfxForShoot;   // 0x7a4
  short m_chargeRumbleHandle; // 0x7a8
  float m_maybeChargeAnim;    // 0x7ac
  char m_pad3[0x60];          // 0x7B0
  uint m_0x810_b0 : 1;
  uint m_0x810_b1 : 1;
  uint m_0x810_b2 : 1;
  uint m_0x810_b3 : 1;
  uint m_0x810_b4 : 1;
  uint m_0x810_b5 : 1;
};
CHECK_SIZEOF(CPlayerGun, 0x814)

#endif // _CPLAYERGUN
