#include "Kyoto/Particles/CElementGen.hpp"

#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Particles/CParticleSpawnSystem.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

#include "alloca.h"

#include "dolphin/gx.h"

// Guessed names for the TU-local scale default and initialization guard.
static const CVector3f skOneVector(1.f, 1.f, 1.f);
static bool sStaticListInitialized;
static const double skTickTime = 1.0 / 60.0;

ushort CElementGen::sSeed = 99;
int CElementGen::sParticleAliveCount;
int CElementGen::sParticleSystemAliveCount;
bool CElementGen::sSubtractBlend;
bool CElementGen::sEnableAlphaModulation;
bool CElementGen::sMoveRedToAlphaBuffer;

// Guessed name, correlated with Prime's back-to-front particle ordering.
struct CParticleListItemViewPointComp {
  bool operator()(const CElementGen::CParticleListItem& a,
                  const CElementGen::CParticleListItem& b) const {
    return a.mViewPoint.GetY() > b.mViewPoint.GetY();
  }
};

CElementGen::CElementGen(TToken< CGenDescription > description, EModelOrientationType orientation,
                         EOptionalSystemFlags flags)
: mGenDesc(description)
, mLoadedGenDesc(*mGenDesc)
, mOrientType(orientation)
, mInternalStartFrame(0)
, mCurFrame(0)
, mCurSeconds(0.0)
, mPrevFrame(-1)
, mParticleEmission(true)
, mGeneratorRemainder(0.f)
, mMAXP(0)
, mRandomSeed(sSeed)
, mGeneratorRate(1.f)
, mTranslation(CVector3f::Zero())
, mGlobalTranslation(CVector3f::Zero())
, mPOFS(CVector3f::Zero())
, mGlobalScale(skOneVector)
, mGlobalScaleTransform(CTransform4f::Identity())
, mGlobalScaleTransformInverse(CTransform4f::Identity())
, mLocalScale(skOneVector)
, mLocalScaleTransform(CTransform4f::Identity())
, mLocalScaleTransformInverse(CTransform4f::Identity())
, mOrientation(CTransform4f::Identity())
, mOrientationInverse(CMatrix3f::Identity())
, mGlobalOrientation(CTransform4f::Identity())
, mActiveParticleCount(0)
, mCumulativeParticles(0)
, mRecursiveParticleCount(0)
, mTranslationDirty(false)
, mZTest(true)
, mORNT(false)
, mMBLR(false)
, mWarmedUp(false)
, mModelsUseLights(false)
, mEnableOPTS(flags & kOSF_Two)
, mEnableDynamicBounds((flags & kOSF_DisableBounds) == 0)
, mEnableFixedBounds(false)
, mEnableADV(false)
, mMBSP(1)
, mBackupLightActive(GX_LIGHT_NULL)
, mRandState(mRandomSeed)
, mSSPO(CVector3f::Zero())
, mSEPO(CVector3f::Zero())
, mUpdateTime(0.f)
, mRenderTime(0.f)
, mAabbMin(CVector3f::Zero())
, mAabbMax(CVector3f::Zero())
, mMaxSize(0.f)
, mSystemBounds(CAABox::MakeMaxInvertedBox())
, mLightType(kLT_None)
, mLCLR(CColor::White())
, mLINT(1.f)
, mLOFF(CVector3f::Zero())
, mLDIR(1.f, 0.f, 0.f)
, mFalloffType(kFT_Linear)
, mLFOR(1.f)
, mLSLA(45.f)
, mModuColor(0xffffffff) {
  SetDrawFlags(mLoadedGenDesc->mDFLG);
  CGlobalRandom random(mRandState);
  CParticleGlobals::SParticleSystem system('PART', this);
  if (mLoadedGenDesc->mSEED) {
    int seed;
    mLoadedGenDesc->mSEED->GetValue(mCurFrame, seed);
    mRandomSeed = seed;
  }
  mRandState.SetSeed(mRandomSeed);
  ++sParticleSystemAliveCount;

  mLIT_ = mLoadedGenDesc->mLIT_;
  mAAPH = mLoadedGenDesc->mAAPH;
  mZBUF = mLoadedGenDesc->mZBUF;
  mORNT = mLoadedGenDesc->mORNT;
  mMBLR = mORNT ? false : mLoadedGenDesc->mMBLR;
  if (mLoadedGenDesc->mMBSP && mMBLR) {
    mLoadedGenDesc->mMBSP->GetValue(mCurFrame, mMBSP);
  }

  int velocitySourceCount = 0;
  if (mLoadedGenDesc->mVEL1) {
    mVELSources[velocitySourceCount] = mLoadedGenDesc->mVEL1;
    mHasVMD[velocitySourceCount++] = mLoadedGenDesc->mVMD1;
  }
  if (mLoadedGenDesc->mVEL2) {
    mVELSources[velocitySourceCount] = mLoadedGenDesc->mVEL2;
    mHasVMD[velocitySourceCount++] = mLoadedGenDesc->mVMD2;
  }
  if (mLoadedGenDesc->mVEL3) {
    mVELSources[velocitySourceCount] = mLoadedGenDesc->mVEL3;
    mHasVMD[velocitySourceCount++] = mLoadedGenDesc->mVMD3;
  }
  if (mLoadedGenDesc->mVEL4) {
    mVELSources[velocitySourceCount] = mLoadedGenDesc->mVEL4;
    mHasVMD[velocitySourceCount++] = mLoadedGenDesc->mVMD4;
  }
  for (int i = velocitySourceCount; i < 4; ++i) {
    mVELSources[i] = nullptr;
  }
  if (mLoadedGenDesc->mADV1 || mLoadedGenDesc->mADV2 || mLoadedGenDesc->mADV3 ||
      mLoadedGenDesc->mADV4 || mLoadedGenDesc->mADV5 || mLoadedGenDesc->mADV6 ||
      mLoadedGenDesc->mADV7 || mLoadedGenDesc->mADV8 || mLoadedGenDesc->mADV9 ||
      mLoadedGenDesc->mVAV1 || mLoadedGenDesc->mVAV2 || mLoadedGenDesc->mVAV3) {
    mEnableADV = true;
  }
  for (int i = 0; i < 16; ++i) {
    mExternalVars[i] = 0.f;
  }

  mCSSD = 0;
  if (mLoadedGenDesc->mCSSD) {
    mLoadedGenDesc->mCSSD->GetValue(0, mCSSD);
  }
  mPISY = 16;
  if (mLoadedGenDesc->mPISY) {
    mLoadedGenDesc->mPISY->GetValue(0, mPISY);
    if (mPISY <= 0) {
      mPISY = 1;
    }
  }
  mSISY = 16;
  if (mLoadedGenDesc->mSISY) {
    mLoadedGenDesc->mSISY->GetValue(0, mSISY);
  }
  mSSSD = 0;
  if (mLoadedGenDesc->mSSSD) {
    mLoadedGenDesc->mSSSD->GetValue(0, mSSSD);
  }
  if (mLoadedGenDesc->mSSPO) {
    mLoadedGenDesc->mSSPO->GetValue(0, mSSPO);
    if (!mLoadedGenDesc->mSSPO->IsFastConstant()) {
      mTranslationDirty = true;
    }
  }
  mSESD = 0;
  if (mLoadedGenDesc->mSESD) {
    mLoadedGenDesc->mSESD->GetValue(0, mSESD);
  }
  if (mLoadedGenDesc->mSEPO) {
    mLoadedGenDesc->mSEPO->GetValue(0, mSEPO);
    if (!mLoadedGenDesc->mSEPO->IsFastConstant()) {
      mTranslationDirty = true;
    }
  }
  if (mLoadedGenDesc->mPOFS) {
    mLoadedGenDesc->mPOFS->GetValue(mCurFrame, mPOFS);
    if (!mLoadedGenDesc->mPOFS->IsFastConstant()) {
      mTranslationDirty = true;
    }
  }
  if (mLoadedGenDesc->mPSLT) {
    mLoadedGenDesc->mPSLT->GetValue(0, mPSLT);
  } else {
    mPSLT = 0x7fffff;
  }
  if (mLoadedGenDesc->mMAXP) {
    mLoadedGenDesc->mMAXP->GetValue(mCurFrame, mMAXP);
  }
  const int initialCapacity = rstl::min_val(mMAXP, 256);
  mParticles.reserve(initialCapacity);
  if (mEnableADV) {
    mAdvValues.assign(initialCapacity);
  }
  if (mOrientType == kMOT_One) {
    mParentMatrices.assign(mMAXP, CMatrix3f::Identity());
  }
  mLINE = mLoadedGenDesc->mLINE;
  mFXLL = mLoadedGenDesc->mFXLL;

  if (mLoadedGenDesc->mLTYP) {
    int lightType = 0;
    mLoadedGenDesc->mLTYP->GetValue(mCurFrame, lightType);
    switch (LightType(lightType)) {
    case kLT_None:
      mLightType = kLT_None;
      break;
    case kLT_Custom:
      mLightType = kLT_Custom;
      break;
    case kLT_Directional:
      mLightType = kLT_Directional;
      break;
    case kLT_Spot:
      mLightType = kLT_Spot;
      break;
    default:
      mLightType = kLT_None;
      break;
    }
    if (mLoadedGenDesc->mLFOT) {
      int falloff = 1;
      mLoadedGenDesc->mLFOT->GetValue(mCurFrame, falloff);
      switch (EFalloffType(falloff)) {
      case kFT_Constant:
        mFalloffType = kFT_Constant;
        break;
      case kFT_Linear:
        mFalloffType = kFT_Linear;
        break;
      case kFT_Quadratic:
        mFalloffType = kFT_Quadratic;
        break;
      default:
        mFalloffType = kFT_Linear;
        break;
      }
    }
    if (mLightType != kLT_None) {
      UpdateLightParameters();
    }
  }
  if (mEnableDynamicBounds) {
    mEnableFixedBounds = mLoadedGenDesc->mFXBR != nullptr;
    if (mEnableFixedBounds) {
      mEnableDynamicBounds = false;
    }
  }
}

