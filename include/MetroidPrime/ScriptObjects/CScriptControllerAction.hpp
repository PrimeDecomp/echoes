#ifndef _CSCRIPTCONTROLLERACTION
#define _CSCRIPTCONTROLLERACTION

#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CEntity.hpp"

class CScriptControllerAction : public CEntity {
public:
  CScriptControllerAction(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                          CControlMapper::ECommands command, bool mapScreenResponse,
                          uint mapScreenSubaction, bool deactivateOnClose);

  // CEntity
  ~CScriptControllerAction() override;
  void Think(float dt, CStateManager& mgr) override;

private:
  CControlMapper::ECommands mCommand;
  uint mMapScreenSubaction;
  uchar mMapScreenResponse : 1;
  bool mDeactivateOnClose : 1;
  bool mPressed : 1;
};
CHECK_SIZEOF(CScriptControllerAction, 0x30)

#endif // _CSCRIPTCONTROLLERACTION
