#ifndef _WEAPONCOMMON
#define _WEAPONCOMMON

#include "Kyoto/SObjectTag.hpp"

#include "rstl/pair.hpp"
#include "rstl/set.hpp"
#include "rstl/vector.hpp"

class CToken;
class CPrimitive;
class CSfxHandle;
class CAnimData;
class CStateManager;
class CVector3f;

namespace NWeaponTypes {

CAssetId get_asset_id_from_name(const char* name);
void get_token_vector(CAnimData& animData, int animIdx, rstl::vector< CToken >& tokensOut,
                      bool preLock);
void get_token_vector(CAnimData& animData, const rstl::vector< int >& animIdxs,
                      rstl::vector< CToken >& tokensOut, bool preLock);
void primitive_set_to_token_vector(const rstl::set< CPrimitive >& primSet,
                                   rstl::vector< CToken >& tokensOut, bool preLock);
void lock_tokens(rstl::vector< CToken >& tokens);
void unlock_tokens(rstl::vector< CToken >& tokens);
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
