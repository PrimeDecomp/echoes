#ifndef _CSCRIPTAREADAMAGE
#define _CSCRIPTAREADAMAGE

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CEntity.hpp"

#include "rstl/list.hpp"
#include "rstl/pair.hpp"

// Guessed class name, based on the ADMG loader and area-wide damage behavior.
class CScriptAreaDamage : public CEntity {
public:
  CScriptAreaDamage(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                    const CDamageInfo& damage, float pulseTime, float graceTime);

  // CEntity
  ~CScriptAreaDamage() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

private:
  typedef rstl::pair< TUniqueId, int > TExclusion;
  typedef rstl::pair< TUniqueId, float > TGraceTimer;

  float mPulseTime;
  CDamageInfo mDamage;
  rstl::list< TExclusion > mExcludedPlayers;
  rstl::list< TGraceTimer > mPlayerGraceTimers;
  float mGraceTime;
  float mPulseAccumulator;
};

CHECK_SIZEOF(CScriptAreaDamage, 0x7c)

#endif // _CSCRIPTAREADAMAGE
