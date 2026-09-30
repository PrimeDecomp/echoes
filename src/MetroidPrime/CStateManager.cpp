#include "MetroidPrime/CStateManager.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CPortalTransition.hpp"
#include "MetroidPrime/CSaveGameScreen.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

#include "Kyoto/Basics/RAssertDolphin.hpp"

#include "rstl/vector.hpp"

const int gkPVSEnabled = 1;

struct queryOutput {
  int* unk0;
  int unk4;
};

void fn_80041518(queryOutput&, MapWorldInfoAreas& allocatedObjectIndices, ushort ourIndex);
void fn_8003C02C(rstl::list< rstl::reserved_vector< CEntity*, 32 > >& v, int);

CStateManager::CStateManager(const rstl::ncrc_ptr< CScriptMailbox >&,
                             const rstl::ncrc_ptr< CMapWorldInfo >&,
                             const rstl::ncrc_ptr< CPlayerState >&,
                             const rstl::ncrc_ptr< CWorldTransManager >&)
: mNextFreeIndex(0)
, mBossId(kInvalidUniqueId)
, mSpecialFunctionId(kInvalidUniqueId)
, mPlanes()
, mPendingDockArea(kInvalidAreaId)
, mPendingDock(0)
, mShowSoftTransition(true) {}

CStateManager::~CStateManager() {}

TUniqueId CStateManager::AllocateUniqueId() {

  const ushort lastIndex = mNextFreeIndex;
  ushort ourIndex;
  queryOutput query;
  do {
    ourIndex = mNextFreeIndex;
    mNextFreeIndex = (ourIndex + 1) % 1024;
    if (mNextFreeIndex == lastIndex) {
      rs_debugger_printf("Object list full!");
    }
    fn_80041518(query, mAllocatedObjectIndices, ourIndex);
  } while ((query.unk4 & *query.unk0) != 0);

  mObjectIndexArray[ourIndex] = (mObjectIndexArray[ourIndex] + 1) & 0x3f;
  if (TUniqueId(mObjectIndexArray[ourIndex], ourIndex) == kInvalidUniqueId) {
    mObjectIndexArray[ourIndex] = 0;
  }

  fn_80041518(query, mAllocatedObjectIndices, ourIndex);
  *query.unk0 = *query.unk0 | query.unk4;

  return TUniqueId(mObjectIndexArray[ourIndex], ourIndex);
}

const CEntity* CStateManager::GetObjectById(TUniqueId uid) const {
  return GetObjectListById(kOL_All).GetObjectById(uid);
}

void CStateManager::SetIsDarkWorld(bool b) {
  mIsDarkWorld = b;
  gpGameState->SetIsDarkWorld(mIsDarkWorld);
}

