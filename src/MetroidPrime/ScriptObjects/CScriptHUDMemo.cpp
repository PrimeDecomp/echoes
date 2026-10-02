#include "MetroidPrime/ScriptObjects/CScriptHUDMemo.hpp"

#include "MetroidPrime/HUD/CSamusHud.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "MetroidPrime/ScriptLoader/SLdrHUDMemo.hpp"

#include "Kyoto/Text/CStringTable.hpp"

CScriptHUDMemo::CScriptHUDMemo(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                               const CHUDMemoParms& parms, bool useOriginator,
                               CScriptHUDMemo::EDisplayType disp, CAssetId msg)
: CEntity(uid, info, name, false)
, m_parms(parms)
, m_useOriginator(useOriginator)
, m_dispType(disp)
, m_stringTableId(msg)
, m_stringTable(msg == kInvalidAssetId ? rstl::optional_object_null()
                                       : rstl::optional_object< TLockedToken< CStringTable > >(
                                             gpSimplePool->GetObj(SObjectTag('STRG', msg)))) {}

CScriptHUDMemo::~CScriptHUDMemo() {}

void CScriptHUDMemo::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CHUDMemoParms parms = m_parms;
  if (m_useOriginator) {
    if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(msg.GetOriginator()))) {
      uint mask = mgr.MaskUIdNumPlayers(msg.GetOriginator());
      parms = CHUDMemoParms(m_parms.GetDisplayTime(), m_parms.IsClearMemoWindow(),
                            m_parms.IsFadeOutOnly(), m_parms.IsHintMemo(), 1 << mask, true);
    }
  }

  switch (msg.GetMessage()) {
  case kSM_SetToZero:
    if (GetActive()) {
      if (m_dispType == kDT_MessageBox) {
        mgr.ShowPausedHUDMemo(m_stringTableId, m_parms.GetDisplayTime());
      } else if (m_stringTable) {
        CSamusHud::DisplayHudMemo((*m_stringTable)->GetString(0), parms);
      } else {
        CSamusHud::DisplayHudMemo(rstl::wstring_l(L""), parms);
      }
    }
    break;
  case kSM_Deactivate:
    if (GetActive() && m_dispType == kDT_StatusMessage) {
      CSamusHud::DisplayHudMemo(rstl::wstring_l(L""),
                                CHUDMemoParms(0.f, false, true, false, 0xf, true));
    }
    break;
  }

  CEntity::AcceptScriptMsg(mgr, msg);
}

CEntity* LoadHUDMemo(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrHUDMemo sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrHUDMemo.inc"

  int mask = 0;
  if (sldrThis.player1) {
    mask |= 1;
  }
  if (sldrThis.player2) {
    mask |= 2;
  }
  if (sldrThis.player3) {
    mask |= 4;
  }
  if (sldrThis.player4) {
    mask |= 8;
  }

  return new CScriptHUDMemo(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                            LdrToEntityInfo(info, sldrThis.editorProperties),
                            CHUDMemoParms(sldrThis.displayTime, sldrThis.clearWindow, false, false,
                                          mask, sldrThis.typeOut),
                            sldrThis.useOriginator,
                            CScriptHUDMemo::EDisplayType(sldrThis.displayType), sldrThis.string

  );
}
