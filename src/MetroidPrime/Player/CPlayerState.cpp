#include "MetroidPrime/Player/CPlayerState.hpp"

#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Streams/CBitStreamReader.hpp"
#include "Kyoto/Streams/CBitStreamWriter.hpp"

#include <math.h>
#include <float.h>

#include "rstl/math.hpp"
#include "rstl/algorithm.hpp"

struct ScanIdLess {
  bool operator()(const CPlayerState::SPersistentState::SScanState& scan, CAssetId id) const {
    return scan.assetId < id;
  }
  bool operator()(CAssetId id, const CPlayerState::SPersistentState::SScanState& scan) const {
    return id < scan.assetId;
  }
};

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
    5, 5, 5, 5,
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
: mAmount(amount), mCapacity(capacity), mTimeLeft(timeLeft) {}

CPlayerState::SPersistentState::SPersistentState()
: unk1(0), unk2(0), unk3(0), vec(), powerups(CPowerUp(0, 0, 0.0f)) {}

CPlayerState::SPersistentState::SPersistentState(const SPersistentState& other)
: unk1(other.unk1), unk2(other.unk2), unk3(other.unk3), vec(other.vec), powerups(other.powerups) {}

CPlayerState::CPlayerState(int playerIndex, SPersistentState* s)
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
, staticInterference(5)
, unkStruct(s ? *s : SPersistentState()) {
  if (!s) {
    unkStruct.unk1 = this->playerIndex;
  }

  SetPersistentState(unkStruct);
  vectorWord.reserve(32);
}

CPlayerState::CPlayerState(int playerIndex, CBitStreamReader& stream)
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
, powerups()
, scanCompletionRateFirst(0)
, scanCompletionRateSecond(0)
, staticInterference(5) {

  stream.ReadBits(32);
  enabledItems = stream.ReadBits(32);

  const uint hpBits = stream.ReadBits(32);
  healthInfo = CHealthInfo(*reinterpret_cast<const float*>(&hpBits), kDefaultKnockbackResistance);
  currentBeam = EBeamId(stream.ReadBits(GetBitCount(4)));
  currentSuit = EPlayerSuit(stream.ReadBits(GetBitCount(3)));
  unkStruct.unk1 = stream.ReadBits(GetBitCount(4));
  unkStruct.unk2 = stream.ReadBits(GetBitCount(4));
  unkStruct.unk3 = stream.ReadBits(GetBitCount(2));

  stream.ReadBits(32);

  for (int i = 0; i < powerups.capacity(); ++i) {
    int amount = 0;
    int capacity = 0;
    const uint maxValue = kPowerUpMax[i];
    if (kShouldPersist[i]) {
      int bitCount = GetBitCount(maxValue);
      amount = stream.ReadBits(bitCount);
      capacity = stream.ReadBits(bitCount);
    }
    powerups.push_back(CPowerUp(amount, capacity, 0.0f));
  }

  stream.ReadBits(32);

  const rstl::vector< CMemoryCard::ScanState >& scanStates = gpMemoryCard->GetScanStates();
  unkStruct.vec.reserve(scanStates.size() + 4);
  for (int i = 0; i < 4; ++i) {
    stream.ReadBits(1);
    stream.ReadBits(1);
    unkStruct.vec.push_back_unsafe(SPersistentState::SScanState(i));
  }
  for (rstl::vector< CMemoryCard::ScanState >::const_iterator it = scanStates.begin();
       it != scanStates.end(); ++it) {
    bool complete = stream.ReadBits(1) != 0;
    bool flag = stream.ReadBits(1) != 0;
    unkStruct.vec.push_back_unsafe(
        SPersistentState::SScanState(it->first, complete ? 255 : 0, flag));
  }

  scanCompletionRateFirst = int(stream.ReadBits(GetBitCount(0x100u)));
  scanCompletionRateSecond = int(stream.ReadBits(GetBitCount(0x100u)));
  stream.ReadBits(32);
  vectorWord.reserve(32);
}

