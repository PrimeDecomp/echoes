#include "MetroidPrime/ScriptObjects/CScriptActorKeyframe.hpp"

#include "Kyoto/Animation/IMetaTrans.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorKeyframe.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/TCastTo.hpp"

CEntity* LoadAIKeyframe(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  CScriptActorKeyframe* keyframe =
      static_cast< CScriptActorKeyframe* >(LoadActorKeyframe(mgr, input, info));
  if (keyframe != nullptr) {
    keyframe->SetIsPassive(true);
  }
  return keyframe;
}

CEntity* LoadActorKeyframe(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrActorKeyframe sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrActorKeyframe.inc"

  return rs_new CScriptActorKeyframe(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                     LdrToEntityInfo(info, sldrThis.editorProperties),
                                     sldrThis.animation, sldrThis.loop, sldrThis.loopDuration,
                                     false, sldrThis.unknown_0x6d62ef74, sldrThis.playbackRate);
}

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
      SendScriptMsgs(kSS_Play, mgr);
    }
    break;
  case kSM_AreaLoaded:
    if (mAnimationId == -1) {
      mAnimationId = 0;
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
      mgr.DeliverScriptMsg(CScriptMsg(GetUniqueId(), actor->GetUniqueId(), kSM_Activate));
    }
    if (actor->HasAnimation()) {
      if (actor->AnimationData()->IsAdditiveAnimation(mAnimationId)) {
        actor->AnimationData()->AddAdditiveAnimation(mAnimationId, 1.f, mLooping, mFadeOut);
      } else {
        const CAnimPlaybackParms parms(mAnimationId, -1, 1.f, true);
        const rstl::rc_ptr< IMetaTrans > transition(
            actor->AnimationData()->BuildMetaTransition(parms));
        uchar noTrans = false;
        if (transition.GetPtr() && transition->GetType() == kMTT_Snap) {
          noTrans = true;
        }
        actor->AnimationData()->SetAnimation(parms, noTrans);
        actor->ModelData()->EnableLooping(mLooping);
        actor->AnimationData()->MultiplyPlaybackRate(mPlaybackRate);
        actor->AnimationData()->SetPoseBuilt(false);
      }
    }
  } else if (CPatterned* ai = TCastToPtr< CPatterned >(entity)) {
    if (ai->AnimationData()->IsAdditiveAnimation(mAnimationId)) {
      ai->AnimationData()->AddAdditiveAnimation(mAnimationId, 1.f, mLooping, mFadeOut);
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

        const TUniqueId uid = mgr.GetIdForScript(it->objId);
        CEntity* entity = mgr.ObjectById(uid);
        if (CScriptActor* actor = TCastToPtr< CScriptActor >(entity)) {
          if (actor->HasAnimation()) {
            if (actor->AnimationData()->IsAdditiveAnimation(mAnimationId)) {
              actor->AnimationData()->DelAdditiveAnimation(mAnimationId);
            } else if (actor->AnimationData()->GetCurrentAnimation() == mAnimationId) {
              actor->ModelData()->EnableLooping(false);
            }
          }
        } else if (CPatterned* ai = TCastToPtr< CPatterned >(entity)) {
          if (ai->AnimationData()->IsAdditiveAnimation(mAnimationId)) {
            ai->AnimationData()->DelAdditiveAnimation(mAnimationId);
          } else if (ai->BodyController()->GetCurrentStateId() == pas::kAS_Scripted &&
                     ai->AnimationData()->GetCurrentAnimation() == mAnimationId) {
            ai->BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
          }
        }
      }
    }
  }
  CEntity::Think(dt, mgr);
}

CScriptActorKeyframe::~CScriptActorKeyframe() {}
