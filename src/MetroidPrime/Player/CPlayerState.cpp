#include "MetroidPrime/Player/CPlayerState.hpp"

#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Streams/CBitStreamReader.hpp"
#include "Kyoto/Streams/CBitStreamWriter.hpp"

#include <float.h>
#include <math.h>

#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

struct ScanIdLess {
  bool operator()(const CPlayerState::SPersistentState::SScanState& scan, CAssetId id) const {
    return scan.mAssetId < id;
  }
  bool operator()(CAssetId id, const CPlayerState::SPersistentState::SScanState& scan) const {
    return id < scan.mAssetId;
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
    5,
    5,
    5,
    5,
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
: mPlayerSelection(0)
, mTeamIndex(0)
, mControlScheme(0)
, mScanStates()
, mPowerups(CPowerUp(0, 0, 0.0f)) {}

CPlayerState::SPersistentState::SPersistentState(const SPersistentState& other)
: mPlayerSelection(other.mPlayerSelection)
, mTeamIndex(other.mTeamIndex)
, mControlScheme(other.mControlScheme)
, mScanStates(other.mScanStates)
, mPowerups(other.mPowerups) {}

CPlayerState::CPlayerState(int playerIndex, SPersistentState* s)
: mPlayerIndex(playerIndex)
, mAlive(true)
, mFiringComboBeam(false)
, mEnabledItems(0)
, mCurrentBeam(kBI_Power)
, mHealth(kBaseHealthCapacity, kDefaultKnockbackResistance)
, mCurrentVisor(kPV_Combat)
, mTransitioningVisor(mCurrentVisor)
, mHudHintIds()
, mChargeBeamFactor(0.0f)
, mChargeAnimStart(0.25f / GetMissileComboChargeFactor())
, mVisorTransitionFactor(kMaxVisorTransitionFactor)
, mCurrentSuit(kPS_Varia)
, mPowerups(CPowerUp(0, 0, 0.0f))
, mScanCompletionRateFirst(0)
, mScanCompletionRateSecond(0)
, mStaticIntf(5)
, mPersistentState(s ? *s : SPersistentState()) {
  if (!s) {
    mPersistentState.mPlayerSelection = this->mPlayerIndex;
  }

  SetPersistentState(mPersistentState);
  mHudHintIds.reserve(32);
}

CPlayerState::CPlayerState(int playerIndex, CBitStreamReader& stream)
: mPlayerIndex(playerIndex)
, mAlive(true)
, mFiringComboBeam(false)
, mEnabledItems(0)
, mCurrentBeam(kBI_Power)
, mHealth(kBaseHealthCapacity, kDefaultKnockbackResistance)
, mCurrentVisor(kPV_Combat)
, mTransitioningVisor(mCurrentVisor)
, mHudHintIds()
, mChargeBeamFactor(0.0f)
, mChargeAnimStart(0.25f / GetMissileComboChargeFactor())
, mVisorTransitionFactor(kMaxVisorTransitionFactor)
, mCurrentSuit(kPS_Varia)
, mPowerups()
, mScanCompletionRateFirst(0)
, mScanCompletionRateSecond(0)
, mStaticIntf(5) {

  stream.ReadBits(32);
  mEnabledItems = stream.ReadBits(32);

  const uint hpBits = stream.ReadBits(32);
  mHealth = CHealthInfo(*reinterpret_cast< const float* >(&hpBits), kDefaultKnockbackResistance);
  mCurrentBeam = EBeamId(stream.ReadBits(GetBitCount(4)));
  mCurrentSuit = EPlayerSuit(stream.ReadBits(GetBitCount(3)));
  mPersistentState.mPlayerSelection = stream.ReadBits(GetBitCount(4));
  mPersistentState.mTeamIndex = stream.ReadBits(GetBitCount(4));
  mPersistentState.mControlScheme = stream.ReadBits(GetBitCount(2));

  stream.ReadBits(32);

  for (int i = 0; i < mPowerups.capacity(); ++i) {
    int amount = 0;
    int capacity = 0;
    const uint maxValue = kPowerUpMax[i];
    if (kShouldPersist[i]) {
      int bitCount = GetBitCount(maxValue);
      amount = stream.ReadBits(bitCount);
      capacity = stream.ReadBits(bitCount);
    }
    mPowerups.push_back(CPowerUp(amount, capacity, 0.0f));
  }

  stream.ReadBits(32);

  const rstl::vector< CMemoryCard::ScanState >& scanStates = gpMemoryCard->GetScanStates();
  mPersistentState.mScanStates.reserve(scanStates.size() + 4);
  for (int i = 0; i < 4; ++i) {
    stream.ReadBits(1);
    stream.ReadBits(1);
    mPersistentState.mScanStates.push_back_unsafe(SPersistentState::SScanState(i));
  }
  for (rstl::vector< CMemoryCard::ScanState >::const_iterator it = scanStates.begin();
       it != scanStates.end(); ++it) {
    bool complete = stream.ReadBits(1) != 0;
    bool flag = stream.ReadBits(1) != 0;
    mPersistentState.mScanStates.push_back_unsafe(
        SPersistentState::SScanState(it->first, complete ? 255 : 0, flag));
  }

  mScanCompletionRateFirst = int(stream.ReadBits(GetBitCount(0x100u)));
  mScanCompletionRateSecond = int(stream.ReadBits(GetBitCount(0x100u)));
  stream.ReadBits(32);
  mHudHintIds.reserve(32);
}

void CPlayerState::FUN_80085c18(uint v) { mPersistentState.mPlayerSelection = v; }

void CPlayerState::PutTo(CBitStreamWriter& stream) {
  stream.WriteBits(0x504c5354, 32);
  stream.WriteBits(mEnabledItems, 32);

  const float realHP = mHealth.GetHP();
  stream.WriteBits(*(int*)(&realHP), 32);
  stream.WriteBits(mCurrentBeam, GetBitCount(4));
  stream.WriteBits(mCurrentSuit, GetBitCount(3));

  stream.WriteBits(mPersistentState.mPlayerSelection, GetBitCount(4));
  stream.WriteBits(mPersistentState.mTeamIndex, GetBitCount(4));
  stream.WriteBits(mPersistentState.mControlScheme, GetBitCount(2));

  stream.WriteBits(0x50525354, 0x20);
  CPowerUp* powup = mPowerups.data();
  for (int i = 0; i < mPowerups.capacity(); ++i) {
    if (kShouldPersist[i]) {
      int bitCount = GetBitCount(kPowerUpMax[i]);
      stream.WriteBits(powup[i].mAmount, bitCount);
      stream.WriteBits(powup[i].mCapacity, bitCount);
    }
  }

  stream.WriteBits(0x504f5752, 0x20);

  for (rstl::vector< SPersistentState::SScanState >::iterator it =
           mPersistentState.mScanStates.begin();
       it != mPersistentState.mScanStates.end(); ++it) {
    int complete = it->mProgress == 255 ? 1 : 0;
    uchar flag = it->mViewedInLogbook;
    stream.WriteBits(complete, 1);
    stream.WriteBits(flag != 0 ? 1 : 0, 1);
  }

  stream.WriteBits(mScanCompletionRateFirst, GetBitCount(0x100));
  stream.WriteBits(mScanCompletionRateSecond, GetBitCount(0x100));
  stream.WriteBits(0x5343414e, 0x20);
}

void CPlayerState::ReInitializePowerUp(CPlayerState::EItemType type, int capacity) {
  mPowerups[type].mCapacity = 0;
  AddPowerUp(type, capacity);
}

void CPlayerState::AddPowerUp(CPlayerState::EItemType type, int delta) {
  if (type < 0 || kIT_Max - 1 < type) {
    return;
  }
  int maxCapacity = kPowerUpMax[type];
  CPowerUp& powerup = mPowerups[type];
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
      mCurrentSuit = kPS_Light;
    } else if (HasPowerUp(kIT_DarkSuit)) {
      mCurrentSuit = kPS_Dark;
    } else {
      mCurrentSuit = kPS_Varia;
    }
  }
}

