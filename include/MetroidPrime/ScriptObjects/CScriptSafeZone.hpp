#ifndef _CSCRIPTSAFEZONE
#define _CSCRIPTSAFEZONE

#include "MetroidPrime/CDarkWorldInfo.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTriggerOrientated.hpp"

// Scaffold for REL module 66 (ScriptSafeZone, G2ME01). The class name is a guess from the module,
// loader ('SAFE') and CSafeZoneManager export names. Evidence: the vtable at .data 0x10 uses the
// DOL TypesMatch for entity type 95 (0x8009B7BC) and inherits CScriptTriggerOrientated's
// GetTouchBounds/Touch/BoundsOverlap; the loader allocates 0x488 bytes; constructor at 0x4C78.
// The constructor takes about 38 parameters (three damage infos, echo parameters, shell settings)
// and is not declared until its signature is recovered. Method bodies are not ported.
class CScriptSafeZone : public CScriptTriggerOrientated {
public:
  // CEntity
  ~CScriptSafeZone() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;
  void SetActive(bool active) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;

  // CScriptTrigger
  void InhabitantAdded(CActor& actor, CStateManager& mgr) override;
  void InhabitantIdle(CActor& actor, CStateManager& mgr) override;
  void InhabitantExited(CActor& actor, CStateManager& mgr) override;
  bool ShouldSendScriptMsgs(CActor& actor, CStateManager& mgr) const override;

  // Two further virtuals follow BoundsOverlap in the vtable (fn_66_1C1C, a state test on the
  // dword at 0x3f0, and fn_66_2990); they are not declared until their purpose is established.

  // Shell appearance used while teleporting; WorldTeleporter reads the first entry.
  const CDarkWorldInfo& GetDarkWorldInfo() const { return mDarkWorldInfos[0]; }

private:
  float x200_;
  float x204_;
  float x208_;
  float x20c_;
  float x210_;
  uchar x214_[0x264 - 0x214];        // Floats and flags initialised by the constructor; unresolved.
  CDarkWorldInfo mDarkWorldInfos[3]; // 0x264, 0x2d4, 0x344
  uchar x3b4_[0x488 - 0x3b4];        // Unresolved: shell data, tokens and two list headers.
};
CHECK_SIZEOF(CScriptSafeZone, 0x488)

#endif // _CSCRIPTSAFEZONE
