#ifndef _CPLAYERSTATE
#define _CPLAYERSTATE

#include "types.h"

#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"


#include "Kyoto/SObjectTag.hpp"

#include "MetroidPrime/CHealthInfo.hpp"

class CStateManager;
class CInputStream;
class COutputStream;

class CPlayerState {
public:
  enum EItemType {
    kIT_Invalid = -1,
    kIT_PowerBeam = 0x0,
    kIT_DarkBeam = 0x1,
    kIT_LightBeam = 0x2,
    kIT_AnnihilatorBeam = 0x3,
    kIT_SuperMissile = 0x4,
    kIT_Darkburst = 0x5,
    kIT_Sunburst = 0x6,
    kIT_SonicBoom = 0x7,
    kIT_CombatVisor = 0x8,
    kIT_ScanVisor = 0x9,
    kIT_DarkVisor = 0xa,
    kIT_EchoVisor = 0xb,
    kIT_VariaSuit = 0xc,
    kIT_DarkSuit = 0xd,
    kIT_LightSuit = 0xe,
    kIT_MorphBall = 0xf,
    kIT_BoostBall = 0x10,
    kIT_SpiderBall = 0x11,
    kIT_MorphBallBombs = 0x12,
    kIT_DarkBomb = 0x13,
    kIT_LightBomb = 0x14,
    kIT_AnnihilatorBomb = 0x15,
    kIT_ChargeBeam = 0x16,
    kIT_GrappleBeam = 0x17,
    kIT_SpaceJumpBoots = 0x18,
    kIT_GravityBoost = 0x19,
    kIT_SeekerLauncher = 0x1a,
    kIT_ScrewAttack = 0x1b,
    kIT_TranslatorUpgrade_TempETM = 0x1c,
    kIT_TempleKey1 = 0x1d,
    kIT_TempleKey2 = 0x1e,
    kIT_TempleKey3 = 0x1f,
    kIT_AgonKey1 = 0x20,
    kIT_AgonKey2 = 0x21,
    kIT_AgonKey3 = 0x22,
    kIT_TorvusKey1 = 0x23,
    kIT_TorvusKey2 = 0x24,
    kIT_TorvusKey3 = 0x25,
    kIT_HiveKey1 = 0x26,
    kIT_HiveKey2 = 0x27,
    kIT_HiveKey3 = 0x28,
    kIT_HealthRefill = 0x29,
    kIT_EnergyTanks = 0x2a,
    kIT_Powerbomb = 0x2b,
    kIT_Missile = 0x2c,
    kIT_DarkAmmo = 0x2d,
    kIT_LightAmmo = 0x2e,
    kIT_ItemPercentage = 0x2f,
    kIT_Multiplayer_NumPlayersJoined = 0x30,
    kIT_Multiplayer_NumPlayersInOptionsMenu = 0x31,
    kIT_MiscCounter3 = 0x32,
    kIT_Multiplayer_Archenemy = 0x33,
    kIT_SwitchWeaponPower = 0x34,
    kIT_SwitchWeaponDark = 0x35,
    kIT_SwitchWeaponLight = 0x36,
    kIT_SwitchWeaponAnnihilator = 0x37,
    kIT_MultiChargeUpgrade = 0x38,
    kIT_Invisibility = 0x39,
    kIT_DoubleDamage = 0x3a,
    kIT_Invincibility = 0x3b,
    kIT_Unknown_60 = 0x3c,
    kIT_Unknown_61 = 0x3d,
    kIT_Unknown_62 = 0x3e,
    kIT_Unknown_63 = 0x3f,
    kIT_FragCount = 0x40,
    kIT_DiedCount = 0x41,
    kIT_ArchenemyCount = 0x42,
    kIT_PersistentCounter1 = 0x43,
    kIT_PersistentCounter2 = 0x44,
    kIT_PersistentCounter3 = 0x45,
    kIT_PersistentCounter4 = 0x46,
    kIT_PersistentCounter5 = 0x47,
    kIT_PersistentCounter6 = 0x48,
    kIT_PersistentCounter7 = 0x49,
    kIT_PersistentCounter8 = 0x4a,
    kIT_SwitchVisorCombat = 0x4b,
    kIT_SwitchVisorScan = 0x4c,
    kIT_SwitchVisorDark = 0x4d,
    kIT_SwitchVisorEcho = 0x4e,
    kIT_CoinAmplifier = 0x4f,
    kIT_CoinCounter = 0x50,
    kIT_UnlimitedMissiles = 0x51,
    kIT_UnlimitedBeamAmmo = 0x52,
    kIT_DarkShield = 0x53,
    kIT_LightShield = 0x54,
    kIT_AbsorbAttack = 0x55,
    kIT_DeathBall = 0x56,
    kIT_ScanVirus = 0x57,
    kIT_VisorStatic = 0x58,
    kIT_BeamWeaponsDisabled = 0x59,
    kIT_MissileWeaponsDisabled = 0x5a,
    kIT_Unknown_91 = 0x5b,
    kIT_DisableBall = 0x5c,
    kIT_DisableSpaceJump = 0x5d,
    kIT_ActivateMorphballBoost = 0x5e,
    kIT_HackedEffect = 0x5f,
    kIT_CannonBall = 0x60,
    kIT_VioletTranslator = 0x61,
    kIT_AmberTranslator = 0x62,
    kIT_EmeraldTranslator = 0x63,
    kIT_CobaltTranslator = 0x64,
    kIT_TempleKey4 = 0x65,
    kIT_TempleKey5 = 0x66,
    kIT_TempleKey6 = 0x67,
    kIT_TempleKey7 = 0x68,
    kIT_TempleKey8 = 0x69,
    kIT_TempleKey9 = 0x6a,
    kIT_EnergyTransferModule = 0x6b,
    kIT_ChargeCombo = 0x6c,

