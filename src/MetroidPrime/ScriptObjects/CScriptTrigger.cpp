#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Enemies/CBouncyGrenade.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTrigger.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "rstl/algorithm.hpp"

CScriptTrigger::CScriptTrigger(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                               const CVector3f& position, const CAABox& bounds,
                               const CDamageInfo& damage, const CVector3f& forceField, uint flags,
                               bool deactivateOnEntered, bool deactivateOnExited)
: CActor(uid, name, info, 0, CTransform4f::Translate(position), CModelData::CModelDataNull(),
         CMaterialList(kMT_Trigger), CActorParameters::None(), kInvalidUniqueId)
, mAttachedTrigger(kInvalidUniqueId)
, mDamageInfo(damage)
, mForceField(forceField)
, mForceMagnitude(forceField.Magnitude())
, mFlags(flags)
, mBounds(bounds)
, mDeactivateOnEntered(deactivateOnEntered)
, mDeactivateOnExited(deactivateOnExited) {
  for (int i = 0; i < 4; ++i) {
    mPlayerInside[i] = false;
    mPlayerEnvironmentDamage[i] = false;
  }
  if (mFlags & kTFL_DetectPlayer) {
    mFlags &= ~kTFL_DetectPlayer;
    mFlags |= kTFL_DetectMorphedPlayer | kTFL_DetectUnmorphedPlayer | kTFL_DetectScrewAttack;
  }
  SetCallTouch(false);
}

CScriptTrigger::~CScriptTrigger() {}

void CScriptTrigger::Touch(CActor& actor, CStateManager& mgr) {
  if (!GetActive() || actor.GetMaterialList().HasMaterial(kMT_Trigger) ||
      HasInhabitant(actor.GetUniqueId())) {
    return;
  }

  uint testFlags = kTFL_None;
  int playerIndex = -1;
  CPlayer* player = TCastToPtr< CPlayer >(actor);
  if (player) {
    playerIndex = mgr.MaskUIdNumPlayers(player->GetUniqueId());
    bool eligible = true;
    if ((mFlags & 0x10007806) != 0 && playerIndex != -1) {
      if (!mgr.GetPlayerState(playerIndex)->IsPlayerAlive()) {
        eligible = false;
      } else if (mgr.IsMultiplayer() &&
                 (mFlags & (0x800 << mgr.GetPlayerState(playerIndex)->GetTeamIndex())) == 0) {
        eligible = false;
      }
    }
    if (eligible) {
      if (mForceMagnitude > 0.f && (mFlags & 0x10000006) != 0 &&
          mgr.GetForceTriggerId(playerIndex) != kInvalidUniqueId) {
        return;
      }
      if ((mFlags & 0x10000006) == 0x10000000) {
        if (player->GetMorphBall()->InScrewAttackMode()) {
          testFlags |= kTFL_DetectScrewAttack;
        }
      } else if ((mFlags & kTFL_DetectMorphedPlayer) == 0 ||
                 (mFlags & kTFL_DetectUnmorphedPlayer) == 0) {
        if (!player->GetMorphBall()->InScrewAttackMode()) {
          if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
            testFlags |= kTFL_DetectMorphedPlayer;
            if (player->GetMorphBall()->GetTimeNotInBoost() < 0.15f) {
              testFlags |= 0x1000000;
            }
            if (player->GetMorphBall()->GetBallState() == CMorphBall::kBS_Spider) {
              testFlags |= 0x8000000;
            }
          } else if (player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
            testFlags |= kTFL_DetectUnmorphedPlayer;
          }
        }
      } else {
        testFlags |= kTFL_DetectMorphedPlayer | kTFL_DetectUnmorphedPlayer;
      }
    }
  }

  if (IsAI(mgr, actor)) {
    testFlags |= kTFL_DetectAI;
  }
  if (TCastToPtr< CBouncyGrenade >(actor)) {
    if ((static_cast< const CBouncyGrenade& >(actor).GetFlags() & 4) != 0) {
      testFlags |= kTFL_DetectAI;
    }
  }
  if (TCastToPtr< CGameProjectile >(actor)) {
    testFlags |= kTFL_DetectProjectiles;
  } else if (TCastToPtr< CWeapon >(actor)) {
    const CWeapon& weapon = static_cast< const CWeapon& >(actor);
    if ((weapon.GetAttribField() & CWeapon::kPA_TriggerBomb) == CWeapon::kPA_TriggerBomb) {
      testFlags |= kTFL_DetectBombs;
    } else if ((weapon.GetAttribField() & CWeapon::kPA_PowerBombs) == CWeapon::kPA_PowerBombs) {
      testFlags |= kTFL_DetectPowerBombs;
    }
  }
  if (TCastToPtr< CGameCamera >(actor)) {
    testFlags = kTFL_DetectCamera;
  }
  if ((mFlags & 0x4000000) != 0) {
    testFlags |= 0x4000000;
  }

  if ((testFlags & mFlags) == 0) {
    InhabitantRejected(actor, mgr);
    return;
  }
  const TUniqueId connectedId = FindConnectedObject(mgr, kSS_Connect, kSM_Attach);
  if (mAttachedTrigger == kInvalidUniqueId) {
    const int trackedPlayer = player && (testFlags & 0x10000006) != 0 ? playerIndex : -1;
    AddInhabitant(mgr, trackedPlayer, actor.GetUniqueId(), GetUniqueId());
  } else if (CScriptTrigger* target = TCastToPtr< CScriptTrigger >(mgr.ObjectById(connectedId))) {
    if (target->GetActive()) {
      const int trackedPlayer = player && (testFlags & 0x10000006) != 0 ? playerIndex : -1;
      target->AddInhabitant(mgr, trackedPlayer, actor.GetUniqueId(), GetUniqueId());
    }
  }
}

