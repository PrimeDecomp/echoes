#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"
#include "dolphin/gx.h"


class CScriptForgottenObject : public CEntity {
public:
  CScriptForgottenObject(TUniqueId uid, const CEntityInfo& info, const rstl::string& name);
  CEntity* TypesMatch(int typeId) const;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg&);
  ~CScriptForgottenObject() {}

  virtual void Render1(CStateManager& mgr);
  virtual void Render2(CStateManager& mgr);

private:
  void RenderInternal(CStateManager& mgr, TUniqueId uid, bool b);

  TUniqueId x24_;
  TUniqueId x28_;
};

CScriptForgottenObject::CScriptForgottenObject(TUniqueId uid, const CEntityInfo& info,
                                               const rstl::string& name)
: CEntity(uid, info, name, false), x24_(kInvalidUniqueId), x28_(kInvalidUniqueId) {}

extern "C" TUniqueId fn_24_478(CScriptForgottenObject* self, CStateManager& mgr,
                               EScriptObjectState state);

TUniqueId fn_24_478(CScriptForgottenObject* self, CStateManager& mgr, EScriptObjectState state) {
  TUniqueId id = self->FindConnectedObject(mgr, state, kSM_None);
  CScriptActor* entity = TCastToPtr< CScriptActor >(mgr.ObjectById(id));
  if (entity && entity->CheckActorRenderOnly()) {
    // TODO
    return id;
  }
  return kInvalidUniqueId;
}

void CScriptForgottenObject::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CEntity::AcceptScriptMsg(mgr, msg);
  if (msg.GetMessage() == kSM_XALD) {
    x24_ = fn_24_478(this, mgr, kSS_Zero);
    x28_ = fn_24_478(this, mgr, kSS_MaxReached);
  }
}

void CScriptForgottenObject::Render1(CStateManager& mgr) { RenderInternal(mgr, x24_, false); }

void CScriptForgottenObject::Render2(CStateManager& mgr) { RenderInternal(mgr, x28_, true); }

void CScriptForgottenObject::RenderInternal(CStateManager& mgr, TUniqueId uid, bool b) {
  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(uid));
  GXSetColorUpdate(GX_FALSE);
  const CModelData* data = actor->GetModelData();
  if (b) {
    // TODO
  } else {
    // TODO
  }
  CModelFlags flags(CModelFlags::kT_Opaque, 1.f);
  data->Render(mgr, actor->GetTransform(), nullptr, flags);
  if (b) {
    // TODO
  }
  GXSetColorUpdate(GX_TRUE);
}

CEntity* LoadForgottenObject(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  SLdrEditorProperties props();
  // TODO
}

static void SetFuncPtrs() {
  static SScriptForgottenObject_FuncPtrs ptrs;
  ptrs.loader = &LoadForgottenObject;
  SetSScriptForgottenObject_FuncPtrs(&ptrs);
}

void RELMain() { SetFuncPtrs(); }

void RELExit() { SetSScriptForgottenObject_FuncPtrs(nullptr); }
