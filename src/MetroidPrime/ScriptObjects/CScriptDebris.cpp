#include "MetroidPrime/ScriptObjects/CScriptDebris.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CScriptObjectLoaderHelper.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"

static CMaterialList skDebrisMaterials(kMT_Unknown59, kMT_Debris);

static float debris_frand(CStateManager& mgr) {
  return (1.f / 16383.5f) * static_cast< short >(mgr.Random()->Next() % 32767) - 1.f;
}

static float debris_frand_range(CStateManager& mgr, float min, float max) {
  return (max - min) * mgr.Random()->Float() + min;
}

static CVector3f debris_cone(CStateManager& mgr, float coneAngle, float minMag, float maxMag) {
  const float magnitude = debris_frand_range(mgr, minMag, maxMag);
  const float cosine = CMath::FastCosR(CRelAngle::FromDegrees(coneAngle * 0.5f).AsRadians());
  const float z = 1.f - (1.f - cosine) * mgr.Random()->Float();
  const float xy = magnitude * CMath::FastSqrtF(CMath::Max(0.f, 1.f - z * z));
  const float angle = M_2PIF * mgr.Random()->Float();
  return CVector3f(xy * CMath::FastCosR(angle), xy * CMath::FastSinR(angle), magnitude * z);
}

CScriptDebris::CScriptDebris(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, const CModelData& model,
                             const CActorParameters& params, CAssetId particleId,
                             const CVector3f& particleScale, float zImpulse,
                             const CVector3f& velocity, const CColor& endsColor, float mass,
                             float restitution, float duration, EScaleType scaleType, bool unused,
                             bool keepGeneratedObject, bool randomAngImpulse)
: CPhysicsActor(uid, name, info, 0, xf, model, skDebrisMaterials, model.GetBounds(xf.GetRotation()),
                SMoverData(mass), params, StepData(0.3f, 0.3f, 0))
, mVelocity(velocity)
, mColor(1.f, 0.5f, 0.5f, 1.f)
, mEndsColor(endsColor)
, mZImpulse(zImpulse)
, mCurTime(0.f)
, mDuration(duration >= 0.f ? duration : 0.5f)
, mOoDuration(1.f / mDuration)
, mRestitution(restitution)
, mScaleType(scaleType)
, mRandomAngImpulse(randomAngImpulse)
, mParticle0GlobalTranslation(false)
, mDeferDeleteTillParticle0Done(false)
, mParticle1GlobalTranslation(false)
, mDeferDeleteTillParticle1Done(false)
, mParticle2Active(false)
, mDebrisExtended(false)
, mDieOnProjectile(false)
, mNoBounce(false)
, mConstrainAngularImpulse(false)
, mSolid(true)
, mFlickerOnFadeOut(false)
, mAlternateStepData(false)
, mSentDead(false)
, mKeepGeneratedObject(keepGeneratedObject)
, mParticleOr0(kOT_NotOriented)
, mParticleOr1(kOT_NotOriented)
, mParticleOr2(kOT_NotOriented)
, mGeneratedObject(kInvalidUniqueId)
, mLinConeAngle(0.f)
, mMovementDirection(CVector3f::Up())
, mLinMinMag(0.f)
, mLinMaxMag(0.f)
, mAngMinMag(0.f)
, mAngMaxMag(0.f)
, mMinDuration(0.f)
, mMaxDuration(0.f)
, mDisableCollisionTime(0.f)
, mColorInT(0.f)
, mColorOutT(0.f)
, mScaleOutStartT(0.f)
, mDisablePhysicsThreshold(0.1f)
, mScale(model.GetScale())
, mEndScale(scaleType == kST_NoScale      ? model.GetScale()
            : scaleType == kST_EndsToZero ? CVector3f::Zero()
                                          : CVector3f(5.f, 5.f, 5.f))
