#include "MetroidPrime/CParticleGenInfoGeneric.hpp"

#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/TCastTo.hpp"

CParticleGenInfoGeneric::CParticleGenInfoGeneric(const SObjectTag& part,
                                                 rstl::ncrc_ptr< CParticleGen > system, int frames,
                                                 CSegId segment, CVector3f scale,
                                                 CParticleData::EParentedMode parentMode, int flags,
                                                 CStateManager* stateMgr, TAreaId areaId,
                                                 uint lightId, EParticleGenType state)

: CParticleGenInfo(part, frames, segment, scale, parentMode, flags, state)
, mSystem(system)
, mLightId(lightId == 0xffffffff ? kInvalidUniqueId
                                 : _initializeLight(system, stateMgr, areaId, lightId)) {}

CParticleGenInfoGeneric::~CParticleGenInfoGeneric() {}

void CParticleGenInfoGeneric::AddToRenderer() { gpRender->AddParticleGen(*mSystem.GetPtr()); }

void CParticleGenInfoGeneric::Render() { mSystem->Render(); }

void CParticleGenInfoGeneric::Update(float dt, CStateManager* stateMgr) {
  mSystem->Update(dt);

  if (mLightId == kInvalidUniqueId) {
    return;
  }

  if (CGameLight* gl = TCastToPtr< CGameLight >(stateMgr->GetObjectByIdFromListAll(mLightId))) {
    gl->SetLight(mSystem->GetLight());
  }
}

void CParticleGenInfoGeneric::SetOrientation(const CTransform4f& xf, CStateManager* stateMgr) {
  mSystem->SetOrientation(xf);

  if (mLightId == kInvalidUniqueId) {
    return;
  }

  if (CGameLight* gl = TCastToPtr< CGameLight >(stateMgr->GetObjectByIdFromListAll(mLightId))) {
    CMatrix3f m1 = xf.BuildMatrix3f();
    gl->SetTransformAlt(
        CQuaternion::FromMatrix(m1.Orthonormalized()).BuildTransform4f(gl->GetTranslation()));
  }
}

void CParticleGenInfoGeneric::SetTranslation(const CVector3f& vec, CStateManager* stateMgr) {
  mSystem->SetTranslation(vec);

  if (mLightId == kInvalidUniqueId) {
    return;
  }

  if (CGameLight* gl = TCastToPtr< CGameLight >(stateMgr->GetObjectByIdFromListAll(mLightId))) {
    gl->SetTranslation(vec);
  }
}

void CParticleGenInfoGeneric::SetGlobalOrientation(const CTransform4f& xf,
                                                   CStateManager* stateMgr) {
  mSystem->SetGlobalOrientation(xf);

  if (mLightId == kInvalidUniqueId) {
    return;
  }

  if (CGameLight* gl = TCastToPtr< CGameLight >(stateMgr->GetObjectByIdFromListAll(mLightId))) {
    gl->SetTransformAlt(CQuaternion::FromMatrix(xf).BuildTransform4f(gl->GetTranslation()));
  }
}

void CParticleGenInfoGeneric::SetGlobalTranslation(const CVector3f& vec, CStateManager* stateMgr) {
  mSystem->SetGlobalTranslation(vec);

  if (mLightId == kInvalidUniqueId) {
    return;
  }

  if (CGameLight* gl = TCastToPtr< CGameLight >(stateMgr->GetObjectByIdFromListAll(mLightId))) {
    gl->SetTranslation(vec);
  }
}

void CParticleGenInfoGeneric::SetGlobalScale(const CVector3f& vec) { mSystem->SetGlobalScale(vec); }

void CParticleGenInfoGeneric::SetParticleEmission(bool isActive, CStateManager* stateMgr) {
  mSystem->SetParticleEmission(isActive);

  if (mLightId == kInvalidUniqueId) {
    return;
  }

  if (CGameLight* gl = TCastToPtr< CGameLight >(stateMgr->GetObjectByIdFromListAll(mLightId))) {
    gl->SetActive(isActive);
  }
}

void CParticleGenInfoGeneric::DeleteLight(CStateManager* stateMgr) {
  if (mLightId == kInvalidUniqueId) {
    return;
  }

  stateMgr->DeleteObjectRequest(mLightId);
  mLightId = kInvalidUniqueId;
}

void CParticleGenInfoGeneric::SetModulationColor(const CColor& color) {
  if ((GetFlags() & 0x4000) == 0) {
    mSystem->SetModulationColor(color);
  }
}

TUniqueId CParticleGenInfoGeneric::GetLightId() const { return mLightId; }

bool CParticleGenInfoGeneric::HasLight() const { return mSystem->SystemHasLight(); }

void CParticleGenInfoGeneric::DestroyParticles() { mSystem->DestroyParticles(); }

bool CParticleGenInfoGeneric::HasActiveParticles() const { return mSystem->GetParticleCount() > 0; }

rstl::optional_object< CAABox > CParticleGenInfoGeneric::GetBounds() const {
  return mSystem->GetBounds();
}

bool CParticleGenInfoGeneric::IsSystemDeletable() const { return mSystem->IsSystemDeletable(); }
