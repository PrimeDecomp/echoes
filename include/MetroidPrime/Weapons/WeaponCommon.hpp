#ifndef _WEAPONCOMMON
#define _WEAPONCOMMON

#include "Kyoto/SObjectTag.hpp"

#include "rstl/set.hpp"
#include "rstl/vector.hpp"

class CToken;
class CSfxHandle;
class CAnimData;
class CStateManager;
class CPrimitive;

namespace NWeaponTypes {

enum EGunAnimType {
  kGAT_BasePosition,
  kGAT_Shoot,
  kGAT_ChargeUp,
  kGAT_ChargeLoop,
  kGAT_ChargeShoot,
  kGAT_FromMissile,
  kGAT_ToMissile,
  kGAT_MissileShoot,
  kGAT_MissileReload,
  kGAT_FromBeam,
  kGAT_ToBeam
};

} // namespace NWeaponTypes

#endif // _WEAPONCOMMON
