#include "Kyoto/Particles/CDeferredParticleEffect.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "Kyoto/Particles/CParticleSpawnSystem.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"

CDeferredParticleEffect::CDeferredParticleEffect(const CToken& effect,
                                                 const CDependencyGroupToken& group)
: mEffect(effect)
, mTag(mEffect.GetTag())
, mDependencies(group)
, mGenerator(nullptr)
, x38_(0.2f)
, mIdleTime(0.f)
, x40_24_(false)
, mCreatePending(false)
, x40_26_(false) {}

CDeferredParticleEffect::~CDeferredParticleEffect() {}

bool CDeferredParticleEffect::Update(double dt) {
  if (mGenerator.get() != nullptr) {
    mGenerator->Update(dt);
    bool done = false;
    if (!x40_24_) {
      if (x40_26_) {
        done = mGenerator->IsSystemDeletable();
      } else {
        bool hasParticles = mGenerator->GetParticleCount() != 0;
        if (!hasParticles) {
          mIdleTime += static_cast< float >(dt);
        } else {
          mIdleTime = 0.f;
        }
        if (mIdleTime > x38_) {
          done = true;
        }
      }
      if (done) {
        mGenerator = nullptr;
        Unload();
      }
    }
  } else if (mCreatePending && mDependencies && mDependencies->IsLocked() &&
             mDependencies->IsLoaded()) {
    mCreatePending = false;
    CreateGenerator();
    mIdleTime = 0.f;
  }
  return false;
}

void CDeferredParticleEffect::Render() const {
  if (mGenerator.get() != nullptr) {
    mGenerator->Render();
  }
}

void CDeferredParticleEffect::SetOrientation(const CTransform4f& orientation) {
  if (mGenerator.get() != nullptr) {
    mGenerator->SetOrientation(orientation);
  }
}

void CDeferredParticleEffect::SetTranslation(const CVector3f& translation) {
  if (mGenerator.get() != nullptr) {
    mGenerator->SetTranslation(translation);
  }
}

void CDeferredParticleEffect::SetParticleEmission(bool emitting) {
  if (mGenerator.get() != nullptr) {
    mGenerator->SetParticleEmission(emitting);
  }
}

void CDeferredParticleEffect::SetGeneratorRate(float rate) {
  if (mGenerator.get() != nullptr) {
    mGenerator->SetGeneratorRate(rate);
  }
}

void CDeferredParticleEffect::Load(bool createGenerator) {
  if (mDependencies && !mDependencies->IsLocked()) {
    mDependencies->Lock();
  }
  if (mGenerator.get() == nullptr) {
    mCreatePending = createGenerator;
  } else {
    mCreatePending = false;
  }
}

void CDeferredParticleEffect::Unload() {
  if (mDependencies && mDependencies->IsLocked()) {
    mDependencies->Unlock();
  }
  mCreatePending = false;
}

void CDeferredParticleEffect::CreateGenerator() {
  CParticleGen* gen;
  switch (mTag.type) {
  case 'PART': {
    TLockedToken< CGenDescription > desc(mEffect);
    gen = rs_new CElementGen(desc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    break;
  }
  case 'SWHC': {
    TToken< CSwooshDescription > desc(mEffect);
    gen = rs_new CParticleSwoosh(desc, 0);
    break;
  }
  case 'ELSC': {
    TToken< CElectricDescription > desc(mEffect);
    gen = rs_new CParticleElectric(desc);
    break;
  }
  case 'SPSC': {
    TToken< CSpawnSystemDescription > desc(mEffect);
    gen = rs_new CParticleSpawnSystem(desc, CElementGen::kOSF_One, false);
    break;
  }
  case 'SRSC': {
    TToken< CSpawnRandomDescription > desc(mEffect);
    gen = rs_new CParticleSpawnRandom(desc, CElementGen::kOSF_One, false);
    break;
  }
  default:
    return;
  }
  mGenerator = gen;
}
