#include "MetroidPrime/Player/CPlayerState.hpp"

#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

#include <math.h>

#include "rstl/math.hpp"

extern "C" void fn_8013c9b0(rstl::vector< CPlayerState::UnknownV >& v, float f);

class CFirstPersonCamera;

static const int kPowerUpMax[] = {
    1,          1,          1,          1,          1,          1,          1,          1,
    1,          1,          1,          1,          1,          1,          1,          1,
    1,          1,          1,          1,          1,          1,          1,          1,
    1,          1,          1,          1,          1,          1,          1,          1,
    1,          1,          1,          1,          1,          1,          1,          1,
    1,          0,          14,         10,         255,        250,        250,        255,
    0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 1,          1,          1,          1,
    2,          1,          1,          1,          0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF,
    999,        0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF,
    0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 1,          1,          1,          1,          1,
    9999,       1,          1,          1,          1,          1,          1,          1,
    1,          1,          1,          1,          1,          1,          1,          4,
    1,          1,          1,          1,          1,          1,          1,          1,
    1,          1,          1,          1,          1

};

static const bool kShouldPersist[] = {
    true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,
    true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,
    true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,
    true,  true,  false, true,  true,  true,  true,  true,  true,  false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, true,  true,  true,  true,  true,  true,  true,
    true,  true,  true,  true,  true

};

// static const float kComboAmmoPeriods[] = {
//     0.2f, 0.1f, 0.2f, 0.2f, 1.f,
// };

static const int kMissileCosts[] = {
    5, 10, 10, 10, 1,
};

// static const char* kVisorNames[] = {
//     "CombatVisor",
//     "XRayVisor",
//     "ScanVisor",
//     "ThermalVisor",
// };

static const float kEnergyTankCapacity = 100.f;
static const float kBaseHealthCapacity = 99.f;

static const float kDefaultKnockbackResistance = 50.f;
static const float kMaxVisorTransitionFactor = 0.2f;

static const CPlayerState::EItemType kItems_803a74bc[11] = {
    CPlayerState::kIT_FragCount,          CPlayerState::kIT_DiedCount,
    CPlayerState::kIT_ArchenemyCount,     CPlayerState::kIT_PersistentCounter1,
    CPlayerState::kIT_PersistentCounter2, CPlayerState::kIT_PersistentCounter3,
    CPlayerState::kIT_PersistentCounter4, CPlayerState::kIT_PersistentCounter5,
    CPlayerState::kIT_PersistentCounter6, CPlayerState::kIT_PersistentCounter7,
    CPlayerState::kIT_PersistentCounter8,
};

// static inline void do_nothing() {}

int CPlayerState::GetPowerUpMaxValue(EItemType type) { return kPowerUpMax[type]; }

uint CPlayerState::GetBitCount(uint val) {
  int bits = 0;
  for (; val != 0; val >>= 1) {
    bits += 1;
  }
  return bits;
}

CPlayerState::CPowerUp::CPowerUp(int amount, int capacity, float timeLeft)
: x0_amount(amount), x4_capacity(capacity), x8_timeLeft(timeLeft) {}

CPlayerState::UnknownPlayerStateStruct::UnknownPlayerStateStruct()
: unk1(0), unk2(0), unk3(0), vec(), powerups(CPowerUp(0, 0, 0.0f)) {}

CPlayerState::UnknownPlayerStateStruct::UnknownPlayerStateStruct(const UnknownPlayerStateStruct& other)
: unk1(other.unk1)
, unk2(other.unk2)
, unk3(other.unk3)
, vec(other.vec)
, powerups(other.powerups)
{}