void CPlayerState::FUN_80085c18(uint v) { unkStruct.unk1 = v; }

void CPlayerState::PutTo(CBitStreamWriter& stream) {
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
      stream.WriteBits(powup[i].mAmount, bitCount);
      stream.WriteBits(powup[i].mCapacity, bitCount);
    }
  }

  stream.WriteBits(0x504f5752, 0x20);

  for (rstl::vector< SPersistentState::SScanState >::iterator it = unkStruct.vec.begin();
       it != unkStruct.vec.end(); ++it) {
    int complete = it->progress == 255 ? 1 : 0;
    uchar flag = it->flag;
    stream.WriteBits(complete, 1);
    stream.WriteBits(flag != 0 ? 1 : 0, 1);
  }

  stream.WriteBits(scanCompletionRateFirst, GetBitCount(0x100));
  stream.WriteBits(scanCompletionRateSecond, GetBitCount(0x100));
  stream.WriteBits(0x5343414e, 0x20);
}

void CPlayerState::ReInitializePowerUp(CPlayerState::EItemType type, int capacity) {
  powerups[type].mCapacity = 0;
  AddPowerUp(type, capacity);
}

void CPlayerState::AddPowerUp(CPlayerState::EItemType type, int delta) {
  if (type < 0 || kIT_Max - 1 < type) {
    return;
  }
  int maxCapacity = kPowerUpMax[type];
  CPowerUp& powerup = powerups[type];
  int newCapacity = delta + powerup.mCapacity;
  if (newCapacity < 0) {
    newCapacity = 0;
  } else if (maxCapacity < newCapacity) {
    newCapacity = maxCapacity;
  }
  powerup.mCapacity = newCapacity;

  int amount = powerup.mAmount;
  int capacity = powerup.mCapacity;
  if (capacity < amount) {
    amount = capacity;
  }
  powerup.mAmount = amount;
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
  return (kEnergyTankCapacity * powerups[kIT_EnergyTanks].mAmount) + kBaseHealthCapacity;
}

