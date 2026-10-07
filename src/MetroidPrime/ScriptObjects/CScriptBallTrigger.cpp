#include "MetroidPrime/ScriptObjects/CScriptBallTrigger.hpp"

#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CAxisAngle.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrBallTrigger.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "rstl/math.hpp"

static CVector3f calculate_ball_extents() {
  const float extent = 0.33f * gpTweakPlayerA->GetBallRadius();
  return CVector3f(extent, extent, extent);
}

CScriptBallTrigger::CScriptBallTrigger(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, const CTransform4f& xf,
                                       const CVector3f& scale, const CDamageInfo& damage,
                                       const CVector3f& forceField, uint flags,
                                       float attractionForce, float attractionAngle,
                                       float attractionDistance,
                                       const CVector3f& attractionDirection, bool noBallMovement)
: CScriptTriggerOrientated(uid, name, info, calculate_ball_extents(), xf, damage, forceField, flags,
                           false, false)
, mAttractionForce(attractionForce)
, mAttractionAngle(attractionAngle)
, mAttractionDistance(attractionDistance)
, mAttractionDirection(CVector3f::Zero())
, mCapturedPlayerIndex(kInvalidPlayerIndex)
, mNoBallMovement(noBallMovement) {
  if (attractionDirection.CanBeNormalized()) {
    mAttractionDirection = attractionDirection.AsNormalized();
  }
}

CScriptBallTrigger::~CScriptBallTrigger() {}

void CScriptBallTrigger::InhabitantAdded(CActor& actor, CStateManager& mgr) {
  if (CPlayer* player = TCastToPtr< CPlayer >(actor)) {
    if (mCapturedPlayerIndex == kInvalidPlayerIndex ||
        mCapturedPlayerIndex == player->GetPlayerIndex()) {
      mCapturedPlayerIndex = player->GetPlayerIndex();
      player->GetMorphBall()->SetBallBoostState(CMorphBall::kBBS_BoostDisabled);
      const CVector3f position =
          GetTranslation() - CVector3f(0.f, 0.f, player->GetMorphBall()->GetBallRadius());
      const CTransform4f xf(player->GetTransform().BuildMatrix3f(), position);
      player->Teleport(xf, mgr, false);
    }
  }
}

void CScriptBallTrigger::InhabitantExited(CActor& actor, CStateManager&) {
  if (CPlayer* player = TCastToPtr< CPlayer >(actor)) {
    if (mCapturedPlayerIndex == player->GetPlayerIndex()) {
      mCapturedPlayerIndex = kInvalidPlayerIndex;
    }
    player->GetMorphBall()->SetBallBoostState(CMorphBall::kBBS_BoostAvailable);
  }
}

void CScriptBallTrigger::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CScriptTriggerOrientated::Think(dt, mgr);

  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    CPlayer& player = *mgr.GetPlayer(i);
    if (mCapturedPlayerIndex != kInvalidPlayerIndex && mCapturedPlayerIndex != i) {
      continue;
    }

    const float ballRadius = player.GetMorphBall()->GetBallRadius();
    const CVector3f position = player.GetTranslation() + CVector3f(0.f, 0.f, ballRadius);
    const CPlayer::EPlayerMorphBallState morphState =
        player.GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
            ? player.GetMorphballTransitionState()
            : CPlayer::kMS_Unmorphed;
    if (morphState != CPlayer::kMS_Morphed) {
      continue;
    }

    const CVector3f difference = GetTranslation() - position;
    const float distance = difference.Magnitude();
    if (!HasInhabitant(player.GetUniqueId())) {
      const CVector3f direction = difference.AsNormalized();
      const float angleCos = cosine(CRelAngle::FromDegrees(mAttractionAngle));
      if (angleCos < CVector3f::Dot(-direction, mAttractionDirection) &&
          distance < mAttractionDistance) {
        const float attraction = mAttractionForce * (mAttractionDistance / (distance * distance));
        const float force = rstl::min_val(attraction, 1.f / dt * distance);
        player.ApplyForceWR(force * (player.GetMass() * direction), CAxisAngle::Identity());
      }
    }

    if (HasInhabitant(player.GetUniqueId())) {
      const CVector3f target = GetTranslation() - CVector3f(0.f, 0.f, ballRadius);
      if (mNoBallMovement) {
        player.Stop();
      }
      player.MoveToWR(target, dt);
    }
  }
}

void CScriptBallTrigger::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Deactivate && GetActive()) {
    for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
      CPlayer& player = *mgr.GetPlayer(i);
      if (HasInhabitant(player.GetUniqueId())) {
        player.GetMorphBall()->SetBallBoostState(CMorphBall::kBBS_BoostAvailable);
      }
    }
  }

  CScriptTrigger::AcceptScriptMsg(mgr, msg);
}

bool CScriptBallTrigger::ShouldSendScriptMsgs(CActor& actor, CStateManager&) const {
  const CPlayer* player = TCastToPtr< CPlayer >(actor);
  if (player != nullptr && mCapturedPlayerIndex != kInvalidPlayerIndex) {
    const bool captured = player->GetPlayerIndex() == mCapturedPlayerIndex;
    return captured;
  }
  return true;
}

CEntity* LoadBallTrigger(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrBallTrigger sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrBallTrigger.inc"

  return rs_new CScriptBallTrigger(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      sldrThis.editorProperties.transform.scale, LdrToDamageInfo(sldrThis.trigger.damage),
      sldrThis.trigger.forceField, sldrThis.trigger.flagsTrigger, sldrThis.attractionForce,
      sldrThis.attractionAngle, sldrThis.attractionDistance, CVector3f(sldrThis.attractionDirection),
      sldrThis.noBallMovement);
}
