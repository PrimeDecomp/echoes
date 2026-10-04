#ifndef _CSCRIPTSAFEZONE
#define _CSCRIPTSAFEZONE

#include "MetroidPrime/CDarkWorldInfo.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTriggerEllipsoid.hpp"

// Scaffold; evidence in Echoes research/CScriptSafeZone-CScriptTriggerEllipsoid-G2ME01.md.
class CScriptSafeZone : public CScriptTriggerEllipsoid {
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

  // CScriptSafeZone; reconstructed name/qualification for the native extra slot.
  virtual bool IsHurtful() const;
  // A following native extra slot remains unidentified.

  // Shell appearance used while teleporting; WorldTeleporter reads the first entry.
  const CDarkWorldInfo& GetDarkWorldInfo() const { return mDarkWorldInfos[0]; }

private:
  float x200_;
  float x204_;
  float x208_;
  float x20c_;
  float x210_;
  uchar x214_[0x264 - 0x214];
  CDarkWorldInfo mDarkWorldInfos[3]; // 0x264, 0x2d4, 0x344
  uchar x3b4_[0x488 - 0x3b4];
};
CHECK_SIZEOF(CScriptSafeZone, 0x488)

// Existing DOL forwarder into the loaded SafeZone REL callback.
void SafeZone_ApplyRenderEffect(CEntity& entity, CStateManager& mgr); // Guessed name.

#endif // _CSCRIPTSAFEZONE
