#include "MetroidPrime/ScriptObjects/CScriptHUDMemo.hpp"

#include "MetroidPrime/HUD/CSamusHud.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "MetroidPrime/ScriptLoader/Struct/SLdrHUDMemo.hpp"

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

void CScriptHUDMemo::AcceptScriptMsg(CStateManager& mgr, CScriptMsg& msg) {
  CHUDMemoParms parms = m_parms;
  if (m_useOriginator) {
    if (CastToPlayer(mgr.GetObjectById(msg.GetOriginator()))) {
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

const CEntityInfo& EntityInfoWithEditorProperties(const CEntityInfo&, const SLdrEditorProperties&);

CScriptHUDMemo* LoadHUDMemo(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  SLdrHUDMemo sldrMemo;

  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
      case 0x255a4580:
        LoadTypedefSLdrEditorProperties(sldrMemo.editorProperties, input);
        break;
      case 0x1a26c1cc: //('display_time', _decode_display_time),
        sldrMemo.displayTime = input.ReadFloat();
        break;
      case 0x84e2496f:
        sldrMemo.clearWindow = input.ReadBool();
        break;
      case 0xa8fadfa5:
        sldrMemo.player1 = input.ReadBool();
        break;
      case 0xef5aa575:
        sldrMemo.player2 = input.ReadBool();
        break;
      case 0xd23a8cc5:
        sldrMemo.player3 = input.ReadBool();
        break;
      case 0x601a50d5:
        sldrMemo.player4 = input.ReadBool();
        break;
      case 0xafd0158e:
        sldrMemo.typeOut = input.ReadBool();
        break;
      case 0xbd6f7b11:
        sldrMemo.useOriginator = input.ReadBool();
        break;
      case 0x4ab3b95b:
        sldrMemo.displayType = input.ReadInt32();
        break;
      case 0x9182250c:
        sldrMemo.string = input.ReadInt32();
        break;

    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }

  int mask = 0;
  if (sldrMemo.player1) {
    mask |= 1;
  }
  if (sldrMemo.player2) {
    mask |= 2;
  }
  if (sldrMemo.player3) {
    mask |= 4;
  }
  if (sldrMemo.player4) {
    mask |= 8;
  }

  return new CScriptHUDMemo(mgr.AllocateUniqueId(), sldrMemo.editorProperties.name,
                            EntityInfoWithEditorProperties(info, sldrMemo.editorProperties),
                            CHUDMemoParms(sldrMemo.displayTime, sldrMemo.clearWindow, false, false,
                                          mask, sldrMemo.typeOut),
                            sldrMemo.useOriginator,
                            CScriptHUDMemo::EDisplayType(sldrMemo.displayType), sldrMemo.string

  );
}
