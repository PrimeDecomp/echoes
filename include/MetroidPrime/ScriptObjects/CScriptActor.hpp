#ifndef _CSCRIPTACTOR
#define _CSCRIPTACTOR

#include "Kyoto/Math/CPlane.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

class COBBTreeGroup;

class CScriptActor : public CPhysicsActor {
public:
  CScriptActor(TUniqueId uid, const rstl::string& name, CEntityInfo& info, const CTransform4f& xf,
               const CModelData& model, const CAABox& bounds, const CMaterialList& materials,
               float mass, float zMomentum, const CHealthInfo& health,
               const CDamageVulnerability& vulnerability, const CActorParameters& parameters,
               const SEchoParameters& echoParameters, bool looping, int shaderIdx, bool castsShadow,
               bool scaleAdvancementDelta, bool unused, float animationTimeVariation,
               CAssetId projectile, const CDamageInfo& projectileDamage, CAssetId collisionTree);

  // CEntity
  ~CScriptActor() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  CHealthInfo* HealthInfo() override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& point,
                                                         const CVector3f& normal,
                                                         const CWeaponMode& mode,
                                                         int attribs) const override;
  CAABox GetSortingBounds(const CStateManager& mgr) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  CTransform4f GetPrimitiveTransform() const override;

  bool CheckActorRenderOnly() const;
  void SetPortalPlane(const CPlane& plane); // Guessed name

  void SetDead(bool dead) { mDead = dead; }
  bool IsAnimating() const { return mAnimating; }
  void SetAnimating(const bool animating) { mAnimating = animating; }
  bool GetProcessmodelFlags() const { return mProcessModelFlags; }
  void SetProcessmodelFlags(const bool processModelFlags) {
    mProcessModelFlags = processModelFlags;
  }
  bool IsPlayerActor() const { return mIsPlayerActor; }
  void SetPlayerActor(const bool playerActor) { mIsPlayerActor = playerActor; }
  bool IsSkipRendering() const { return mSkipRendering; }
  void SetSkipRendering(const bool skipRendering) { mSkipRendering = skipRendering; }
  void SetRenderImmediately(const bool renderImmediately) { // Guessed name
    mRenderImmediately = renderImmediately;
  }

private:
  void FireProjectile(CStateManager& mgr, const rstl::string& locator); // Guessed name

protected:
  CHealthInfo mInitialHealth;
  CHealthInfo mCurrentHealth;
  CDamageVulnerability mDamageVulnerability;
  rstl::optional_object< CProjectileInfo > mProjectileInfo;
  rstl::optional_object< TLockedToken< const COBBTreeGroup > > mTreeGroupContainer;
  rstl::single_ptr< CCollisionPrimitive > mCollisionPrimitive;
  rstl::single_ptr< CPlane > mPortalPlane;
  float mFadeInTime;
  float mFadeOutTime;
  float mAnimationTimeVariation; // Guessed name; consumed once, on the next animated Think.
  int mShaderIdx;
  TUniqueId mTriggerId;
  bool mDead : 1;
  bool mAnimating : 1;
  bool mProcessModelFlags : 1;
  bool mScaleAdvancementDelta : 1;
  bool mIsPlayerActor : 1;
  bool mSkipRendering : 1;     // Guessed name
  bool mRenderImmediately : 1; // Guessed name
};
CHECK_SIZEOF(CScriptActor, 0x398)

#endif // _CSCRIPTACTOR
