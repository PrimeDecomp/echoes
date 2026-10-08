#include "MetroidPrime/Enemies/CDarkSamusBattleStage.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"

CDarkSamusBattleStage::CDarkSamusBattleStage(TUniqueId uid, const rstl::string& name,
                                             const CEntityInfo& info,
                                             const SLdrDSStageInfo& stageInfo)
: CEntity(uid, info, name, 0), mStageInfo(stageInfo) {}

CDarkSamusBattleStage::~CDarkSamusBattleStage() {}

const SLdrDSStageInfo& CDarkSamusBattleStage::GetStageInfo() const { return mStageInfo; }

CEntity* REL_LoadDarkSamusBattleStage(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrDarkSamusBattleStage sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrDarkSamusBattleStage.inc"

  return rs_new CDarkSamusBattleStage(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                      LdrToEntityInfo(info, sldrThis.editorProperties),
                                      sldrThis.stage);
}

static void SetFuncPtrs() {
  static SScriptDarkSamusBattleStage_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadDarkSamusBattleStage;
  SetSScriptDarkSamusBattleStage_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSScriptDarkSamusBattleStage_FuncPtrs(nullptr); }
