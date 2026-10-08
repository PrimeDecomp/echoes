#include "MetroidPrime/ScriptObjects/CScriptCoin.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CScriptObjectLoaderHelper.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrCoin.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "REL/REL_Setup.h"
#include "rstl/math.hpp"

static inline float coin_frand(CStateManager& mgr) {
  const short value = static_cast< short >(mgr.Random()->Next() % 32767);
  return (1.f / 16383.5f) * CCast::StoF(value) - 1.f;
}

static inline float coin_frand_range(CStateManager& mgr, float min, float max) {
  return (max - min) * mgr.Random()->Float() + min;
}

static CVector3f coin_cone(CStateManager& mgr, float coneAngle, float minMag, float maxMag) {
  const float mag = coin_frand_range(mgr, minMag, maxMag);
  const float cosAngle = CMath::FastCosR((M_PIF / 360.f) * coneAngle);
  const float side = 1.f - (1.f - cosAngle) * mgr.Random()->Float();
  const float sideSquared = side * side;
  const float hyp = mag * CMath::FastSqrtF(rstl::max_val(1.f - sideSquared, 0.f));
  const float angle = M_2PIF * mgr.Random()->Float();
  return CVector3f(hyp * CMath::FastCosR(angle), hyp * CMath::FastSinR(angle), mag * side);
}

CScriptCoin::CScriptCoin(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CModelData& model, const CActorParameters& params, float linConeAngle, float linMinMag,
    float linMaxMag, float angMinMag, float angMaxMag, float minDuration, float maxDuration,
    float disableCollisionTime, float colorInT, float colorOutT, const CColor& color,
    const CColor& endsColor, float scaleOutStartT, const CVector3f& scale,
    const CVector3f& endScale, float restitution, float downwardSpeed, const CVector3f& localOffset,
    TSfxId bounceSound, uchar maxBounceSounds, float bounceSoundSpeedThreshold,
    float bounceSoundVolumeDecay, CAssetId particle0, const CVector3f& particle0Scale,
    bool particle0GlobalTranslation, bool deferDeleteTillParticle0Done,
    EOrientationType particleOr0, CAssetId particle1, const CVector3f& particle1Scale,
    bool particle1GlobalTranslation, bool deferDeleteTillParticle1Done,
    EOrientationType particleOr1, CAssetId particle2, const CVector3f& particle2Scale,
    EOrientationType particleOr2, bool solid, bool dieOnProjectile, bool noBounce,
    bool constrainAngularImpulse, bool flickerOnFadeOut, float disablePhysicsThreshold,
    bool alternateStepData)
: CPhysicsActor(uid, name, info, 0, xf, model, CMaterialList(kMT_Solid, kMT_Debris),
                model.IsNull() ? CAABox(-0.5f * scale, 0.5f * scale)
                               : model.GetBounds(xf.GetRotation()),
                SMoverData(1.f), params,
                alternateStepData ? StepData(0.3f, 0.1f, 1) : CPhysicsActor::skDefaultStepData)
, mVelocity(CVector3f::Zero())
, mColor(color)
, mEndsColor(endsColor)
, mZImpulse(0.f)
, mCurTime(0.f)
, mDuration(0.f)
, mOoDuration(0.f)
, mRestitution(restitution)
, mScaleType(0)
, mRandomAngImpulse(false)
, mParticle0GlobalTranslation(particle0GlobalTranslation)
, mDeferDeleteTillParticle0Done(deferDeleteTillParticle0Done)
, mParticle1GlobalTranslation(particle1GlobalTranslation)
, mDeferDeleteTillParticle1Done(deferDeleteTillParticle1Done)
, mParticle2Active(false)
, mCoinExtended(true)
, mDieOnProjectile(dieOnProjectile)
, mNoBounce(noBounce)
, mConstrainAngularImpulse(constrainAngularImpulse)
, mSolid(solid)
, mFlickerOnFadeOut(flickerOnFadeOut)
, mAlternateStepData(alternateStepData)
, mParticleOr0(particleOr0)
, mParticleOr1(particleOr1)
, mParticleOr2(particleOr2)
, mGeneratedObject(kInvalidUniqueId)
, mLinConeAngle(linConeAngle)
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
  SetTranslation(GetTranslation() + GetTransform().Rotate(localOffset));
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