, mCollisionNormal(CVector3f::Zero())
, mLocalOffset(CVector3f::Zero())
, mParticleGen0(nullptr)
, mParticleGen1(nullptr)
, mParticleGen2(nullptr)
, mSpeedHistory(2.f)
, mBounceSound(CSfxManager::kInternalInvalidSfxId)
, mBounceSoundCount(0)
, mBounceSoundSpeedThreshold(0.f)
, mBounceSoundVolumeDecay(1.f)
, mBounceSoundVolume(127)
, mUpdateFrameIndex(0) {
  SetDoTargetDistanceTest(false);
  if (HasActorLights()) {
    ActorLights()->SetFramesBetweenRecalculation(ActorLights()->GetFramesBetweenRecalculation() *
                                                 2);
  }
  SetUseInSortedLists(false);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Unknown59),
      CMaterialList(kMT_Debris, kMT_Character, kMT_Player, kMT_NoPlatformCollision)));

  if (gpResourceFactory->GetResourceTypeById(particleId) != 0) {
    TToken< CGenDescription > description = gpSimplePool->GetObj(SObjectTag('PART', particleId));
    mParticleGen0 =
        rs_new CElementGen(description, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    mParticleGen0->SetGlobalScale(particleScale);
  }

  SetMomentumWR(CVector3f(0.f, 0.f, -GravityConstant() * GetMass()));
  if (HasActorLights()) {
    ActorLights()->SetAmbienceGenerated(true);
  }
}

CScriptDebris::CScriptDebris(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CModelData& model, const CActorParameters& params, float linConeAngle,
    const CVector3f& movementDirection, float linMinMag, float linMaxMag, float angMinMag,
    float angMaxMag, float minDuration, float maxDuration, float disableCollisionTime,
    float colorInT, float colorOutT, const CColor& color, const CColor& endsColor,
    float scaleOutStartT, const CVector3f& scale, const CVector3f& endScale, float restitution,
    float downwardSpeed, const CVector3f& localOffset, TSfxId bounceSound, uchar maxBounceSounds,
    float bounceSoundSpeedThreshold, float bounceSoundVolumeDecay, CAssetId particle0,
    const CVector3f& particle0Scale, bool particle0GlobalTranslation,
    bool deferDeleteTillParticle0Done, EOrientationType particleOr0, CAssetId particle1,
    const CVector3f& particle1Scale, bool particle1GlobalTranslation,
    bool deferDeleteTillParticle1Done, EOrientationType particleOr1, CAssetId particle2,
    const CVector3f& particle2Scale, EOrientationType particleOr2, bool solid, bool dieOnProjectile,
    bool noBounce, bool constrainAngularImpulse, bool flickerOnFadeOut,
    float disablePhysicsThreshold, bool keepGeneratedObject, bool alternateStepData)
: CPhysicsActor(uid, name, info, 0, xf, model, skDebrisMaterials,
                model.IsNull() ? CAABox(-0.5f * scale, 0.5f * scale)
                               : model.GetBounds(xf.GetRotation()),
                SMoverData(1.f), params,
                alternateStepData ? StepData(0.3f, 0.1f, 1) : StepData(0.3f, 0.3f, 0))
