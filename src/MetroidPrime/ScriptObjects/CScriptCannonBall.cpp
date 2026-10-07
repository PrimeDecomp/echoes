
#include "MetroidPrime/ScriptObjects/CScriptCannonBall.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader/SLdrCannonBall.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/TCastTo.hpp"

CScriptCannonBall::CScriptCannonBall(TUniqueId uid, const rstl::string& name,
                                     const CEntityInfo& info, const CTransform4f& xf,
                                     CAssetId effect)

: CActor(uid, name, info, 0, xf, CModelData(), CMaterialList(), CActorParameters::None(),
         kInvalidUniqueId)
, m_effect(effect) {}

CScriptCannonBall::~CScriptCannonBall() {}

void CScriptCannonBall::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {

  case kSM_Increment: {
    if (CPlayer* player =
            TCastToPtr< CPlayer >(mgr.GetObjectByIdFromListAll(msg.GetOriginator()))) {
      CMorphBall* morph = player->GetMorphBall();
      CTransform4f xf = morph->GetSurfaceToWorld();
      player->SetTransform(
          CTransform4f(xf.BuildMatrix3f(), player->GetTranslation())); // todo use position
      morph->SwitchToTire();
      m_fields[player->GetPlayerIndex()].OnIncrementMsg(mgr, true);
    }
    break;
  }

  case kSM_Create: {
    for (int playerIndex = 0; playerIndex < mgr.GetNumPlayers(); ++playerIndex) {
      TUniqueId id = mgr.AllocateUniqueId();

      CLightParameters lParams;
      CGameSplineDesc spline(SLdrSpline(), CMotionSpline::kST_Bezier, 1.0f, false);

      CScriptEffect* newEffect =
          new CScriptEffect(id, rstl::string_l("CannonBall Effect"),
                            CEntityInfo(GetCurrentAreaId(), rstl::vector< SConnection >(), true),
                            CTransform4f::Identity(), CVector3f::One(), m_effect, 1, 0, 0, 0, 1.0f,
                            1.0f, 0.0f, 0.0f, false, 1.0f, 2.0f, 1.0f, true, true, true, lParams,
                            false, spline, false, false, false, CScriptEffect::kRO_Normal);
      newEffect->SetNextDrawNode(mgr.GetPlayer(playerIndex)->GetUniqueId());
      mgr.AddObject(newEffect);

      m_fields.push_back(TrackedShot(id, false));
    }
    break;
  }

  case kSM_Delete: {
    for (int i = 0; i < m_fields.size(); ++i) {
      m_fields[i].FreeScriptObject(mgr);
    }
    m_fields.clear();
    break;
  }

  default:
    break;
  }
}

void CScriptCannonBall::Think(float dt, CStateManager& mgr) {
  for (int i = 0; i < m_fields.size(); ++i) {
    m_fields[i].Think(dt, mgr, i);
  }
}

CScriptCannonBall::TrackedShot::TrackedShot(TUniqueId id, const bool b)
: m_scriptObject(id), m_f(1.0), m_updateFrameIdx(0), m_b(b) {}

void CScriptCannonBall::TrackedShot::Think(float dt, CStateManager& mgr, int index) {
  if (!m_flag2) {
    return;
  }
  CScriptEffect* effect = TCastToPtr< CScriptEffect >(mgr.GetObjectByIdFromListAll(m_scriptObject));
  if (!effect) {
    return;
  }

  CPlayer* player = mgr.GetPlayer(index);
  if (m_f > 0.0f) {
    CTransform4f mat = player->GetMorphBall()->GetBallToWorld();
    effect->SetTransform(
        CTransform4f::LookAt(mat.GetTranslation(), mat.GetTranslation() + player->GetLookDir()));
  }
  if (m_b) {
    if (!effect->IsEmitting()) {
      effect->AcceptScriptMsg(mgr,
                              CScriptMsg(kInvalidUniqueId, effect->GetUniqueId(), kSM_Activate));
      player->GetPlayerState()->SetItemAmount(CPlayerState::kIT_CannonBall, 1);
    }

    CMorphBall* morph = player->GetMorphBall();

    int uVar4 = (morph->GetLastWallCollisionFrame() - m_updateFrameIdx) < 0;
    int uVar3 = (morph->GetLastFloorCollisionFrame() - m_updateFrameIdx) < 0;
    if (uVar3 < uVar4) {
      uVar3 = uVar4;
    }
    bool disable = true;
    if (uVar3 < 6 && morph->GetBallState() != CMorphBall::kBS_Spider) {
      CPlayer::EPlayerMorphBallState state = CPlayer::kMS_Unmorphed;
      if (player->GetSpawnedMorphballState() == CPlayer::kMS_Morphed) {
        state = player->GetMorphballTransitionState();
      }
      if (state == CPlayer::kMS_Morphed) {
        disable = false;
      }
    }
    if (disable) {

      effect->AcceptScriptMsg(mgr,
                              CScriptMsg(kInvalidUniqueId, effect->GetUniqueId(), kSM_Deactivate));
      player->GetPlayerState()->SetItemAmount(CPlayerState::kIT_CannonBall, 0);
      m_b = false;
    }

  } else {
    m_f -= dt / 0.25f;
    if (m_f < 0.0f) {
      m_f = 0.0f;
      effect->AcceptScriptMsg(mgr,
                              CScriptMsg(kInvalidUniqueId, effect->GetUniqueId(), kSM_Deactivate));
      m_flag2 = false;
    }
  }

  CColor color = CColor::White().WithAlphaModulatedBy(m_f * player->fn_8000BE98());
  effect->SetModelFlags(CModelFlags(CModelFlags::kT_One, color));
}

void CScriptCannonBall::TrackedShot::OnIncrementMsg(CStateManager& mgr, bool param) {
  const bool b = param;
  m_b = b;
  m_flag2 = b;
  if (!b) {
    return;
  }
  m_updateFrameIdx = mgr.GetUpdateFrameIdx();
  m_f = 1.0f;
}

void CScriptCannonBall::TrackedShot::FreeScriptObject(CStateManager& mgr) {
  mgr.DeleteObjectRequest(m_scriptObject);
}


CEntity* REL_LoadCannonBall(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrCannonBall sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrCannonBall.inc"

  return new CScriptCannonBall(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                               LdrToEntityInfo(info, sldrThis.editorProperties),
                               LdrToTransform4f(sldrThis.editorProperties), sldrThis.effect

  );
}

SCannonBall_FuncPtrs REL_loader_CannonBall;

void SetRelLoaderFunctionToLoader() {
  REL_loader_CannonBall.mLoadCannonBall = REL_LoadCannonBall;
  SetSCannonBall_FuncPtrs(&REL_loader_CannonBall);
}

extern "C" void RELMain() { SetRelLoaderFunctionToLoader(); }

extern "C" void RELExit() { SetSCannonBall_FuncPtrs(nullptr); }