CScriptTrigger::CObjectTracker::CObjectTracker(TUniqueId id, TUniqueId triggerId) : mId(id) {
  mTriggers.push_back(triggerId);
}

void CScriptTrigger::AddInhabitant(CStateManager& mgr, int playerIndex, TUniqueId id,
                                   TUniqueId triggerId) {
  CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id));
  if (!actor) {
    return;
  }

  for (rstl::list< CObjectTracker >::iterator it = mInhabitants.begin(); it != mInhabitants.end();
       ++it) {
    if (it->GetObjectId() == id) {
      const rstl::list< TUniqueId >& triggers = it->GetTriggers();
      for (rstl::list< TUniqueId >::const_iterator trigger = triggers.begin();
           trigger != triggers.end(); ++trigger) {
        if (*trigger == triggerId) {
          return;
        }
      }
      it->Triggers().push_back(triggerId);
      return;
    }
  }

  mInhabitants.push_back(CObjectTracker(id, triggerId));
  if (playerIndex != -1) {
    SetPlayerInside(mgr, true, playerIndex);
    if (mForceMagnitude > 0.f) {
      mgr.SetForceTriggerId(playerIndex, GetUniqueId());
    }
  }
  NotifyInhabitantAdded(*actor, mgr);

  if (mDeactivateOnEntered) {
    mgr.DeliverScriptMsg(CScriptMsg(GetUniqueId(), GetUniqueId(), kSM_Deactivate));
    if (actor->HealthInfo() && mDamageInfo.GetDamage() > 0.f) {
      const CMaterialFilter filter =
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59), CMaterialList());
      mgr.ApplyDamage(GetUniqueId(), id, GetUniqueId(), mDamageInfo, filter, CVector3f::Zero());
    }
  }
  if (mFlags & kTFL_KillOnEnter) {
    if (CHealthInfo* health = actor->HealthInfo()) {
      static CWeaponMode killWeaponMode(kWT_Power, false, false, true);
      const CDamageInfo damage(killWeaponMode, 10.f * health->GetHP(), 0.f, 0.f);
      const CMaterialFilter filter =
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59), CMaterialList());
      mgr.ApplyDamage(GetUniqueId(), id, GetUniqueId(), damage, filter, CVector3f::Zero());
    }
  }
  if ((mFlags & 0x4000000) != 0 && !TCastToPtr< CPlayer >(actor) &&
      !TCastToPtr< CGameCamera >(actor)) {
    mgr.DeleteObjectRequest(id);
  }
}

