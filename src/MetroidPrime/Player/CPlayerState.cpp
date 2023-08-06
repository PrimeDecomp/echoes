#include "MetroidPrime/Player/CPlayerState.hpp"

#include <math.h>

#include "rstl/math.hpp"


extern "C" void fn_8013c9b0(rstl::vector< CPlayerState::UnknownV >& v, float f);

class CFirstPersonCamera;

static const int kPowerUpMax[] = {
    1, 1, 1, 1,  250, 1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 14, 1,   0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static const int kMissileCosts[] = {
    5, 10, 10, 10, 1,
};

static const float kComboAmmoPeriods[] = {
    0.2f, 0.1f, 0.2f, 0.2f, 1.f,
};

static const char* kVisorNames[] = {
    "CombatVisor",
    "XRayVisor",
    "ScanVisor",
    "ThermalVisor",
};

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

uint CPlayerState::GetBitCount(uint val) {
  int bits = 0;
  for (; val != 0; val >>= 1) {
    bits += 1;
  }
  return bits;
}

CPlayerState::CPowerUp::CPowerUp(int amount, int capacity, float timeLeft)
: x0_amount(amount), x4_capacity(capacity), x8_timeLeft(timeLeft) {}

CPlayerState::CPlayerState(int playerIndex, UnknownPlayerStateStruct* s)
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
, currentSuit(kPS_Varia)
// , x24_powerups(CPowerUp(0, 0))
, unkStruct(s ? *s : UnknownPlayerStateStruct()) {

  // fn_80084928(unkStruct);
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
  //   x4_enabledItems = uint(stream.ReadBits(32));

  //   const uint integralHP = uint(stream.ReadBits(32));
  //   xc_health.SetHP(*(float*)(&integralHP));
  //   xc_health.SetKnockbackResistance(kDefaultKnockbackResistance);

  //   x8_currentBeam = EBeamId(stream.ReadBits(GetBitCount(5)));
  //   x20_currentSuit = EPlayerSuit(stream.ReadBits(GetBitCount(4)));

  //   for (int i = 0; i < x24_powerups.capacity(); ++i) {
  //     int amount = 0;
  //     int capacity = 0;

  //     int maxValue = kPowerUpMax[i];
  //     if (maxValue != 0) {
  //       uint bitCount = GetBitCount(maxValue);
  //       amount = stream.ReadBits(bitCount);
  //       capacity = stream.ReadBits(bitCount);
  //     }
  //     CPowerUp pw(amount, capacity);
  //     x24_powerups.push_back(pw);
  //   }

  //   // Scan
  //   const rstl::vector< CMemoryCard::ScanState >& scanStates = gpMemoryCard->GetScanStates();
  //   x170_scanTimes.reserve(scanStates.size());
  //   for (rstl::vector< CMemoryCard::ScanState >::const_iterator it = scanStates.begin();
  //        it != scanStates.end(); ++it) {
  //     float time = stream.ReadPackedBool() ? 1.f : 0.f;
  //     x170_scanTimes.push_back(rstl::pair< CAssetId, float >(it->first, time));
  //   }

  //   x180_scanCompletionRateFirst = uint(stream.ReadBits(GetBitCount(0x100u)));
  //   x184_scanCompletionRateSecond = uint(stream.ReadBits(GetBitCount(0x100u)));
}

void CPlayerState::PutTo(COutputStream& stream) {
  //   stream.WriteBits(x4_enabledItems, 32);

  //   const float realHP = xc_health.GetHP();
  //   stream.WriteBits(*(int*)(&realHP), 32);
  //   stream.WriteBits(x8_currentBeam, GetBitCount(5));
  //   stream.WriteBits(x20_currentSuit, GetBitCount(4));

  //   CPowerUp* powup = x24_powerups.data();
  //   for (int i = 0; i < x24_powerups.capacity(); ++i) {
  //     if (0 < kPowerUpMax[i]) {
  //       int bitCount = GetBitCount(kPowerUpMax[i]);
  //       stream.WriteBits(powup[i].x0_amount, bitCount);
  //       stream.WriteBits(powup[i].x4_capacity, bitCount);
  //     }
  //   }

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

  //   stream.WriteBits(x180_scanCompletionRateFirst, GetBitCount(0x100));
  //   stream.WriteBits(x184_scanCompletionRateSecond, GetBitCount(0x100));
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
  float newHealth = healthInfo.GetHP() + delta; // TODO: should be healthB
  // TODO: clamp newHealth with maximum and 0
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
  //   if (type < 0 || kIT_Max - 1 < type) {
  //     return;
  //   }

  //   switch (type) {
  //   case kIT_Missiles:
  //   case kIT_PowerBombs:
  //   case kIT_Flamethrower:
  //     x24_powerups[type].Dec(amount);
  //   default:
  //     return;
  //   }
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

  // if (currentVisor == transitioningVisor) {
  //   x1c_visorTransitionFactor = rstl::min_val(0.2f, x1c_visorTransitionFactor + dt);
  // } else {
  //   x1c_visorTransitionFactor -= dt;
  //   if (x1c_visorTransitionFactor < 0.f) {
  //     currentVisor = transitioningVisor;
  //     x1c_visorTransitionFactor = fabs(x1c_visorTransitionFactor);
  //     x1c_visorTransitionFactor = rstl::min_val(x1c_visorTransitionFactor, 0.19999f);
  //   }
  // }
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
  //   const CFirstPersonCamera* cam = TCastToConstPtr< CFirstPersonCamera >(
  //       stateMgr.GetCameraManager()->GetCurrentCamera(stateMgr));
  //   return (cam ? currentVisor : kPV_Combat);
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
  // return gpTweakGame->GetTotalPercentage();
  return 100;
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

void CPlayerState::DecrementAmmoAndDisplayAlertIfOut(const CStateManager& mgr, CPlayerState::EItemType type, int quantity) {
  int oldAmount = GetItemAmount(type);
  DecrPickUp(type, quantity);
  if (oldAmount > 0 && GetItemAmount(type) == 0) {
    // mgr.DisplayAlertAboutOutOfAmmo(mgr.GetPlayerState(playerIndex), type);
  }
}