CElementGen::~CElementGen() {
  --sParticleSystemAliveCount;
  sParticleAliveCount -= mParticles.size();
  for (int i = 0; i < mActivePartChildren.size(); ++i) {
    delete mActivePartChildren[i];
  }
}

void CElementGen::Initialize() {
  if (sStaticListInitialized) {
    return;
  }
  sParticleAliveCount = 0;
  sParticleSystemAliveCount = 0;
  sStaticListInitialized = true;
}

void CElementGen::ShutDown() { sStaticListInitialized = false; }

void CElementGen::SetTranslation(const CVector3f& translation) {
  mTranslation = translation;
  for (int i = 0; i < mActivePartChildren.size(); ++i) {
    CParticleGen* child = mActivePartChildren[i];
    switch (child->Get4CharId()) {
    case 'ELSC':
      child->SetTranslation(translation + mSEPO);
      break;
    case 'SWHC':
      child->SetTranslation(translation + mSSPO);
      // The original also applies the unoffset translation to swooshes.
    default:
      child->SetTranslation(translation);
      break;
    }
  }
}

void CElementGen::SetGlobalTranslation(const CVector3f& translation) {
  mGlobalTranslation = translation;
  for (int i = 0; i < mActivePartChildren.size(); ++i) {
    mActivePartChildren[i]->SetGlobalTranslation(translation);
  }
}

void CElementGen::SetModulationColor(const CColor& color) {
  mModuColor = color;
  for (int i = 0; i < mActivePartChildren.size(); ++i) {
    mActivePartChildren[i]->SetModulationColor(color);
  }
}

const CColor& CElementGen::GetModulationColor() const { return mModuColor; }

void CElementGen::SetGlobalScale(const CVector3f& scale) {
  mGlobalScale = scale;
  for (int i = 0; i < 3; ++i) {
    if (close_enough(mGlobalScale[i], 0.f, 0.0001f)) {
      mGlobalScale[i] = 0.0001f * (mGlobalScale[i] < 0.f ? -1.f : 1.f);
    }
  }
  mGlobalScaleTransform = CTransform4f::Scale(mGlobalScale);
  mGlobalScaleTransformInverse = CTransform4f::Scale(
      1.f / mGlobalScale.GetX(), 1.f / mGlobalScale.GetY(), 1.f / mGlobalScale.GetZ());
  for (int i = 0; i < mActivePartChildren.size(); ++i) {
    mActivePartChildren[i]->SetGlobalScale(scale);
  }
}

void CElementGen::SetLocalScale(const CVector3f& scale) {
  mLocalScale = scale;
  for (int i = 0; i < 3; ++i) {
    if (close_enough(mLocalScale[i], 0.f, 0.0001f)) {
      mLocalScale[i] = 0.0001f * (mLocalScale[i] < 0.f ? -1.f : 1.f);
    }
  }
  mLocalScaleTransform = CTransform4f::Scale(mLocalScale);
  mLocalScaleTransformInverse = CTransform4f::Scale(
      1.f / mLocalScale.GetX(), 1.f / mLocalScale.GetY(), 1.f / mLocalScale.GetZ());
  for (int i = 0; i < mActivePartChildren.size(); ++i) {
    mActivePartChildren[i]->SetLocalScale(scale);
  }
}

void CElementGen::SetOrientation(const CTransform4f& orientation) {
  mOrientation = orientation;
  mOrientationInverse = mOrientation.GetQuickInverse().BuildMatrix3f();
  for (int i = 0; i < mActivePartChildren.size(); ++i) {
    mActivePartChildren[i]->SetOrientation(orientation);
  }
}

void CElementGen::SetGlobalOrientation(const CTransform4f& orientation) {
  mGlobalOrientation.SetRotation(orientation);
  for (int i = 0; i < mActivePartChildren.size(); ++i) {
    mActivePartChildren[i]->SetGlobalOrientation(mGlobalOrientation);
  }
}

void CElementGen::SetGlobalOrientAndTrans(const CTransform4f& transform) {
  SetGlobalOrientation(transform);
  SetGlobalTranslation(transform.GetTranslation());
}

const bool CElementGen::Update(double dt) {
  CParticleGlobals::SParticleSystem system('PART', this);
  if (mLoadedGenDesc->mPSWT && !mWarmedUp) {
    int warmUpFrames = 0;
    mLoadedGenDesc->mPSWT->GetValue(mCurFrame, warmUpFrames);
    InternalUpdate(skTickTime * warmUpFrames);
    mWarmedUp = true;
  }
  return InternalUpdate(dt);
}

