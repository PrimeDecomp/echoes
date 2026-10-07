#include "Kyoto/Particles/CParticleSpawnSystem.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Particles/CSpawnSystemDescription.hpp"
#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"

ushort CParticleSpawnSystem::sSeed = 99;

static const CVector3f skOne(1.f, 1.f, 1.f);

CParticleSpawnSystem::CParticleSpawnSystem(TToken< CSpawnSystemDescription > description,
                                           CElementGen::EOptionalSystemFlags flags,
                                           bool modelsUseLights)
: mDescription(description)
, mRandom(sSeed)
, mPrevFrame(-1)
, mCurFrame(0)
, mCurSeconds(0.0)
, mParticleCount(0)
, mOptionalFlags(flags)
, mModelsUseLights(modelsUseLights)
, mParticleEmission(true)
, mIgnoreGlobalTransform(false)
, mIgnoreLocalTransform(false)
, mTranslationMode(kTM_Local)
, mVelocity(CVector3f::Zero())
, mTranslation(CVector3f::Zero())
, mOrientation(CTransform4f::Identity())
, mLocalScale(skOne)
, mOrientationInverse(CMatrix3f::Identity())
, mGlobalTranslation(CVector3f::Zero())
, mGlobalOrientation(CTransform4f::Identity())
, mGlobalScale(skOne)
, mTranslationOffset(CVector3f::Zero())
, mOrientationOffset(CTransform4f::Identity())
, mLocalScaleMultiplier(skOne)
, mGlobalTranslationOffset(CVector3f::Zero())
, mGlobalOrientationOffset(CTransform4f::Identity())
, mGlobalScaleMultiplier(skOne)
, mBillboardAxisTransform(CTransform4f::Identity())
, mBillboardAxis(CVector3f::Up())
, mParticleColor(CColor::White())
, mModulationColor(CColor::White())
, mBounds(CAABox::MakeMaxInvertedBox()) {
  CGlobalRandom random(mRandom);
  uint sourceCount = 0;
  if (mDescription->mVLM1) {
    mVelocitySources[sourceCount] = mDescription->mVLM1;
    mVelocitySourceLocal[sourceCount++] = mDescription->mVMD1;
  }
  if (mDescription->mVLM2) {
    mVelocitySources[sourceCount] = mDescription->mVLM2;
    mVelocitySourceLocal[sourceCount++] = mDescription->mVMD2;
  }
  for (uint i = sourceCount; i < 2; ++i) {
    mVelocitySources[i] = nullptr;
  }
  if (mDescription->mIVEC) {
    mDescription->mIVEC->GetValue(mCurFrame, mVelocity);
  }
  if (mDescription->mGIVL) {
    int mode = 0;
    mDescription->mGIVL->GetValue(mCurFrame, mode);
    if (mode >= kTM_None && mode <= kTM_Global) {
      mTranslationMode = ETranslationMode(mode);
    }
  }
  if (mDescription->mPSLT) {
    mDescription->mPSLT->GetValue(mCurFrame, mLifetime);
  } else {
    mLifetime = 0x7fffff;
  }
  mIgnoreGlobalTransform = mDescription->mIGGT;
  mIgnoreLocalTransform = mDescription->mIGLT;
}

CParticleSpawnSystem::~CParticleSpawnSystem() {
  for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
       ++it) {
    delete *it;
  }
}