CPlayerState::CPlayerState(int playerIndex, UnknownPlayerStateStruct* s)
: playerIndex(playerIndex)
, alive(true)
, firingComboBeam(false)
, enabledItems(0)
, currentBeam(kBI_Power)
, healthInfo(kBaseHealthCapacity, kDefaultKnockbackResistance)
, currentVisor(kPV_Combat)
, transitioningVisor(currentVisor)
, vectorWord()
, chargeBeamFactor(0.0f)
, chargeAnimStart(0.25f / GetMissileComboChargeFactor())
, visorTransitionFactor(kMaxVisorTransitionFactor)
, currentSuit(kPS_Varia)
, powerups(CPowerUp(0, 0, 0.0f))
, scanCompletionRateFirst(0)
, scanCompletionRateSecond(0)
, unkStruct(s ? *s : UnknownPlayerStateStruct())
, vectorUnk(5) {

  fn_80084928(unkStruct);
  vectorWord.reserve(32);
}

CPlayerState::CPlayerState(int playerIndex, CInputStream& stream)
: playerIndex(playerIndex)
, alive(true)
, firingComboBeam(false)
, enabledItems(0)
, currentBeam(kBI_Power)
, healthInfo(kBaseHealthCapacity, kDefaultKnockbackResistance)
, currentVisor(kPV_Combat)
, transitioningVisor(currentVisor)
// TODO: vectorWord
, chargeBeamFactor(0.0f)
, chargeAnimStart(0.25f / GetMissileComboChargeFactor())
, visorTransitionFactor(kMaxVisorTransitionFactor)
, currentSuit(kPS_Varia) {

  stream.ReadBits(32);
  enabledItems = stream.ReadBits(32);

  healthInfo = CHealthInfo((float)stream.ReadBits(32), kDefaultKnockbackResistance);
  currentBeam = EBeamId(stream.ReadBits(GetBitCount(4)));
  currentSuit = EPlayerSuit(stream.ReadBits(GetBitCount(3)));
  unkStruct.unk1 = stream.ReadBits(GetBitCount(4));
  unkStruct.unk2 = stream.ReadBits(GetBitCount(4));
  unkStruct.unk3 = stream.ReadBits(GetBitCount(2));

  stream.ReadBits(32);

  CPowerUp* powup = powerups.data();
  for (int i = 0; i < powerups.capacity(); ++i) {
    int amount = 0;
    int capacity = 0;
    if (kShouldPersist[i]) {
      int bitCount = GetBitCount(kPowerUpMax[i]);
      amount = stream.ReadBits(bitCount);
      capacity = stream.ReadBits(bitCount);
    }
    powup[i] = CPowerUp(amount, capacity, 0.0f);
  }

  stream.ReadBits(32);

  //   // Scan
  //   const rstl::vector< CMemoryCard::ScanState >& scanStates = gpMemoryCard->GetScanStates();
  //   x170_scanTimes.reserve(scanStates.size());
  //   for (rstl::vector< CMemoryCard::ScanState >::const_iterator it = scanStates.begin();
  //        it != scanStates.end(); ++it) {
  //     float time = stream.ReadPackedBool() ? 1.f : 0.f;
  //     x170_scanTimes.push_back(rstl::pair< CAssetId, float >(it->first, time));
  //   }

  scanCompletionRateFirst = int(stream.ReadBits(GetBitCount(0x100u)));
  scanCompletionRateSecond = int(stream.ReadBits(GetBitCount(0x100u)));
  stream.ReadBits(32);
  vectorWord.reserve(32);
}

void CPlayerState::FUN_80085c18(uint v) { unkStruct.unk1 = v; }

