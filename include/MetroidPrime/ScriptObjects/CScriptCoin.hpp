#ifndef _CSCRIPTCOIN
#define _CSCRIPTCOIN

#include "types.h"

#include "MetroidPrime/CPhysicsActor.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/TReservedAverage.hpp"

#include "rstl/single_ptr.hpp"

class CElementGen;

// Guessed class: a bouncing, fading pickup spawned like debris (ScriptCoin REL).
class CScriptCoin : public CPhysicsActor {
public:
  enum EOrientationType { // Guessed names
    kOT_NotOriented,
    kOT_AlongVelocity,
    kOT_ToObject,
    kOT_AlongCollisionNormal,
  };

  CScriptCoin(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
              const CTransform4f& xf, const CModelData& model, const CActorParameters& params,
              float linConeAngle, float linMinMag, float linMaxMag, float angMinMag,
              float angMaxMag, float minDuration, float maxDuration, float disableCollisionTime,
              float colorInT, float colorOutT, const CColor& color, const CColor& endsColor,
              float scaleOutStartT, const CVector3f& scale, const CVector3f& endScale,
              float restitution, float downwardSpeed, const CVector3f& localOffset,
              TSfxId bounceSound, uchar maxBounceSounds, float bounceSoundSpeedThreshold,
              float bounceSoundVolumeDecay, CAssetId particle0, const CVector3f& particle0Scale,
              bool particle0GlobalTranslation, bool deferDeleteTillParticle0Done,
              EOrientationType particleOr0, CAssetId particle1, const CVector3f& particle1Scale,
              bool particle1GlobalTranslation, bool deferDeleteTillParticle1Done,
              EOrientationType particleOr1, CAssetId particle2, const CVector3f& particle2Scale,
              EOrientationType particleOr2, bool solid, bool dieOnProjectile, bool noBounce,
              bool constrainAngularImpulse, bool flickerOnFadeOut, float disablePhysicsThreshold,
              bool alternateStepData);

  // CEntity
  ~CScriptCoin() override;
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
  void SetSolid(bool solid); // Guessed name

  CVector3f mVelocity;                           // Guessed name
  CColor mColor;                                 // Guessed name
  CColor mEndsColor;                             // Guessed name
  float mZImpulse;                               // Guessed name
  float mCurTime;                                // Guessed name
  float mDuration;                               // Guessed name
  float mOoDuration;                             // Guessed name
  float mRestitution;                            // Guessed name
  uchar mScaleType;                              // Guessed name
  bool mRandomAngImpulse : 1;                    // Guessed name
  bool mParticle0GlobalTranslation : 1;          // Guessed name
  bool mDeferDeleteTillParticle0Done : 1;        // Guessed name
  bool mParticle1GlobalTranslation : 1;          // Guessed name
  bool mDeferDeleteTillParticle1Done : 1;        // Guessed name
  bool mParticle2Active : 1;                     // Guessed name
  bool mCoinExtended : 1;                        // Guessed name
  bool mDieOnProjectile : 1;                     // Guessed name
  bool mNoBounce : 1;                            // Guessed name
  bool mConstrainAngularImpulse : 1;             // Guessed name
  bool mSolid : 1;                               // Guessed name
  bool mFlickerOnFadeOut : 1;                    // Guessed name
  bool mAlternateStepData : 1;                   // Guessed name
  signed char mParticleOr0;                      // Guessed name
  signed char mParticleOr1;                      // Guessed name
  signed char mParticleOr2;                      // Guessed name
  TUniqueId mGeneratedObject;                    // Guessed name
  float mLinConeAngle;                           // Guessed name
  float mLinMinMag;                              // Guessed name
  float mLinMaxMag;                              // Guessed name
  float mAngMinMag;                              // Guessed name
  float mAngMaxMag;                              // Guessed name
  float mMinDuration;                            // Guessed name
  float mMaxDuration;                            // Guessed name
  float mDisableCollisionTime;                   // Guessed name
  float mColorInT;                               // Guessed name
  float mColorOutT;                              // Guessed name
  float mScaleOutStartT;                         // Guessed name
  float mDisablePhysicsThreshold;                // Guessed name
  CVector3f mScale;                              // Guessed name
  CVector3f mEndScale;                           // Guessed name
  CVector3f mCollisionNormal;                    // Guessed name
  rstl::single_ptr< CElementGen > mParticleGen0; // Guessed name
  rstl::single_ptr< CElementGen > mParticleGen1; // Guessed name
  rstl::single_ptr< CElementGen > mParticleGen2; // Guessed name
  TReservedAverage< float, 8 > mSpeedHistory;    // Guessed name
  TSfxId mBounceSound;                           // Guessed name
  uchar mMaxBounceSounds;                        // Guessed name
  uchar mBounceSoundCount;                       // Guessed name
  float mBounceSoundSpeedThreshold;              // Guessed name
  float mBounceSoundVolumeDecay;                 // Guessed name
  uchar mBounceSoundVolume;                      // Guessed name
  uint mUpdateFrameIndex;                        // Guessed name
};
CHECK_SIZEOF(CScriptCoin, 0x398)

#endif // _CSCRIPTCOIN
