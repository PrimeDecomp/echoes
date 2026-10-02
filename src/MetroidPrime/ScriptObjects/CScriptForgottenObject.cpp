#include "MetroidPrime/ScriptObjects/CScriptForgottenObject.hpp"

#include "Kyoto/Graphics/CGX.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader/SLdrForgottenObject.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"
#include "dolphin/gx.h"

static const float skDefaultAlpha = 1.f;
CScriptForgottenObject::CScriptForgottenObject(const TUniqueId uid, const CEntityInfo& info,
                                               const rstl::string& name)
: CEntity(uid, info, name, 0), x24_(kInvalidUniqueId), x28_(kInvalidUniqueId) {}

TUniqueId CScriptForgottenObject::DisableTargetRendering(CStateManager& mgr,
                                                         const EScriptObjectState state) const {
  const TUniqueId id = FindConnectedObject(mgr, state, kSM_None);
  if (CScriptActor* entity = TCastToPtr< CScriptActor >(mgr.ObjectById(id))) {
    if (entity->CheckActorRenderOnly()) {
      entity->SetSkipRendering(true);
      return id;
    }
  }
  return kInvalidUniqueId;
}

void CScriptForgottenObject::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CEntity::AcceptScriptMsg(mgr, msg);
  if (msg.GetMessage() == kSM_XALD) {
    x24_ = DisableTargetRendering(mgr, kSS_Zero);
    x28_ = DisableTargetRendering(mgr, kSS_MaxReached);
  }
}

void CScriptForgottenObject::Render1(CStateManager& mgr) { RenderInternal(mgr, x24_, false); }

void CScriptForgottenObject::Render2(CStateManager& mgr) { RenderInternal(mgr, x28_, true); }

void CScriptForgottenObject::RenderInternal(CStateManager& mgr, TUniqueId uid, bool b) const {
  if (!GetActive()) {
    return;
  }
  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(uid));
  CGX::SetColorUpdate(false);
  if (actor && !actor->GetPreRenderClipped()) {
    const CModelData* data = actor->GetModelData();
    if (b) {
      gpRender->SetDestinationAlpha(255);
    } else {
      gpRender->DisableDestinationAlpha();
    }
    data->Render(mgr, actor->GetTransform(), nullptr, CModelFlags::Normal());
    if (b) {
      gpRender->DisableDestinationAlpha();
    }
  }
  CGX::SetColorUpdate(GX_TRUE);
}

CEntity* LoadForgottenObject(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrForgottenObject sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrForgottenObject.inc"

  return rs_new CScriptForgottenObject(mgr.AllocateUniqueId(),
                                       LdrToEntityInfo(info, sldrThis.editorProperties),
                                       sldrThis.editorProperties.name);
}

static void SetFuncPtrs() {
  static SScriptForgottenObject_FuncPtrs funcPtrs;
  funcPtrs.loader = &LoadForgottenObject;
  SetSScriptForgottenObject_FuncPtrs(&funcPtrs);
}

void RELMain() { SetFuncPtrs(); }

void RELExit() { SetSScriptForgottenObject_FuncPtrs(nullptr); }
CScriptForgottenObject::~CScriptForgottenObject() {}