float CPlayerState::CalculateHealth() {
  return (kEnergyTankCapacity * mPowerups[kIT_EnergyTanks].mAmount) + kBaseHealthCapacity;
}

void CPlayerState::ResetAndIncrPickUp(CPlayerState::EItemType type, int amount) {
  mPowerups[int(type)].mAmount = 0;
  IncrPickUp(type, amount);
}

void CPlayerState::IncrementHealth(float delta) {
  float maximum = CalculateHealth();
  float newHealth = mHealth.GetHP() + delta;
  newHealth = 0.0f > newHealth ? 0.0f : (maximum < newHealth ? maximum : newHealth);
  mHealth.SetHP(newHealth);
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
    mPowerups[type].Add(amount);
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
    mPowerups[type].mAmount -= amount;
    if (mPowerups[type].mAmount < 0) {
      mPowerups[type].mAmount = 0;
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
    if (mPowerups[kIT_MissileWeaponsDisabled].mAmount != 0) {
      return kFQ_Minimum;
    }
    if (mPowerups[kIT_UnlimitedMissiles].mAmount != 0) {
      return kFQ_Maximum;
    }
    break;
  case kIT_DarkAmmo:
  case kIT_LightAmmo:
    if (mPowerups[kIT_BeamWeaponsDisabled].mAmount != 0) {
      return kFQ_Minimum;
    }
    if (mPowerups[kIT_UnlimitedBeamAmmo].mAmount != 0) {
      return kFQ_Maximum;
    }
    break;
  case kIT_MorphBall:
    if (mPowerups[kIT_DeathBall].mAmount != 0) {
      return kFQ_Minimum;
    }
    if (mPowerups[kIT_DisableBall].mAmount != 0) {
      return kFQ_Minimum;
    }
    if (mPowerups[kIT_Unknown_91].mAmount != 0) {
      return kFQ_Minimum;
    }
    break;
  case kIT_BoostBall:
    if (mPowerups[kIT_DeathBall].mAmount != 0) {
      return kFQ_Minimum;
    }
    break;
  case kIT_SpiderBall:
    if (mPowerups[kIT_DeathBall].mAmount != 0) {
      return kFQ_Minimum;
    }
    break;
  case kIT_MorphBallBombs:
    if (mPowerups[kIT_DeathBall].mAmount != 0) {
      return kFQ_Minimum;
    }
    break;
  case kIT_Powerbomb:
    if (mPowerups[kIT_DeathBall].mAmount != 0) {
      return kFQ_Minimum;
    }
    break;
  case kIT_SpaceJumpBoots:
    if (mPowerups[kIT_DisableSpaceJump].mAmount != 0) {
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
  return mPowerups[type].mAmount;
}

void CPlayerState::SetItemAmount(CPlayerState::EItemType type, int amount) {
  if (type < 0 || kIT_Max - 1 < type) {
    return;
  }
  mPowerups[type].mAmount = amount;
  CPowerUp& powerup = mPowerups[type];
  if (powerup.mCapacity < powerup.mAmount) {
    powerup.mCapacity = powerup.mAmount;
  }
}

int CPlayerState::GetItemCapacity(CPlayerState::EItemType type) const {
  if (type < 0 || kIT_Max - 1 < type) {
    return 0;
  }
  return mPowerups[uint(type)].mCapacity;
}

bool CPlayerState::HasPowerUp(CPlayerState::EItemType type) const {
  if (type < 0 || kIT_Max - 1 < type) {
    return false;
  }
  return mPowerups[uint(type)].mCapacity > 0;
}

int CPlayerState::GetItemCapacity2(CPlayerState::EItemType type) const {
  if (type < 0 || kIT_Max - 1 < type) {
    return 0;
  }
  return mPowerups[uint(type)].mCapacity;
}

void CPlayerState::EnableItem(CPlayerState::EItemType type) {
  if (HasPowerUp(type))
    mEnabledItems |= (1 << uint(type));
}

void CPlayerState::DisableItem(CPlayerState::EItemType type) {
  if (HasPowerUp(type))
    mEnabledItems &= ~(1 << uint(type));
}

bool CPlayerState::ItemEnabled(CPlayerState::EItemType type) const {
  if (HasPowerUp(type))
    return (mEnabledItems & (1 << uint(type)));
  return false;
}

void CPlayerState::ResetVisor() {
  mCurrentVisor = mTransitioningVisor = kPV_Combat;
  mVisorTransitionFactor = 0.0f;
}

void CPlayerState::StartTransitionToVisor(CPlayerState::EPlayerVisor visor) {
  if (visor == mTransitioningVisor)
    return;

  mTransitioningVisor = visor;

  if (mTransitioningVisor == mCurrentVisor)
    return;
}

uchar CPlayerState::UpdateVisorTransition(float dt) {
  bool changed = false;
  if (GetIsVisorTransitioning()) {
    if (mCurrentVisor == mTransitioningVisor) {
      mVisorTransitionFactor =
          rstl::min_val(kMaxVisorTransitionFactor, mVisorTransitionFactor + dt);
    } else {
      mVisorTransitionFactor -= dt;
      if (mVisorTransitionFactor < 0.f) {
        mCurrentVisor = mTransitioningVisor;
        mVisorTransitionFactor = fabs(mVisorTransitionFactor);
        mVisorTransitionFactor =
            rstl::min_val(mVisorTransitionFactor, kMaxVisorTransitionFactor - FLT_EPSILON);
        changed = true;
      }
    }
  }
  return changed;
}

float CPlayerState::GetVisorTransitionFactor() const {
  return mVisorTransitionFactor / kMaxVisorTransitionFactor;
}

bool CPlayerState::GetIsVisorTransitioning() const {
  return mCurrentVisor != mTransitioningVisor || kMaxVisorTransitionFactor > mVisorTransitionFactor;
}

float CPlayerState::GetBaseHealthCapacity() { return kBaseHealthCapacity; }

float CPlayerState::GetEnergyTankCapacity() { return kEnergyTankCapacity; }

rstl::vector< CPlayerState::SPersistentState::SScanState >& CPlayerState::ScanStates() {
  return mPersistentState.mScanStates;
}

void CPlayerState::InitializeScanTimes() {
  if (mPersistentState.mScanStates.size())
    return;

  const rstl::vector< CMemoryCard::ScanState >& scanStates = gpMemoryCard->GetScanStates();
  mPersistentState.mScanStates.reserve(scanStates.size() + 4);
  uint i = 0;
  do {
    mPersistentState.mScanStates.push_back_unsafe(SPersistentState::SScanState(i));
    ++i;
  } while (i < 4);
  for (rstl::vector< CMemoryCard::ScanState >::const_iterator it = scanStates.begin();
       it != scanStates.end(); ++it) {
    mPersistentState.mScanStates.push_back_unsafe(SPersistentState::SScanState(it->first));
  }
}

float CPlayerState::GetScanTime(CAssetId res) {
  rstl::vector< SPersistentState::SScanState >::iterator it = rstl::binary_find(
      mPersistentState.mScanStates.begin(), mPersistentState.mScanStates.end(), res, ScanIdLess());
  return CCast::ToReal32(it->mProgress) / 255.f;
}

void CPlayerState::SetScanTime(CAssetId res, float time) {
  rstl::vector< SPersistentState::SScanState >::iterator it = rstl::binary_find(
      mPersistentState.mScanStates.begin(), mPersistentState.mScanStates.end(), res, ScanIdLess());
  it->mProgress = CCast::ToUint8(255.f * time);
}

void CPlayerState::SetScanFlag(uint res, bool flag) {
  rstl::vector< SPersistentState::SScanState >::iterator it = rstl::binary_find(
      mPersistentState.mScanStates.begin(), mPersistentState.mScanStates.end(), res, ScanIdLess());
  it->mViewedInLogbook = flag;
}

void CPlayerState::UpdateStaticInterference(const CStateManager& mgr, const float& dt) {
  mStaticIntf.Update(mgr, dt);
}

CPlayerState::EPlayerVisor CPlayerState::GetActiveVisor(const CStateManager& stateMgr) const {
  const CGameCamera* camera =
      stateMgr.GetCameraManager(mPlayerIndex)->GetCurrentCamera(stateMgr, 1);
  const CGameCamera* firstCamera = CCameraManager::CastGameCameratoFirstPersonCamera(camera);
  return (firstCamera ? mCurrentVisor : kPV_Combat);
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
  return GetRenderSuit(mgr, *this, mCurrentSuit);
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
  return HasPowerUp(kIT_GrappleBeam) && mCurrentSuit < kPS_Light;
}

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

int CPlayerState::GetMissileCostForAltAttack() const { return kMissileCosts[int(mCurrentBeam)]; }

float CPlayerState::GetMissileComboChargeFactor() { return 1.8f; }

CPlayerState::SPersistentState& CPlayerState::GetPersistentState() {
  for (int i = 0; i < 11; ++i) {
    mPersistentState.mPowerups[i] = mPowerups[kItems_803a74bc[i]];
  }
  return mPersistentState;
}

CPlayerState::SPersistentState&
CPlayerState::SPersistentState::operator=(const SPersistentState& other) {
  mPlayerSelection = other.mPlayerSelection;
  mTeamIndex = other.mTeamIndex;
  mControlScheme = other.mControlScheme;
  mScanStates = other.mScanStates;
  mPowerups = other.mPowerups;
  return *this;
}

void CPlayerState::SetPersistentState(const CPlayerState::SPersistentState& s) {
  mPersistentState = s;
  for (int i = 0; i < 11; ++i) {
    CPowerUp& otherPowerup = mPersistentState.mPowerups[i];
    CPowerUp& powerup = mPowerups[kItems_803a74bc[i]];
    powerup.mAmount = otherPowerup.mAmount;
    powerup.mCapacity = otherPowerup.mCapacity;
    powerup.mTimeLeft = otherPowerup.mTimeLeft;
  }
}

void CPlayerState::IncrementChargeBeamFactor(float delta) {
  mChargeBeamFactor = CMath::Clamp(0.f, mChargeBeamFactor + delta, 1.f);
}

void CPlayerState::DecrementAmmoAndDisplayAlertIfOut(CStateManager& mgr,
                                                  CPlayerState::EItemType type, int quantity) {
  int oldAmount = GetItemAmount(type);
  DecrPickUp(type, quantity);
  if (oldAmount > 0 && GetItemAmount(type) == 0) {
    mgr.DisplayAlertAboutOutOfAmmo(*mgr.GetPlayer(mPlayerIndex), type);
  }
}

const rstl::vector< TUniqueId >& CPlayerState::GetIds() const { return mHudHintIds; }

bool CPlayerState::HasId(TUniqueId id) const {
  rstl::vector< TUniqueId >::const_iterator it =
      rstl::binary_find(mHudHintIds.begin(), mHudHintIds.end(), id);
  return it != mHudHintIds.end();
}

void CPlayerState::AddId(TUniqueId id) {
  if (mHudHintIds.size() == mHudHintIds.capacity()) {
    return;
  }
  rstl::vector< TUniqueId >::iterator it =
      rstl::lower_bound(mHudHintIds.begin(), mHudHintIds.end(), id);
  mHudHintIds.insert(it, id);
}

void CPlayerState::RemoveId(TUniqueId id) {
  rstl::vector< TUniqueId >::iterator it =
      rstl::binary_find(mHudHintIds.begin(), mHudHintIds.end(), id);
  if (it != mHudHintIds.end()) {
    mHudHintIds.erase(it);
  }
}