void CPlayerState::PutTo(COutputStream& stream) {
  stream.WriteBits(0x504c5354, 32);
  stream.WriteBits(enabledItems, 32);

  const float realHP = healthInfo.GetHP();
  stream.WriteBits(*(int*)(&realHP), 32);
  stream.WriteBits(currentBeam, GetBitCount(4));
  stream.WriteBits(currentSuit, GetBitCount(3));

  stream.WriteBits(unkStruct.unk1, GetBitCount(4));
  stream.WriteBits(unkStruct.unk2, GetBitCount(4));
  stream.WriteBits(unkStruct.unk3, GetBitCount(2));

  stream.WriteBits(0x50525354, 0x20);
  CPowerUp* powup = powerups.data();
  for (int i = 0; i < powerups.capacity(); ++i) {
    if (kShouldPersist[i]) {
      int bitCount = GetBitCount(kPowerUpMax[i]);
      stream.WriteBits(powup[i].x0_amount, bitCount);
      stream.WriteBits(powup[i].x4_capacity, bitCount);
    }
  }

  stream.WriteBits(0x504f5752, 0x20);

  for (rstl::vector< UnknownV >::iterator it = vectorUnk.begin(); it != vectorUnk.end(); ++it) {
    // TODO
    stream.WriteBits(it->unk2, 1);
    stream.WriteBits(2, 1);
  }

  //   for (rstl::vector< rstl::pair< CAssetId, float > >::iterator it = x170_scanTimes.begin();
  //        it != x170_scanTimes.end(); ++it) {
  //     int flag;
  //     if (it->second >= 1.f) {
  //       flag = 1;
  //     } else {
  //       flag = 0;
  //     }
  //     stream.WriteBits(flag, 1);
  //   }

  stream.WriteBits(scanCompletionRateFirst, GetBitCount(0x100));
  stream.WriteBits(scanCompletionRateSecond, GetBitCount(0x100));
  stream.WriteBits(0x5343414e, 0x20);
}

void CPlayerState::ReInitializePowerUp(CPlayerState::EItemType type, int capacity) {
  powerups[type].x4_capacity = 0;
  AddPowerUp(type, capacity);
}

void CPlayerState::AddPowerUp(CPlayerState::EItemType type, int delta) {
  if (type < 0 || kIT_Max - 1 < type) {
    return;
  }
  int newCapacity = delta + powerups[type].x4_capacity;
  if (newCapacity < 0) {
    newCapacity = 0;
  } else if (kPowerUpMax[type] < newCapacity) {
    newCapacity = kPowerUpMax[type];
  }
  powerups[type].x4_capacity = newCapacity;

  int amount = powerups[type].x0_amount;
  int capacity = powerups[type].x4_capacity;
  if (capacity < amount) {
    amount = capacity;
  }
  powerups[type].x0_amount = amount;
  if (kIT_VariaSuit <= type && type <= kIT_LightSuit) {
    if (HasPowerUp(kIT_LightSuit)) {
      currentSuit = kPS_Light;
    } else if (HasPowerUp(kIT_DarkSuit)) {
      currentSuit = kPS_Dark;
    } else {
      currentSuit = kPS_Varia;
    }
  }
}

float CPlayerState::CalculateHealth() {
  return (kEnergyTankCapacity * powerups[kIT_EnergyTanks].x0_amount) + kBaseHealthCapacity;
}

void CPlayerState::ResetAndIncrPickUp(CPlayerState::EItemType type, int amount) {
  powerups[int(type)].x0_amount = 0;
  IncrPickUp(type, amount);
}

void CPlayerState::IncrementHealth(float delta) {
  float maximum = CalculateHealth();
  float newHealth = healthInfo.GetHP() + delta;
  newHealth = 0.0f > newHealth ? 0.0f : (maximum < newHealth ? maximum : newHealth);
  healthInfo.SetHP(newHealth);
}

void CPlayerState::IncrPickUp(EItemType type, int amount) {
  if (type < 0 || kIT_Max - 1 < type) {
    return;
  }
  if (amount < 0) {
    return;
  }

  if (type == kIT_HealthRefill) {
    IncrementHealth((float)amount);
  } else {
    int capacity = powerups[type].x4_capacity;
    powerups[type].x0_amount += amount;
    if (powerups[type].x0_amount > capacity) {
      powerups[type].x0_amount = capacity;
    }
  }
  if (type == kIT_EnergyTanks) {
    IncrPickUp(kIT_HealthRefill, 9999);
  }
}

