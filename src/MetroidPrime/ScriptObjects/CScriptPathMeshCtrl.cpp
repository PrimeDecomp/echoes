#include "MetroidPrime/ScriptObjects/CScriptPathMeshCtrl.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPathMeshCtrl.hpp"

CScriptPathMeshCtrl::CScriptPathMeshCtrl(TUniqueId uid, const rstl::string& name,
                                         const CEntityInfo& info, const CTransform4f& xf,
                                         int initialCount, bool useObstruction0)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mInitialCount(initialCount)
, mObstructionType(useObstruction0 ? kPFO_Unknown0 : kPFO_Unknown2) {
  SetUseInSortedLists(false);
  SetCallTouch(false);
}

CScriptPathMeshCtrl::~CScriptPathMeshCtrl() {}

void CScriptPathMeshCtrl::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_Increment:
    if (GetActive()) {
      ModifyObstructionCount(mgr, 1);
    }
    break;
  case kSM_Decrement:
    if (GetActive()) {
      ModifyObstructionCount(mgr, -1);
    }
    break;
  case kSM_XALD:
    ModifyObstructionCount(mgr, mInitialCount);
    break;
  }
}

void CScriptPathMeshCtrl::ModifyObstructionCount(CStateManager& mgr, int delta) {
  CPFArea* area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea;
  if (area != nullptr) {
    const CTransform4f& transform = area->GetTransform();
    const CVector3f point =
        transform.TransposeRotate(GetTranslation() - transform.GetTranslation());
    for (int i = 0; i < area->GetNumRegions(); ++i) {
      CPFRegion* region = area->GetRegionPtr(i);
      if (region != nullptr && region->IsPointInside(point)) {
        region->ModifyObstructionCount(mObstructionType, delta);
      }
    }
  }
  mgr.InformListeners(GetTranslation(), static_cast< EListenNoiseType >(4));
}

CEntity* LoadPathMeshCtrl(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrPathMeshCtrl sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrPathMeshCtrl.inc"
  return rs_new CScriptPathMeshCtrl(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                    LdrToEntityInfo(info, sldrThis.editorProperties),
                                    LdrToTransform4f(sldrThis.editorProperties),
                                    sldrThis.initialCount, sldrThis.type == 1);
}