bool CStateManager::ApplyLocalDamage(const CVector3f& pos, const CVector3f& dir, CActor& damagee,
                                     float damage, const TUniqueId& uid1, const TUniqueId& uid2,
                                     const CDamageInfo& damageInfo, int unkParam) {
  CHealthInfo* healthInfo = damagee.HealthInfo();
  if (!healthInfo || damage < 0.0f) {
    return false;
  }

  float hp = healthInfo->GetHP();
  if (hp <= 0.0f) {
    fn_8003dd88(damagee, uid1, damageInfo, false, unkParam);
    return true;
  }

  CPlayer* player = TCastToPtr< CPlayer >(damagee);

  if (player && player->Get_x12f8() != 0) {
    if (player->Get_x12f8() != 3) {
      return false;
    }
    player->fn_8000d3ac(pos, *this);
    if (!player->fn_8000d40c(dir, *this)) {
      return false;
    }
  }
  TUniqueId playerId = player ? player->GetUniqueId() : kInvalidUniqueId;
  if (player) {
    int playerIndex = MaskUIdNumPlayers(playerId);
    CPlayerState& playerState = *PlayerState(playerIndex);

    if (GetCameraManager(playerIndex)->IsInCinematicCamera()) {
      return false;
    }

    if (gpGameState->GetHardModeEnabled()) {
      switch ((EWeaponType)damageInfo.GetWeaponMode1()) {
      case kWT_Power:
      case kWT_Dark:
      case kWT_Light:
      case kWT_Annihilator:
      case kWT_Bomb:
      case kWT_PowerBomb:
      case kWT_Missile:
      case kWT_BoostBall:
      case kWT_CannonBall:
      case kWT_ScrewAttack:
      case kWT_AI:
      case kWT_PoisonWater1:
      case kWT_PoisonWater2:
      case kWT_Lava:
      case kWT_Heat:
      case kWT_Unused1:
      case kWT_AreaDark:
        damage *= gpGameState->GetHardModeDamageMultiplier();
        break;
      }
    }

    float damageReduction = 0.0f;

    if (playerState.HasPowerUp(CPlayerState::kIT_VariaSuit)) {
      damageReduction = player->GetTweakPlayer()->GetVariaSuitDamageReduction();
    }
    if (playerState.HasPowerUp(CPlayerState::kIT_DarkSuit)) {
      float reduction = player->GetTweakPlayer()->GetDarkSuitDamageReduction();
      if (reduction > damageReduction) {
        damageReduction = reduction;
      }
    }
    if (playerState.HasPowerUp(CPlayerState::kIT_LightSuit)) {
      float reduction = player->GetTweakPlayer()->GetLightSuitDamageReduction();
      if (reduction > damageReduction) {
        damageReduction = reduction;
      }
    }
    if (playerState.GetItemAmount(CPlayerState::kIT_AbsorbAttack, true) != 0) {
      float reduction = 1.5f;
      if (reduction > damageReduction) {
        damageReduction = reduction;
      }
    }
    if (playerState.GetItemAmount(CPlayerState::kIT_LightShield, true) != 0) {
      // TODO: flag
      float reduction = 0.75f;
      if (reduction > damageReduction) {
        damageReduction = reduction;
      }
    }
    if (playerState.GetItemAmount(CPlayerState::kIT_DarkShield, true) != 0) {
      // TODO: flag
      float reduction = 0.75f;
      if (reduction > damageReduction) {
        damageReduction = reduction;
      }
    }
    hp = playerState.CalculateHealth();
    damage = -(damageReduction * damage - damage);
  }
}

void CStateManager::fn_8003BF84(CEntity* ent) {
  // Clear Graveyard?
  if (mGraveyard.empty()) {
    fn_8003C02C(mGraveyard, 0);
  } else if ((--mGraveyard.end())->size() == 32) {
    fn_8003C02C(mGraveyard, 0);
  }
  (--mGraveyard.end())->push_back(ent);
}

void CStateManager::fn_8003BE54() {
  while (!mScriptMsgs.empty()) {
    CScriptMsg msg = mScriptMsgs.fn_8019E6BC();
    CEntity* ent = GetObjectByIdFromListAll(msg.GetId());
    if (ent) {
      bool flag = ent->GetActive();
      ent->AcceptScriptMsg(*this, msg);
      if (flag != ent->GetActive()) {
        if (CActor* actor = TCastToPtr< CActor >(ent)) {
          UpdateActorInSortedLists(actor);
        }
      }
      if (msg.GetMessage() == kSM_XDelete) {
        fn_8003BF84(ent);
        fn_800412EC(ent->GetUniqueId());
      }
    }
  }
}

void CStateManager::DeferStateTransition(EStateManagerTransition t) {
  if (!fn_80036F10()) {
    if (t == kSMT_InGame) {
      if (mDeferredTransition != kSMT_InGame) {
        mWorld->SetLoadPauseState(false);
        mDeferredTransition = kSMT_InGame;
      }
    } else if (mDeferredTransition == kSMT_InGame) {
      mWorld->SetLoadPauseState(true);
      mDeferredTransition = t;
      if (mDeferredTransition == kSMT_Unk) {
        mSaveGameScreen = new CSaveGameScreen(kSC_InGame, gpGameState->GetCardSerial());
      }
    }
  }
}

void CStateManager::ShowPausedHUDMemo(CAssetId strg, float time) {
  mHudMessageTime = time;
  mPauseHudMessage = strg;
  DeferStateTransition(kSMT_MessageScreen);
}

void CStateManager::SendScriptMsg(const CScriptMsg& msg) {
  mScriptMsgs.Append(msg);
  int v = mScriptMsgs.fn_8019E69C();
  if (0x80 < v && !mDispatchingScriptMessages) {
    mDispatchingScriptMessages = true;
    fn_8003BE54();
    mDispatchingScriptMessages = false;
  }
}

