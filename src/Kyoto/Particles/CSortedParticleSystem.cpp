#include "Kyoto/Particles/CSortedParticleSystem.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Particles/CSortedParticleSystemDescription.hpp"
#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"

ushort CSortedParticleSystem::sSeed = 99;

static const CVector3f skOne(1.f, 1.f, 1.f);

CSortedParticleSystem::CSortedParticleSystem(TToken< CSortedParticleSystemDescription > description,
                                             CElementGen::EOptionalSystemFlags flags,
                                             bool modelsUseLights)
: mDescription(description)
, mRandom(sSeed)
, mCurFrame(0)
, mCurSeconds(0.0)
, mGeneratorRate(1.f)
, mParticleCount(0)
, mPrevFrame(-1)
, mOptionalFlags(flags)
, mModelsUseLights(modelsUseLights)
, mParticleEmission(true)
, mGlobalTranslation(CVector3f::Zero())
, mGlobalScale(CVector3f::Zero())
, mGlobalOrientation(CTransform4f::Identity())
, mBounds(CAABox::MakeMaxInvertedBox()) {}

CSortedParticleSystem::~CSortedParticleSystem() {
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    delete *it;
  }
}

const bool CSortedParticleSystem::Update(double dt) {
  CGlobalRandom random(mRandom);
  bool updated = false;
  double frameTime = mCurFrame * (1.0 / 60.0);
  mCurSeconds += dt;
  while (frameTime < mCurSeconds) {
    CParticleGlobals::SetEmitterTime(mCurFrame);
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

void CSortedParticleSystem::Render() {
  if (!mChildren.empty()) {
    CElementGen* visible[8];
    int count = 0;
    for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
         it != mChildren.end(); ++it) {
      if ((*it)->ShouldDraw()) {
        visible[count++] = static_cast< CElementGen* >(*it);
      }
    }
    CElementGen::RenderParticlesFlameThrower(visible, count, &mGlobalTranslation,
                                             &mGlobalOrientation, &mGlobalScale, nullptr);
  }
}

void CSortedParticleSystem::SetOrientation(const CTransform4f& orientation) {
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    (*it)->SetOrientation(orientation);
  }
}

void CSortedParticleSystem::SetTranslation(const CVector3f& translation) {
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    (*it)->SetTranslation(translation);
  }
}

void CSortedParticleSystem::SetGlobalOrientation(const CTransform4f& orientation) {
  mGlobalOrientation = orientation;
}

void CSortedParticleSystem::SetGlobalTranslation(const CVector3f& translation) {
  mGlobalTranslation = translation;
}

void CSortedParticleSystem::SetGlobalScale(const CVector3f& scale) { mGlobalScale = scale; }

void CSortedParticleSystem::SetLocalScale(const CVector3f& scale) {
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    (*it)->SetLocalScale(scale);
  }
}

void CSortedParticleSystem::SetParticleEmission(bool emission) {
  mParticleEmission = emission;
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    (*it)->SetParticleEmission(emission);
  }
}

void CSortedParticleSystem::SetModulationColor(const CColor& color) {
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    (*it)->SetModulationColor(color);
  }
}

void CSortedParticleSystem::SetGeneratorRate(float rate) {
  mGeneratorRate = rate;
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    (*it)->SetGeneratorRate(rate);
  }
}

const CTransform4f& CSortedParticleSystem::GetOrientation() const {
  if (mChildren.empty()) {
    return CTransform4f::Identity();
  }
  return mChildren.front()->GetOrientation();
}

const CVector3f& CSortedParticleSystem::GetTranslation() const {
  if (mChildren.empty()) {
    return CVector3f::Zero();
  }
  return mChildren.front()->GetTranslation();
}

const CTransform4f& CSortedParticleSystem::GetGlobalOrientation() const {
  return mGlobalOrientation;
}

const CVector3f& CSortedParticleSystem::GetGlobalTranslation() const { return mGlobalTranslation; }

const CVector3f& CSortedParticleSystem::GetGlobalScale() const { return mGlobalScale; }

const CColor& CSortedParticleSystem::GetModulationColor() const {
  if (mChildren.empty()) {
    return CColor::White();
  }
  return mChildren.front()->GetModulationColor();
}

float CSortedParticleSystem::GetGeneratorRate() const { return mGeneratorRate; }

bool CSortedParticleSystem::IsSystemDeletable() {
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    if (!(*it)->IsSystemDeletable()) {
      return false;
    }
  }
  return true;
}

rstl::optional_object< CAABox > CSortedParticleSystem::GetBounds() {
  if (GetParticleCount() <= 0) {
    return rstl::optional_object< CAABox >();
  }
  return mBounds;
}

int CSortedParticleSystem::GetSystemCount() {
  int count = GetParticleCount() > 0 ? 1 : 0;
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    count += (*it)->GetSystemCount();
  }
  return count;
}

bool CSortedParticleSystem::SystemHasLight() { return false; }

CLight CSortedParticleSystem::GetLight() {
  return CLight::BuildLocalAmbient(CVector3f::Zero(), CColor::White());
}

void CSortedParticleSystem::DestroyParticles() {
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    (*it)->DestroyParticles();
  }
}

void CSortedParticleSystem::BuildParticleSystemBounds() {
  mParticleCount = 0;
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    CParticleGen* child = *it;
    mParticleCount += child->Get4CharId() == 'PART'
                          ? static_cast< CElementGen* >(child)->GetParticleCountAll()
                          : child->GetParticleCount();
  }
  mBounds = CAABox::MakeMaxInvertedBox();
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    rstl::optional_object< CAABox > bounds = (*it)->GetBounds();
    if (bounds) {
      const CAABox& box = *bounds;
      mBounds.AccumulateBounds(box.GetMinPoint());
      mBounds.AccumulateBounds(box.GetMaxPoint());
    }
  }
}

void CSortedParticleSystem::UpdateChildParticleSystems(double dt) {
  if (close_enough(dt, 0.0, 1e-7)) {
    return;
  }
  CGlobalRandom random(mRandom);
  if (mDescription->mSPWN && mPrevFrame != mCurFrame &&
      uint(mCurFrame) <= mDescription->mSPWN->GetEndFrame()) {
    rstl::vector< CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo >& spawns =
        mDescription->mSPWN->GetSpawnedSystemsAtFrame(mCurFrame);
    if (!spawns.empty()) {
      const ushort backupSeed = sSeed;
      for (int i = 0; i < spawns.size(); ++i) {
        CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo& info = spawns[i];
        const ushort seed = (i + mCurFrame + 1) * 100 + mRandom.GetSeed();
        if (info.GetType() == 'PART' && mChildren.size() < mChildren.capacity()) {
          CParticleGen* child = CElementGen::ConstructChildParticleSystem(
              *info.GetToken(), info.GetType(), seed, mOptionalFlags, mModelsUseLights,
              mParticleEmission, CVector3f::Zero(), CTransform4f::Identity(), mGlobalTranslation,
              mGlobalOrientation, mGlobalScale, CColor::White(), CVector3f::One());
          if (child) {
            mChildren.push_back(child);
          }
        }
      }
      sSeed = backupSeed;
    }
  }

  rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
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

uint CSortedParticleSystem::Get4CharId() const { return 'SRSC'; }

int CSortedParticleSystem::GetEmitterTime() const { return mCurFrame; }

int CSortedParticleSystem::GetParticleCount() { return mParticleCount; }

bool CSortedParticleSystem::GetParticleEmission() const { return mParticleEmission; }
