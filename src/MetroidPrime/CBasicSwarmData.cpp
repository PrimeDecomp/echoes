#include "MetroidPrime/CBasicSwarmData.hpp"

CBasicSwarmData::CBasicSwarmData(const CDamageInfo& damage, const CHealthInfo& health,
                               const CDamageVulnerability& vulnerability,
                               CAssetId deathParticleEffect)
: mContactDamage(damage)
, mHealth(health)
, mDamageVulnerability(vulnerability)
, mDeathParticleEffect(deathParticleEffect) {}
