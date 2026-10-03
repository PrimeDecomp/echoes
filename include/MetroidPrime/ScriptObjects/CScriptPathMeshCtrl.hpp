#ifndef _CSCRIPTPATHMESHCTRL
#define _CSCRIPTPATHMESHCTRL

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/PathFinding/CPathFindRegion.hpp"

// Guessed class name, based on the PMCT loader and obstruction-count consumers.
class CScriptPathMeshCtrl : public CActor {
public:
  CScriptPathMeshCtrl(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                      const CTransform4f& xf, int initialCount, bool useObstruction0);

  // CEntity
  ~CScriptPathMeshCtrl() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

private:
  // Reconstructed helper/member names; obstruction category meanings remain unknown.
  void ModifyObstructionCount(CStateManager& mgr, int delta);

  int mInitialCount;
  EPathFindObstructions mObstructionType;
};
CHECK_SIZEOF(CScriptPathMeshCtrl, 0x160)

#endif // _CSCRIPTPATHMESHCTRL