CAABox CScriptTrigger::GetTriggerBoundsWR() const {
  return CAABox(mBounds.GetMinPoint() + GetTranslation(), mBounds.GetMaxPoint() + GetTranslation());
}

rstl::optional_object< CAABox > CScriptTrigger::GetTouchBounds() const {
  if (GetActive()) {
    return GetTriggerBoundsWR();
  }
  return rstl::optional_object_null();
}

void CScriptTrigger::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  if (message == kSM_AreaLoaded) {
    mAttachedTrigger = FindConnectedObject(mgr, kSS_Connect, kSM_Attach);
  }
  if (GetActive() && (message == kSM_Deactivate || message == kSM_Delete)) {
    ClearInhabitants(mgr);
    for (int i = 0; i < 4; ++i) {
      SetPlayerInside(mgr, false, i);
    }
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

void CScriptTrigger::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    UpdateInhabitants(dt, mgr);
  }
}

bool CScriptTrigger::BoundsOverlap(const CAABox& bounds) const {
  const rstl::optional_object< CAABox > touchBounds = GetTouchBounds();
  return touchBounds && touchBounds->DoBoundsOverlap(bounds);
}

void CScriptTrigger::ClearInhabitants(CStateManager& mgr) {
  for (rstl::list< CObjectTracker >::iterator it = mInhabitants.begin(); it != mInhabitants.end();
       ++it) {
    int playerIndex = kInvalidPlayerIndex;
    for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
      if (mgr.GetPlayer(i)->GetUniqueId() == it->GetObjectId()) {
        playerIndex = i;
        break;
      }
    }
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(it->GetObjectId()))) {
      if (playerIndex != kInvalidPlayerIndex) {
        SetPlayerInside(mgr, false, playerIndex);
      }
      NotifyInhabitantExited(*actor, mgr);
    }
  }
  mInhabitants.clear();
}

void CScriptTrigger::NotifyInhabitantExited(CActor& actor, CStateManager& mgr) {
  InhabitantExited(actor, mgr);
  if (ShouldSendScriptMsgs(actor, mgr)) {
    SendScriptMsgs(kSS_Exited, mgr, actor.GetUniqueId(), kSM_None);
  }
}

void CScriptTrigger::NotifyInhabitantAdded(CActor& actor, CStateManager& mgr) {
  InhabitantAdded(actor, mgr);
  if (ShouldSendScriptMsgs(actor, mgr)) {
    SendScriptMsgs(kSS_Entered, mgr, actor.GetUniqueId(), kSM_None);
  }
}

void CScriptTrigger::NotifyInhabitantIdle(CActor& actor, CStateManager& mgr, float dt) {
  InhabitantIdle(actor, mgr, dt);
  if (ShouldSendScriptMsgs(actor, mgr)) {
    SendScriptMsgs(kSS_Inside, mgr, actor.GetUniqueId(), kSM_None);
  }
}

