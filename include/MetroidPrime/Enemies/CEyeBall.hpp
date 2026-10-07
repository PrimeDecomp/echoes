#ifndef _CEYEBALL
#define _CEYEBALL

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

// Echoes layout recovered from the EyeBall REL constructor (0x8B0 bytes). Names follow Prime's
// CEyeBall where the member exists there.
class CEyeBall : public CPatterned {
public:
  CEyeBall(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
           const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
           const CActorParameters& actParams, float attackDelay, float attackStartTime,
           CAssetId wpscId, const CDamageInfo& dInfo, CAssetId beamContactFxId,
           CAssetId beamPulseFxId, CAssetId beamTextureId, CAssetId beamGlowTextureId, uint anim0,
           uint anim1, uint anim2, uint anim3, uint beamSfx, bool attackDisabled,
           const CColor& laserInnerColor, const CColor& laserOuterColor,
           float maxAudibleDistance, float dropOff);
  ~CEyeBall() override;

  // CEntity
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;

  // CEyeBall
  virtual void Active(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void InActive(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Flinch(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Cover(CStateManager& mgr, EStateMsg msg, float dt);
  virtual bool CloseDelay(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldFire(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ScriptingTriggered(CStateManager& mgr, const CTriggerData& data) const;

private:
  void CreateBeam(CStateManager& mgr);
  void TurnLaserOff(CStateManager& mgr);
  void TurnLaserOn(CStateManager& mgr, const CTransform4f& xf);
  void UpdateCycleAnimation(float dt);

  float mAttackDelay;
  float mAttackStartTime;
  CBoneTracking mBoneTracking;
  CTransform4f mLaserLocatorXf; // Guessed name; refreshed from skEyeLocator in PreRender.
  CVector3f mTargetPosition;
  CProjectileInfo mProjectileInfo;
  CAssetId mBeamContactFxId;
  CAssetId mBeamPulseFxId;
  CAssetId mBeamTextureId;
  CAssetId mBeamGlowTextureId;
  TUniqueId mProjectileId;
  int mCurrentAnim;
  int mAnimIndices[4];
  ushort mBeamSfxId;
  CSfxHandle mBeamSfx;
  CColor mLaserInnerColor;
  CColor mLaserOuterColor;
  float mMaxAudibleDistance;
  float mDropOff;
  bool mCanAttack : 1;
  bool mPlayerInRange : 1;
  bool mAlert : 1;
  bool mAttackDisabled : 1;
  bool mFiringBeam : 1;

  static const char* const skEyeLocator;
};
CHECK_SIZEOF(CEyeBall, 0x8b0)

#endif // _CEYEBALL