const bool CParticleSpawnSystem::Update(double dt) {
  CGlobalRandom random(mRandom);
  if (mCurFrame > mLifetime && !mChildren.empty()) {
    if (mDescription->mDEOL) {
      for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
           ++it) {
        delete *it;
      }
      mChildren.clear();
    } else {
      EndLifetime();
    }
  }

  bool updated = false;
  mCurSeconds += dt;
  double frameTime = mCurFrame * (1.0 / 60.0);
  while (mCurSeconds > frameTime) {
    CParticleGlobals::SetEmitterTime(mCurFrame);
    for (int i = 0; i < 2 && mVelocitySources[i]; ++i) {
      UpdateVelocitySource(i);
    }
    mTranslationOffset += mVelocity;
    if (mDescription->mPCOL) {
      mDescription->mPCOL->GetValue(mCurFrame, mParticleColor);
      const CColor color = CColor::Modulate(mParticleColor, mModulationColor);
      for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
           ++it) {
        (*it)->SetModulationColor(color);
      }
    }
    if (mDescription->mSCLE) {
      mDescription->mSCLE->GetValue(mCurFrame, mGlobalScaleMultiplier);
      const CVector3f scale = CVector3f::ByElementMultiply(mGlobalScaleMultiplier, mGlobalScale);
      for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
           ++it) {
        (*it)->SetGlobalScale(scale);
      }
    }
    if (mDescription->mLSCL) {
      mDescription->mLSCL->GetValue(mCurFrame, mLocalScaleMultiplier);
      const CVector3f scale = CVector3f::ByElementMultiply(mLocalScaleMultiplier, mLocalScale);
      for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
           ++it) {
        (*it)->SetLocalScale(scale);
      }
    }
    if (mDescription->mFRCO && mDescription->mFROV) {
      mDescription->mFROV->GetValue(mCurFrame, mBillboardAxis);
    }
    UpdateTranslation(true, false, nullptr);
    UpdateOrientation(true, false, nullptr);
    UpdateGlobalTranslation(true, false, nullptr);
    UpdateGlobalOrientation(true, false, nullptr);
    UpdateChildParticleSystems(1.0 / 60.0);
    frameTime += 1.0 / 60.0;
    updated = true;
    ++mCurFrame;
  }
  if (updated) {
    BuildParticleSystemBounds();
  }
  return updated;
}

void CParticleSpawnSystem::Render() {
  if (mDescription->mFRCO) {
    rstl::optional_object< CTransform4f > orientation;
    const CTransform4f& view = CGraphics::GetViewMatrix();
    if (mDescription->mFROV) {
      const CVector3f axis = mBillboardAxisTransform * mBillboardAxis;
      CVector3f right = CVector3f::Cross(axis, view.GetForward());
      float magnitudeSquared = right.MagSquared();
      if (magnitudeSquared > FLT_EPSILON) {
        right *= CMath::FastInvSqrtF(magnitudeSquared);
      } else {
        right =
            CVector3f::Cross(axis, (view.GetTranslation() - GetGlobalTranslation()).AsNormalized());
        magnitudeSquared = right.MagSquared();
        if (magnitudeSquared > FLT_EPSILON) {
          right *= CMath::FastInvSqrtF(magnitudeSquared);
        }
      }
      orientation =
          CTransform4f::FromColumns(right, -1.f * view.GetForward(), axis, CVector3f::Zero()) *
          mGlobalOrientationOffset;
    } else {
      orientation = CTransform4f::FromColumns(-1.f * view.GetRight(), -1.f * view.GetForward(),
                                              view.GetUp(), CVector3f::Zero()) *
                    mGlobalOrientationOffset;
    }
    for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
         ++it) {
      if ((*it)->Get4CharId() == 'SPSC') {
        static_cast< CParticleSpawnSystem* >(*it)->ForceSetGlobalOrientation(*orientation);
      } else {
        (*it)->SetGlobalOrientation(*orientation);
      }
    }
  }
  for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
       ++it) {
    (*it)->Render();
  }
}

void CParticleSpawnSystem::UpdateGlobalTranslation(bool evaluate, bool force,
                                                   const CVector3f* translation) {
  if (mTranslationMode == kTM_Global || force) {
    if (translation) {
      mGlobalTranslation = *translation;
    }
    if (evaluate && mDescription->mGTRN) {
      mDescription->mGTRN->GetValue(mCurFrame, mGlobalTranslationOffset);
    }
    const CVector3f result = mGlobalTranslation + GetGlobalTranslationOffset();
    for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
         ++it) {
      if (force && (*it)->Get4CharId() == 'SPSC') {
        static_cast< CParticleSpawnSystem* >(*it)->ForceSetGlobalTranslation(result);
      } else {
        (*it)->SetGlobalTranslation(result);
      }
    }
  }
}