, mVelocity(CVector3f::Zero())
, mColor(color)
, mEndsColor(endsColor)
, mZImpulse(0.f)
, mCurTime(0.f)
, mDuration(0.f)
, mOoDuration(0.f)
, mRestitution(restitution)
, mScaleType(kST_NoScale)
, mRandomAngImpulse(false)
, mParticle0GlobalTranslation(particle0GlobalTranslation)
, mDeferDeleteTillParticle0Done(deferDeleteTillParticle0Done)
, mParticle1GlobalTranslation(particle1GlobalTranslation)
, mDeferDeleteTillParticle1Done(deferDeleteTillParticle1Done)
, mParticle2Active(false)
, mDebrisExtended(true)
, mDieOnProjectile(dieOnProjectile)
, mNoBounce(noBounce)
, mConstrainAngularImpulse(constrainAngularImpulse)
, mSolid(solid)
, mFlickerOnFadeOut(flickerOnFadeOut)
, mAlternateStepData(alternateStepData)
, mKeepGeneratedObject(keepGeneratedObject)
, mParticleOr0(particleOr0)
, mParticleOr1(particleOr1)
, mParticleOr2(particleOr2)
, mGeneratedObject(kInvalidUniqueId)
, mLinConeAngle(linConeAngle)
, mMovementDirection(movementDirection)
, mLinMinMag(linMinMag)
, mLinMaxMag(linMaxMag)
, mAngMinMag(angMinMag)
, mAngMaxMag(angMaxMag)
, mMinDuration(minDuration)
, mMaxDuration(maxDuration)
, mDisableCollisionTime(disableCollisionTime)
, mColorInT(colorInT / 100.f)
, mColorOutT(colorOutT / 100.f)
, mScaleOutStartT(scaleOutStartT / 100.f)
, mDisablePhysicsThreshold(disablePhysicsThreshold)
, mScale(scale)
, mEndScale(CVector3f::ByElementMultiply(scale, endScale))
, mCollisionNormal(CVector3f::Zero())
, mLocalOffset(localOffset)
, mParticleGen0(nullptr)
, mParticleGen1(nullptr)
, mParticleGen2(nullptr)
, mSpeedHistory(2.f)
, mBounceSound(bounceSound)
, mMaxBounceSounds(maxBounceSounds)
, mBounceSoundCount(0)
, mBounceSoundSpeedThreshold(bounceSoundSpeedThreshold)
, mBounceSoundVolumeDecay(bounceSoundVolumeDecay)
, mBounceSoundVolume(127)
, mUpdateFrameIndex(0) {
  SetUseInSortedLists(false);
  SetSolid(solid && disableCollisionTime == 0.f);

  if (gpResourceFactory->GetResourceTypeById(particle0) != 0) {
    TToken< CGenDescription > description = gpSimplePool->GetObj(SObjectTag('PART', particle0));
    mParticleGen0 =
        rs_new CElementGen(description, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    mParticleGen0->SetGlobalScale(particle0Scale);
  }
  if (gpResourceFactory->GetResourceTypeById(particle1) != 0) {
    TToken< CGenDescription > description = gpSimplePool->GetObj(SObjectTag('PART', particle1));
    mParticleGen1 =
        rs_new CElementGen(description, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    mParticleGen1->SetGlobalScale(particle1Scale);
  }
  if (gpResourceFactory->GetResourceTypeById(particle2) != 0) {
    TToken< CGenDescription > description = gpSimplePool->GetObj(SObjectTag('PART', particle2));
    mParticleGen2 =
        rs_new CElementGen(description, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    mParticleGen2->SetGlobalScale(particle2Scale);
  }

  SetMomentumWR(CVector3f(0.f, 0.f, -downwardSpeed * GetMass()));
}

CScriptDebris::~CScriptDebris() {}

void CScriptDebris::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  mCurTime += dt;
  mUpdateFrameIndex = mgr.GetUpdateFrameIdx();
  bool done = mCurTime >= mDuration;

  if (!mParticleGen0.null()) {
    if (done) {
      mParticleGen0->SetParticleEmission(false);
    } else {
      if (mParticle0GlobalTranslation) {
        mParticleGen0->SetGlobalTranslation(GetTranslation());
      } else {
        mParticleGen0->SetTranslation(GetTranslation());
      }
      if (mParticleOr0 == kOT_AlongVelocity && GetVelocityWR().CanBeNormalized()) {
        const CVector3f velocity = GetVelocityWR().AsNormalized();
        const CVector3f up = CMath::AbsF(velocity.GetZ()) < 0.99f ? CVector3f(0.f, 0.f, 1.f)
                                                                  : CVector3f(0.f, 1.f, 0.f);
        mParticleGen0->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), velocity, up));
      } else if (mParticleOr0 == kOT_ToObject) {
        mParticleGen0->SetOrientation(GetTransform().GetRotation());
      }
    }
    if (mDeferDeleteTillParticle0Done && mParticleGen0->GetParticleCount() != 0) {
      done = false;
    }
    if (mCurTime < mDuration || mDeferDeleteTillParticle0Done) {
      mParticleGen0->Update(dt);
    }
  }

  if (!mParticleGen1.null()) {
    if (mCurTime >= mDuration) {
      mParticleGen1->SetParticleEmission(false);
    } else {
      if (mParticle1GlobalTranslation) {
        mParticleGen1->SetGlobalTranslation(GetTranslation());
      } else {
        mParticleGen1->SetTranslation(GetTranslation());
      }
      if (mParticleOr1 == kOT_AlongVelocity && GetVelocityWR().CanBeNormalized()) {
        const CVector3f velocity = GetVelocityWR().AsNormalized();
        const CVector3f up = CMath::AbsF(velocity.GetZ()) < 0.99f ? CVector3f(0.f, 0.f, 1.f)
                                                                  : CVector3f(0.f, 1.f, 0.f);
        mParticleGen1->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), velocity, up));
      } else if (mParticleOr1 == kOT_ToObject) {
        mParticleGen1->SetOrientation(GetTransform().GetRotation());
      }
    }
    if (mDeferDeleteTillParticle1Done && mParticleGen1->GetParticleCount() != 0) {
      done = false;
    }
    if (mCurTime < mDuration || mDeferDeleteTillParticle1Done) {
      mParticleGen1->Update(dt);
    }
  }

  if (!mParticleGen2.null()) {
    if (mCurTime >= mDuration && !mParticle2Active) {
      mParticleGen2->SetGlobalTranslation(GetTranslation());
      if (mParticleOr2 == kOT_AlongVelocity && GetVelocityWR().CanBeNormalized()) {
        const CVector3f velocity = GetVelocityWR().AsNormalized();
        const CVector3f up = CMath::AbsF(velocity.GetZ()) < 0.99f ? CVector3f(0.f, 0.f, 1.f)
                                                                  : CVector3f(0.f, 1.f, 0.f);
        mParticleGen2->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), velocity, up));
      } else if (mParticleOr2 == kOT_ToObject) {
        mParticleGen2->SetOrientation(GetTransform().GetRotation());
      } else if (mParticleOr2 == kOT_AlongCollisionNormal) {
        if (mCollisionNormal.MagSquared() == 0.f) {
          mCollisionNormal = CVector3f::Up();
        }
        const CVector3f up = CMath::AbsF(CVector3f::Dot(CVector3f::Up(), mCollisionNormal)) > 0.99f
                                 ? CVector3f(1.f, 0.f, 0.f)
                                 : CVector3f(0.f, 0.f, 1.f);
        mParticleGen2->SetOrientation(
            CTransform4f::LookAt(CVector3f::Zero(), mCollisionNormal, up));
      }
      mParticle2Active = true;
    }
    if (mParticle2Active) {
      mParticleGen2->Update(dt);
      if (!mParticleGen2->IsSystemDeletable()) {
        done = false;
      }
    }
  }

  if (HasModelData()) {
    const float t =
        mCurTime / mDuration > mScaleOutStartT
            ? (mCurTime - mDuration * mScaleOutStartT) / (mDuration * (1.f - mScaleOutStartT))
            : 0.f;
    ModelData()->SetScale(CVector3f::Lerp(mScale, mEndScale, t));
  }

  if (mCurTime >= mDuration) {
    SetMomentumWR(CVector3f::Zero());
    SetMaterialFilter(CMaterialFilter::MakeExclude(
        CMaterialList(kMT_Debris, kMT_Character, kMT_Player, kMT_Projectile)));
    if (!mSentDead && mDebrisExtended) {
      SendScriptMsgs(kSS_Dead, mgr, GetUniqueId(), kSM_None);
      mSentDead = true;
    }
    if (done) {
      mgr.DeleteObjectRequest(GetUniqueId());
      return;
    }
  }

  if (mGeneratedObject != kInvalidUniqueId && mgr.GetObjectById(mGeneratedObject) == nullptr) {
    mgr.DeleteObjectRequest(GetUniqueId());
    mGeneratedObject = kInvalidUniqueId;
    return;
  }

  if (GetMovable()) {
    mSpeedHistory.AddValue(GetVelocityWR().Magnitude());
    const rstl::optional_object< float > maxSpeed = mSpeedHistory.GetMax();
    if (maxSpeed && *maxSpeed < mDisablePhysicsThreshold) {
      DisablePhysics();
    }
  }

  if (mGeneratedObject != kInvalidUniqueId) {
    CActor* generatedActor = TCastToPtr< CActor >(mgr.ObjectById(mGeneratedObject));
    if (generatedActor) {
      generatedActor->SetTranslation(GetTranslation());
    } else {
      mGeneratedObject = kInvalidUniqueId;
    }
  }
  SetSolid(mSolid && mDisableCollisionTime < mCurTime);
}

