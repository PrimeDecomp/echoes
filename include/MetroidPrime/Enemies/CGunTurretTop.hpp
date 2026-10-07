#ifndef _CGUNTURRETTOP
#define _CGUNTURRETTOP

#include "types.h"

#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "Kyoto/Graphics/CColor.hpp"

class CGunTurretTop : public CPatterned {
public:
  // Guessed names; the gun head of a pirate/GF gun turret, powered by its CGunTurretBase.
  enum EState {
    kS_Sleep,
    kS_Patrol,
    kS_PowerUp,
    kS_Attack,
    kS_PowerDown,
  };

  CGunTurretTop(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& modelData,
                const CPatternedInfo& patternedInfo, const CActorParameters& actorParameters,
                float powerUpTime, float powerDownTime, CAssetId gfChargeEffect,
                CAssetId pirateChargeEffect, const CColor& lightColor, ushort powerUpSfx,
                ushort powerDownSfx);

  // CEntity
  ~CGunTurretTop() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;
  bool IsScanVisorSelfRender() const override { return true; }
  CAABox GetScanVisorRenderBounds(const CStateManager& mgr) const override;
  void ScanVisorRender(const CStateManager& mgr, const CTransform4f& xf,
                       const CModelFlags& flags) const override;

  void Sleep(CStateManager& mgr, EStateMsg msg, float dt);
  void PowerUp(CStateManager& mgr, EStateMsg msg, float dt);
  void Attack(CStateManager& mgr, EStateMsg msg, float dt);
  void PowerDown(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);

  bool ShouldAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;
  bool Attacked(CStateManager& mgr, const CTriggerData& data) const;
  bool PowerUpOver(CStateManager& mgr, const CTriggerData& data) const;
  bool PowerDownOver(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldPatrol(CStateManager& mgr, const CTriggerData& data) const;

  void SetChargeEffect(CStateManager& mgr, bool active, bool pirate);
  void Revive(CStateManager& mgr);
  void SetShouldAttack(bool attack) { mShouldAttack = attack; }
  void SetShouldPatrol(bool patrol) { mShouldPatrol = patrol; }
  void SetTarget(const CVector3f& target) { mTarget = target; }
  int GetState() const { return mState; }
  CAssetId GetChargeEffect(bool pirate) const { return pirate ? mPirateChargeEffect : mGFChargeEffect; }
  CAABox GetModelBounds() const;

private:
  void PlaySfx(const ushort& sfx, CStateManager& mgr) {
    ProcessSoundEvent(sfx, 1.f, 0, mSfxFallOff, mSfxMaxDistance, CSegId(0), 0, 0, 0.f, 20, 127,
                      GetClosestCameraDistanceSq(mgr), GetTranslation(),
                      mgr.GetNextAreaId().Value(), mgr, true);
  }
  void PlayLoopedSfx(ushort sfx, CStateManager& mgr) {
    ProcessSoundEvent(sfx | 0x80000000, 1.f, 0, mSfxFallOff, mSfxMaxDistance, CSegId(0), 0, 0, 0.f, 20,
                      127, GetClosestCameraDistanceSq(mgr), GetTranslation(),
                      mgr.GetNextAreaId().Value(), mgr, true);
  }
  float GetClosestCameraDistanceSq(CStateManager& mgr) const;

  TUniqueId mBaseId;
  int mState;
  CHealthInfo mHealthInfo;
  bool mShouldAttack;
  float mPowerUpTime;
  float mPowerDownTime;
  float mStateTime;
  int mAdditiveAnim;
  CVector3f mTarget;
  bool mShouldPatrol;
  bool x809_;
  CAssetId mGFChargeEffect;
  CAssetId mPirateChargeEffect;
  TUniqueId mLightId;
  CColor mLightColor;
  ushort mPowerUpSfx;
  ushort mPowerDownSfx;
  float mSfxFallOff;
  float mSfxMaxDistance;
};
CHECK_SIZEOF(CGunTurretTop, 0x828)

#endif // _CGUNTURRETTOP
