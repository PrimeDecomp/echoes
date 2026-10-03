#ifndef _CSCRIPTLAYERCONTROLLER
#define _CSCRIPTLAYERCONTROLLER

#include "MetroidPrime/CEntity.hpp"

class CGameArea;
class CWorldLayerState;

// Guessed name: controls persistent and dynamically loaded script layers.
class CScriptLayerController : public CEntity {
public:
  CScriptLayerController(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         uint areaSaveId, int layer, bool isDynamic);

  // CEntity
  ~CScriptLayerController() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

private:
  // Guessed names.
  CGameArea* GetAreaForAreaId(CStateManager& mgr, TAreaId area);
  TAreaId GetAreaIdAndWorldLayerState(CStateManager& mgr, CWorldLayerState** layers);

  uint mAreaSaveId;
  TLayerId mLayerId;
  bool mIsDynamic : 1;
  bool mWaitingForLoad : 1;
  bool mActivateWhenLoaded : 1;
};
CHECK_SIZEOF(CScriptLayerController, 0x30)

#endif // _CSCRIPTLAYERCONTROLLER