bool CElementGen::InternalUpdate(double dt) {
  CStopwatch stopwatch;
  CGlobalRandom random(mRandState);
  int frameUpdateCount = 0;
  double time = mCurFrame * skTickTime;
  const double tolerance = skTickTime / 1000.0;
  double scaledDt = close_enough(dt, skTickTime, tolerance) ? skTickTime : dt;
  CParticleGlobals::SetEmitterTime(mCurFrame);
  if (mLoadedGenDesc->mPSTS) {
    float timeScale = 1.f;
    mLoadedGenDesc->mPSTS->GetValue(mCurFrame, timeScale);
    scaledDt = rstl::max_val(0.0, scaledDt * timeScale);
  }
  mCurSeconds += scaledDt;
  if (mMBLR && dt > 0.0 && mLoadedGenDesc->mMBSP) {
    mLoadedGenDesc->mMBSP->GetValue(mCurFrame, mMBSP);
  }

  while (time < mCurSeconds && !close_enough(time, mCurSeconds, tolerance)) {
    mAabbMin = CVector3f(FLT_MAX, FLT_MAX, FLT_MAX);
    mAabbMax = CVector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    mMaxSize = 0.f;
    CParticleGlobals::SetEmitterTime(mCurFrame);
    UpdateExistingParticles();
    CParticleGlobals::SetParticleLifetime(mPSLT);
    if (mCurFrame < mPSLT && mParticleEmission) {
      float generationRate = 0.f;
      if (mLoadedGenDesc->mGRTE->GetValue(mCurFrame, generationRate)) {
        mParticles.clear();
        return true;
      }
      generationRate = rstl::max_val(0.f, generationRate * mGeneratorRate);
      mGeneratorRemainder += generationRate;
      const int count = static_cast< int >(floor(mGeneratorRemainder));
      mGeneratorRemainder -= static_cast< float >(count);
      if (mLoadedGenDesc->mMAXP) {
        mLoadedGenDesc->mMAXP->GetValue(mCurFrame, mMAXP);
      }
      CreateNewParticles(count);
    }
    if (mTranslationDirty) {
      UpdatePSTranslationAndOrientation();
    }
    if (mLightType != kLT_None) {
      UpdateLightParameters();
    }
    UpdateChildParticleSystems(skTickTime);
    ++frameUpdateCount;
    time += skTickTime;
    ++mCurFrame;
  }

  if (close_enough(time, mCurSeconds, tolerance)) {
    mCurSeconds = time;
    mTimeDeltaScale = 1.f;
  } else {
    UpdateChildParticleSystems(scaledDt - frameUpdateCount * skTickTime);
    mTimeDeltaScale = 1.f - static_cast< float >((time - mCurSeconds) / skTickTime);
  }
  if (mEnableFixedBounds) {
    float radius = 0.f;
    if (mLoadedGenDesc->mFXBR) {
      mLoadedGenDesc->mFXBR->GetValue(mCurFrame, radius);
    }
    CVector3f offset = CVector3f::Zero();
    if (mLoadedGenDesc->mFXBO) {
      mLoadedGenDesc->mFXBO->GetValue(mCurFrame, offset);
    }
    offset = mOrientation.Rotate(offset) +
             (mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mTranslation + mPOFS;
    AccumulateBounds(offset, 2.f * radius);
  }
  BuildParticleSystemBounds();
  mUpdateTime = stopwatch.GetElapsedTime();
  return false;
}

void CElementGen::UpdateLightParameters() {
  mLoadedGenDesc->mLCLR->GetValue(mCurFrame, mLCLR);
  mLoadedGenDesc->mLINT->GetValue(mCurFrame, mLINT);
  if (mLightType != kLT_Directional) {
    mLoadedGenDesc->mLOFF->GetValue(mCurFrame, mLOFF);
    mLoadedGenDesc->mLFOR->GetValue(mCurFrame, mLFOR);
    if (mLightType == kLT_Spot) {
      mLoadedGenDesc->mLSLA->GetValue(mCurFrame, mLSLA);
    }
  }
  if (mLightType != kLT_Custom) {
    mLoadedGenDesc->mLDIR->GetValue(mCurFrame, mLDIR);
  }
}

void CElementGen::UpdateAdvanceAccessParameters(int particleIndex, int particleFrame) {
  float* parameters = mAdvValues[particleIndex].mValues;
  CParticleGlobals::SetParticleAccessParameters(parameters);
  if (mLoadedGenDesc->mVAV1) {
    mLoadedGenDesc->mVAV1->GetValue(particleFrame, *reinterpret_cast< CVector3f* >(parameters));
  }
  if (mLoadedGenDesc->mADV1) {
    mLoadedGenDesc->mADV1->GetValue(particleFrame, parameters[0]);
  }
  if (mLoadedGenDesc->mADV2) {
    mLoadedGenDesc->mADV2->GetValue(particleFrame, parameters[1]);
  }
  if (mLoadedGenDesc->mADV3) {
    mLoadedGenDesc->mADV3->GetValue(particleFrame, parameters[2]);
  }
  if (mLoadedGenDesc->mVAV2) {
    mLoadedGenDesc->mVAV2->GetValue(particleFrame, *reinterpret_cast< CVector3f* >(parameters + 3));
  }
  if (mLoadedGenDesc->mADV4) {
    mLoadedGenDesc->mADV4->GetValue(particleFrame, parameters[3]);
  }
  if (mLoadedGenDesc->mADV5) {
    mLoadedGenDesc->mADV5->GetValue(particleFrame, parameters[4]);
  }
  if (mLoadedGenDesc->mADV6) {
    mLoadedGenDesc->mADV6->GetValue(particleFrame, parameters[5]);
  }
  if (mLoadedGenDesc->mVAV3) {
    mLoadedGenDesc->mVAV3->GetValue(particleFrame, *reinterpret_cast< CVector3f* >(parameters + 6));
  }
  if (mLoadedGenDesc->mADV7) {
    mLoadedGenDesc->mADV7->GetValue(particleFrame, parameters[6]);
  }
  if (mLoadedGenDesc->mADV8) {
    mLoadedGenDesc->mADV8->GetValue(particleFrame, parameters[7]);
  }
  if (mLoadedGenDesc->mADV9) {
    mLoadedGenDesc->mADV9->GetValue(particleFrame, parameters[8]);
  }
}

bool CElementGen::UpdateVelocitySource(int sourceIndex, int particleFrame, CParticle& particle,
                                       const CVector3f& scaledTranslation) {
  bool expired;
  if (mHasVMD[sourceIndex]) {
    CVector3f localVelocity = mOrientationInverse * particle.mVel;
    CVector3f localPosition = mOrientationInverse * (particle.mPos - scaledTranslation);
    expired = mVELSources[sourceIndex]->GetValue(particleFrame, localVelocity, localPosition);
    particle.mVel = mOrientation.Rotate(localVelocity);
    particle.mPos = mOrientation.Rotate(localPosition) + scaledTranslation;
  } else {
    expired = mVELSources[sourceIndex]->GetValue(particleFrame, particle.mVel, particle.mPos);
  }
  if (expired) {
    particle.mEndFrame = -1;
  }
  return expired;
}

void CElementGen::UpdateExistingParticles() {
  mActiveParticleCount = 0;
  CParticleGlobals::SetEmitterTime(mCurFrame);
  CParticleGlobals::SetParticleAccessParameters(nullptr);
  const CVector3f scaledTranslation =
      (mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mTranslation;

  for (int i = 0; i < mParticles.size();) {
    CParticle& particle = mParticles[i];
    if (particle.mEndFrame < mCurFrame) {
      --sParticleAliveCount;
      if (i + 1 == mParticles.size()) {
        mParticles.pop_back();
        break;
      }
      particle = mParticles.back();
      if (mOrientType == kMOT_One) {
        mParentMatrices[i] = mParentMatrices[mParticles.size() - 1];
      }
      if (mEnableADV) {
        mAdvValues[i] = mAdvValues[mParticles.size() - 1];
      }
      mParticles.pop_back();
      if (particle.mEndFrame < mCurFrame) {
        continue;
      }
    }
    particle.mPrevPos = particle.mPos;
    particle.mPos += particle.mVel;
    const int particleFrame = mCurFrame - particle.mStartFrame;
    CParticleGlobals::SetCurrentParticle(&particle);
    CParticleGlobals::SetParticleLifetime(particle.mEndFrame - particle.mStartFrame);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(particleFrame);
    if (mEnableADV) {
      UpdateAdvanceAccessParameters(mActiveParticleCount, particleFrame);
    }
    ++mActiveParticleCount;
    for (int source = 0; source < 4 && mVELSources[source]; ++source) {
      UpdateVelocitySource(source, particleFrame, particle, scaledTranslation);
    }
    if (mLINE) {
      if (mLoadedGenDesc->mLENG) {
        mLoadedGenDesc->mLENG->GetValue(particleFrame, particle.mLineLengthOrSize);
      }
      if (mLoadedGenDesc->mWIDT) {
        mLoadedGenDesc->mWIDT->GetValue(particleFrame, particle.mLineWidthOrRota);
      }
    } else {
      if (mLoadedGenDesc->mROTA) {
        mLoadedGenDesc->mROTA->GetValue(particleFrame, particle.mLineWidthOrRota);
      }
      if (mLoadedGenDesc->mSIZE) {
        mLoadedGenDesc->mSIZE->GetValue(particleFrame, particle.mLineLengthOrSize);
      }
    }
    if (mLoadedGenDesc->mCOLR) {
      mLoadedGenDesc->mCOLR->GetValue(particleFrame, particle.mColor);
    }
    if (mEnableDynamicBounds) {
      AccumulateBounds(particle.mPos, particle.mLineLengthOrSize);
    }
    ++i;
  }

  if (!mParticles.empty()) {
    for (rstl::list< CWarp* >::iterator it = mModifiersList.begin(); it != mModifiersList.end();
         ++it) {
      CWarp* warp = *it;
      if (warp->UpdateWarp()) {
        CParticle& first = mParticles.front();
        warp->ModifyParticles(mParticles.size(), sizeof(CParticle), &first.mEndFrame,
                              &first.mPrevPos, &first.mPos, &first.mVel, &first.mColor,
                              &first.mLineLengthOrSize, &first.mLineWidthOrRota);
      }
    }
  }
}

void CElementGen::CreateNewParticles(int count) {
  if (!sStaticListInitialized) {
    Initialize();
  }
  if (mParticles.size() >= mMAXP) {
    return;
  }
  if (count + mParticles.size() > mMAXP) {
    count = mMAXP - mParticles.size();
  }
  if (count + sParticleAliveCount > 0xa00) {
    count = 0xa00 - sParticleAliveCount;
  }
  CGlobalRandom random(mRandState);
  mParticles.reserve(count + mParticles.size());
  if (mEnableADV && mAdvValues.capacity() < count + mParticles.size()) {
    mAdvValues.reserve(rstl::min_val(mMAXP, (count + mAdvValues.capacity()) * 2));
    while (mAdvValues.size() < mAdvValues.capacity()) {
      mAdvValues.push_back(CAdvancedValues());
    }
  }
  CParticleGlobals::SetParticleAccessParameters(nullptr);
  const CVector3f scaledTranslation =
      (mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mTranslation;

  for (int i = 0; i < count; ++i) {
    mParticles.push_back(CParticle());
    const int particleIndex = mParticles.size() - 1;
    if (mOrientType == kMOT_One) {
      mParentMatrices[particleIndex] = mOrientation.BuildMatrix3f();
    }
    CParticle& particle = mParticles[particleIndex];
    particle.mStartFrame = mCurFrame;
    CParticleGlobals::SetParticleLifetime(1);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(0);
    CParticleGlobals::SetCurrentParticle(&particle);
    if (mEnableADV) {
      UpdateAdvanceAccessParameters(particleIndex, 0);
    }
    if (mLoadedGenDesc->mLTME) {
      mLoadedGenDesc->mLTME->GetValue(0, particle.mEndFrame);
    }
    particle.mEndFrame += mCurFrame;
    if (mLoadedGenDesc->mEMTR) {
      mLoadedGenDesc->mEMTR->GetValue(mCurFrame, particle.mPos, particle.mVel);
      particle.mPos = mOrientation.Rotate(particle.mPos) +
                      (mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mTranslation +
                      mPOFS;
      particle.mVel = mOrientation.Rotate(particle.mVel);
    } else {
      particle.mPos =
          (mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mTranslation + mPOFS;
      particle.mVel = CVector3f::Zero();
    }
    particle.mPrevPos = particle.mPos;
    if (mLoadedGenDesc->mCOLR) {
      mLoadedGenDesc->mCOLR->GetValue(0, particle.mColor);
    } else {
      particle.mColor = CColor(0xffffffff);
    }
    if (mLINE) {
      if (mLoadedGenDesc->mLENG) {
        mLoadedGenDesc->mLENG->GetValue(0, particle.mLineLengthOrSize);
      } else {
        particle.mLineLengthOrSize = 1.f;
      }
      if (mLoadedGenDesc->mWIDT) {
        mLoadedGenDesc->mWIDT->GetValue(0, particle.mLineWidthOrRota);
      } else {
        particle.mLineWidthOrRota = 1.f;
      }
    } else {
      if (mLoadedGenDesc->mROTA) {
        mLoadedGenDesc->mROTA->GetValue(0, particle.mLineWidthOrRota);
      } else {
        particle.mLineWidthOrRota = 0.f;
      }
      if (mLoadedGenDesc->mSIZE) {
        mLoadedGenDesc->mSIZE->GetValue(0, particle.mLineLengthOrSize);
      } else {
        particle.mLineLengthOrSize = 0.1f;
      }
    }
    if (mLoadedGenDesc->mVMPC) {
      for (int source = 0; source < 4 && mVELSources[source]; ++source) {
        UpdateVelocitySource(source, 0, particle, scaledTranslation);
      }
    }
    ++mCumulativeParticles;
    if (particle.mEndFrame == -1) {
      mParticles.pop_back();
    } else {
      if (mEnableDynamicBounds) {
        AccumulateBounds(particle.mPos, particle.mLineLengthOrSize);
      }
      ++sParticleAliveCount;
      ++mActiveParticleCount;
    }
  }
}

void CElementGen::UpdatePSTranslationAndOrientation() {
  CGlobalRandom random(mRandState);
  if (mCurFrame <= mPSLT) {
    if (mLoadedGenDesc->mPOFS) {
      mLoadedGenDesc->mPOFS->GetValue(mCurFrame, mPOFS);
    }
    if (mLoadedGenDesc->mSSPO) {
      mLoadedGenDesc->mSSPO->GetValue(mCurFrame, mSSPO);
    }
    if (mLoadedGenDesc->mSEPO) {
      mLoadedGenDesc->mSEPO->GetValue(mCurFrame, mSEPO);
    }
  }
}

CParticleGen* CElementGen::ConstructChildParticleSystem(
    const CToken& description, uint type, short seed, EOptionalSystemFlags flags,
    bool modelsUseLights, bool emission, const CVector3f& translation,
    const CTransform4f& orientation, const CVector3f& globalTranslation,
    const CTransform4f& globalOrientation, const CVector3f& globalScale,
    const CColor& modulationColor, const CVector3f& localScale) {
  CParticleGen* child;
  switch (type) {
  case 'PART': {
    const short backupSeed = sSeed;
    if (seed != 0) {
      sSeed = seed;
    }
    TLockedToken< CGenDescription > particleDescription(description);
    if ((flags & kOSF_Two) && particleDescription->mOPTS) {
      return nullptr;
    }
    CElementGen* particles = rs_new CElementGen(particleDescription, kMOT_Normal, flags);
    particles->SetLeaveLightsEnabledForModelRender(modelsUseLights);
    child = particles;
    sSeed = backupSeed;
    break;
  }
  case 'SWHC': {
    TToken< CSwooshDescription > swooshDescription(description);
    child = rs_new CParticleSwoosh(swooshDescription, 0);
    break;
  }
  case 'ELSC': {
    const short backupSeed = CParticleElectric::GetGlobalSeed();
    if (seed != 0) {
      CParticleElectric::SetGlobalSeed(seed);
    }
    TToken< CElectricDescription > electricDescription(description);
    child = rs_new CParticleElectric(electricDescription);
    CParticleElectric::SetGlobalSeed(backupSeed);
    break;
  }
  case 'SPSC': {
    const short backupSeed = CParticleSpawnSystem::GetGlobalSeed();
    if (seed != 0) {
      CParticleSpawnSystem::SetGlobalSeed(seed);
    }
    TToken< CSpawnSystemDescription > spawnDescription(description);
    child = rs_new CParticleSpawnSystem(spawnDescription, flags, modelsUseLights);
    CParticleSpawnSystem::SetGlobalSeed(backupSeed);
    break;
  }
  case 'SRSC': {
    const short backupSeed = CParticleSpawnRandom::GetGlobalSeed();
    if (seed != 0) {
      CParticleSpawnRandom::SetGlobalSeed(seed);
    }
    TToken< CSpawnRandomDescription > spawnDescription(description);
    child = rs_new CParticleSpawnRandom(spawnDescription, flags, modelsUseLights);
    CParticleSpawnRandom::SetGlobalSeed(backupSeed);
    break;
  }
  default:
    return nullptr;
  }

  if (child) {
    if (child->Get4CharId() == 'SPSC') {
      CParticleSpawnSystem* spawnSystem = static_cast< CParticleSpawnSystem* >(child);
      spawnSystem->ForceSetTranslation(translation);
      spawnSystem->ForceSetOrientation(orientation);
      spawnSystem->ForceSetGlobalTranslation(globalTranslation);
      spawnSystem->ForceSetGlobalOrientation(globalOrientation);
    } else {
      child->SetGlobalTranslation(globalTranslation);
      child->SetGlobalOrientation(globalOrientation);
      child->SetTranslation(translation);
      child->SetOrientation(orientation);
    }
    child->SetGlobalScale(globalScale);
    child->SetLocalScale(localScale);
    child->SetParticleEmission(emission);
    child->SetModulationColor(modulationColor);
  }
  return child;
}

CParticleGen* CElementGen::ConstructChildParticleSystem(const CToken& description, uint type,
                                                        ushort seed) const {
  const EOptionalSystemFlags boundsFlags = mEnableDynamicBounds ? kOSF_One : kOSF_DisableBounds;
  const EOptionalSystemFlags flags =
      EOptionalSystemFlags((mEnableOPTS ? kOSF_Two : kOSF_One) | boundsFlags);
  return ConstructChildParticleSystem(
      description, type, seed, flags, mModelsUseLights, mParticleEmission, mTranslation,
      mOrientation, mGlobalTranslation, mGlobalOrientation, mGlobalScale, mModuColor, mLocalScale);
}

void CElementGen::UpdateChildParticleSystems(double dt) {
  if (close_enough(dt, 0.0, 1e-7)) {
    return;
  }
  CGlobalRandom random(mRandState);

  // ICTS: children spawned at CSSD.
  if (mLoadedGenDesc->mICTS && mPrevFrame != mCurFrame && mCurFrame == mCSSD) {
    int count = 1;
    if (mLoadedGenDesc->mNCSY) {
      mLoadedGenDesc->mNCSY->GetValue(mCurFrame, count);
    }
    mActivePartChildren.reserve(count + mActivePartChildren.size());
    for (int i = 0; i < count; ++i) {
      TLockedToken< CGenDescription > description = mLoadedGenDesc->mICTS->GetToken();
      if (mEnableOPTS && description->mOPTS) {
        break;
      }
      mActivePartChildren.push_back(ConstructChildParticleSystem(description, 'PART', sSeed));
    }
  }

  // IITS: children spawned periodically while the emitter is alive.
  if (mLoadedGenDesc->mIITS && mPrevFrame != mCurFrame && mCurFrame < mPSLT && mParticleEmission &&
      mCurFrame >= mSISY && (mCurFrame - mSISY) % mPISY == 0) {
    TLockedToken< CGenDescription > description = mLoadedGenDesc->mIITS->GetToken();
    if (!(mEnableOPTS && description->mOPTS)) {
      mActivePartChildren.reserve(mActivePartChildren.size() + 1);
      mActivePartChildren.push_back(ConstructChildParticleSystem(description, 'PART', sSeed));
    }
  }

  // KSSM: keyframes can spawn several different effect types.
  if (mLoadedGenDesc->mKSSM && mPrevFrame != mCurFrame && mCurFrame < mPSLT) {
    rstl::vector< CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo >& spawns =
        mLoadedGenDesc->mKSSM->GetSpawnedSystemsAtFrame(mCurFrame);
    if (!spawns.empty()) {
      const ushort backupSeed = sSeed;
      mActivePartChildren.reserve(spawns.size() + mActivePartChildren.size());
      for (int i = 0; i < spawns.size(); ++i) {
        CParticleGen* child = ConstructChildParticleSystem(
            *spawns[i].GetToken(), spawns[i].GetType(), mCurFrame + backupSeed + i);
        if (child) {
          mActivePartChildren.push_back(child);
        }
      }
      sSeed = backupSeed;
    }
  }

  // IDTS: children spawned when this system reaches its lifetime.
  if (mCurFrame == mPSLT && mPrevFrame != mCurFrame && mLoadedGenDesc->mIDTS) {
    int count = 1;
    if (mLoadedGenDesc->mNDSY) {
      mLoadedGenDesc->mNDSY->GetValue(0, count);
    }
    mActivePartChildren.reserve(count + mActivePartChildren.size());
    for (int i = 0; i < count; ++i) {
      TLockedToken< CGenDescription > description = mLoadedGenDesc->mIDTS->GetToken();
      if (mEnableOPTS && description->mOPTS) {
        break;
      }
      mActivePartChildren.push_back(ConstructChildParticleSystem(description, 'PART', sSeed));
    }
  }

  if (mLoadedGenDesc->mSSWH && mPrevFrame != mCurFrame && mCurFrame == mSSSD) {
    CParticleSwoosh* swoosh = rs_new CParticleSwoosh(*mLoadedGenDesc->mSSWH, 0);
    swoosh->SetGlobalTranslation(mGlobalTranslation);
    swoosh->SetGlobalScale(mGlobalScale);
    swoosh->SetLocalScale(mLocalScale);
    swoosh->SetTranslation(mTranslation + mSSPO);
    swoosh->SetOrientation(mOrientation);
    swoosh->SetParticleEmission(mParticleEmission);
    mActivePartChildren.reserve(mActivePartChildren.size() + 1);
    mActivePartChildren.push_back(swoosh);
  }

  if (mLoadedGenDesc->mSELC && mPrevFrame != mCurFrame && mCurFrame == mSESD) {
    CParticleElectric* electric = rs_new CParticleElectric(*mLoadedGenDesc->mSELC);
    electric->SetGlobalTranslation(mGlobalTranslation);
    electric->SetGlobalScale(mGlobalScale);
    electric->SetLocalScale(mLocalScale);
    electric->SetTranslation(mTranslation + mSEPO);
    electric->SetOrientation(mOrientation);
    electric->SetParticleEmission(mParticleEmission);
    mActivePartChildren.reserve(mActivePartChildren.size() + 1);
    mActivePartChildren.push_back(electric);
  }

  rstl::vector< CParticleGen* >::iterator it = mActivePartChildren.begin();
  while (it != mActivePartChildren.end()) {
    CParticleGen* child = *it;
    child->Update(dt);
    if (child->IsSystemDeletable() == true) {
      delete child;
      it = mActivePartChildren.erase(it);
    } else {
      ++it;
    }
  }
  mPrevFrame = mCurFrame;
}

void CElementGen::SetParticleEmission(bool emission) {
  mParticleEmission = emission;
  for (int i = 0; i < mActivePartChildren.size(); ++i) {
    mActivePartChildren[i]->SetParticleEmission(emission);
  }
}

void CElementGen::RenderModels() {
  CGlobalRandom gr(mRandState);
  CParticleGlobals::SetParticleAccessParameters(nullptr);
  SModelRenderState state;
  if (IsIndirectTextured()) {
    if (!mLoadedGenDesc->mPMUS) {
      return;
    }
    BeginIndirectModelRender(state);
  } else {
    BeginModelRender(state);
  }

  CVector3f offset(CVector3f::Zero());
  CTransform4f orientation(CTransform4f::Identity());
  if (!mLoadedGenDesc->mPMOO) {
    orientation = mOrientation;
  }
  orientation = orientation * mGlobalOrientation;
  const bool constantRotation =
      mLoadedGenDesc->mPMRT != nullptr && mLoadedGenDesc->mPMRT->IsFastConstant();
  const CVector3f translation =
      (mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mGlobalTranslation;
  CTransform4f rotation(CTransform4f::Identity());
  if (constantRotation) {
    CVector3f angles(CVector3f::Zero());
    mLoadedGenDesc->mPMRT->GetValue(mCurFrame, angles);
    rotation = CTransform4f::RotateZ(CRelAngle::FromDegrees(angles.GetZ()));
    rotation.RotateLocalY(CRelAngle::FromDegrees(angles.GetY()));
    rotation.RotateLocalX(CRelAngle::FromDegrees(angles.GetX()));
  }
  rotation = orientation * rotation;
  CParticleGlobals::SetEmitterTime(mCurFrame);

  CParticleListItem* sortItems = nullptr;
  CColor color = mModuColor;
  const bool sorted = mLoadedGenDesc->mSORT;
  if (sorted) {
    CTransform4f view(CGraphics::GetViewMatrix());
    view.SetTranslation(CVector3f::Zero());
    CTransform4f camera(view.GetQuickInverse() * mGlobalOrientation);
    const int count = mParticles.size();
    sortItems = static_cast< CParticleListItem* >(alloca(count * sizeof(CParticleListItem)));
    for (int i = 0; i < count; ++i) {
      const CParticle& particle = mParticles[i];
      sortItems[i].mViewPoint =
          camera * (mTimeDeltaScale * (particle.mPos - particle.mPrevPos) + particle.mPrevPos);
      sortItems[i].mPartIdx = static_cast< ushort >(i);
    }
    static CParticleListItemViewPointComp comparator;
    rstl::sort(sortItems, sortItems + count, comparator);
  }

  for (int i = 0; i < mParticles.size(); ++i) {
    const int index = sorted ? sortItems[i].mPartIdx : i;
    CParticle& particle = mParticles[index];
    if (particle.mEndFrame == -1) {
      continue;
    }
    const int frame = mCurFrame - particle.mStartFrame - 1;
    CParticleGlobals::SetParticleLifetime(particle.mEndFrame - particle.mStartFrame);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(frame);
    CParticleGlobals::SetCurrentParticle(&particle);
    if (mEnableADV) {
      CParticleGlobals::SetParticleAccessParameters(mAdvValues[index].mValues);
    }
    if (mLoadedGenDesc->mPMOP != nullptr) {
      mLoadedGenDesc->mPMOP->GetValue(frame, offset);
    }

    CTransform4f transform = CTransform4f::Translate(
        translation +
        mGlobalOrientation.Rotate(mTimeDeltaScale * (particle.mPos - particle.mPrevPos) +
                                  particle.mPrevPos));
    if (mOrientType == kMOT_One) {
      CTransform4f parent(mParentMatrices[index], CVector3f::Zero());
      transform.AddTranslation((orientation * parent) * offset);
      transform *= parent;
    } else {
      transform.AddTranslation(orientation * offset);
    }
    if (mLoadedGenDesc->mPMOV != nullptr) {
      CVector3f direction(CVector3f::Zero());
      mLoadedGenDesc->mPMOV->GetValue(frame, direction);
      transform *= CTransform4f::LookAt(CVector3f::Zero(), direction, CVector3f::Up());
    }
    if (constantRotation) {
      transform *= rotation;
    } else if (mLoadedGenDesc->mPMRT != nullptr) {
      CVector3f angles(CVector3f::Zero());
      mLoadedGenDesc->mPMRT->GetValue(frame, angles);
      rotation = CTransform4f::RotateZ(CRelAngle::FromDegrees(angles.GetZ()));
      rotation.RotateLocalY(CRelAngle::FromDegrees(angles.GetY()));
      rotation.RotateLocalX(CRelAngle::FromDegrees(angles.GetX()));
      transform *= orientation * rotation;
    } else {
      transform *= rotation;
    }
    if (mLoadedGenDesc->mPMSC != nullptr) {
      CVector3f scale(CVector3f::Zero());
      mLoadedGenDesc->mPMSC->GetValue(frame, scale);
      transform *= CTransform4f::Scale(scale.GetX(), scale.GetY(), scale.GetZ());
    }
    if (mLoadedGenDesc->mPMCL != nullptr) {
      mLoadedGenDesc->mPMCL->GetValue(frame, color);
      color = CColor::Modulate(color, mModuColor);
    }
    CGraphics::SetModelMatrix(mGlobalScaleTransform * transform * mLocalScaleTransform);
    if (IsIndirectTextured()) {
      RenderIndirectModelParticle(state, color, particle);
    } else {
      RenderModelParticle(state, color, particle);
    }
  }

  if (IsIndirectTextured()) {
    EndIndirectModelRender();
  } else {
    EndModelRender(state);
  }
}

void CElementGen::EndIndirectModelRender() {
  CGraphics::SetCullMode(kCM_Front);
  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(GX_TEVSTAGE1);
  if (sMoveRedToAlphaBuffer) {
    GXSetAlphaUpdate(GX_TRUE);
  }
}

void CElementGen::EndModelRender(const SModelRenderState& state) {
  if (mModelsUseLights) {
    CGraphics::DisableAllLights();
  }
  CGraphics::SetCullMode(kCM_Front);
  CTevCombiners::ResetStates();
  if (state.mModulateAlpha) {
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
  }
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  if (sMoveRedToAlphaBuffer) {
    GXSetAlphaUpdate(GX_TRUE);
  }
}

void CElementGen::RenderModelParticle(SModelRenderState& state, const CColor& color,
                                      const CParticle& particle) {
  if (mLoadedGenDesc->mPMUS) {
    SUVElementSet& uvs = state.mUV;
    if (!state.mConstantUV) {
      mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - particle.mStartFrame - 1, uvs);
    }
    if (state.mModulateAlpha) {
      CGX::Begin(GX_QUADS, GX_VTXFMT0, 4);
      GXPosition3f32(0.5f, 0.f, 0.5f);
      const uint packedColor = color.GetColor_u32();
      GXColor1u32(packedColor);
      GXTexCoord2f32(uvs.xMax, uvs.yMax);
      GXPosition3f32(-0.5f, 0.f, 0.5f);
      GXColor1u32(packedColor);
      GXTexCoord2f32(uvs.xMin, uvs.yMax);
      GXPosition3f32(-0.5f, 0.f, -0.5f);
      GXColor1u32(packedColor);
      GXTexCoord2f32(uvs.xMin, uvs.yMin);
      GXPosition3f32(0.5f, 0.f, -0.5f);
      GXColor1u32(packedColor);
      GXTexCoord2f32(uvs.xMax, uvs.yMin);
      CGX::End();
    } else {
      CGraphics::StreamBegin(kP_Quads);
      CGraphics::StreamColor(color.GetColor_u32());
      CGraphics::StreamTexcoord(uvs.xMax, uvs.yMax);
      CGraphics::StreamVertex(0.5f, 0.f, 0.5f);
      CGraphics::StreamTexcoord(uvs.xMin, uvs.yMax);
      CGraphics::StreamVertex(-0.5f, 0.f, 0.5f);
      CGraphics::StreamTexcoord(uvs.xMin, uvs.yMin);
      CGraphics::StreamVertex(-0.5f, 0.f, -0.5f);
      CGraphics::StreamTexcoord(uvs.xMax, uvs.yMin);
      CGraphics::StreamVertex(0.5f, 0.f, -0.5f);
      CGraphics::StreamEnd();
    }
  } else {
    CModel* model = mLoadedGenDesc->mPMDL->GetObject();
    if (sSubtractBlend) {
      model->Draw(CModelFlags::AlphaBlended(0.5f).DepthCompareUpdate(true, false));
    } else if (mLoadedGenDesc->mPMAB) {
      model->Draw(CModelFlags::Additive(color).DepthCompareUpdate(true, false));
    } else if (color.GetAlpha() == 1.f) {
      model->Draw(CModelFlags(
          CModelFlags::kT_One, 0,
          CModelFlags::EFlags(CModelFlags::kF_DepthCompare | CModelFlags::kF_DepthUpdate), color));
    } else {
      model->Draw(CModelFlags(CModelFlags::kT_Blend, 0, CModelFlags::kF_DepthCompare, color));
    }
  }
}

void CElementGen::RenderIndirectModelParticle(SModelRenderState& state, const CColor& color,
                                              const CParticle& particle) {
  SUVElementSet& uvs = state.mUV;
  if (!state.mConstantUV) {
    mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - particle.mStartFrame, uvs);
  }
  SUVElementSet& indirectUVs = state.mIndirectUV;
  if (!state.mConstantIndirectUV) {
    mLoadedGenDesc->mTIND->GetValueUV(mCurFrame - particle.mStartFrame, indirectUVs);
  }

  CGraphics::CClippedScreenQuad clip = CGraphics::ClipScreenQuadFromMS(
      CVector3f(0.5f, 0.f, 0.5f), CVector3f(-0.5f, 0.f, 0.5f), CVector3f(-0.5f, 0.f, -0.5f),
      CVector3f(0.5f, 0.f, -0.5f), kTF_RGB565);
  void* dest = CGraphics::GetDolphinSpareBuffer();
  if (!clip.IsValid()) {
    return;
  }
  const bool halfSize = mLoadedGenDesc->mINDM;
  GXSetTexCopySrc(clip.GetX(), clip.GetY(), clip.GetWidth(), clip.GetHeight());
  GXSetTexCopyDst(clip.GetTexWidth() >> halfSize, clip.GetHeight() >> halfSize, GX_TF_RGB565,
                  halfSize);
  const size_t bufferSize = CGraphics::GetSpareBufferSize();
  const size_t textureSize = GXGetTexBufferSize(
      clip.GetTexWidth() >> halfSize, clip.GetHeight() >> halfSize, GX_TF_RGB565, false, 0);
  if (textureSize > bufferSize) {
    return;
  }
  const bool useVideoFilter = CGraphics::GetUseVideoFilter();
  CGraphics::SetUseVideoFilter(false);
  GXCopyTex(dest, GX_FALSE);
  CGraphics::SetUseVideoFilter(useVideoFilter);
  GXPixModeSync();
  CGraphics::LoadDolphinSpareTexture(clip.GetTexWidth() >> halfSize, clip.GetHeight() >> halfSize,
                                     GX_TF_RGB565, nullptr, CGraphics::kSpareBufferTexMapID);

  const uint packedColor = color.GetColor_u32();
  CGX::Begin(GX_QUADS, GX_VTXFMT0, 4);
  GXPosition3f32(0.5f, 0.f, 0.5f);
  GXColor1u32(packedColor);
  GXTexCoord2f32(uvs.xMax, uvs.yMax);
  GXTexCoord2f32(clip.GetTexCoord(0).GetX(), clip.GetTexCoord(0).GetY());
  GXTexCoord2f32(indirectUVs.xMax, indirectUVs.yMax);
  GXPosition3f32(-0.5f, 0.f, 0.5f);
  GXColor1u32(packedColor);
  GXTexCoord2f32(uvs.xMin, uvs.yMax);
  GXTexCoord2f32(clip.GetTexCoord(1).GetX(), clip.GetTexCoord(1).GetY());
  GXTexCoord2f32(indirectUVs.xMin, indirectUVs.yMax);
  GXPosition3f32(-0.5f, 0.f, -0.5f);
  GXColor1u32(packedColor);
  GXTexCoord2f32(uvs.xMin, uvs.yMin);
  GXTexCoord2f32(clip.GetTexCoord(2).GetX(), clip.GetTexCoord(2).GetY());
  GXTexCoord2f32(indirectUVs.xMin, indirectUVs.yMin);
  GXPosition3f32(0.5f, 0.f, -0.5f);
  GXColor1u32(packedColor);
  GXTexCoord2f32(uvs.xMax, uvs.yMin);
  GXTexCoord2f32(clip.GetTexCoord(3).GetX(), clip.GetTexCoord(3).GetY());
  GXTexCoord2f32(indirectUVs.xMax, indirectUVs.yMin);
  CGX::End();
}

void CElementGen::BeginIndirectModelRender(SModelRenderState& state) {
  if (sMoveRedToAlphaBuffer) {
    GXSetAlphaUpdate(GX_FALSE);
  }
  CGraphics::SetCullMode(kCM_None);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  if (mAAPH) {
    CGX::SetZMode(true, GX_LEQUAL, false);
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
  } else {
    CGX::SetZMode(true, GX_LEQUAL, mZBUF);
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  }

  TToken< CTexture > texture =
      mLoadedGenDesc->mTEXR->GetValueTexture(mCurFrame - mParticles[0].mStartFrame);
  texture->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - mParticles[0].mStartFrame, state.mUV);
  state.mConstantUV = mLoadedGenDesc->mTEXR->HasConstantUV();
  TToken< CTexture > indirectTexture =
      mLoadedGenDesc->mTIND->GetValueTexture(mCurFrame - mParticles[0].mStartFrame);
  indirectTexture->Load(GX_TEXMAP2, CTexture::kCM_Repeat);
  state.mConstantIndirectUV = mLoadedGenDesc->mTIND->HasConstantUV();
  mLoadedGenDesc->mTIND->GetValueUV(mCurFrame - mParticles[0].mStartFrame, state.mIndirectUV);

  CGX::SetNumTexGens(3);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX2, GX_IDENTITY, false, GX_PTIDENTITY);
  float indirectMatrix[2][3] = {{0.5f, 0.f, 0.f}, {0.f, 0.5f, 0.f}};
  GXSetIndTexMtx(GX_ITM_0, indirectMatrix, 1);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD2, GX_TEXMAP2);
  CGX::SetTevIndirect(GX_TEVSTAGE1, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                      GX_ITW_OFF, false, false, GX_ITBA_OFF);
  CGX::SetNumIndStages(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, CGraphics::kSpareBufferTexMapID, GX_COLOR0A0);
  CGX::SetNumTevStages(2);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_8_8);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_8_8);
  if (mLoadedGenDesc->mCIND) {
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
  } else {
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_ZERO);
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_CPREV);
  }
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_RASA, GX_CA_ZERO);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  CGX::SetNumChans(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  static const GXVtxDescList skDescList[] = {
      {GX_VA_POS, GX_DIRECT},  {GX_VA_CLR0, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_TEX1, GX_DIRECT}, {GX_VA_TEX2, GX_DIRECT}, {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(skDescList);
}

void CElementGen::BeginModelRender(SModelRenderState& state) {
  if (sMoveRedToAlphaBuffer) {
    GXSetAlphaUpdate(GX_FALSE);
  }
  if (mModelsUseLights) {
    CGraphics::SetLightState(mBackupLightActive);
  } else {
    CGraphics::SetAmbientColor(CColor::White());
  }
  if (!mLoadedGenDesc->mPMUS) {
    return;
  }

  state.mModulateAlpha =
      sEnableAlphaModulation && mLoadedGenDesc->mPMAB && mLoadedGenDesc->mTEXR != nullptr;
  if (mLoadedGenDesc->mPMAB) {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
    if (state.mModulateAlpha) {
      CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
    } else {
      CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
      CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
    }
  } else {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, mZBUF);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
    CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
  }
  CGraphics::SetCullMode(kCM_None);

  if (mLoadedGenDesc->mTEXR != nullptr) {
    TToken< CTexture > texture =
        mLoadedGenDesc->mTEXR->GetValueTexture(mCurFrame - mParticles[0].mStartFrame);
    texture->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    if (state.mModulateAlpha) {
      CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
      CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
      CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
      CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
      GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP1);
      CGX::SetNumTevStages(2);
      static const GXVtxDescList skDescList[] = {
          {GX_VA_POS, GX_DIRECT},
          {GX_VA_CLR0, GX_DIRECT},
          {GX_VA_TEX0, GX_DIRECT},
          {GX_VA_NULL, GX_NONE},
      };
      CGX::SetVtxDescv(skDescList);
      CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                       GX_AF_NONE);
      CGX::SetNumChans(1);
      CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false,
                          GX_PTIDENTITY);
      CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
      CGX::SetNumTexGens(1);
    } else {
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    }
    mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - mParticles[0].mStartFrame, state.mUV);
    state.mConstantUV = mLoadedGenDesc->mTEXR->HasConstantUV();
  } else {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  }
}