void CScriptTrigger::UpdateCameraInhabitant(const TUniqueId& id, CStateManager& mgr) {
  CGameCamera* camera = TCastToPtr< CGameCamera >(mgr.ObjectById(id));
  if (!camera) {
    return;
  }
  if (!(mFlags & kTFL_DetectCamera)) {
    return;
  }
  const rstl::optional_object< CAABox > triggerBounds = GetTouchBounds();
  const rstl::optional_object< CAABox > cameraBounds = camera->GetTouchBounds();
  if (!triggerBounds) {
    return;
  }
  if (!cameraBounds) {
    return;
  }

  if (HasInhabitant(id)) {
    if (!BoundsOverlap(*cameraBounds)) {
      for (rstl::list< CObjectTracker >::iterator it = mInhabitants.begin();
           it != mInhabitants.end(); ++it) {
        if (it->GetObjectId() == id) {
          mInhabitants.erase(it);
          NotifyInhabitantExited(*camera, mgr);
          break;
        }
      }
    }
  } else if (BoundsOverlap(*cameraBounds)) {
    const TUniqueId connectedId = FindConnectedObject(mgr, kSS_Connect, kSM_Attach);
    if (mAttachedTrigger != kInvalidUniqueId) {
      if (CScriptTrigger* target = TCastToPtr< CScriptTrigger >(mgr.ObjectById(connectedId))) {
        if (target->GetActive()) {
          target->AddInhabitant(mgr, camera->GetControllerNumber(), id, GetUniqueId());
        }
      }
    } else {
      AddInhabitant(mgr, camera->GetControllerNumber(), id, GetUniqueId());
    }
  }
}

void CScriptTrigger::SetPlayerInside(CStateManager& mgr, bool inside, int playerIndex) {
  if (inside == mPlayerInside[playerIndex]) {
    return;
  }

  mPlayerInside[playerIndex] = inside;
  CPlayer* player = mgr.GetPlayer(playerIndex);
  if (mPlayerEnvironmentDamage[playerIndex]) {
    player->PopSustainedDamage();
    mPlayerEnvironmentDamage[playerIndex] = false;
  }

  if (!inside) {
    if (mgr.GetForceTriggerId(playerIndex) == GetUniqueId()) {
      mgr.SetForceTriggerId(playerIndex, kInvalidUniqueId);
    }
  } else {
    if (!(mDamageInfo.GetDamage() <= 0.f) &&
        player->GetDamageVulnerability()->WeaponHits(mDamageInfo.GetWeaponMode(), 0)) {
      player->PushSustainedDamage();
      mPlayerEnvironmentDamage[playerIndex] = true;
    }
  }
}

