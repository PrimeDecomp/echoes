#ifndef _CPUFFER
#define _CPUFFER

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/reserved_vector.hpp"

class CGenDescription;

class CPuffer : public CPatterned {
public:
  CPuffer(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
          const CModelData& modelData, const CActorParameters& actorParameters,
          const CPatternedInfo& patternedInfo, float hoverSpeed, CAssetId cloudEffect,
          const CDamageInfo& cloudDamage, CAssetId cloudSteam, float cloudSteamAlpha,
          bool cloudInCombatOrScan, bool cloudInDark, bool cloudInEcho,
          const CDamageInfo& explosionDamage, ushort sfxId);

  // CEntity
  ~CPuffer() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

  void SetParticleEnabled(const int idx, const bool enabled) {
    mEnabledParticles = enabled ? mEnabledParticles | (1 << idx) : mEnabledParticles & ~(1 << idx);
  }
  const bool IsParticleEnabled(const int idx) const {
    return (mEnabledParticles & (1 << idx)) != 0;
  }

private:
  CVector3f mFace;
  TToken< CGenDescription > mCloudEffect;
  CDamageInfo mCloudDamage;
  bool mCloudInCombatOrScan : 1;
  bool mCloudInEcho : 1;
  bool mCloudInDark : 1;
  ushort mSfxId;
  CDamageInfo mExplosionDamage;
  float mCloudSteamAlpha;
  CAssetId mCloudSteam;
  CVector3f mMove;
  TUniqueId mLastDestObj;
  uint mEnabledParticles;
  rstl::reserved_vector< CSegId, 14 > mGasLocators;

  void UpdateJets(CStateManager& mgr);
};
CHECK_SIZEOF(CPuffer, 0x840)

#endif // _CPUFFER