void CElementGen::RenderLines() {
  CGlobalRandom gr(mRandState);
  const bool hasModuColor = mModuColor.GetColor_u32() != 0xffffffff;

  CTransform4f systemViewPointMatrix(CGraphics::GetViewMatrix());
  systemViewPointMatrix.SetTranslation(CVector3f::Zero());
  CTransform4f systemCameraMatrix(systemViewPointMatrix.GetQuickInverse() * mGlobalOrientation);
  systemViewPointMatrix = CTransform4f::Translate(mGlobalTranslation) * mGlobalScaleTransform *
                          systemViewPointMatrix * mLocalScaleTransform;
  CGraphics::SetModelMatrix(systemViewPointMatrix);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);

  if (mAAPH) {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
  } else {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  }

  bool constUVs = true;
  SUVElementSet uvs;
  uvs.xMin = 0.f;
  uvs.xMax = 1.f;
  uvs.yMin = 0.f;
  uvs.yMax = 1.f;
  const bool widtConst = mLoadedGenDesc->mWIDT != nullptr && mLoadedGenDesc->mWIDT->IsConstant();

  if (mLoadedGenDesc->mTEXR != nullptr) {
    TToken< CTexture > tex =
        mLoadedGenDesc->mTEXR->GetValueTexture(mCurFrame - mParticles[0].mStartFrame);
    tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);

    if (hasModuColor) {
      static CTevCombiners::CTevPass ModulatePreviousTEVWithRasColor(
          CTevCombiners::ColorPass(CTevCombiners::kCS_Zero, CTevCombiners::kCS_PreviousColor,
                                   CTevCombiners::kCS_RasterColor, CTevCombiners::kCS_Zero),
          CTevCombiners::AlphaPass(CTevCombiners::kAS_Zero, CTevCombiners::kAS_PreviousAlpha,
                                   CTevCombiners::kAS_RasterAlpha, CTevCombiners::kAS_Zero));
      CGraphics::SetTevOp(kTS_Stage1, ModulatePreviousTEVWithRasColor);
    } else {
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    }

    mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - mParticles[0].mStartFrame, uvs);
    constUVs = mLoadedGenDesc->mTEXR->HasConstantUV();
  } else {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  }

  static const GXVtxDescList skDescList[] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_CLR0, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(skDescList);

  if (hasModuColor) {
    CGX::SetNumChans(2);
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);
    CGX::SetChanMatColor(CGX::Channel1, mModuColor.GetGXColor());
    CGX::SetChanCtrl(CGX::Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                     GX_AF_NONE);
  } else {
    CGX::SetNumChans(1);
  }
  CGX::SetNumTexGens(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);

  if (widtConst) {
    float lineWidth = 1.f;
    mLoadedGenDesc->mWIDT->GetValue(0, lineWidth);
    if (lineWidth < 0.f) {
      lineWidth = 0.f;
    } else if (lineWidth > 42.5f) {
      lineWidth = 42.5f;
    }
    CGX::SetLineWidth(CCast::ToUint8(6.f * lineWidth), GX_TO_ZERO);
    CGX::Begin(GX_LINES, GX_VTXFMT0, static_cast< ushort >(mParticles.size() * 2));
  }

  for (int i = 0; i < mParticles.size(); ++i) {
    CParticle& particle = mParticles[i];
    if (!constUVs) {
      mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - particle.mStartFrame, uvs);
    }

    CVector3f direction = particle.mPos - particle.mPrevPos;
    if (mFXLL) {
      const float magnitude = direction.Magnitude();
      if (magnitude > 0.f) {
        direction *= 1.f / magnitude;
      }
    }
    const CVector3f p1 = systemCameraMatrix * particle.mPos;
    const CVector3f p2 =
        systemCameraMatrix * (particle.mPos + particle.mLineLengthOrSize * direction);

    if (widtConst) {
      const uint color = particle.mColor.GetColor_u32();
      GXPosition3f32(p1[0], p1[1], p1[2]);
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMin, uvs.yMin);
      GXPosition3f32(p2[0], p2[1], p2[2]);
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMax, uvs.yMax);
    } else {
      float lineWidth = particle.mLineWidthOrRota;
      if (lineWidth < 0.f) {
        lineWidth = 0.f;
      } else if (lineWidth > 42.5f) {
        lineWidth = 42.5f;
      }
      CGX::SetLineWidth(CCast::ToUint8(6.f * lineWidth), GX_TO_ZERO);
      CGX::Begin(GX_LINES, GX_VTXFMT0, 2);
      const uint color = particle.mColor.GetColor_u32();
      GXPosition3f32(p1[0], p1[1], p1[2]);
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMin, uvs.yMin);
      GXPosition3f32(p2[0], p2[1], p2[2]);
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMax, uvs.yMax);
      CGX::End();
    }
  }

  if (widtConst) {
    CGX::End();
  }
  CGraphics::SetLineWidth(1.f, kTO_Zero);
}

