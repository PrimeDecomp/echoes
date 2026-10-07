#include "MetroidPrime/ScriptObjects/CScriptHUDHint.hpp"

#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHUDHint.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

CScriptHUDHint::CScriptHUDHint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                               const CTransform4f& transform, CAssetId hudTexture,
                               float minScreenSize, float maxScreenSize, float iconScale,
                               float animationTime, int animationFrames, int visorMask)
: CActor(uid, name, info, 0, transform, CModelData::CModelDataNull(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
, mHudTexture()
, mMinScreenSize(minScreenSize)
, mMaxScreenSize(maxScreenSize)
, mIconScale(iconScale)
, mAnimationTime(animationTime)
, mAnimationFrames(animationFrames)
, mVisorMask(visorMask)
, mAnimationPosition(0.f)
, mAnimationState(kAS_Stopped) {
  if (hudTexture != kInvalidAssetId) {
    mHudTexture = TLockedToken< CTexture >(gpSimplePool->GetObj(SObjectTag('TXTR', hudTexture)));
  }
}

void CScriptHUDHint::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CActor::AcceptScriptMsg(mgr, msg);
  switch (msg.GetMessage()) {
  case kSM_ToggleActive:
    if (GetActive()) {
      break;
    }
  case kSM_Deactivate:
  case kSM_Delete: {
    const int numPlayers = mgr.GetNumPlayers();
    for (uint player = 0; int(player) < numPlayers; ++player) {
      CPlayerState* state = mgr.PlayerState(player);
      if (state->HasId(GetUniqueId())) {
        state->RemoveId(GetUniqueId());
      }
    }
    break;
  }
  default:
    break;
  }
  if (!GetActive()) {
    return;
  }

  switch (msg.GetMessage()) {
  case kSM_InternalMessage00:
    mAnimationState = kAS_Forward;
    break;
  case kSM_InternalMessage01:
    mAnimationState = kAS_Backward;
    break;
  case kSM_Increment:
    if (mAnimationFrames > 0 && mAnimationPosition < mAnimationTime) {
      mAnimationPosition += mAnimationTime / mAnimationFrames;
      if (mAnimationPosition >= mAnimationTime) {
        SetAnimationToMax(mgr);
      }
    }
    break;
  case kSM_Decrement:
    if (mAnimationFrames > 0 && mAnimationPosition > 0.f) {
      mAnimationPosition -= mAnimationTime / mAnimationFrames;
      if (mAnimationPosition <= 0.f) {
        SetAnimationToZero(mgr);
      }
    }
    break;
  case kSM_SetToZero:
    SetAnimationToZero(mgr);
    break;
  case kSM_SetToMax:
    SetAnimationToMax(mgr);
    break;
  case kSM_Stop:
    mAnimationState = kAS_Stopped;
    break;
  default:
    break;
  }
}

void CScriptHUDHint::Think(float dt, CStateManager& mgr) {
  CActor::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }

  const CSphere sphere(GetTranslation(), 1.f);
  const uint numPlayers = mgr.GetNumPlayers();
  for (int player = 0; player < numPlayers; ++player) {
    CPlayerState* state = mgr.PlayerState(player);
    bool visible = false;
    if ((mVisorMask & (1 << state->GetCurrentVisor())) != 0 &&
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetOcclusionState() ==
            CGameArea::kOS_Visible) {
      const CCameraManager* cameraManager = mgr.GetCameraManager(player);
      const CTransform4f transform = cameraManager->GetCurrentCameraTransform(mgr, true);
      const CGameCamera* camera = cameraManager->GetCurrentCamera(mgr, true);
      const CFrustumPlanes frustum(transform, CRelAngle::FromDegrees(camera->GetFov()).AsRadians(),
                                   camera->GetAspectRatio(), camera->GetNearClipDistance(), false,
                                   100.f);
      visible = frustum.SphereInFrustumPlanes(sphere);
    }

    const bool registered = state->HasId(GetUniqueId());
    if (visible && !registered) {
      state->AddId(GetUniqueId());
    } else if (!visible && registered) {
      state->RemoveId(GetUniqueId());
    }
  }

  if (mAnimationState == kAS_Forward) {
    mAnimationPosition += dt;
    if (mAnimationPosition >= mAnimationTime) {
      SetAnimationToMax(mgr);
    }
  } else if (mAnimationState == kAS_Backward) {
    mAnimationPosition -= dt;
    if (mAnimationPosition <= 0.f) {
      SetAnimationToZero(mgr);
    }
  }
}

void CScriptHUDHint::SetAnimationToZero(CStateManager& mgr) {
  mAnimationPosition = 0.f;
  SendScriptMsgs(kSS_Zero, mgr);
  mAnimationState = kAS_Stopped;
}

void CScriptHUDHint::SetAnimationToMax(CStateManager& mgr) {
  mAnimationPosition = mAnimationTime;
  SendScriptMsgs(kSS_MaxReached, mgr);
  mAnimationState = kAS_Stopped;
}

CTexture* CScriptHUDHint::GetTexture() const { return mHudTexture ? **mHudTexture : nullptr; }

CScriptHUDHint::TTextureCoordinates CScriptHUDHint::GetTextureCoordinates() const {
  if (mAnimationFrames > 1 && CMath::AbsF(mAnimationTime - 0.f) >= Real32::Epsilon()) {
    const int frame = int((mAnimationFrames - 1) * (mAnimationPosition / mAnimationTime));
    const float width = 1.f / mAnimationFrames;
    return TTextureCoordinates(CVector2f(frame * width, 0.f), CVector2f((frame + 1) * width, 1.f));
  }
  return TTextureCoordinates(CVector2f(0.f, 0.f), CVector2f(1.f, 1.f));
}

CEntity* LoadHUDHint(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrHUDHint sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrHUDHint.inc"
  return rs_new CScriptHUDHint(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                               LdrToEntityInfo(info, sldrThis.editorProperties),
                               LdrToTransform4f(sldrThis.editorProperties), sldrThis.hudTexture,
                               sldrThis.unknown_0x6078a651, sldrThis.unknown_0xf00bb6bb,
                               sldrThis.iconScale, sldrThis.animationTime, sldrThis.animationFrames,
                               sldrThis.unknown_0xd993f97b);
}
