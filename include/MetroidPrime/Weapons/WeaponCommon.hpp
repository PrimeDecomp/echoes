#ifndef _WEAPONCOMMON
#define _WEAPONCOMMON

#include "Kyoto/SObjectTag.hpp"

#include "rstl/set.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

class CToken;
class CSfxHandle;
class CAnimData;
class CStateManager;
class CPrimitive;
class CVector3f;

namespace NWeaponTypes {

CAssetId get_asset_id_from_name(const char* name);
void lock_tokens(rstl::vector< CToken >& tokens);
bool are_tokens_ready(const rstl::vector< CToken >& tokens);
void do_sound_event(rstl::pair< ushort, CSfxHandle >& sound, int& pitch, bool doPitchBend,
                    uint soundId, float weight, uint flags, float falloff, float maxDistance,
                    uchar minVolume, uchar maxVolume, const CVector3f& posToCamera,
                    const CVector3f& pos, int areaId, short pan, CStateManager& mgr);

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
