#ifndef _CINGMINIPORTALATTACK
#define _CINGMINIPORTALATTACK

#include "types.h"

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPlasmaBeamInfo.hpp"

class CGenDescription;

// Original Wii export name; the class lives in the Ing REL (module 29), which is not ported. Only
// the constructors used by the IngSpaceJumpGuardian REL are declared, and the layout is not
// reconstructed.
class CIngMiniPortalInfo {
public:
  CIngMiniPortalInfo(TUniqueId target, float x4, float x8,
                     const TLockedToken< CGenDescription >& effect, ushort sound, float x1c,
                     float x20, const CDamageInfo& damage, const SLdrPlasmaBeamInfo& beamInfo);

private:
  TUniqueId mTarget; // Guessed name
  float x4_;
  float x8_;
  TLockedToken< CGenDescription > mEffect; // Guessed name
  ushort mSound;                           // Guessed name
  float x1c_;
  float x20_;
  CDamageInfo mDamage;          // Guessed name
  SLdrPlasmaBeamInfo mBeamInfo; // Guessed name
};

// Original Wii export name; the actor that is spawned by a mini portal attack.
class CIngMiniPortalAttack : public CActor {
public:
  CIngMiniPortalAttack(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CTransform4f& xf, TUniqueId owner, const CVector3f& scale,
                       const CIngMiniPortalInfo& portalInfo);

private:
  uchar mUnported[0x238 - sizeof(CActor)]; // Guessed layout; defined by the Ing REL.
};
CHECK_SIZEOF(CIngMiniPortalAttack, 0x238)

#endif // _CINGMINIPORTALATTACK
