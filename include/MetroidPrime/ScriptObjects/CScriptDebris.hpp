#ifndef _CSCRIPTDEBRIS
#define _CSCRIPTDEBRIS

#include "MetroidPrime/CPhysicsActor.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/TReservedAverage.hpp"

#include "rstl/single_ptr.hpp"

class CElementGen;

class CScriptDebris : public CPhysicsActor {
public:
  enum EOrientationType {
    kOT_NotOriented,
    kOT_AlongVelocity,
    kOT_ToObject,
    kOT_AlongCollisionNormal,
  };

  enum EScaleType {
    kST_NoScale,
    kST_EndsToZero,
  };

  CScriptDebris(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& model, const CActorParameters& params,
                float linConeAngle, const CVector3f& movementDirection, float linMinMag,
                float linMaxMag, float angMinMag, float angMaxMag, float minDuration,
                float maxDuration, float disableCollisionTime, float colorInT, float colorOutT,
                const CColor& color, const CColor& endsColor, float scaleOutStartT,
                const CVector3f& scale, const CVector3f& endScale, float restitution,
                float downwardSpeed, const CVector3f& localOffset, TSfxId bounceSound,
                uchar maxBounceSounds, float bounceSoundSpeedThreshold,
                float bounceSoundVolumeDecay, CAssetId particle0, const CVector3f& particle0Scale,
                bool particle0GlobalTranslation, bool deferDeleteTillParticle0Done,
                EOrientationType particleOr0, CAssetId particle1, const CVector3f& particle1Scale,
                bool particle1GlobalTranslation, bool deferDeleteTillParticle1Done,
                EOrientationType particleOr1, CAssetId particle2, const CVector3f& particle2Scale,
                EOrientationType particleOr2, bool solid, bool dieOnProjectile, bool noBounce,
                bool constrainAngularImpulse, bool flickerOnFadeOut, float disablePhysicsThreshold,
                bool keepGeneratedObject, bool alternateStepData);

  CScriptDebris(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& model, const CActorParameters& params,
                CAssetId particleId, const CVector3f& particleScale, float zImpulse,
                const CVector3f& velocity, const CColor& endsColor, float mass, float restitution,
                float duration, EScaleType scaleType, bool unused, bool keepGeneratedObject,
                bool randomAngImpulse);

  // CEntity
  ~CScriptDebris() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& other, CStateManager& mgr) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

private:
  // Guessed name for the native helper that clears the physics movable flag.
  void DisablePhysics();
  void SetSolid(bool solid);

  CVector3f mVelocity;
  CColor mColor;
  CColor mEndsColor;
  float mZImpulse;
  float mCurTime;
  float mDuration;
  float mOoDuration;
  float mRestitution;
  uchar mScaleType;
  bool mRandomAngImpulse : 1;
  bool mParticle0GlobalTranslation : 1;
  bool mDeferDeleteTillParticle0Done : 1;
  bool mParticle1GlobalTranslation : 1;
  bool mDeferDeleteTillParticle1Done : 1;
  bool mParticle2Active : 1;
  bool mDebrisExtended : 1;
  bool mDieOnProjectile : 1;
  bool mNoBounce : 1;
  bool mConstrainAngularImpulse : 1;
  bool mSolid : 1;
  bool mFlickerOnFadeOut : 1;
  bool mAlternateStepData : 1;
  bool mSentDead : 1;
  bool mKeepGeneratedObject : 1;
  char mParticleOr0;
  char mParticleOr1;
  char mParticleOr2;
  TUniqueId mGeneratedObject;
  float mLinConeAngle;
  CVector3f mMovementDirection;
  float mLinMinMag;
  float mLinMaxMag;
  float mAngMinMag;
  float mAngMaxMag;
  float mMinDuration;
  float mMaxDuration;
  float mDisableCollisionTime;
  float mColorInT;
  float mColorOutT;
  float mScaleOutStartT;
  float mDisablePhysicsThreshold;
  CVector3f mScale;
  CVector3f mEndScale;
  CVector3f mCollisionNormal;
  CVector3f mLocalOffset;
  rstl::single_ptr< CElementGen > mParticleGen0;
  rstl::single_ptr< CElementGen > mParticleGen1;
  rstl::single_ptr< CElementGen > mParticleGen2;
  TReservedAverage< float, 8 > mSpeedHistory;
  TSfxId mBounceSound;
  uchar mMaxBounceSounds;
  uchar mBounceSoundCount;
  float mBounceSoundSpeedThreshold;
  float mBounceSoundVolumeDecay;
  uchar mBounceSoundVolume;
  uint mUpdateFrameIndex;
};
CHECK_SIZEOF(CScriptDebris, 0x3b0)

#endif // _CSCRIPTDEBRIS
