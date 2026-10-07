#include "MetroidPrime/Weapons/WeaponCommon.hpp"
#include "MetroidPrime/Weapons/WeaponSound.hpp"

#include "Kyoto/Animation/CPrimitive.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

namespace NWeaponTypes {

CAssetId get_asset_id_from_name(const char* name) {
  const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(name);
  if (!tag) {
    return kInvalidAssetId;
  }
  return tag->GetId();
}

void get_token_vector(CAnimData& animData, int animIdx, rstl::vector< CToken >& tokensOut,
                      bool preLock) {
  rstl::set< CPrimitive > prims;
  CAnimPlaybackParms parms(animIdx, -1, 1.f, true);
  animData.GetAnimationPrimitives(parms, prims);
  primitive_set_to_token_vector(prims, tokensOut, preLock);
}

void get_token_vector(CAnimData& animData, const rstl::vector< int >& animIdxs,
                      rstl::vector< CToken >& tokensOut, bool preLock) {
  rstl::set< CPrimitive > prims;
  for (rstl::vector< int >::const_iterator it = animIdxs.begin(); it != animIdxs.end(); ++it) {
    CAnimPlaybackParms parms(*it, -1, 1.f, true);
    animData.GetAnimationPrimitives(parms, prims);
  }
  primitive_set_to_token_vector(prims, tokensOut, preLock);
}

bool are_tokens_ready(const rstl::vector< CToken >& anims) {
  for (int i = anims.size(); i > 0; --i) {
    if (!anims[i - 1].IsLoaded()) {
      return false;
    }
  }
  return true;
}

void lock_tokens(rstl::vector< CToken >& anims) {
  for (rstl::vector< CToken >::iterator it = anims.begin(); it != anims.end(); ++it) {
    it->Lock();
  }
}

void unlock_tokens(rstl::vector< CToken >& anims) {
  for (rstl::vector< CToken >::iterator it = anims.begin(); it != anims.end(); ++it) {
    it->Unlock();
  }
}

void primitive_set_to_token_vector(const rstl::set< CPrimitive >& primSet,
                                   rstl::vector< CToken >& tokensOut, bool preLock) {
  tokensOut = rstl::vector< CToken >();
  tokensOut.reserve(primSet.size());

  for (rstl::set< CPrimitive >::const_iterator it = primSet.begin(); it != primSet.end(); ++it) {
    CToken token = gpSimplePool->GetObj(SObjectTag('ANIM', it->GetAnimResId()));
    if (preLock) {
      token.Lock();
    }
    tokensOut.push_back(token);
  }
}

void do_sound_event(rstl::pair< ushort, CSfxHandle >& sfxHandle, int& pitch, bool doPitchBend,
                    uint soundId, float weight, uint flags, float falloff, float maxDist,
                    uchar minVol, uchar maxVol, const CVector3f& posToCam, const CVector3f& pos,
                    int aid, short pan, CStateManager& mgr) {
  if (!(posToCam.MagSquared() < maxDist * maxDist)) {
    return;
  }

  const ushort useSfxId = ushort(soundId);
  const bool looping = (soundId & 0x80000000) != 0;
  const bool nonPositional = (soundId & 0x40000000) != 0;
  const bool useAcoustics = (flags & 0x80) == 0;
  uint useFlags = 0x1;
  if ((flags & 0x8) != 0) {
    useFlags |= 0x8;
  }

  CAudioSys::C3DEmitterParmData parms(maxDist, falloff, useFlags, maxVol, minVol);
  parms.mPos = pos;
  parms.mDir = CVector3f::Up();
  parms.mSfxId = useSfxId;

  if (mgr.Random()->Float() <= weight) {
    if (looping) {
      const CSfxHandle currentHandle = sfxHandle.second;
      const ushort currentId = sfxHandle.first;
      if (!currentHandle) {
        CSfxHandle hnd;
        if (nonPositional) {
          hnd = CSfxManager::SfxStart(useSfxId, 0x7f, pan, aid, useAcoustics, true);
        } else {
          hnd = CSfxManager::AddEmitter(parms, aid, useAcoustics, true);
        }
        if (hnd) {
          sfxHandle.first = useSfxId;
          sfxHandle.second = hnd;
          if (doPitchBend) {
            CSfxManager::PitchBend(hnd, pitch);
          }
        }
      } else {
        if (currentId == useSfxId) {
          CSfxManager::UpdateEmitter(currentHandle, parms.mPos, parms.mDir, maxVol);
        } else if ((flags & 0x4) != 0) {
          CSfxManager::RemoveEmitter(currentHandle);
          CSfxHandle hnd = CSfxManager::AddEmitter(parms, aid, useAcoustics, true);
          if (hnd) {
            sfxHandle.first = useSfxId;
            sfxHandle.second = hnd;
            if (doPitchBend) {
              CSfxManager::PitchBend(hnd, pitch);
            }
          }
        }
      }
    } else {
      CSfxHandle hnd;
      if (nonPositional) {
        hnd = CSfxManager::SfxStart(useSfxId, 0x7f, pan, aid, useAcoustics, false);
      } else {
        hnd = CSfxManager::AddEmitter(parms, aid, useAcoustics, false);
      }
      if (doPitchBend) {
        CSfxManager::PitchBend(hnd, pitch);
      }
    }
  }
}

} // namespace NWeaponTypes

CSfxHandle PlaySfxForPlayer(CPlayer* player, ushort sfx, short pan, int area, bool underwater,
                            bool looped) {
  CSfxHandle hnd = CSfxManager::SfxStart(sfx, 0x7f, pan, area, true, looped);
  CSfxManager::SfxSpan(hnd, 0);
  if (player && player->IsInSafeZone()) {
    CSfxManager::SetIgnoreAreaLowPass(hnd, true);
  }
  if (underwater) {
    CSfxManager::PitchBend(hnd, 0);
  }
  return hnd;
}

CSfxHandle AddEmitter(const CActor& actor, const ushort sfx, const bool useAcoustics, bool looped,
                      short priority, uchar maxVolume, uchar minVolume, float maxDistance,
                      float distanceCompensation) {
  CAudioSys::C3DEmitterParmData parms(maxDistance, distanceCompensation, 1, maxVolume, minVolume);
  parms.mPos = actor.GetTranslation();
  parms.mDir = CVector3f::Zero();
  parms.mSfxId = sfx;
  return CSfxManager::AddEmitter(parms, actor.GetCurrentAreaId().Value(), useAcoustics, looped,
                                 priority);
}