void CScriptDebris::Touch(CActor& other, CStateManager& mgr) {
  if (mDieOnProjectile && TCastToPtr< CGameProjectile >(other)) {
    SendScriptMsgs(kSS_Dead, mgr, kInvalidUniqueId, kSM_None);
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

rstl::optional_object< CAABox > CScriptDebris::GetTouchBounds() const {
  if (mDieOnProjectile) {
    return GetBoundingBox();
  }
  return rstl::optional_object_null();
}

void CScriptDebris::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Delete:
    if (!mKeepGeneratedObject && mGeneratedObject != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mGeneratedObject);
      mGeneratedObject = kInvalidUniqueId;
    }
    break;
  case kSM_Unlock:
    mGeneratedObject = kInvalidUniqueId;
    mgr.DeleteObjectRequest(GetUniqueId());
    break;
  case kSM_Activate:
    if (!GetActive()) {
      if (!mDebrisExtended) {
        const float mass = GetMass();
        const float z = mass * mVelocity.GetZ() * CMath::AbsF(debris_frand(mgr)) + mZImpulse;
        const float y = mass * mVelocity.GetY() * debris_frand(mgr);
        const float x = mass * mVelocity.GetX() * debris_frand(mgr);
        const CVector3f impulse = GetTransform().GetColumn(kDZ) + CVector3f(x, y, z);

        CAxisAngle angularImpulse = CAxisAngle::Identity();
        if (mRandomAngImpulse && mgr.Random()->Next() % 100 < 50) {
          angularImpulse = CAxisAngle(CVector3f(45.f * debris_frand(mgr), 15.f * debris_frand(mgr),
                                                35.f * debris_frand(mgr)));
        }
        ApplyImpulseWR(impulse, angularImpulse);
      } else {
        SetTranslation(GetTransform() * mLocalOffset);
        const CVector3f impulse = debris_cone(mgr, mLinConeAngle, mLinMinMag, mLinMaxMag);
        const float angularCone = mConstrainAngularImpulse ? 0.f : 360.f;
        const CAxisAngle angularImpulse(debris_cone(mgr, angularCone, mAngMinMag, mAngMaxMag));
        const CQuaternion rotation =
            CQuaternion::ShortestRotationArc(CVector3f::Up(), mMovementDirection);
        ApplyImpulseOR(rotation.Transform(impulse), angularImpulse);
        mDuration = debris_frand_range(mgr, mMinDuration, mMaxDuration);
      }

      if (!mParticleGen0.null()) {
        mParticleGen0->SetParticleEmission(true);
      }
      if (!mParticleGen1.null()) {
        mParticleGen1->SetParticleEmission(true);
      }

      const rstl::vector< SConnection >& connections = GetConnectionList();
      for (int i = 0; i < connections.size(); ++i) {
        const SConnection& connection = connections[i];
        if (connection.state != kSS_Generate || connection.msg != kSM_Activate) {
          continue;
        }

        const CScriptObjectLoaderHelper::SGeneratedObject generated =
            mgr.ScriptObjectLoaderHelper().GenerateScriptObject(connection.objId, mgr);
        CActor* actor = TCastToPtr< CActor >(generated.mEntity);
        if (actor) {
          mGeneratedObject = generated.mUniqueId;
          actor->SetTranslation(GetTranslation());

          const CAABox& baseBounds = GetBaseBoundingBox();
          if (close_enough(baseBounds.GetMaxPoint() - baseBounds.GetMinPoint(), CVector3f::Zero(),
                           0.0001f)) {
            const CAABox bounds = actor->GetModelData()->GetBounds();
            SetCollisionPrimitive(CCollidableAABox(bounds, skDebrisMaterials));
            const rstl::optional_object< CAABox > touchBounds = actor->GetTouchBounds();
            SetBoundingBox(touchBounds ? *touchBounds : bounds);
          }

          mgr.SendScriptMsg(actor, GetUniqueId(), kSM_Activate, kInvalidUniqueId);
          break;
        }
        mgr.DeleteObjectRequest(generated.mUniqueId);
      }
    }
    break;
  case kSM_Landed:
    if (!mNoBounce) {
      ApplyImpulseWR(-mRestitution * GetConstantForceWR(), -mRestitution * GetAngularMomentumWR());
    }
    break;
  default:
    break;
  }

  CActor::AcceptScriptMsg(mgr, msg);
}

