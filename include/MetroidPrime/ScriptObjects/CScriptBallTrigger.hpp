#ifndef _CSCRIPTBALLTRIGGER
#define _CSCRIPTBALLTRIGGER

#include "MetroidPrime/ScriptObjects/CScriptTriggerOrientated.hpp"

// Guessed name; Prime's ball trigger adapted to Echoes's oriented-trigger hierarchy.
class CScriptBallTrigger : public CScriptTriggerOrientated {
public:
  CScriptBallTrigger(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, const CVector3f& scale, const CDamageInfo& damage,
                     const CVector3f& forceField, uint flags, float attractionForce,
                     float attractionAngle, float attractionDistance,
                     const CVector3f& attractionDirection, bool noBallMovement);

  // CEntity
  ~CScriptBallTrigger() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CScriptTrigger
  void InhabitantAdded(CActor& actor, CStateManager& mgr) override;
  void InhabitantExited(CActor& actor, CStateManager& mgr) override;
  bool ShouldSendScriptMsgs(CActor& actor, CStateManager& mgr) const override;

private:
  float mAttractionForce;
  float mAttractionAngle;
  float mAttractionDistance;
  CVector3f mAttractionDirection;
  uint mCapturedPlayerIndex; // 0xffffffff when no player owns the trigger.
  bool mNoBallMovement : 1;
};
CHECK_SIZEOF(CScriptBallTrigger, 0x248)

#endif // _CSCRIPTBALLTRIGGER