bool CStateManager::fn_80036F10() const {
  int v = gpGameState->GetGameMode().GetGameModeType();
  return v != 'SNGL' && v != 'FRND';
}

uint CStateManager::MaskUIdNumPlayers(TUniqueId id) const {
  // TODO
  return id.Value() & mNumPlayers;
}

void CStateManager::MoveActors(float dt) {
  CObjectList* physicsList = mObjectLists[kOL_PhysicsActor].get();
  for (int i = physicsList->GetFirstObjectIndex(); i != -1;
       i = physicsList->GetNextObjectIndex(i)) {
    CPhysicsActor* actor = static_cast< CPhysicsActor* >((*physicsList)[i]);
    if (actor == nullptr || !actor->GetActive() || actor->GetMass() == 0.f ||
        (!actor->GetUpdateDuringCinematicSkip() && gpMain->GetMaxSpeed())) {
      continue;
    }

    if (!actor->GetUpdateWhileOccluded() && actor->GetCurrentAreaId() != kInvalidAreaId) {
      const CGameArea& area = mWorld->GetAreaAlways(actor->GetCurrentAreaId());
      const float occludedTime = area.IsLoaded() ? area.GetPostConstructed()->mOccludedTime : 0.f;
      if (occludedTime > 5.f) {
        continue;
      }
    }

    CPatterned* patterned = TCastToPtr< CPatterned >(actor);
    if (patterned != nullptr && !ShouldUpdatePatterned(*patterned)) {
      SendScriptMsg(patterned->GetUniqueId(), kInvalidUniqueId, kSM_SuspendedMove,
                    kInvalidUniqueId);
      continue;
    }

    if (TCastToPtr< CPlayer >(actor) == nullptr &&
        TCastToPtr< CScriptPlatform >(actor) == nullptr) {
      CGameCollision::Move(*this, *actor, dt, nullptr);
    }
  }
}

void CStateManager::ThinkEntity(float dt, CEntity& entity) { entity.Think(dt, *this); }

bool CStateManager::ShouldUpdatePatterned(const CPatterned& actor) {
  bool update = !mCinematicPause;
  if (update && actor.GetCurrentAreaId() != kInvalidAreaId) {
    const CGameArea& area = mWorld->GetAreaAlways(actor.GetCurrentAreaId());
    const float occludedTime = area.IsLoaded() ? area.GetPostConstructed()->mOccludedTime : 0.f;
    if (occludedTime > 5.f) {
      update = false;
    }
  }
  return update;
}

void CStateManager::Think(float dt) {
  if (!fn_80036F10() && mPlayers[0]->GetDeathTime() > 0.f) {
    mPlayers[0]->DoThink(dt, *this);
    return;
  }

  CObjectList* allList = mObjectLists[kOL_All].get();
  if (mGameState == kGS_SoftPaused) {
    for (int i = allList->GetFirstObjectIndex(); i != -1; i = allList->GetNextObjectIndex(i)) {
      CScriptEffect* effect = TCastToPtr< CScriptEffect >((*allList)[i]);
      if (effect != nullptr) {
        effect->Think(dt, *this);
      }
    }
  } else {
    for (long i = allList->GetFirstObjectIndex(); i != -1; i = allList->GetNextObjectIndex(i)) {
      CEntity* entity = (*allList)[i];
      if (entity == nullptr || (!entity->GetUpdateDuringCinematicSkip() && gpMain->GetMaxSpeed())) {
        continue;
      }

      if (!entity->GetUpdateWhileOccluded() && entity->GetCurrentAreaId() != kInvalidAreaId) {
        const CGameArea& area = mWorld->GetAreaAlways(entity->GetCurrentAreaId());
        const float occludedTime = area.IsLoaded() ? area.GetPostConstructed()->mOccludedTime : 0.f;
        if (occludedTime > 5.f) {
          continue;
        }
      }

      CPatterned* patterned = TCastToPtr< CPatterned >((*allList)[i]);
      if (patterned != nullptr && !ShouldUpdatePatterned(*patterned)) {
        continue;
      }
      if (TCastToPtr< CGameCamera >(entity) == nullptr) {
        ThinkEntity(dt, *entity);
      }
    }
  }
}