void CPlayerState::DecrPickUp(CPlayerState::EItemType type, int amount) {
  if (type < 0 || kIT_Max - 1 < type) {
    return;
  }
  if (GetPowerUpFieldToQuery(type) != kFQ_Maximum) {
    powerups[type].x0_amount -= amount;
    if (powerups[type].x0_amount < 0) {
      powerups[type].x0_amount = 0;
    }
    switch (type) {
    case kIT_EnergyTanks:
      IncrementHealth(0.0f);
      break;
    }
  }
}

CPlayerState::EPowerUpFieldToQuery CPlayerState::GetPowerUpFieldToQuery(EItemType itemType) const {
  switch (itemType) {
  case kIT_MorphBall:
    if (powerups[0x56].x0_amount != 0) {
      return kFQ_Minimum;
    }
    if (powerups[0x5c].x0_amount != 0) {
      return kFQ_Minimum;
    }
    if (powerups[0x5b].x0_amount != 0) {
      return kFQ_Minimum;
    }
    break;
  case kIT_BoostBall:
    if (powerups[0x56].x0_amount != 0) {
      return kFQ_Minimum;
    }
    break;
  case kIT_SpiderBall:
    if (powerups[0x56].x0_amount != 0) {
      return kFQ_Minimum;
    }
    break;
  case kIT_MorphBallBombs:
    if (powerups[0x56].x0_amount != 0) {
      return kFQ_Minimum;
    }
    break;
  case kIT_SpaceJumpBoots:
    if (powerups[0x5d].x0_amount != 0) {
      return kFQ_Minimum;
    }
    break;
  case kIT_Powerbomb:
    if (powerups[0x5d].x0_amount != 0) {
      return kFQ_Minimum;
    }
    break;
  case kIT_Missile:
    if (powerups[0x5a].x0_amount != 0) {
      return kFQ_Minimum;
    }
    if (powerups[0x51].x0_amount != 0) {
      return kFQ_Maximum;
    }
    break;
  case kIT_DarkAmmo:
  case kIT_LightAmmo:
    if (powerups[0x59].x0_amount != 0) {
      return kFQ_Minimum;
    }
    if (powerups[0x52].x0_amount != 0) {
      return kFQ_Maximum;
    }
    break;
  }
  return kFQ_Actual;
}

int CPlayerState::GetItemAmount(CPlayerState::EItemType type, bool respectFieldToQuery) const {
  if (type < 0 || kIT_Max - 1 < type) {
    return 0;
  }

  EPowerUpFieldToQuery field = respectFieldToQuery ? GetPowerUpFieldToQuery(type) : kFQ_Actual;

  if (field == kFQ_Maximum) {
    return kPowerUpMax[type];
  }
  if (field == kFQ_Minimum) {
    return 0;
  }
  return powerups[type].x0_amount;
}

void CPlayerState::SetItemAmount(CPlayerState::EItemType type, int amount) {
  if (type < 0 || kIT_Max - 1 < type) {
    return;
  }
  powerups[type].x0_amount = amount;
  int newAmount = powerups[type].x0_amount;
  if (newAmount <= powerups[type].x4_capacity) {
    return;
  }
  powerups[type].x4_capacity = newAmount;
}

int CPlayerState::GetItemCapacity(CPlayerState::EItemType type) const {
  if (type < 0 || kIT_Max - 1 < type) {
    return 0;
  }
  return powerups[uint(type)].x4_capacity;
}

bool CPlayerState::HasPowerUp(CPlayerState::EItemType type) const {
  if (type < 0 || kIT_Max - 1 < type) {
    return false;
  }
  return powerups[uint(type)].x4_capacity > 0;
}

int CPlayerState::GetItemCapacity2(CPlayerState::EItemType type) const {
  if (type < 0 || kIT_Max - 1 < type) {
    return 0;
  }
  return powerups[uint(type)].x4_capacity;
}

uint CPlayerState::GetPowerUp(CPlayerState::EItemType type) {
  if (type < 0 || kIT_Max - 1 < type) {
    return 0;
  }
  return powerups[uint(type)].x4_capacity;
}

void CPlayerState::EnableItem(CPlayerState::EItemType type) {
  if (HasPowerUp(type))
    enabledItems |= (1 << uint(type));
}

