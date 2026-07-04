#ifndef _CPLAYERGUN
#define _CPLAYERGUN

#include "MetroidPrime//TGameTypes.hpp"

class CStateManager;
class CPlayer;

enum EChargeState {
  kCS_Normal,
  kCS_Charged,
};

class CPlayerGun {
public:
  enum EChargePhase {
    kCP_NotCharging,
    kCP_Phase_1,
    kCP_Phase_2,
    kCP_Phase_3,
    kCP_AnimAndSfx,
    kCP_Phase_5,
  };

  CPlayerGun(TUniqueId, int);
  ~CPlayerGun();

  CPlayer* GetPlayer(CStateManager& mgr) const;

  void PlayAnim(CStateManager&, int animType, int loop);

  bool IsOutOfAmmoToShoot(CStateManager&) const;
  void ResetCharge(CStateManager&, bool);
  void StopChargeSound(CStateManager&, bool);
  void EnableChargeFx(CStateManager&, bool);


  void Charging(CStateManager&, int);

private:
  char m_pad0[0x3a4];         // 0x000
  EChargeState m_chargeState; // 0x3a4
  char m_pad1[0x2EC];         // 0x3a8
  EChargePhase m_chargePhase; // 0x694
  char m_pad2[0xD8];          // 0x698
  int m_0x770;
  char m_pad3[0x9C];          // 0x774
  uint m_0x810_b0 : 1;
  uint m_0x810_b1 : 1;
  uint m_0x810_b2 : 1;
  uint m_0x810_b3 : 1;
  uint m_0x810_b4 : 1;
  uint m_0x810_b5 : 1;
};
CHECK_SIZEOF(CPlayerGun, 0x814)

#endif // _CPLAYERGUN
