#ifndef _CSCRIPTMEMORYRELAY
#define _CSCRIPTMEMORYRELAY

#include "MetroidPrime/CEntity.hpp"

class CScriptMemoryRelay : public CEntity {
public:
  CScriptMemoryRelay(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const bool defaultActive, const bool skipSendActive);

  // CEntity
  ~CScriptMemoryRelay() override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  bool GetDefaultActive() const { return mDefaultActive; }

private:
  bool mDefaultActive : 1;
  bool mSkipSendActive : 1;
};
CHECK_SIZEOF(CScriptMemoryRelay, 0x28)

#endif // _CSCRIPTMEMORYRELAY