void CParticleSpawnSystem::UpdateGlobalOrientation(bool evaluate, bool force,
                                                   const CTransform4f* orientation) {
  if (evaluate && mDescription->mGORN && (mDescription->mFRCO || !mIgnoreGlobalTransform)) {
    CVector3f rotation = CVector3f::Zero();
    mDescription->mGORN->GetValue(mCurFrame, rotation);
    mGlobalOrientationOffset = CTransform4f::RotateZ(CRelAngle::FromDegrees(rotation.GetZ()));
    mGlobalOrientationOffset.RotateLocalY(CRelAngle::FromDegrees(rotation.GetY()));
    mGlobalOrientationOffset.RotateLocalX(CRelAngle::FromDegrees(rotation.GetX()));
  }
  if (!mDescription->mFRCO) {
    if (!mIgnoreGlobalTransform) {
      if (orientation) {
        mGlobalOrientation = *orientation;
      }
      const CTransform4f result = mGlobalOrientation * mGlobalOrientationOffset;
      for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
           ++it) {
        if (force && (*it)->Get4CharId() == 'SPSC') {
          static_cast< CParticleSpawnSystem* >(*it)->ForceSetGlobalOrientation(result);
        } else {
          (*it)->SetGlobalOrientation(result);
        }
      }
    }
  } else if (mDescription->mFROV && orientation) {
    mBillboardAxisTransform = *orientation;
  }
}

void CParticleSpawnSystem::UpdateTranslation(bool evaluate, bool force,
                                             const CVector3f* translation) {
  if (mTranslationMode == kTM_Local || force) {
    if (translation) {
      mTranslation = *translation;
    }
    if (evaluate && mDescription->mTRNL) {
      mDescription->mTRNL->GetValue(mCurFrame, mTranslationOffset);
    }
    if (mTranslationMode != kTM_None) {
      const CVector3f result = mTranslation + GetTranslationOffset();
      for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
           ++it) {
        if (force && (*it)->Get4CharId() == 'SPSC') {
          static_cast< CParticleSpawnSystem* >(*it)->ForceSetTranslation(result);
        } else {
          (*it)->SetTranslation(result);
        }
      }
    }
  }
}

void CParticleSpawnSystem::UpdateOrientation(bool evaluate, bool force,
                                             const CTransform4f* orientation) {
  if (mTranslationMode != kTM_None || force) {
    if (orientation) {
      mOrientation = *orientation;
    }
    if (evaluate && mDescription->mORNT) {
      CVector3f rotation = CVector3f::Zero();
      mDescription->mORNT->GetValue(mCurFrame, rotation);
      mOrientationOffset = CTransform4f::RotateZ(CRelAngle::FromDegrees(rotation.GetZ()));
      mOrientationOffset.RotateLocalY(CRelAngle::FromDegrees(rotation.GetY()));
      mOrientationOffset.RotateLocalX(CRelAngle::FromDegrees(rotation.GetX()));
    }
    const CTransform4f result = mOrientation * mOrientationOffset;
    mOrientationInverse = result.GetQuickInverse().BuildMatrix3f();
    for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
         ++it) {
      if (force && (*it)->Get4CharId() == 'SPSC') {
        static_cast< CParticleSpawnSystem* >(*it)->ForceSetOrientation(result);
      } else {
        (*it)->SetOrientation(result);
      }
    }
  }
}

void CParticleSpawnSystem::SetOrientation(const CTransform4f& orientation) {
  if (!mIgnoreLocalTransform) {
    UpdateOrientation(false, false, &orientation);
  }
}

void CParticleSpawnSystem::SetTranslation(const CVector3f& translation) {
  if (!mIgnoreLocalTransform) {
    UpdateTranslation(false, false, &translation);
  }
}

void CParticleSpawnSystem::SetGlobalOrientation(const CTransform4f& orientation) {
  if (!mIgnoreGlobalTransform) {
    UpdateGlobalOrientation(false, false, &orientation);
  }
}

void CParticleSpawnSystem::SetGlobalTranslation(const CVector3f& translation) {
  if (!mIgnoreGlobalTransform) {
    UpdateGlobalTranslation(false, false, &translation);
  }
}

void CParticleSpawnSystem::SetGlobalScale(const CVector3f& scale) {
  mGlobalScale = scale;
  const CVector3f result = CVector3f::ByElementMultiply(mGlobalScaleMultiplier, mGlobalScale);
  for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
       ++it) {
    (*it)->SetGlobalScale(result);
  }
}

void CParticleSpawnSystem::SetLocalScale(const CVector3f& scale) {
  mLocalScale = scale;
  const CVector3f result = CVector3f::ByElementMultiply(mLocalScaleMultiplier, mLocalScale);
  for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
       ++it) {
    (*it)->SetLocalScale(result);
  }
}

