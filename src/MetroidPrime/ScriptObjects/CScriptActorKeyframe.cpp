#include "MetroidPrime/ScriptObjects/CScriptActorKeyframe.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/TCastTo.hpp"

CScriptActorKeyframe::CScriptActorKeyframe(TUniqueId uid, const rstl::string& name,
                                           const CEntityInfo& info, int animationId, bool looping,
                                           float lifetime, bool passive, uint modeFlags,
                                           float playbackRate)
: CEntity(uid, info, name, 0)
, mAnimationId(animationId)
, mInitialLifetime(lifetime)
, mPlaybackRate(playbackRate)
, mLifetime(lifetime)
, mLooping(looping)
, mPassive(passive)
, mFadeOut((modeFlags & 1) != 0)
, mTimedLoop((modeFlags & 2) != 0)
, mPlaying(false)
, mUseOriginator((modeFlags & 4) != 0) {}

void CScriptActorKeyframe::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_AreaLoaded:
    if (mAnimationId == -1) {
      mAnimationId = 0;
    }
    break;
  case kSM_Action:
    if (GetActive()) {
      if (mUseOriginator && msg.GetOriginator() != kInvalidUniqueId) {
        UpdateEntity(msg.GetOriginator(), mgr);
      } else if (!mPassive) {
        const rstl::vector< SConnection >& connections = GetConnectionList();
        for (rstl::vector< SConnection >::const_iterator it = connections.begin();
             it != connections.end(); ++it) {
          if (it->state != kSS_Play || it->msg != kSM_Play) {
            continue;
          }
          CStateManager::TIdListResult ids = mgr.GetIdListForScript(it->objId);
          for (CStateManager::TIdList::const_iterator id = ids.first; id != ids.second; ++id) {
            UpdateEntity(id->second, mgr);
          }
        }
      }
      mPlaying = true;
      mLifetime = mInitialLifetime;
      SendScriptMsgs(kSS_Play, mgr, kInvalidUniqueId, kSM_None);
    }
    break;
  default:
    break;
  }
  CEntity::AcceptScriptMsg(mgr, msg);
}

void CScriptActorKeyframe::UpdateEntity(TUniqueId uid, CStateManager& mgr) {
  CEntity* entity = mgr.ObjectById(uid);
  CActor* actor = TCastToPtr< CScriptActor >(entity);
  if (!actor) {
    actor = TCastToPtr< CScriptPlatform >(entity);
  }

  if (actor) {
    if (!actor->GetActive()) {
      mgr.DeliverScriptMsg(CScriptMsg(GetUniqueId(), kInvalidUniqueId, actor->GetUniqueId(),
                                      kSM_Activate, kSS_InvalidState));
    }
    if (actor->HasAnimation()) {
      CAnimData* animation = actor->AnimationData();
      if (animation->IsAdditiveAnimation(mAnimationId)) {
        animation->AddAdditiveAnimation(mAnimationId, 1.f, mLooping, mFadeOut);
      } else {
        // TODO: Echoes derives noTrans from the transition tree's type and clears
        // an animation-data flag after starting the animation.
        animation->SetAnimation(CAnimPlaybackParms(mAnimationId, -1, 1.f, true), false);
        actor->ModelData()->EnableLooping(mLooping);
        animation->MultiplyPlaybackRate(mPlaybackRate);
      }
    }
  } else if (CPatterned* ai = TCastToPtr< CPatterned >(entity)) {
    CAnimData* animation = ai->AnimationData();
    if (animation->IsAdditiveAnimation(mAnimationId)) {
      animation->AddAdditiveAnimation(mAnimationId, 1.f, mLooping, mFadeOut);
    } else {
      ai->BodyController()->CommandMgr().DeliverCmd(
          CBCScriptedCmd(mAnimationId, mLooping, mTimedLoop, mInitialLifetime));
    }
  }
}

void CScriptActorKeyframe::Think(float dt, CStateManager& mgr) {
  if (!mPassive && mLooping && mTimedLoop && mPlaying && mLifetime > 0.f) {
    mLifetime -= dt;
    if (mLifetime <= 0.f) {
      mPlaying = false;
      const rstl::vector< SConnection >& connections = GetConnectionList();
      for (rstl::vector< SConnection >::const_iterator it = connections.begin();
           it != connections.end(); ++it) {
        if (it->state != kSS_Play || it->msg != kSM_Play) {
          continue;
        }

        CEntity* entity = mgr.ObjectById(mgr.GetIdForScript(it->objId));
        if (CScriptActor* actor = TCastToPtr< CScriptActor >(entity)) {
          if (actor->HasAnimation()) {
            CAnimData* animation = actor->AnimationData();
            if (animation->IsAdditiveAnimation(mAnimationId)) {
              animation->DelAdditiveAnimation(mAnimationId);
            } else if (animation->GetCurrentAnimation() == mAnimationId) {
              actor->ModelData()->EnableLooping(false);
            }
          }
        } else if (CPatterned* ai = TCastToPtr< CPatterned >(entity)) {
          CAnimData* animation = ai->AnimationData();
          if (animation->IsAdditiveAnimation(mAnimationId)) {
            animation->DelAdditiveAnimation(mAnimationId);
          } else if (ai->BodyController()->GetCurrentStateId() == pas::kAS_Scripted &&
                     animation->GetCurrentAnimation() == mAnimationId) {
            ai->BodyController()->CommandMgr().DeliverCmd(kBSC_ExitState);
          }
        }
      }
    }
  }
  CEntity::Think(dt, mgr);
}

CScriptActorKeyframe::~CScriptActorKeyframe() {}