void CElementGen::Render() {
  CStopwatch stopwatch;
  mBackupLightActive = CGraphics::GetLightMask();
  CGraphics::DisableAllLights();
  for (int i = 0; i < mActivePartChildren.size(); ++i) {
    if (mActivePartChildren[i]->ShouldDraw()) {
      mActivePartChildren[i]->Render();
    }
  }

  CParticleGlobals::SParticleSystem system('PART', this);
  if (!mParticles.empty() && ShouldDraw()) {
    if (mLoadedGenDesc->mPMDL || mLoadedGenDesc->mPMUS) {
      RenderModels();
    }
    if (mLINE) {
      RenderLines();
    } else {
      bool zeroSize = false;
      if (mLoadedGenDesc->mSIZE && mLoadedGenDesc->mSIZE->IsConstant()) {
        CGlobalRandom random(mRandState);
        float size = 1.f;
        mLoadedGenDesc->mSIZE->GetValue(0, size);
        if (size == 0.f) {
          size = 1.f;
          mLoadedGenDesc->mSIZE->GetValue(1, size);
          zeroSize = size == 0.f;
        }
      }
      if (!zeroSize && mLoadedGenDesc->mRDOP) {
        if (IsIndirectTextured()) {
          RenderParticlesIndirectTexture();
        } else {
          RenderParticles();
        }
      }
    }
  }
  mRenderTime = stopwatch.GetElapsedTime();
}

