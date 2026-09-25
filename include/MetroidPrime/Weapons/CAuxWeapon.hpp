#ifndef _CAUXWEAPON
#define _CAUXWEAPON

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CElementGen;
class CWeaponDescription;

// Dependency layout recovered from construction/destruction; behavior is a separate pass.
class CAuxWeapon {
public:
  explicit CAuxWeapon(TUniqueId playerId);
  ~CAuxWeapon();
  bool HasChargeCombo(int beam, CStateManager& mgr) const;
  void SetTargetId(TUniqueId target);
  TUniqueId GetTargetId() const;

private:
  TLockedToken< CWeaponDescription > mMissile;
  rstl::single_ptr< CElementGen > mMuzzleGenerator; // Prime-derived type; unused in observed ctor.
  rstl::reserved_vector< TCachedToken< CWeaponDescription >, 4 > mCombos;
  TUniqueId mPlayerId;
  CPlayerState::EBeamId mFiringBeamId;
  int mLoadBeamId;
  CSfxHandle mComboSfx;
  short mSoundVolume;
  bool mLoaded : 1;
};
CHECK_SIZEOF(CAuxWeapon, 0x58)

#endif // _CAUXWEAPON