void CScriptDebris::PreRenderAllViewports(CStateManager& mgr) {
  CActor::PreRenderAllViewports(mgr);

  const float time = CMath::Min(mCurTime, mDuration);
  const float relativeTime = time / mDuration;
  float fade = 0.f;
  if (relativeTime < mColorInT) {
    if (mColorInT > 0.f) {
      fade = 1.f - time / (mDuration * mColorInT);
    }
  } else if (relativeTime > mColorOutT) {
    fade = (time - mDuration * mColorOutT) / (mDuration * (1.f - mColorOutT));
    if (mFlickerOnFadeOut && mUpdateFrameIndex % 6 > 2) {
      fade = 1.f;
    }
  }

  const CColor color = CColor::Lerp(CColor::White(), mEndsColor, fade);
  SetModelFlags(
      CModelFlags::AlphaBlended(color).DepthCompareUpdate(true, color.GetAlphau8() == 255));
}

void CScriptDebris::Render(const CStateManager& mgr) const { CPhysicsActor::Render(mgr); }

void CScriptDebris::AddToRenderer(const CStateManager& mgr) const {
  if (!mParticleGen0.null() && (mCurTime < mDuration || mDeferDeleteTillParticle0Done)) {
    gpRender->AddParticleGen(*mParticleGen0);
  }
  if (!mParticleGen1.null() && (mCurTime < mDuration || mDeferDeleteTillParticle1Done)) {
    gpRender->AddParticleGen(*mParticleGen1);
  }
  if (mParticle2Active) {
    gpRender->AddParticleGen(*mParticleGen2);
  }
  if (HasModelData() && mCurTime < mDuration) {
    CActor::AddToRenderer(mgr);
  }
}

