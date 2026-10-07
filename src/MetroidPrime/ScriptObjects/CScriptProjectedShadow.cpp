#include "MetroidPrime/ScriptObjects/CScriptProjectedShadow.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CProjectedShadow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrShadowProjector.hpp"
#include "MetroidPrime/TCastTo.hpp"

CEntity* LoadShadowProjector(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrShadowProjector sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrShadowProjector.inc"

  return rs_new CScriptShadowProjector(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      sldrThis.shadowOffset, sldrThis.unknown_0xbca8b742, sldrThis.shadowScale,
      sldrThis.shadowHeight, sldrThis.shadowAlpha, sldrThis.shadowFadeTime,
      sldrThis.unknown_0x606e341c);
}

CScriptShadowProjector::CScriptShadowProjector(TUniqueId uid, const rstl::string& name,
                                               const CEntityInfo& info,
                                               const CTransform4f& transform,
                                               const CVector3f& offset, bool persistent,
                                               float scale, float zOffsetAdjust, float opacity,
                                               float opacityChange, int textureSize)
: CActor(uid, name, info, 0, transform, CModelData::CModelDataNull(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
, mScale(scale)
, mOffset(offset)
, mZOffsetAdjust(zOffsetAdjust)
, mOpacity(opacity)
, mOpacityRecip(close_enough(opacity, 0.f) ? 1.f : opacityChange / opacity)
, mTarget(kInvalidUniqueId)
, mTextureSize(textureSize)
, mPersistent(persistent)
, mShadowInvalidated(false) {}

void CScriptShadowProjector::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_AreaLoaded:
    for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
         it != GetConnectionList().end(); ++it) {
      if (it->state != kSS_Play) {
        continue;
      }
      if (CActor* actor =
              TCastToPtr< CActor >(mgr.GetObjectByIdFromListAll(mgr.GetIdForScript(it->objId)))) {
        if (actor->HasModelData()) {
          mTarget = actor->GetUniqueId();
          break;
        }
      }
    }
    if (mTarget == kInvalidUniqueId) {
      mgr.DeleteObjectRequest(GetUniqueId());
      return;
    }
    // Fall through: initialize the shadow using the current active state.
  case kSM_Activate:
  case kSM_Deactivate:
    if (GetActive() && mTarget != kInvalidUniqueId && mOpacity > 0.f) {
      mProjectedShadow = rs_new CProjectedShadow(mTextureSize, mTextureSize, mPersistent, 0);
    } else {
      mProjectedShadow = nullptr;
    }
    break;
  case kSM_Decrement:
    if (GetActive() && mOpacity > 0.f) {
      mShadowInvalidated = true;
    }
    break;
  default:
    break;
  }
}

void CScriptShadowProjector::AddToRenderer(const CStateManager& mgr) const {}

void CScriptShadowProjector::PreRender(CStateManager& mgr) {
  SetPreRenderClipped(true);
  if (mProjectedShadow.null()) {
    return;
  }

  CActor* actor = TCastToPtr< CActor >(mgr.GetObjectByIdFromListAll(mTarget));
  if (actor == nullptr || !actor->HasModelData()) {
    mTarget = kInvalidUniqueId;
    return;
  }
  if (!actor->GetActive()) {
    return;
  }

  if (actor->HasAnimation()) {
    actor->AnimationData()->PreRender();
  }
  mProjectedShadow->SetOpacity(mOpacity);
  mProjectedShadow->RenderShadowBuffer(mgr, *actor->GetModelData(), actor->GetTransform(), 0,
                                       mOffset, mScale, mZOffsetAdjust);
}

void CScriptShadowProjector::Think(float dt, CStateManager& mgr) {
  if (GetActive() && mShadowInvalidated) {
    mOpacity -= mOpacityRecip * dt;
    if (mOpacity <= 0.f) {
      mOpacity = 0.f;
      mProjectedShadow = nullptr;
      mShadowInvalidated = false;
      SendScriptMsgs(kSS_Zero, mgr, kInvalidUniqueId, kSM_None);
    }
  }
}

CScriptShadowProjector::~CScriptShadowProjector() {}
