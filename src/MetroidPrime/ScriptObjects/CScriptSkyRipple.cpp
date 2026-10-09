#include "MetroidPrime/ScriptObjects/CScriptSkyRipple.hpp"

#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSkyRipple.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/TCastTo.hpp"

static EScriptObjectState sFirstSkyState = kSS_InternalState0;
static EScriptObjectState sSecondSkyState = kSS_InternalState1;

CScriptSkyRipple::CScriptSkyRipple(TUniqueId uid, const CEntityInfo& info, const rstl::string& name)
: CActor(uid, name, info, 0, CTransform4f::Identity(), CModelData::None(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
, mFirstSkyActor(kInvalidUniqueId)
, mSecondSkyActor(kInvalidUniqueId) {}

void CScriptSkyRipple::PreRender(CStateManager& mgr) {
  if (GetActive()) {
    mgr.RenderFirstSorted(GetUniqueId());
  }
}

TUniqueId CScriptSkyRipple::FindSkyActor(CStateManager& mgr, EScriptObjectState state) {
  const TUniqueId id = FindConnectedObject(mgr, state, kSM_Attach);
  CScriptActor* actor = TCastToPtr< CScriptActor >(mgr.ObjectById(id));
  if (actor != nullptr && actor->CheckActorRenderOnly()) {
    actor->SetSkipRendering(true);
    return id;
  }
  return kInvalidUniqueId;
}

void CScriptSkyRipple::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CActor::AcceptScriptMsg(mgr, msg);
  if (msg.GetMessage() == kSM_AreaLoaded) {
    mFirstSkyActor = FindSkyActor(mgr, sFirstSkyState);
    mSecondSkyActor = FindSkyActor(mgr, sSecondSkyState);
  }
}

void CScriptSkyRipple::Render(const CStateManager& mgr) const {
  CGraphics::DisableAllLights();
  gpRender->SetAmbientColor(CColor::White());
  CGX::SetColorUpdate(false);

  GXFogType fogType;
  float fogStart;
  float fogEnd;
  float fogNear;
  float fogFar;
  GXColor fogColor;
  CGX::GetFog(&fogType, &fogStart, &fogEnd, &fogNear, &fogFar, &fogColor);
  CGX::SetFog(GX_FOG_NONE, fogStart, fogEnd, fogNear, fogFar, fogColor);

  CGraphics::SetDepthRange(0.9999999f, 0.9999999f);
  const CVector3f viewPos = CGraphics::GetViewPoint();
  if (mFirstSkyActor != kInvalidUniqueId) {
    RenderSkyActor(mgr, mFirstSkyActor, viewPos, 0);
  }
  CGX::SetColorUpdate(true);
  RenderSkyActor(mgr, mSecondSkyActor, viewPos, 1);

  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
  CGraphics::SetDepthRange(0.125f, 1.f);
  CGX::SetFog(fogType, fogStart, fogEnd, fogNear, fogFar, fogColor);
  gpRender->SetModelMatrix(CTransform4f::Identity());
}

void CScriptSkyRipple::RenderSkyActor(const CStateManager& mgr, TUniqueId id, const CVector3f& pos,
                                      int pass) const {
  if (!GetActive()) {
    return;
  }
  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(id));
  if (actor == nullptr) {
    return;
  }
  if (actor->GetPreRenderClipped()) {
    return;
  }

  CModelFlags flags = CModelFlags(CModelFlags::kT_Opaque, 1.f);
  if (pass == 1) {
    const CModelFlags& actorFlags = actor->GetModelFlags();
    if (mFirstSkyActor == kInvalidUniqueId) {
      flags = actorFlags;
    } else {
      flags = actorFlags.DepthForwards();
    }
  }
  if (pass == 1 && flags.GetTrans() == CModelFlags::kT_Blend &&
      flags.GetColorRef().GetAlphau8() == 0) {
    return;
  }

  const CModelData* modelData = actor->GetModelData();
  CTransform4f xf = actor->GetTransform();
  xf.SetTranslation(pos);
  modelData->Render(mgr, xf, nullptr, flags);
}

void CScriptSkyRipple::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    CActor* first = TCastToPtr< CActor >(mgr.ObjectById(mFirstSkyActor));
    CActor* second = TCastToPtr< CActor >(mgr.ObjectById(mSecondSkyActor));
    const CVector3f& cameraPos =
        mgr.GetCameraManager(0)->GetCurrentCamera(mgr, true)->GetTranslation();
    if (first != nullptr) {
      first->SetTranslation(cameraPos);
    }
    if (second != nullptr) {
      second->SetTranslation(cameraPos);
    }
  }
  CActor::Think(dt, mgr);
}

CEntity* LoadSkyRipple(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSkyRipple sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSkyRipple.inc"

  return rs_new CScriptSkyRipple(mgr.AllocateUniqueId(),
                                 LdrToEntityInfo(info, sldrThis.editorProperties),
                                 sldrThis.editorProperties.name);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SScriptSkyRipple_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadSkyRipple;
  SetSScriptSkyRipple_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSScriptSkyRipple_FuncPtrs(nullptr); }
#endif