void CScriptDebris::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                 CStateManager& mgr) {
  if (list.GetCount() == 0) {
    return;
  }

  if (mNoBounce) {
    if (mCurTime < mDuration && !mSentDead) {
      SendScriptMsgs(kSS_Dead, mgr, GetUniqueId(), kSM_None);
      mSentDead = true;
    }
    mDuration = mCurTime;
    SetVelocityWR(CVector3f::Zero());
  }

  mCollisionNormal = list[0].GetNormalLeft();
  if (GetVelocityWR().Magnitude() > mBounceSoundSpeedThreshold &&
      mBounceSound != CSfxManager::kInternalInvalidSfxId && mBounceSoundCount < mMaxBounceSounds) {
    CSfxManager::AddEmitter(mBounceSound, GetTranslation(), mBounceSoundVolume,
                            GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
    ++mBounceSoundCount;
    mBounceSoundVolume =
        static_cast< uchar >(CMath::Max(0.f, mBounceSoundVolumeDecay * mBounceSoundVolume));
  }
}

void CScriptDebris::SetSolid(bool solid) {
  if (solid) {
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        CMaterialList(kMT_Unknown59),
        CMaterialList(kMT_Debris, kMT_Character, kMT_Player, kMT_NoPlatformCollision)));
  } else {
    CMaterialList excluded(kMT_Debris, kMT_Character, kMT_Player, kMT_Projectile, kMT_Unknown59);
    excluded.Add(kMT_NoPlatformCollision);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(CMaterialList(), excluded));
  }
}

void CScriptDebris::DisablePhysics() { SetMovable(false); }