void CScriptTrigger::UpdateInhabitants(float dt, CStateManager& mgr) {
  rstl::optional_object< CDamageInfo > damage;
  bool exited = false;
  for (rstl::list< CObjectTracker >::iterator it = mInhabitants.begin();
       it != mInhabitants.end();) {
    rstl::list< CObjectTracker >::iterator next = it;
    ++next;
    const TUniqueId id = it->GetObjectId();
    int playerIndex = -1;
    for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
      if (mgr.GetPlayer(i)->GetUniqueId() == id) {
        playerIndex = i;
        break;
      }
    }
    CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id));
    if (!actor) {
      mInhabitants.erase(it);
      if (playerIndex != -1) {
        SetPlayerInside(mgr, false, playerIndex);
      }
      it = next;
      continue;
    }

    rstl::list< TUniqueId >& contributingTriggers = it->Triggers();
    for (rstl::list< TUniqueId >::iterator triggerId = contributingTriggers.begin();
         triggerId != contributingTriggers.end();) {
      rstl::list< TUniqueId >::iterator nextTrigger = triggerId;
      ++nextTrigger;
      CScriptTrigger* trigger = TCastToPtr< CScriptTrigger >(mgr.ObjectById(*triggerId));
      bool valid = trigger != nullptr;
      const uint flags = trigger ? trigger->mFlags : 0;
      if (valid && playerIndex != -1) {
        const CPlayer* player = mgr.GetPlayer(playerIndex);
        if ((flags & (kTFL_DetectMorphedPlayer | kTFL_DetectUnmorphedPlayer)) !=
            (kTFL_DetectMorphedPlayer | kTFL_DetectUnmorphedPlayer)) {
          const bool screwAttack = player->GetMorphBall()->InScrewAttackMode();
          if ((flags & 0x10000006) == kTFL_DetectScrewAttack) {
            valid = screwAttack;
          } else if (screwAttack) {
            valid = false;
          } else if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
            valid = (flags & kTFL_DetectUnmorphedPlayer) == 0;
          } else if (player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
            valid = (flags & kTFL_DetectMorphedPlayer) == 0;
          }
          if (flags & 0x1000000) {
            valid = valid && player->GetMorphballTransitionState() == CPlayer::kMS_Morphed &&
                    player->GetMorphBall()->GetTimeNotInBoost() < 0.15f;
          }
          if (flags & 0x8000000) {
            valid = valid && player->GetMorphballTransitionState() == CPlayer::kMS_Morphed &&
                    player->GetMorphBall()->GetBallState() == CMorphBall::kBS_Spider;
          }
        }
      }

      if (!valid) {
        contributingTriggers.erase(triggerId);
        triggerId = nextTrigger;
        continue;
      }
      const rstl::optional_object< CAABox > triggerBounds = trigger->GetTouchBounds();
      const rstl::optional_object< CAABox > actorBounds = actor->GetTouchBounds();
      if (!actorBounds || !triggerBounds || !trigger->BoundsOverlap(*actorBounds)) {
        contributingTriggers.erase(triggerId);
      } else {
        if (actor->HealthInfo() && trigger->mDamageInfo.GetDamage() > 0.f) {
          if (!damage) {
            damage = CDamageInfo(trigger->mDamageInfo, dt);
          }
          const CMaterialFilter filter =
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59), CMaterialList());
          mgr.ApplyDamage(trigger->GetUniqueId(), id, GetUniqueId(), *damage, filter,
                          CVector3f::Zero());
        }
        if (trigger->mForceMagnitude > 0.f) {
          if (CPhysicsActor* physics = TCastToPtr< CPhysicsActor >(actor)) {
            float forceScale = 1.f;
            if (flags & kTFL_UseBooleanIntersection) {
              forceScale = triggerBounds->GetBooleanIntersection(*actorBounds).GetVolume() /
                           actorBounds->GetVolume();
            }
            const CVector3f force = forceScale * mForceField;
            if (flags & kTFL_UseCollisionImpulses) {
              physics->ApplyImpulseWR((60.f * dt) * force, CAxisAngle::Identity());
              physics->UseCollisionImpulses();
            } else {
              physics->ApplyForceWR(force, CAxisAngle::Identity());
            }
          }
        }
      }
      triggerId = nextTrigger;
    }

    if (contributingTriggers.empty()) {
      mInhabitants.erase(it);
      exited = true;
      if (playerIndex != -1) {
        SetPlayerInside(mgr, false, playerIndex);
      }
      NotifyInhabitantExited(*actor, mgr);
    } else {
      NotifyInhabitantIdle(*actor, mgr, dt);
    }
    it = next;
  }
  if (exited && mDeactivateOnExited) {
    mgr.DeliverScriptMsg(CScriptMsg(GetUniqueId(), GetUniqueId(), kSM_Deactivate));
  }
}

bool CScriptTrigger::HasInhabitant(TUniqueId id) const {
  for (rstl::list< CObjectTracker >::const_iterator it = mInhabitants.begin();
       it != mInhabitants.end(); ++it) {
    if (it->GetObjectId() == id) {
      const rstl::list< TUniqueId >& triggers = it->GetTriggers();
      if (rstl::find(triggers.begin(), triggers.end(), GetUniqueId()) != triggers.end()) {
        return true;
      }
    }
  }
  return false;
}

void CScriptTrigger::InhabitantAdded(CActor&, CStateManager&) {}

void CScriptTrigger::InhabitantIdle(CActor&, CStateManager&, float) {}

void CScriptTrigger::InhabitantExited(CActor&, CStateManager&) {}

void CScriptTrigger::InhabitantRejected(CActor&, CStateManager&) {}

bool CScriptTrigger::ShouldSendScriptMsgs(CActor& actor, CStateManager& mgr) const {
  if (const CGameCamera* camera = TCastToPtr< CGameCamera >(actor)) {
    const TUniqueId cameraId = camera->GetUniqueId();
    if (camera->CameraManager(mgr).GetCurrentCameraId(true) != cameraId) {
      return false;
    }
  }
  return true;
}