void CParticleSpawnSystem::ForceSetOrientation(const CTransform4f& orientation) {
  UpdateOrientation(false, true, &orientation);
}

void CParticleSpawnSystem::ForceSetTranslation(const CVector3f& translation) {
  UpdateTranslation(false, true, &translation);
}

void CParticleSpawnSystem::ForceSetGlobalOrientation(const CTransform4f& orientation) {
  UpdateGlobalOrientation(false, true, &orientation);
}

void CParticleSpawnSystem::ForceSetGlobalTranslation(const CVector3f& translation) {
  UpdateGlobalTranslation(false, true, &translation);
}

void CParticleSpawnSystem::SetParticleEmission(bool emission) {
  mParticleEmission = emission;
  for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
       ++it) {
    (*it)->SetParticleEmission(emission);
  }
}

void CParticleSpawnSystem::SetModulationColor(const CColor& color) {
  mModulationColor = color;
  const CColor result = CColor::Modulate(mParticleColor, mModulationColor);
  for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
       ++it) {
    (*it)->SetModulationColor(result);
  }
}

const CTransform4f& CParticleSpawnSystem::GetOrientation() const { return mOrientation; }

const CVector3f& CParticleSpawnSystem::GetTranslation() const { return mTranslation; }

const CTransform4f& CParticleSpawnSystem::GetGlobalOrientation() const {
  return mGlobalOrientation;
}

const CVector3f& CParticleSpawnSystem::GetGlobalTranslation() const { return mGlobalTranslation; }

const CVector3f& CParticleSpawnSystem::GetGlobalScale() const { return mGlobalScale; }

const CColor& CParticleSpawnSystem::GetModulationColor() const { return mModulationColor; }

bool CParticleSpawnSystem::IsSystemDeletable() {
  const rstl::vector< CParticleGen* >::iterator end = mChildren.end();
  for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != end; ++it) {
    if (!(*it)->IsSystemDeletable()) {
      return false;
    }
  }
  if (mCurFrame <= mLifetime && mDescription->mSPWN &&
      uint(mCurFrame) <= mDescription->mSPWN->GetEndFrame()) {
    return false;
  }
  return true;
}

rstl::optional_object< CAABox > CParticleSpawnSystem::GetBounds() {
  if (GetParticleCount() <= 0) {
    return rstl::optional_object< CAABox >();
  }
  if (mDescription->mFRCO) {
    const CVector3f extent = mBounds.GetMaxPoint() - mBounds.GetMinPoint();
    const CVector3f center = mBounds.GetCenterPoint();
    return CAABox(center - extent, center + extent);
  }
  return mBounds;
}

int CParticleSpawnSystem::GetSystemCount() {
  int count = GetParticleCount() > 0 ? 1 : 0;
  const rstl::vector< CParticleGen* >::iterator end = mChildren.end();
  for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != end; ++it) {
    count += (*it)->GetSystemCount();
  }
  return count;
}

bool CParticleSpawnSystem::SystemHasLight() { return false; }

CLight CParticleSpawnSystem::GetLight() {
  return CLight::BuildLocalAmbient(CVector3f::Zero(), CColor::White());
}

void CParticleSpawnSystem::DestroyParticles() {
  for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
       ++it) {
    (*it)->DestroyParticles();
  }
}

void CParticleSpawnSystem::BuildParticleSystemBounds() {
  mParticleCount = 0;
  for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
       ++it) {
    CParticleGen* child = *it;
    mParticleCount += child->Get4CharId() == 'PART'
                          ? static_cast< CElementGen* >(child)->GetParticleCountAll()
                          : child->GetParticleCount();
  }
  mBounds = CAABox::MakeMaxInvertedBox();
  for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
       ++it) {
    rstl::optional_object< CAABox > bounds = (*it)->GetBounds();
    if (bounds) {
      const CAABox& box = *bounds;
      mBounds.AccumulateBounds(box.GetMinPoint());
      mBounds.AccumulateBounds(box.GetMaxPoint());
    }
  }
}

