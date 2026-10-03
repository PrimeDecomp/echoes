#ifndef _CSCRIPTCONTROLHINT
#define _CSCRIPTCONTROLHINT

#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CGameHint.hpp"

// Guessed names, based on the CTLH loader and command-filter consumers.
class CScriptControlHint : public CGameHint {
public:
  enum EDisableFlags {
    kDF_All = 0x1,
    kDF_Movement = 0x2,
    kDF_Weapons = 0x4,
    kDF_Beams = 0x8,
    kDF_Visors = 0x10,
    kDF_Orbit = 0x20,
    kDF_Unmorph = 0x40,
    kDF_Morph = 0x80,
    kDF_Look = 0x100,
    kDF_Command73 = 0x200,
    kDF_SpiderBall = 0x400,
    kDF_ResetGun = 0x80000000
  };
  typedef rstl::reserved_vector< rstl::pair< CControlMapper::ECommands, int >, 8 > TCommandStates;
  typedef rstl::reserved_vector< bool, CControlMapper::kC_Count > TCommandEnabled;

  CScriptControlHint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, int priority, float timer, uint disableFlags,
                     const TCommandStates& commandStates, EBreakHintType breakType,
                     uint deleteOnRemoval, uint requiredPresses, float unknown16c,
                     SCallback onExpire, SCallback onBreak, float breakDelay, int acrossAreas);

  // CEntity
  ~CScriptControlHint() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  uint GetDisableFlags() const { return mDisableFlags; }
  TCommandEnabled GetCommandEnabled() const { return mCommandEnabled; }

private:
  uint mDisableFlags;
  TCommandEnabled mCommandEnabled;
};
CHECK_SIZEOF(CScriptControlHint, 0x200)

#endif // _CSCRIPTCONTROLHINT