void CPlayerState::ResetAndIncrPickUp(CPlayerState::EItemType type, int amount) {
  powerups[int(type)].mAmount = 0;
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
    powerups[type].Add(amount);
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
    powerups[type].mAmount -= amount;
    if (powerups[type].mAmount < 0) {
      powerups[type].mAmount = 0;
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
  case kIT_Missile:
    if (powerups[kIT_MissileWeaponsDisabled].mAmount != 0) {
      return kFQ_Minimum;
    }
    if (powerups[kIT_UnlimitedMissiles].mAmount != 0) {
      return kFQ_Maximum;
    }
    break;
  case kIT_DarkAmmo:
  case kIT_LightAmmo:
    if (powerups[kIT_BeamWeaponsDisabled].mAmount != 0) {
      return kFQ_Minimum;
    }
    if (powerups[kIT_UnlimitedBeamAmmo].mAmount != 0) {
      return kFQ_Maximum;
    }
    break;
  case kIT_MorphBall:
    if (powerups[kIT_DeathBall].mAmount != 0) {
      return kFQ_Minimum;
    }
    if (powerups[kIT_DisableBall].mAmount != 0) {
      return kFQ_Minimum;
    }
    if (powerups[kIT_Unknown_91].mAmount != 0) {
      return kFQ_Minimum;
    }
    break;
  case kIT_BoostBall:
    if (powerups[kIT_DeathBall].mAmount != 0) {
      return kFQ_Minimum;
    }
    break;
  case kIT_SpiderBall:
    if (powerups[kIT_DeathBall].mAmount != 0) {
      return kFQ_Minimum;
    }
    break;
  case kIT_MorphBallBombs:
    if (powerups[kIT_DeathBall].mAmount != 0) {
      return kFQ_Minimum;
    }
    break;
  case kIT_Powerbomb:
    if (powerups[kIT_DeathBall].mAmount != 0) {
      return kFQ_Minimum;
    }
    break;
  case kIT_SpaceJumpBoots:
    if (powerups[kIT_DisableSpaceJump].mAmount != 0) {
      return kFQ_Minimum;
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
  return powerups[type].mAmount;
}

void CPlayerState::SetItemAmount(CPlayerState::EItemType type, int amount) {
  if (type < 0 || kIT_Max - 1 < type) {
    return;
  }
  powerups[type].mAmount = amount;
  CPowerUp& powerup = powerups[type];
  if (powerup.mCapacity < powerup.mAmount) {
    powerup.mCapacity = powerup.mAmount;
  }
}

int CPlayerState::GetItemCapacity(CPlayerState::EItemType type) const {
  if (type < 0 || kIT_Max - 1 < type) {
    return 0;
  }
  return powerups[uint(type)].mCapacity;
}

bool CPlayerState::HasPowerUp(CPlayerState::EItemType type) const {
  if (type < 0 || kIT_Max - 1 < type) {
    return false;
  }
  return powerups[uint(type)].mCapacity > 0;
}

int CPlayerState::GetItemCapacity2(CPlayerState::EItemType type) const {
  if (type < 0 || kIT_Max - 1 < type) {
    return 0;
  }
  return powerups[uint(type)].mCapacity;
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

uchar CPlayerState::UpdateVisorTransition(float dt) {
  bool changed = false;
  if (GetIsVisorTransitioning()) {
    if (currentVisor == transitioningVisor) {
      visorTransitionFactor = rstl::min_val(kMaxVisorTransitionFactor, visorTransitionFactor + dt);
    } else {
      visorTransitionFactor -= dt;
      if (visorTransitionFactor < 0.f) {
        currentVisor = transitioningVisor;
        visorTransitionFactor = fabs(visorTransitionFactor);
        visorTransitionFactor =
            rstl::min_val(visorTransitionFactor, kMaxVisorTransitionFactor - FLT_EPSILON);
        changed = true;
      }
    }
  }
  return changed;
}

float CPlayerState::GetVisorTransitionFactor() const {
  return visorTransitionFactor / kMaxVisorTransitionFactor;
}

bool CPlayerState::GetIsVisorTransitioning() const {
  return currentVisor != transitioningVisor || kMaxVisorTransitionFactor > visorTransitionFactor;
}

float CPlayerState::GetBaseHealthCapacity() { return kBaseHealthCapacity; }

float CPlayerState::GetEnergyTankCapacity() { return kEnergyTankCapacity; }

rstl::vector< CPlayerState::SPersistentState::SScanState >& CPlayerState::ScanStates() {
  return unkStruct.vec;
}

void CPlayerState::InitializeScanTimes() {
  if (unkStruct.vec.size())
    return;

  const rstl::vector< CMemoryCard::ScanState >& scanStates = gpMemoryCard->GetScanStates();
  unkStruct.vec.reserve(scanStates.size() + 4);
  uint i = 0;
  do {
    unkStruct.vec.push_back_unsafe(SPersistentState::SScanState(i));
    ++i;
  } while (i < 4);
  for (rstl::vector< CMemoryCard::ScanState >::const_iterator it = scanStates.begin();
       it != scanStates.end(); ++it) {
    unkStruct.vec.push_back_unsafe(SPersistentState::SScanState(it->first));
  }
}

float CPlayerState::GetScanTime(CAssetId res) {
  rstl::vector< SPersistentState::SScanState >::iterator it =
      rstl::binary_find(unkStruct.vec.begin(), unkStruct.vec.end(), res, ScanIdLess());
  return CCast::ToReal32(it->progress) / 255.f;
}

void CPlayerState::SetScanTime(CAssetId res, float time) {
  rstl::vector< SPersistentState::SScanState >::iterator it =
      rstl::binary_find(unkStruct.vec.begin(), unkStruct.vec.end(), res, ScanIdLess());
  it->progress = CCast::ToUint8(255.f * time);
}

void CPlayerState::SetScanFlag(uint res, bool flag) {
  rstl::vector< SPersistentState::SScanState >::iterator it =
      rstl::binary_find(unkStruct.vec.begin(), unkStruct.vec.end(), res, ScanIdLess());
  it->flag = flag;
}

void CPlayerState::UpdateStaticInterference(const CStateManager& mgr, const float& dt) {
  staticInterference.Update(mgr, dt);
}

CPlayerState::EPlayerVisor CPlayerState::GetActiveVisor(const CStateManager& stateMgr) const {
  const CGameCamera* camera = stateMgr.GetCameraManager(playerIndex)->GetCurrentCamera(stateMgr, 1);
  const CGameCamera* firstCamera = CCameraManager::CastGameCameratoFirstPersonCamera(camera);
  return (firstCamera ? currentVisor : kPV_Combat);
}

uchar CPlayerState::HasVisor(CPlayerState::EPlayerVisor visor) const {
  bool hasVisor = false;
  switch (visor) {
  case kPV_Combat:
    hasVisor = HasPowerUp(kIT_CombatVisor);
    break;
  case kPV_Echo:
    hasVisor = HasPowerUp(kIT_EchoVisor);
    break;
  case kPV_Scan:
    hasVisor = HasPowerUp(kIT_ScanVisor);
    break;
  case kPV_Dark:
    hasVisor = HasPowerUp(kIT_DarkVisor);
    break;
  }
  return hasVisor;
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
    break;
  case kPS_Light:
    if (!mgr.GetIsDarkWorld()) {
      result = 4;
    }
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

CPlayerState::SPersistentState& CPlayerState::GetPersistentState() {
  for (int i = 0; i < 11; ++i) {
    unkStruct.powerups[i] = powerups[kItems_803a74bc[i]];
  }
  return unkStruct;
}

CPlayerState::SPersistentState& CPlayerState::SPersistentState::operator=(
    const SPersistentState& other) {
  unk1 = other.unk1;
  unk2 = other.unk2;
  unk3 = other.unk3;
  vec = other.vec;
  powerups = other.powerups;
  return *this;
}

void CPlayerState::SetPersistentState(const CPlayerState::SPersistentState& s) {
  unkStruct = s;
  for (int i = 0; i < 11; ++i) {
    CPowerUp& otherPowerup = unkStruct.powerups[i];
    CPowerUp& powerup = powerups[kItems_803a74bc[i]];
    powerup.mAmount = otherPowerup.mAmount;
    powerup.mCapacity = otherPowerup.mCapacity;
    powerup.mTimeLeft = otherPowerup.mTimeLeft;
  }
}

void CPlayerState::IncrementChargeBeamFactor(float delta) {
  chargeBeamFactor = rstl::min_val(rstl::max_val(chargeBeamFactor + delta, 0.f), 1.f);
}

void CPlayerState::DecrementAmmoAndDisplayAlertIfOut(const CStateManager& mgr,
                                                     CPlayerState::EItemType type, int quantity) {
  int oldAmount = GetItemAmount(type);
  DecrPickUp(type, quantity);
  if (oldAmount > 0 && GetItemAmount(type) == 0) {
    mgr.DisplayAlertAboutOutOfAmmo(*mgr.GetPlayer(playerIndex), type);
  }
}

const rstl::vector< TUniqueId >& CPlayerState::GetIds() const { return vectorWord; }

bool CPlayerState::HasId(TUniqueId id) const {
  rstl::vector< TUniqueId >::const_iterator it =
      rstl::binary_find(vectorWord.begin(), vectorWord.end(), id);
  return it != vectorWord.end();
}

void CPlayerState::AddId(TUniqueId id) {
  if (vectorWord.size() == vectorWord.capacity()) {
    return;
  }
  rstl::vector< TUniqueId >::iterator it = rstl::lower_bound(vectorWord.begin(), vectorWord.end(), id);
  vectorWord.insert(it, id);
}

void CPlayerState::RemoveId(TUniqueId id) {
  rstl::vector< TUniqueId >::iterator it = rstl::binary_find(vectorWord.begin(), vectorWord.end(), id);
  if (it != vectorWord.end()) {
    vectorWord.erase(it);
  }
}