CScriptCoin::~CScriptCoin() {}

void CScriptCoin::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  mCurTime += dt;
  mUpdateFrameIndex = mgr.GetUpdateFrameIdx();
  bool done = mCurTime >= mDuration;

  if (!mParticleGen0.null()) {
    if (mCurTime >= mDuration) {
      mParticleGen0->SetParticleEmission(false);
    } else {
      if (mParticle0GlobalTranslation) {
        mParticleGen0->SetGlobalTranslation(GetTranslation());
      } else {
        mParticleGen0->SetTranslation(GetTranslation());
      }
      if (mParticleOr0 == kOT_AlongVelocity) {
        if (GetVelocityWR().CanBeNormalized()) {
          const CVector3f velocity = GetVelocityWR().AsNormalized();
          const CVector3f up = CMath::AbsF(velocity.GetZ()) < 0.99f ? CVector3f(0.f, 0.f, 1.f)
                                                                    : CVector3f(0.f, 1.f, 0.f);
          mParticleGen0->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), velocity, up));
        }
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
      if (mParticleOr1 == kOT_AlongVelocity) {
        if (GetVelocityWR().CanBeNormalized()) {
          const CVector3f velocity = GetVelocityWR().AsNormalized();
          const CVector3f up = CMath::AbsF(velocity.GetZ()) < 0.99f ? CVector3f(0.f, 0.f, 1.f)
                                                                    : CVector3f(0.f, 1.f, 0.f);
          mParticleGen1->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), velocity, up));
        }
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
      if (mParticleOr2 == kOT_AlongVelocity) {
        if (GetVelocityWR().CanBeNormalized()) {
          const CVector3f velocity = GetVelocityWR().AsNormalized();
          const CVector3f up = CMath::AbsF(velocity.GetZ()) < 0.99f ? CVector3f(0.f, 0.f, 1.f)
                                                                    : CVector3f(0.f, 1.f, 0.f);
          mParticleGen2->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), velocity, up));
        }
      } else if (mParticleOr2 == kOT_ToObject) {
        mParticleGen2->SetOrientation(GetTransform().GetRotation());
      } else if (mParticleOr2 == kOT_AlongCollisionNormal) {
        if (mCollisionNormal.MagSquared() == 0.f) {
          mCollisionNormal = CVector3f::Up();
        }
        const CTransform4f orientation = CTransform4f::LookAt(
            CVector3f::Zero(), mCollisionNormal,
            CMath::AbsF(CVector3f::Dot(CVector3f::Up(), mCollisionNormal)) > 0.99f
                ? CVector3f::Right()
                : CVector3f::Up());
        mParticleGen2->SetOrientation(orientation);
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
    if (*mSpeedHistory.GetAverage() < mDisablePhysicsThreshold) {
      SetMovable(false);
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

void CScriptCoin::Touch(CActor& other, CStateManager& mgr) {
  if (mDieOnProjectile && TCastToPtr< CGameProjectile >(other)) {
    SendScriptMsgs(kSS_Dead, mgr);
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

rstl::optional_object< CAABox > CScriptCoin::GetTouchBounds() const {
  if (mDieOnProjectile) {
    return GetBoundingBox();
  }
  return rstl::optional_object_null();
}

void CScriptCoin::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Activate:
    if (!GetActive()) {
      if (!mCoinExtended) {
        const float mass = GetMass();
        const float zRand = coin_frand(mgr);
        const float z = mass * mVelocity.GetZ() * CMath::AbsF(zRand) + mZImpulse;
        const float yRand = coin_frand(mgr);
        const float yScale = mass * mVelocity.GetY();
        const float y = yScale * yRand;
        const float xRand = coin_frand(mgr);
        const float xScale = mass * mVelocity.GetX();
        const float x = xScale * xRand;
        const CVector3f impulse = GetTransform().GetColumn(kDZ) + CVector3f(x, y, z);

        CAxisAngle angularImpulse = CAxisAngle::Identity();
        if (mRandomAngImpulse && mgr.Random()->Next() % 100 < 50) {
          angularImpulse = CAxisAngle(
              CVector3f(45.f * coin_frand(mgr), 15.f * coin_frand(mgr), 35.f * coin_frand(mgr)));
        }
        ApplyImpulseWR(impulse, angularImpulse);
      } else {
        const CVector3f impulse = coin_cone(mgr, mLinConeAngle, mLinMinMag, mLinMaxMag);
        const float angularCone = mConstrainAngularImpulse ? 0.f : 360.f;
        const CAxisAngle angularImpulse(coin_cone(mgr, angularCone, mAngMinMag, mAngMaxMag));
        ApplyImpulseOR(impulse, angularImpulse);
        mDuration = coin_frand_range(mgr, mMinDuration, mMaxDuration);
      }

      if (!mParticleGen0.null()) {
        mParticleGen0->SetParticleEmission(true);
      }
      if (!mParticleGen1.null()) {
        mParticleGen1->SetParticleEmission(true);
      }

      rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
      for (; it != GetConnectionList().end(); ++it) {
        const SConnection& connection = *it;
        if (connection.state != kSS_Generate || connection.msg != kSM_Activate) {
          continue;
        }

        const CScriptObjectLoaderHelper::SGeneratedObject generated =
            mgr.ScriptObjectLoaderHelper().GenerateScriptObject(connection.objId, mgr);
        CActor* actor = TCastToPtr< CActor >(generated.mEntity);
        if (actor) {
          mGeneratedObject = generated.mUniqueId;
          actor->SetTranslation(GetTranslation());
          mgr.SendScriptMsg(actor, GetUniqueId(), kSM_Activate);
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
  case kSM_Delete:
    mgr.DeleteObjectRequest(mGeneratedObject);
    mGeneratedObject = kInvalidUniqueId;
    break;
  case kSM_Unlock:
    if (msg.GetOriginator() == mGeneratedObject) {
      mGeneratedObject = kInvalidUniqueId;
    }
    break;
  default:
    break;
  }

  CActor::AcceptScriptMsg(mgr, msg);
}

void CScriptCoin::PreRenderAllViewports(CStateManager& mgr) {
  CActor::PreRenderAllViewports(mgr);

  const float time = rstl::min_val(mCurTime, mDuration);
  const float relativeTime = time / mDuration;
  float fade = 0.f;
  if (relativeTime < mColorInT) {
    if (mColorInT > 0.f) {
      fade = 1.f - time / (mDuration * mColorInT);
    }
  } else if (relativeTime > mColorOutT) {
    const bool flicker = mFlickerOnFadeOut ? mUpdateFrameIndex % 6 > 2 : false;
    if (flicker) {
      fade = 1.f;
    } else {
      fade = (time - mDuration * mColorOutT) / (mDuration * (1.f - mColorOutT));
    }
  }

  const CColor color = CColor::Lerp(CColor::White(), mEndsColor, fade);
  SetModelFlags(
      CModelFlags::AlphaBlended(color).DepthCompareUpdate(true, color.GetAlphau8() == 255));
}

void CScriptCoin::Render(const CStateManager& mgr) const { CPhysicsActor::Render(mgr); }

void CScriptCoin::AddToRenderer(const CStateManager& mgr) const {
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

void CScriptCoin::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                               CStateManager& mgr) {
  if (list.GetCount() == 0) {
    return;
  }

  if (mNoBounce) {
    if (mCurTime < mDuration) {
      SendScriptMsgs(kSS_Dead, mgr);
    }
    mDuration = mCurTime;
    SetVelocityWR(CVector3f::Zero());
  }

  mCollisionNormal = list[0].GetNormalLeft();
  if (GetVelocityWR().Magnitude() > mBounceSoundSpeedThreshold &&
      mBounceSound != CSfxManager::kInternalInvalidSfxId && mBounceSoundCount < mMaxBounceSounds) {
    const CVector3f position = GetTranslation();
    CSfxManager::AddEmitter(mBounceSound, position, mBounceSoundVolume, GetCurrentAreaId().Value(),
                            true, false, CSfxManager::kMedPriority);
    ++mBounceSoundCount;
    const float volume = mBounceSoundVolumeDecay * CCast::ToReal32(mBounceSoundVolume);
    mBounceSoundVolume = CCast::ToUint8(0.f < volume ? volume : 0.f);
  }
}

void CScriptCoin::SetSolid(bool solid) {
  if (solid) {
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        CMaterialList(kMT_Solid), CMaterialList(kMT_Debris, kMT_Character, kMT_Player)));
  } else {
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        CMaterialList(),
        CMaterialList(kMT_Debris, kMT_Character, kMT_Player, kMT_Projectile, kMT_Solid)));
  }
}

CEntity* REL_LoadCoin(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrCoin sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrCoin.inc"

  if (sldrThis.model != kInvalidAssetId &&
      gpResourceFactory->GetResourceTypeById(sldrThis.model) == 0) {
    return nullptr;
  }

  const CVector3f& scale = sldrThis.editorProperties.transform.scale;
  return rs_new CScriptCoin(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      sldrThis.model == kInvalidAssetId ? CModelData::CModelDataNull()
                                        : CModelData(CStaticRes(sldrThis.model, scale)),
      LdrToActorParameters(sldrThis.actorInformation), sldrThis.coneSpread, sldrThis.minimumSpeed,
      sldrThis.maximumSpeed, sldrThis.maximumSpinSpeed, sldrThis.maximumSpinSpeed,
      sldrThis.minimumLifeTime, sldrThis.maximumLifeTime, sldrThis.disableCollisionTime,
      sldrThis.fadeInEndPercentage, sldrThis.fadeOutStartPercentage, sldrThis.startColor,
      sldrThis.endColor, sldrThis.scaleStartPercentage, scale, sldrThis.finalScale,
      sldrThis.bounciness, sldrThis.gravity, sldrThis.positionOffset, sldrThis.bounceSound,
      sldrThis.maxBounceSounds, sldrThis.bounceSoundSpeedThreshold, sldrThis.bounceSoundVolumeDecay,
      sldrThis.particle1, sldrThis.particleSystem1Scale,
      sldrThis.particleSystem1UsesGlobalTranslation, sldrThis.particleSystem1WaitForParticlesToDie,
      static_cast< CScriptCoin::EOrientationType >(sldrThis.particleSystem1Orientation),
      sldrThis.particle2, sldrThis.particleSystem2Scale,
      sldrThis.particleSystem2UsesGlobalTranslation, sldrThis.particleSystem2WaitForParticlesToDie,
      static_cast< CScriptCoin::EOrientationType >(sldrThis.particleSystem2Orientation),
      sldrThis.deathParticle, sldrThis.deathParticleSystemScale,
      static_cast< CScriptCoin::EOrientationType >(sldrThis.deathParticleSystemOrientation),
      sldrThis.isCollider, sldrThis.isShootable, sldrThis.dieOnCollision,
      sldrThis.unknown_0xdcaa0f22, sldrThis.flickerOnFadeOut, sldrThis.disablePhysicsThreshold,
      sldrThis.unknown_0x723d42d6);
}

static void SetFuncPtrs() {
  static SCoin_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadCoin;
  SetSCoin_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSCoin_FuncPtrs(nullptr); }
