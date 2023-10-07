#ifndef _CSCRIPTHUDMEMO
#define _CSCRIPTHUDMEMO

#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/HUD/CHUDMemoParms.hpp"

#include "Kyoto/TToken.hpp"
#include "Kyoto/SObjectTag.hpp"

#include "rstl/optional_object.hpp"

class CStringTable;
class CScriptHUDMemo : public CEntity {
public:
  enum EDisplayType {
    kDT_StatusMessage,
    kDT_MessageBox,
  };

private:
  CHUDMemoParms m_parms;  // 0x24
  bool m_useOriginator; // 0x30
  EDisplayType m_dispType; // 0x34
  CAssetId m_stringTableId; // 0x38
  rstl::optional_object< TLockedToken< CStringTable > > m_stringTable; // 0x3c

public:
  CScriptHUDMemo(TUniqueId, const rstl::string&, const CEntityInfo&, const CHUDMemoParms&,
                 bool useOriginator, CScriptHUDMemo::EDisplayType, CAssetId);
  ~CScriptHUDMemo();
  CEntity* TypesMatch(int) const override;

  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg&) override;
};
// CHECK_SIZEOF(CScriptHUDMemo, 0x4c)

#endif // _CSCRIPTHUDMEMO