void CPlayerState::DisableItem(CPlayerState::EItemType type) {
  if (HasPowerUp(type))
    enabledItems &= ~(1 << uint(type));
}

bool CPlayerState::ItemEnabled(CPlayerState::EItemType type) const {
  if (HasPowerUp(type))
    return (enabledItems & (1 << uint(type)));
  return false;
}

void CPlayerState::ResetVisor() {
  currentVisor = transitioningVisor = kPV_Combat;
  visorTransitionFactor = 0.0f;
}

void CPlayerState::StartTransitionToVisor(CPlayerState::EPlayerVisor visor) {
  if (visor == transitioningVisor)
    return;

  transitioningVisor = visor;

  if (transitioningVisor == currentVisor)
    return;
}

void CPlayerState::UpdateVisorTransition(float dt) {
  if (!GetIsVisorTransitioning())
    return;

  if (currentVisor == transitioningVisor) {
    float newVal = visorTransitionFactor + dt;
    if (0.2f < newVal) {
      newVal = 0.2f;
    }
    visorTransitionFactor = newVal;
  } else {
    visorTransitionFactor -= dt;
    if (visorTransitionFactor < 0.f) {
      currentVisor = transitioningVisor;
      visorTransitionFactor = fabs(visorTransitionFactor);
      float newVal = visorTransitionFactor;
      if (0.19999f < newVal) {
        newVal = 0.19999f;
      }
      visorTransitionFactor = newVal;
    }
  }
}

float CPlayerState::GetVisorTransitionFactor() const {
  return visorTransitionFactor / kMaxVisorTransitionFactor;
}

bool CPlayerState::GetIsVisorTransitioning() const {
  return currentVisor != transitioningVisor || kMaxVisorTransitionFactor > visorTransitionFactor;
}

float CPlayerState::GetBaseHealthCapacity() { return kBaseHealthCapacity; }

float CPlayerState::GetEnergyTankCapacity() { return kEnergyTankCapacity; }

rstl::vector< CPlayerState::UnknownPlayerStateStruct::Nested >& CPlayerState::fn_800851DC() {
  return unkStruct.vec;
}

void CPlayerState::InitializeScanTimes() {
  //   if (x170_scanTimes.size())
  //     return;

  //   const rstl::vector< CMemoryCard::ScanState >& scanStates = gpMemoryCard->GetScanStates();
  //   x170_scanTimes.reserve(scanStates.size());
  //   for (rstl::vector< CMemoryCard::ScanState >::const_iterator it = scanStates.begin();
  //        it != scanStates.end(); ++it) {
  //     x170_scanTimes.push_back(rstl::pair< CAssetId, float >(it->first, 0.f));
  //   }
}

float CPlayerState::GetScanTime(CAssetId res) const {
  //   rstl::vector< rstl::pair< CAssetId, float > >::const_iterator it =
  //   rstl::find_by_key(x170_scanTimes, res); return it->second;
}

void CPlayerState::SetScanTime(CAssetId res, float time) {
  //   rstl::vector< rstl::pair< CAssetId, float > >::iterator it =
  //   rstl::find_by_key_nc(x170_scanTimes, res); it->second = time;
}

void CPlayerState::fn_80084EAC(uint, bool) {}

void CPlayerState::fn_80084E84(const CStateManager& mgr, float* f) { fn_8013c9b0(vectorUnk, *f); }

// void CPlayerState::UpdateStaticInterference(CStateManager& stateMgr, const float& dt) {
//   x188_staticIntf.Update(stateMgr, dt);
// }

CPlayerState::EPlayerVisor CPlayerState::GetActiveVisor(const CStateManager& stateMgr) const {
  const CGameCamera* camera = stateMgr.GetCameraManager(playerIndex)->GetCurrentCamera(stateMgr, 1);
  const CGameCamera* firstCamera = CCameraManager::CastGameCameratoFirstPersonCamera(camera);
  return (firstCamera ? currentVisor : kPV_Combat);
}

