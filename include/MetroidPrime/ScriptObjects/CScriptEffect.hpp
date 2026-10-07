#ifndef _CSCRIPTEFFECT
#define _CSCRIPTEFFECT

#include "MetroidPrime/CActor.hpp"

#include "Kyoto/Math/CGameSpline.hpp"
#include "Kyoto/Math/CGameSplineDesc.hpp"

class CParticleGen;

class CScriptEffect : public CActor {
public:
  // Original enum type from Wii; enumerator names are guessed. The two special
  // queues' positions in the renderer are not yet recovered.
  enum ERenderOrder { kRO_Normal, kRO_Queue1, kRO_Queue2 };

  CScriptEffect(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CVector3f& scale, CAssetId effectId,
                bool noTimerUnlessAreaOccluded, bool rebuildSystemsOnActivate, bool emitting,
                bool useRateInverseCamDist, float rateInverseCamDist, float rateInverseCamDistRate,
                float duration, float durationResetWhileVisible, bool useRateCamDistRange,
                float rateCamDistRangeMin, float rateCamDistRangeMax, float rateCamDistRangeFarRate,
                bool combatVisorVisible, bool darkVisorVisible, bool echoVisorVisible,
                const CLightParameters& lightParameters, bool dieWhenSystemsDone,
                const CGameSplineDesc& spline, bool useLocalTranslation,
                bool destroyParticlesOnDeactivate, bool orientToSpline, ERenderOrder renderOrder);

  // CEntity
  ~CScriptEffect() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;
  void SetActive(const bool active) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  CAABox GetSortingBounds(const CStateManager& mgr) const override;

  void SetGlobalTranslation(const CVector3f& translation);
  void SetGlobalScale(const CVector3f& scale);
  CVector3f GetGlobalScale() const;             // Guessed name.
  CToken GetDescription() const;                // Guessed name; returns an owning resource handle.
  bool IsEmitting() const { return mEmitting; } // Guessed name; independent of GetActive().
  static void ResetParticleCounts();

private:
  // Guessed names for Echoes-specific operations.
  void UpdateSpline(float dt);
  void CreateSystem(const CVector3f& scale, const CColor& color);
  void UpdateGeneratorRate(CStateManager& mgr);
  void UpdateModelLighting();
  bool IsSystemDeletable() const;

  static uint mNumParticlesDrawing;
  static uint mNumParticlesUpdating;

  rstl::single_ptr< CToken > mDescription;
  rstl::single_ptr< CParticleGen > mParticleSystem;
  TUniqueId mLightId;
  CAssetId mEffectId;
  float mRateInverseCamDist;
  float mRateInverseCamDistSq;
  float mRateInverseCamDistRate;
  float mRateCamDistRangeMin;
  float mRateCamDistRangeMax;
  float mRateCamDistRangeFarRate;
  float mRemTime;
  float mDuration;
  float mDurationResetWhileVisible;
  rstl::single_ptr< CActorLights > mEffectLights;
  TUniqueId mTriggerId;
  float mDestroyDelayTimer;
  CGameSpline mSpline;
  float mSplineTime;
  uint mEmitting : 1;
  uint mEnable : 1;
  uint mNoTimerUnlessAreaOccluded : 1;
  uint mRebuildSystemsOnActivate : 1;
  uint mUseRateInverseCamDist : 1;
  uint mCombatVisorVisible : 1;
  uint mDarkVisorVisible : 1;
  uint mEchoVisorVisible : 1;
  uint mAnyVisorVisible : 1;
  uint mUseRateCamDistRange : 1;
  uint mDieWhenSystemsDone : 1;
  uint mCanRender : 1;
  uint mLoopSpline : 1;
  uint mHasSpline : 1;
  uint mUseLocalTranslation : 1;
  uint mDestroyParticlesOnDeactivate : 1;
  uint mOrientToSpline : 1;
  uint mRenderOrder : 2;
};
CHECK_SIZEOF(CScriptEffect, 0x2d0)

#endif // _CSCRIPTEFFECT