    /* This must remain at the end of the list */
    kIT_Max
  };

  enum EPlayerVisor {
    kPV_Combat,
    kPV_Echo,
    kPV_Scan,
    kPV_Dark,
  };
  enum EPlayerSuit {
    kPS_Invalid = -1,
    kPS_Varia,
    kPS_Dark,
    kPS_Light,
  };
  enum EBeamId {
    kBI_Invalid = -1,
    kBI_Power,
    kBI_Dark,
    kBI_Light,
    kBI_Annihilator,
  };
  enum EChargeStage { kCS_Normal, kCS_Charged };
  
  enum EPowerUpFieldToQuery {
    kFQ_Actual,
    kFQ_Minimum,
    kFQ_Maximum,
  };

  struct CPowerUp {
    int x0_amount;
    int x4_capacity;
    float x8_timeLeft;

    CPowerUp() : x0_amount(0), x4_capacity(0), x8_timeLeft(0.0f) {}
    CPowerUp(int amount, int capacity, float timeLeft);

    void Add(int amount) {
      int capacity = x4_capacity;
      x0_amount += amount;
      if (x0_amount > capacity) {
        x0_amount = capacity;
      }
    }

    void Dec(int amount) {
      x0_amount -= amount;
      if (x0_amount < 0) {
        x0_amount = 0;
      }
    }
  };

  struct UnknownV {
    uint unk1;
    bool unk2;
    bool unk3;
    // short unk1;
    // float unk2;
    // float unk3;
  };

  struct UnknownPlayerStateStruct {
    UnknownPlayerStateStruct();
    UnknownPlayerStateStruct(const UnknownPlayerStateStruct&);
    void operator=(const UnknownPlayerStateStruct&);

    struct Nested {
      uint unk1;
      bool b1;
      bool b2;
    };

    uint unk1;
    uint unk2;
    uint unk3;
    rstl::vector< Nested > vec;
    rstl::reserved_vector< CPowerUp, 11 > powerups;
  };

  CPlayerState(int playerIndex, UnknownPlayerStateStruct*);
  explicit CPlayerState(int playerIndex, CInputStream& stream);

  void FUN_80085c18(uint);
  void PutTo(COutputStream& stream);

  int GetMissileCostForAltAttack() const;
  float GetComboFireAmmoPeriod() const;
  static float GetMissileComboChargeFactor();
  int CalculateItemCollectionRate() const;
  int GetTotalPickupCount() const;
  int GetItemPercentageRatio() const;

