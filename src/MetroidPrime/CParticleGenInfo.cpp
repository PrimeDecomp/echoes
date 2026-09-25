#include "MetroidPrime/CParticleGenInfo.hpp"

#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"

CParticleGenInfo::CParticleGenInfo(const SObjectTag& part, int frameCount, CSegId segment,
                                   const CVector3f& scale, CParticleData::EParentedMode parentMode,
                                   int flags, EParticleGenType type)
: mPart(part)
, mSeconds(frameCount * (1.f / 60.f))
, mSegment(segment)
, mCurrentTime(0.f)
, mActive(false)
, mParentMode(parentMode)
, mFlags(flags)
, mParticleScale(scale)
, mFinishTime(0.f)
, mGrabInitialData(false)
, mTransform(CTransform4f::Identity())
, mOffset(CVector3f::Zero())
, mType(type) {}

TUniqueId _initializeLight(const rstl::ncrc_ptr< CParticleGen >& system, CStateManager* mgr,
                           TAreaId areaId, uint lightId) {
  if (!system->SystemHasLight() || mgr == nullptr) {
    return kInvalidUniqueId;
  }

  TUniqueId id = mgr->AllocateUniqueId();
  mgr->AddObject(rs_new CGameLight(
      id, areaId, false, rstl::string_l("ParticleLight"),
      CTransform4f(system->GetOrientation().BuildMatrix3f(), system->GetTranslation()),
      kInvalidUniqueId, system->GetLight(), lightId, 0, 0.f));
  return id;
}

CParticleGenInfo::~CParticleGenInfo() {}