bool CPlayerState::HasVisor(CPlayerState::EPlayerVisor visor) const {
  switch (visor) {
  case kPV_Combat:
    return HasPowerUp(kIT_CombatVisor);
  case kPV_Echo:
    return HasPowerUp(kIT_EchoVisor);
  case kPV_Scan:
    return HasPowerUp(kIT_ScanVisor);
  case kPV_Dark:
    return HasPowerUp(kIT_DarkVisor);
  default:
    return false;
  }
}

bool CPlayerState::CanVisorSeeFog(const CStateManager& stateMgr) const {
  EPlayerVisor visor = GetActiveVisor(stateMgr);
  return visor == kPV_Combat || visor == kPV_Scan;
}

int CPlayerState::ShouldDrawGravityBoost(const CStateManager& mgr) const {
  return GetRenderSuit(mgr, *this, currentSuit);
}

int CPlayerState::GetRenderSuit(const CStateManager& mgr, const CPlayerState& state,
                                CPlayerState::EPlayerSuit suit) {
  int result = (int)suit;
  switch (suit) {
  case kPS_Dark:
    if (state.HasPowerUp(kIT_GravityBoost)) {
      result = 5;
    }

  case kPS_Light:
    // TODO: check x294c in mgr
    result = 4;
    break;
  }
  return result;
}

bool CPlayerState::ShouldDrawGrapple() const {
  return HasPowerUp(kIT_GrappleBeam) && currentSuit < kPS_Light;
}

// CPlayerState::EPlayerSuit CPlayerState::GetCurrentSuit() const {
//   if (GetIsFusionEnabled())
//     return kPS_FusionPower;

//   return x20_currentSuit;
// }

int CPlayerState::GetTotalPickupCount() const {
  return gpTweakGame->GetTotalPercentage();
  // return 100;
}

int CPlayerState::CalculateItemCollectionRate() const {
  return GetItemAmount(kIT_ItemPercentage, true);
}

int CPlayerState::GetItemPercentageRatio() const {
  return (CalculateItemCollectionRate() * 100) / GetTotalPickupCount();
}

int CPlayerState::GetMissileCostForAltAttack() const { return kMissileCosts[int(currentBeam)]; }

// float CPlayerState::GetComboFireAmmoPeriod() const {
//   return kComboAmmoPeriods[size_t(x8_currentBeam)];
// }

float CPlayerState::GetMissileComboChargeFactor() { return 1.8f; }

void CPlayerState::fn_80084B6C() {}

void CPlayerState::UnknownPlayerStateStruct::operator=(
    const CPlayerState::UnknownPlayerStateStruct& o) {

  // probably should just be default

  unk1 = o.unk1;
  unk2 = o.unk2;
  unk3 = o.unk3;
  vec = o.vec;
  powerups = o.powerups;
}

void CPlayerState::fn_80084928(const CPlayerState::UnknownPlayerStateStruct& s) {
  unkStruct = s;
  for (int i = 0; i < 11; ++i) {
    CPowerUp& otherPowerup = unkStruct.powerups[i];
    CPowerUp& powerup = powerups[kItems_803a74bc[i]];
    powerup.x0_amount = otherPowerup.x0_amount;
    powerup.x4_capacity = otherPowerup.x4_capacity;
    powerup.x8_timeLeft = otherPowerup.x8_timeLeft;
  }
}

void CPlayerState::IncrementChargeBeamFactor(float delta) {
  // TODO: clamp to [0, 1]
  chargeBeamFactor = chargeBeamFactor + delta;
}

void CPlayerState::DecrementAmmoAndDisplayAlertIfOut(const CStateManager& mgr,
                                                     CPlayerState::EItemType type, int quantity) {
  int oldAmount = GetItemAmount(type);
  DecrPickUp(type, quantity);
  if (oldAmount > 0 && GetItemAmount(type) == 0) {
    // mgr.DisplayAlertAboutOutOfAmmo(mgr.GetPlayerState(playerIndex), type);
  }
}