bool CScriptTrigger::GetPlayerInside(int playerIndex) const { return mPlayerInside[playerIndex]; }

bool CScriptTrigger::IsAI(CStateManager& mgr, CActor& actor) const {
  if (TCastToPtr< CPatterned >(&actor)) {
    return true;
  }
  if (CCollisionActor* collisionActor = TCastToPtr< CCollisionActor >(&actor)) {
    return TCastToConstPtr< CPatterned >(mgr.GetObjectById(collisionActor->GetOwnerId())) !=
           nullptr;
  }
  return false;
}

const bool CScriptTrigger::ReplaceInhabitant(TUniqueId oldId, TUniqueId newId, CStateManager& mgr) {
  const CActor* oldActor = TCastToConstPtr< CActor >(mgr.GetObjectById(oldId));
  const CActor* newActor = TCastToConstPtr< CActor >(mgr.GetObjectById(newId));
  if (oldActor == nullptr || newActor == nullptr) {
    return false;
  }

  bool replaced = false;
  const bool alreadyInside = HasInhabitant(newId);
  if (!alreadyInside) {
    for (rstl::list< CObjectTracker >::iterator it = mInhabitants.begin(); it != mInhabitants.end();
         ++it) {
      if (it->GetObjectId() == oldId) {
        it->SetObjectId(newId);
        replaced = true;
        break;
      }
    }
  } else {
    for (rstl::list< CObjectTracker >::iterator it = mInhabitants.begin(); it != mInhabitants.end();
         ++it) {
      if (it->GetObjectId() == oldId) {
        mInhabitants.erase(it);
        break;
      }
    }
  }
  return replaced;
}

const bool CScriptTrigger::RemoveInhabitantIfOutside(TUniqueId id, CStateManager& mgr) {
  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(id));
  if (actor == nullptr) {
    return false;
  }
  bool removed = false;
  for (rstl::list< CObjectTracker >::iterator it = mInhabitants.begin(); it != mInhabitants.end();
       ++it) {
    if (it->GetObjectId() == id) {
      const rstl::optional_object< CAABox > bounds = GetTouchBounds();
      const rstl::optional_object< CAABox > actorBounds = actor->GetTouchBounds();
      if (bounds && actorBounds && !BoundsOverlap(*actorBounds)) {
        mInhabitants.erase(it);
        removed = true;
      }
      break;
    }
  }
  return removed;
}

const bool CScriptTrigger::RemoveInhabitant(TUniqueId id, CStateManager& mgr) {
  if (TCastToConstPtr< CActor >(mgr.GetObjectById(id)) == nullptr) {
    return false;
  }
  bool removed = false;
  for (rstl::list< CObjectTracker >::iterator it = mInhabitants.begin(); it != mInhabitants.end();
       ++it) {
    if (it->GetObjectId() == id) {
      mInhabitants.erase(it);
      removed = true;
      break;
    }
  }
  return removed;
}

CEntity* LoadTrigger(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrTrigger sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrTrigger.inc"

  const float hx = 0.5f * sldrThis.editorProperties.transform.scale.GetX();
  const float hy = 0.5f * sldrThis.editorProperties.transform.scale.GetY();
  const float hz = 0.5f * sldrThis.editorProperties.transform.scale.GetZ();
  const CAABox bounds(CVector3f(-hx, -hy, -hz), CVector3f(hx, hy, hz));
  const CVector3f forceField =
      mgr.GetWorld()->GetAreaAlways(info.GetAreaId()).GetTM().Rotate(sldrThis.trigger.forceField);
  return rs_new CScriptTrigger(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                               LdrToEntityInfo(info, sldrThis.editorProperties),
                               sldrThis.editorProperties.transform.position, bounds,
                               LdrToDamageInfo(sldrThis.trigger.damage), forceField,
                               sldrThis.trigger.flagsTrigger, sldrThis.deactivateOnEnter,
                               sldrThis.deactivateOnExit);
}