void CElementGen::RenderBasicParticlesNoRotTS(const CTransform4f& xf) const {
  for (int i = 0; i < mParticles.size(); ++i) {
    const CParticle& particle = mParticles[i];
    const CVector3f viewPos = xf * particle.mPos;
    const float halfSize = 0.5f * particle.mLineLengthOrSize;
    const uint color = particle.mColor.GetColor_u32();
    float x = viewPos.GetX() + halfSize;
    const float y = viewPos.GetY();
    float z = viewPos.GetZ() + halfSize;

    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    x -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    z -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    x += particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderBasicParticlesRotTS(const CTransform4f& xf) const {
  for (int i = 0; i < mParticles.size(); ++i) {
    const CParticle& particle = mParticles[i];
    const CVector3f viewPos = xf * particle.mPos;
    const uint color = particle.mColor.GetColor_u32();
    const float halfSize = 0.5f * particle.mLineLengthOrSize;
    const float theta = CRelAngle::FromDegrees(particle.mLineWidthOrRota).AsRadians();
    const float sinT = CMath::FastSinR(theta) * halfSize;
    const float cosT = CMath::FastCosR(theta) * halfSize;
    const float sinPlusCos = sinT + cosT;
    const float sinMinusCos = sinT - cosT;
    const float x = viewPos.GetX();
    const float y = viewPos.GetY();
    const float z = viewPos.GetZ();

    GXPosition3f32(x + sinPlusCos, y, z - sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    GXPosition3f32(x + sinMinusCos, y, z + sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    GXPosition3f32(x - sinPlusCos, y, z + sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    GXPosition3f32(x - sinMinusCos, y, z - sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderBasicParticlesNoRotNoTS(const CTransform4f& xf) const {
  for (int i = 0; i < mParticles.size(); ++i) {
    const CParticle& particle = mParticles[i];
    const CVector3f viewPos =
        xf * ((particle.mPos - particle.mPrevPos) * mTimeDeltaScale + particle.mPrevPos);
    const uint color = particle.mColor.GetColor_u32();
    const float halfSize = 0.5f * particle.mLineLengthOrSize;
    float x = viewPos.GetX() + halfSize;
    const float y = viewPos.GetY();
    float z = viewPos.GetZ() + halfSize;

    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    x -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    z -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    x += particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderBasicParticlesRotNoTS(const CTransform4f& xf) const {
  for (int i = 0; i < mParticles.size(); ++i) {
    const CParticle& particle = mParticles[i];
    const CVector3f viewPos =
        xf * ((particle.mPos - particle.mPrevPos) * mTimeDeltaScale + particle.mPrevPos);
    const uint color = particle.mColor.GetColor_u32();
    const float halfSize = 0.5f * particle.mLineLengthOrSize;
    const float theta = CRelAngle::FromDegrees(particle.mLineWidthOrRota).AsRadians();
    const float sinT = CMath::FastSinR(theta) * halfSize;
    const float cosT = CMath::FastCosR(theta) * halfSize;
    const float sinPlusCos = sinT + cosT;
    const float sinMinusCos = sinT - cosT;
    const float x = viewPos.GetX();
    const float y = viewPos.GetY();
    const float z = viewPos.GetZ();

    GXPosition3f32(x + sinPlusCos, y, z - sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    GXPosition3f32(x + sinMinusCos, y, z + sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    GXPosition3f32(x - sinPlusCos, y, z + sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    GXPosition3f32(x - sinMinusCos, y, z - sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderBasicParticlesNoRotTSModulated(const CTransform4f& xf) const {
  for (int i = 0; i < mParticles.size(); ++i) {
    const CParticle& particle = mParticles[i];
    const CVector3f viewPos = xf * particle.mPos;
    const uint color = CColor::Modulate(mModuColor, particle.mColor).GetColor_u32();
    const float halfSize = 0.5f * particle.mLineLengthOrSize;
    float x = viewPos.GetX() + halfSize;
    const float y = viewPos.GetY();
    float z = viewPos.GetZ() + halfSize;

    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    x -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    z -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    x += particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderBasicParticlesRotTSModulated(const CTransform4f& xf) const {
  for (int i = 0; i < mParticles.size(); ++i) {
    const CParticle& particle = mParticles[i];
    const CVector3f viewPos = xf * particle.mPos;
    const uint color = CColor::Modulate(mModuColor, particle.mColor).GetColor_u32();
    const float halfSize = 0.5f * particle.mLineLengthOrSize;
    const float theta = CRelAngle::FromDegrees(particle.mLineWidthOrRota).AsRadians();
    const float sinT = CMath::FastSinR(theta) * halfSize;
    const float cosT = CMath::FastCosR(theta) * halfSize;
    const float sinPlusCos = sinT + cosT;
    const float sinMinusCos = sinT - cosT;
    const float x = viewPos.GetX();
    const float y = viewPos.GetY();
    const float z = viewPos.GetZ();

    GXPosition3f32(x + sinPlusCos, y, z - sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    GXPosition3f32(x + sinMinusCos, y, z + sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    GXPosition3f32(x - sinPlusCos, y, z + sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    GXPosition3f32(x - sinMinusCos, y, z - sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderBasicParticlesNoRotNoTSModulated(const CTransform4f& xf) const {
  for (int i = 0; i < mParticles.size(); ++i) {
    const CParticle& particle = mParticles[i];
    const CVector3f viewPos =
        xf * ((particle.mPos - particle.mPrevPos) * mTimeDeltaScale + particle.mPrevPos);
    const uint color = CColor::Modulate(mModuColor, particle.mColor).GetColor_u32();
    const float halfSize = 0.5f * particle.mLineLengthOrSize;
    float x = viewPos.GetX() + halfSize;
    const float y = viewPos.GetY();
    float z = viewPos.GetZ() + halfSize;

    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    x -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    z -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    x += particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderBasicParticlesRotNoTSModulated(const CTransform4f& xf) const {
  for (int i = 0; i < mParticles.size(); ++i) {
    const CParticle& particle = mParticles[i];
    const CVector3f viewPos =
        xf * ((particle.mPos - particle.mPrevPos) * mTimeDeltaScale + particle.mPrevPos);
    const uint color = CColor::Modulate(mModuColor, particle.mColor).GetColor_u32();
    const float halfSize = 0.5f * particle.mLineLengthOrSize;
    const float theta = CRelAngle::FromDegrees(particle.mLineWidthOrRota).AsRadians();
    const float sinT = CMath::FastSinR(theta) * halfSize;
    const float cosT = CMath::FastCosR(theta) * halfSize;
    const float sinPlusCos = sinT + cosT;
    const float sinMinusCos = sinT - cosT;
    const float x = viewPos.GetX();
    const float y = viewPos.GetY();
    const float z = viewPos.GetZ();

    GXPosition3f32(x + sinPlusCos, y, z - sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    GXPosition3f32(x + sinMinusCos, y, z + sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    GXPosition3f32(x - sinPlusCos, y, z + sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    GXPosition3f32(x - sinMinusCos, y, z - sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::ForceParticleCreation(int count) {
  CParticleGlobals::SParticleSystem system('PART', this);
  CParticleGlobals::SetEmitterTime(mCurFrame);
  CreateNewParticles(count);
}

void CElementGen::EndLifetime() {
  mPSLT = 0;
  for (int i = 0; i < mActivePartChildren.size(); ++i) {
    CParticleGen* child = mActivePartChildren[i];
    if (child->Get4CharId() == 'PART') {
      static_cast< CElementGen* >(child)->EndLifetime();
    } else {
      child->SetParticleEmission(false);
    }
  }
}

void CElementGen::DestroyParticles() {
  sParticleAliveCount -= mParticles.size();
  mParticles.clear();
  for (int i = 0; i < mActivePartChildren.size(); ++i) {
    mActivePartChildren[i]->DestroyParticles();
  }
  mActiveParticleCount = 0;
  mRecursiveParticleCount = GetParticleCountAllInternal();
}

bool CElementGen::IsSystemDeletable() {
  for (int i = 0; i < mActivePartChildren.size(); ++i) {
    if (!mActivePartChildren[i]->IsSystemDeletable()) {
      return false;
    }
  }
  return mCurFrame > mPSLT && mActiveParticleCount == 0;
}

int CElementGen::GetParticleCountAll() const { return mRecursiveParticleCount; }

int CElementGen::GetParticleCountAllInternal() const {
  int count = mActiveParticleCount;
  for (int i = 0; i < mActivePartChildren.size(); ++i) {
    CParticleGen* child = mActivePartChildren[i];
    if (child->Get4CharId() == 'PART') {
      count += static_cast< CElementGen* >(child)->GetParticleCountAll();
    } else {
      count += child->GetParticleCount();
    }
  }
  return count;
}

int CElementGen::GetSystemCount() {
  int count = static_cast< int >(mActiveParticleCount) > 0;
  for (int i = 0; i < mActivePartChildren.size(); ++i) {
    count += mActivePartChildren[i]->GetSystemCount();
  }
  return count;
}

void CElementGen::AccumulateBounds(const CVector3f& position, float size) {
  for (int i = 0; i < 3; ++i) {
    if (position[i] > mAabbMax[i]) {
      mAabbMax[i] = position[i];
    }
    if (position[i] < mAabbMin[i]) {
      mAabbMin[i] = position[i];
    }
  }
  if (size > mMaxSize) {
    mMaxSize = size;
  }
}

void CElementGen::BuildParticleSystemBounds() {
  CAABox childBounds = CAABox::MakeMaxInvertedBox();
  bool accumulated = false;
  if (mEnableDynamicBounds) {
    for (int i = 0; i < mActivePartChildren.size(); ++i) {
      rstl::optional_object< CAABox > bounds = mActivePartChildren[i]->GetBounds();
      if (bounds) {
        accumulated = true;
        childBounds.AccumulateBounds(bounds->GetMinPoint());
        childBounds.AccumulateBounds(bounds->GetMaxPoint());
      }
    }
  }
  mRecursiveParticleCount = GetParticleCountAllInternal();

  if ((mEnableDynamicBounds && GetParticleCount() > 0) || mEnableFixedBounds) {
    const CVector3f extent = (mMaxSize * 0.5f) * mGlobalScale;
    const CAABox box = CAABox(mAabbMin, mAabbMax)
                           .GetTransformedAABox((mGlobalScaleTransform * mGlobalOrientation) *
                                                mLocalScaleTransform);
    mSystemBounds = CAABox(box.GetMinPoint() + mGlobalTranslation - extent,
                           box.GetMaxPoint() + mGlobalTranslation + extent);
  } else {
    mSystemBounds = CAABox::MakeMaxInvertedBox();
  }
  if (accumulated) {
    mSystemBounds.AccumulateBounds(childBounds.GetMinPoint());
    mSystemBounds.AccumulateBounds(childBounds.GetMaxPoint());
  }
}

rstl::optional_object< CAABox > CElementGen::GetBounds() {
  if (GetParticleCountAll() <= 0) {
    return rstl::optional_object< CAABox >();
  }
  return rstl::optional_object< CAABox >(mSystemBounds);
}

bool CElementGen::SystemHasLight() { return mLightType != kLT_None; }

CLight CElementGen::GetLight() {
  switch (mLightType) {
  case kLT_Directional:
    return CLight::BuildDirectional(mLDIR.AsNormalized(),
                                    CColor(rstl::min_val(1.f, mLINT * mLCLR.GetRed()),
                                           rstl::min_val(1.f, mLINT * mLCLR.GetGreen()),
                                           rstl::min_val(1.f, mLINT * mLCLR.GetBlue()),
                                           rstl::min_val(1.f, mLINT * mLCLR.GetAlpha())));
  case kLT_Spot: {
    CLight light = CLight::BuildSpot(mLOFF, mLDIR.AsNormalized(),
                                     CColor(rstl::min_val(1.f, mLINT * mLCLR.GetRed()),
                                            rstl::min_val(1.f, mLINT * mLCLR.GetGreen()),
                                            rstl::min_val(1.f, mLINT * mLCLR.GetBlue()),
                                            rstl::min_val(1.f, mLINT * mLCLR.GetAlpha())),
                                     mLSLA);
    const float quadratic = mFalloffType == kFT_Quadratic ? mLFOR : 0.f;
    const float linear = mFalloffType == kFT_Linear ? mLFOR : 0.f;
    const float constant = mFalloffType == kFT_Constant ? 1.f : 0.f;
    light.SetAttenuation(constant, linear, quadratic);
    return light;
  }
  default: {
    const float quadratic = mFalloffType == kFT_Quadratic ? mLFOR : 0.f;
    const float linear = mFalloffType == kFT_Linear ? mLFOR : 0.f;
    const float constant = mFalloffType == kFT_Constant ? 1.f : 0.f;
    return CLight::BuildCustom(mLOFF, CVector3f::Right(), mLCLR, constant, linear, quadratic, mLINT,
                               0.f, 0.f);
  }
  }
}

float CElementGen::GetGenerationRate() {
  float rate = 0.f;
  if (mLoadedGenDesc->mGRTE->GetValue(mCurFrame, rate)) {
    return 0.f;
  }
  return rstl::max_val(0.f, rate * mGeneratorRate);
}

void CElementGen::SetGeneratorRate(float rate) {
  mGeneratorRate = rstl::max_val(rate, 0.f);
  for (int i = 0; i < mActivePartChildren.size(); ++i) {
    CParticleGen* child = mActivePartChildren[i];
    if (child->Get4CharId() == 'PART') {
      child->SetGeneratorRate(mGeneratorRate);
    }
  }
}

float CElementGen::GetGeneratorRate() const { return mGeneratorRate; }

int CElementGen::GetNumSpawnedParticleSystems() const { return mActivePartChildren.size(); }

CParticleGen* CElementGen::SpawnedParticleSystem(int index) { return mActivePartChildren[index]; }

void CElementGen::SetExternalParam(uint index, float value) { mExternalVars[index] = value; }

float CElementGen::GetExternalVar(int index) const { return mExternalVars[index]; }

bool CElementGen::IsIndirectTextured() const {
  return mLoadedGenDesc->mTEXR != nullptr && mLoadedGenDesc->mTIND != nullptr;
}

const CElementGen::CAdvancedValues* CElementGen::GetParticleAdditionalData(uint index) const {
  return mEnableADV ? &mAdvValues[index] : nullptr;
}

CElementGen::CAdvancedValues* CElementGen::ParticleAdditionalData(uint index) {
  return mEnableADV ? &mAdvValues[index] : nullptr;
}

uint CElementGen::Get4CharId() const { return 'PART'; }

const CVector3f& CElementGen::GetGlobalScale() const { return mGlobalScale; }

const CTransform4f& CElementGen::GetGlobalOrientation() const { return mGlobalOrientation; }

const CTransform4f& CElementGen::GetOrientation() const { return mOrientation; }

const CVector3f& CElementGen::GetGlobalTranslation() const { return mGlobalTranslation; }