CParticleGen* CParticleSpawnSystem::ConstructChildParticleSystem(const CToken& description,
                                                                 uint type, ushort seed) {
  const CVector3f translation = mTranslation + GetTranslationOffset();
  const CTransform4f orientation = mOrientation * mOrientationOffset;
  const CVector3f globalTranslation = mGlobalTranslation + GetGlobalTranslationOffset();
  const CTransform4f globalOrientation = mGlobalOrientation * mGlobalOrientationOffset;
  const CVector3f globalScale = CVector3f::ByElementMultiply(mGlobalScaleMultiplier, mGlobalScale);
  const CVector3f localScale = CVector3f::ByElementMultiply(mLocalScaleMultiplier, mLocalScale);
  const CColor color = CColor::Modulate(mParticleColor, mModulationColor);
  return CElementGen::ConstructChildParticleSystem(
      description, type, seed, mOptionalFlags, mModelsUseLights, mParticleEmission, translation,
      orientation, globalTranslation, globalOrientation, globalScale, color, localScale);
}

void CParticleSpawnSystem::UpdateChildParticleSystems(double dt) {
  if (close_enough(dt, 0.0, 1e-7)) {
    return;
  }
  CGlobalRandom random(mRandom);
  if (mCurFrame < mLifetime && mDescription->mSPWN && mPrevFrame != mCurFrame &&
      uint(mCurFrame) <= mDescription->mSPWN->GetEndFrame()) {
    rstl::vector< CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo >& spawns =
        mDescription->mSPWN->GetSpawnedSystemsAtFrame(mCurFrame);
    if (!spawns.empty()) {
      const ushort backupSeed = sSeed;
      mChildren.reserve(spawns.size() + mChildren.size());
      for (int i = 0; i < spawns.size(); ++i) {
        CParticleGen* child =
            ConstructChildParticleSystem(*spawns[i].GetToken(), spawns[i].GetType(),
                                         (i + mCurFrame + 1) * 100 + mRandom.GetSeed());
        if (child) {
          mChildren.push_back_unsafe(child);
        }
      }
      sSeed = backupSeed;
    }
  }

  rstl::vector< CParticleGen* >::iterator it = mChildren.begin();
  while (it != mChildren.end()) {
    CParticleGen* child = *it;
    child->Update(dt);
    if (child->IsSystemDeletable()) {
      delete child;
      it = mChildren.erase(it);
    } else {
      ++it;
    }
  }
  mPrevFrame = mCurFrame;
}

void CParticleSpawnSystem::UpdateVelocitySource(int index) {
  if (mVelocitySourceLocal[index]) {
    CVector3f velocity = mOrientationInverse * mVelocity;
    CVector3f position = mOrientationInverse * mTranslationOffset;
    mVelocitySources[index]->GetValue(mCurFrame, velocity, position);
    const CTransform4f orientation = mOrientation * mOrientationOffset;
    mVelocity = orientation.Rotate(velocity);
    mTranslationOffset = orientation.Rotate(position);
  } else {
    mVelocitySources[index]->GetValue(mCurFrame, mVelocity, mTranslationOffset);
  }
}

CVector3f CParticleSpawnSystem::GetTranslationOffset() const {
  switch (mTranslationMode) {
  case kTM_Global:
    return CVector3f::Zero();
  case kTM_None:
  case kTM_Local:
    return mTranslationOffset;
  }
  return CVector3f::Zero();
}

CVector3f CParticleSpawnSystem::GetGlobalTranslationOffset() const {
  switch (mTranslationMode) {
  case kTM_None:
    return CVector3f::Zero();
  case kTM_Local:
    return mGlobalTranslationOffset;
  case kTM_Global:
    return mGlobalOrientation.Rotate(mTranslationOffset);
  }
  return CVector3f::Zero();
}

void CParticleSpawnSystem::EndLifetime() {
  mLifetime = 0;
  for (rstl::vector< CParticleGen* >::iterator it = mChildren.begin(); it != mChildren.end();
       ++it) {
    CParticleGen* child = *it;
    if (child->Get4CharId() == 'PART') {
      static_cast< CElementGen* >(child)->EndLifetime();
    } else {
      child->SetParticleEmission(false);
    }
  }
}

uint CParticleSpawnSystem::Get4CharId() const { return 'SPSC'; }

int CParticleSpawnSystem::GetEmitterTime() const { return mCurFrame; }

int CParticleSpawnSystem::GetParticleCount() { return mParticleCount; }

bool CParticleSpawnSystem::GetParticleEmission() const { return mParticleEmission; }