  EPlayerSuit GetCurrentSuit() const;
  EPlayerSuit GetCurrentSuitRaw() const { return currentSuit; }
  EBeamId GetCurrentBeam() const { return currentBeam; }
  void SetCurrentBeam(EBeamId beam) { currentBeam = beam; }
  bool CanVisorSeeFog(const CStateManager& stateMgr) const;
  EPlayerVisor GetCurrentVisor() const { return currentVisor; }
  EPlayerVisor GetTransitioningVisor() const { return transitioningVisor; }
  EPlayerVisor GetActiveVisor(const CStateManager& mgr) const;
  bool HasVisor(EPlayerVisor) const;
  int ShouldDrawGravityBoost(const CStateManager& mgr) const;
  static int GetRenderSuit(const CStateManager&, const CPlayerState&, EPlayerSuit suit);
  bool ShouldDrawGrapple() const;

  void IncrementHealth(float);

  // void UpdateStaticInterference(CStateManager& stateMgr, const float& dt);
  void IncreaseScanTime(uint time, float val);
  void SetScanTime(CAssetId res, float time);
  float GetScanTime(CAssetId time) const;
  void fn_80084EAC(uint, bool);
  void fn_80084E84(const CStateManager& mgr, float*);

  bool GetIsVisorTransitioning() const;
  float GetVisorTransitionFactor() const;
  void UpdateVisorTransition(float dt);
  void StartTransitionToVisor(EPlayerVisor visor);
  void ResetVisor();
  bool IsPlayerAlive() const { return alive; }

  bool ItemEnabled(EItemType type) const;
  void DisableItem(EItemType type);
  void EnableItem(EItemType type);
  bool HasPowerUp(EItemType type) const;
  uint GetPowerUp(EItemType type);
  int GetItemCapacity2(EItemType type) const;

  void AddPowerUp(EItemType type, int delta);
  void ReInitializePowerUp(EItemType type, int capacity);

  int GetItemCapacity(EItemType type) const;
  int GetItemAmount(EItemType type, bool respectFieldToQuery = true) const;
  void SetItemAmount(EItemType type, int amount);
  EPowerUpFieldToQuery GetPowerUpFieldToQuery(EItemType itemType) const;

  void DecrPickUp(EItemType type, int amount);
  void IncrPickUp(EItemType type, int amount);
  void ResetAndIncrPickUp(EItemType type, int amount);
  static float GetEnergyTankCapacity();
  static float GetBaseHealthCapacity();
  rstl::vector<UnknownPlayerStateStruct::Nested>& fn_800851DC();

  float CalculateHealth();

  void InitializeScanTimes();

  static uint GetBitCount(uint);
  static int GetPowerUpMaxValue(EItemType);

  // CStaticInterference& StaticInterference() { return x188_staticIntf; }
  // const CStaticInterference& GetStaticInterference() const { return x188_staticIntf; }

  // const rstl::vector< rstl::pair< CAssetId, float > >& GetScanTimes() const {
  //   return x170_scanTimes;
  // }

  CHealthInfo* HealthInfo() { return &healthInfo; }

  const CHealthInfo& GetHealthInfo() const { return healthInfo; }

  void fn_80084B6C();
  void fn_80084928(const UnknownPlayerStateStruct&);
  void IncrementChargeBeamFactor(float);
  void DecrementAmmoAndDisplayAlertIfOut(const CStateManager&, EItemType, int quantity);

private:
  int playerIndex;
  bool alive : 1;
  bool firingComboBeam : 1;
  uint enabledItems;
  EBeamId currentBeam;
  CHealthInfo healthInfo;
  EPlayerVisor currentVisor;
  EPlayerVisor transitioningVisor;
  rstl::vector<TUniqueId> vectorWord;
  float chargeBeamFactor;
  float chargeAnimStart;
  float visorTransitionFactor;
  EPlayerSuit currentSuit;
  rstl::reserved_vector< CPowerUp, 109 > powerups;
  int scanCompletionRateFirst;
  int scanCompletionRateSecond;
  rstl::vector<UnknownV> vectorUnk;
  UnknownPlayerStateStruct unkStruct;
};
CHECK_SIZEOF(CPlayerState, 0x634)

#endif // _CPLAYERSTATE
