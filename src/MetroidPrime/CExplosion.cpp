#include "MetroidPrime/CExplosion.hpp"

#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "rstl/math.hpp"

CExplosion::CExplosion(const TLockedToken< CGenDescription >& particle, TUniqueId uid,
                       const CEntityInfo& info, const rstl::string& name, const CTransform4f& xf,
                       const uint flags, const CVector3f& scale, const CColor& color,
                       int playerIndex)
: CEffect(uid, info, name, xf)
, mParticleGen(rs_new CElementGen(TToken< CGenDescription >(particle), CElementGen::kMOT_Normal,
                                  flags & 1 ? CElementGen::kOSF_Two : CElementGen::kOSF_One))
, mExplosionLight(kInvalidUniqueId)
, mSourceId(CToken(particle).GetTag().GetId())
, mScale(scale)
, mFlags(flags)
, mPlayerIndex(playerIndex)
, mHasRenderBounds(true)
, mUnknownFlag(flags & 2)
, mFixedTimeStep(flags & 4)
, mTime(0.f) {
  mParticleGen->SetGlobalTranslation(xf.GetTranslation());
  mParticleGen->SetOrientation(xf.GetRotation());
  mParticleGen->SetGlobalScale(scale);
  mParticleGen->SetModulationColor(color);
}

CExplosion::CExplosion(const TLockedToken< CElectricDescription >& electric, TUniqueId uid,
                       const CEntityInfo& info, const rstl::string& name, const CTransform4f& xf,
                       uint flags, const CVector3f& scale, const CColor& color, int playerIndex)
: CEffect(uid, info, name, xf)
, mParticleGen(rs_new CParticleElectric(TToken< CElectricDescription >(electric)))
, mExplosionLight(kInvalidUniqueId)
, mSourceId(CToken(electric).GetTag().GetId())
, mScale(scale)
, mFlags(flags)
, mPlayerIndex(playerIndex)
, mHasRenderBounds(true)
, mUnknownFlag(flags & 2)
, mFixedTimeStep(flags & 4) {
  // The native electric constructor does not initialize mTime.
  mParticleGen->SetGlobalTranslation(xf.GetTranslation());
  mParticleGen->SetOrientation(xf.GetRotation());
  mParticleGen->SetGlobalScale(scale);
  mParticleGen->SetModulationColor(color);
}

CExplosion::~CExplosion() {}

void CExplosion::AddToRenderer(const CStateManager& mgr) const {
  if (!GetPreRenderClipped()) {
    EnsureRendered(mgr);
  }
}

void CExplosion::Render(const CStateManager&) const { mParticleGen->Render(); }

void CExplosion::PreRender(CStateManager& mgr) {
  SetPreRenderClipped(!mHasRenderBounds);
  if (GetPreRenderClipped()) {
    return;
  }

  CActor::PreRender(mgr);
  if (GetPreRenderClipped()) {
    return;
  }

  if (mPlayerIndex == mgr.GetCurrentRenderPlayerIndex() &&
      mgr.GetCurrentRenderCameraManager()->IsInFPCamera()) {
    SetPreRenderClipped(true);
  } else if (mFlags & 8) {
    const CGameCamera* camera =
        mgr.CameraManager(mgr.GetCurrentRenderPlayerIndex())->CurrentCamera(mgr, true);
    const float distance = (GetTranslation() - camera->GetTranslation()).Magnitude();
    float scale = rstl::min_val(4.f, rstl::max_val(0.2f, distance));
    scale *= 0.25f;
    mParticleGen->SetGlobalScale(mScale * scale);
  }
}

void CExplosion::Think(float dt, CStateManager& mgr) {
  if (GetTransformDirtySpare()) {
    mParticleGen->SetGlobalTranslation(GetTranslation());
    mParticleGen->SetOrientation(GetTransform().GetRotation());
    SetTransformDirtySpare(false);
  }
  mParticleGen->Update(mFixedTimeStep ? 1.0 / 60.0 : dt);

  if (mExplosionLight != kInvalidUniqueId) {
    CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mExplosionLight));
    if (light && GetActive()) {
      light->SetLight(mParticleGen->GetLight());
    }
  }

  mTime += dt;
  if (mTime > 15.f) {
    mgr.DeleteObjectRequest(GetUniqueId());
  } else {
    if (mParticleGen->IsSystemDeletable()) {
      mgr.DeleteObjectRequest(GetUniqueId());
    }
    CActor::Think(dt, mgr);
  }
}

void CExplosion::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  const TUniqueId sender = msg.GetUnk();
  switch (message) {
  case kSM_XCRT:
    if (mgr.GetNumPlayers() < 3u && mParticleGen->SystemHasLight()) {
      mExplosionLight = mgr.AllocateUniqueId();
      const uint sourceId = mSourceId;
      mgr.AddObject(rs_new CGameLight(mExplosionLight, GetCurrentAreaId(), GetActive(),
                                      rstl::string_l(""), GetTransform(), GetUniqueId(),
                                      mParticleGen->GetLight(), sourceId, 1, 0.f));
    }
    break;
  case kSM_XDelete:
    if (mExplosionLight != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mExplosionLight);
      mExplosionLight = kInvalidUniqueId;
    }
    break;
  default:
    break;
  }

  CActor::AcceptScriptMsg(mgr, msg);
  if (mExplosionLight != kInvalidUniqueId) {
    mgr.SendScriptMsg(mExplosionLight, sender, message, kInvalidUniqueId);
  }
}

void CExplosion::PreRenderAllViewports(CStateManager& mgr) {
  rstl::optional_object< CAABox > bounds = mParticleGen->GetBounds();
  if (bounds) {
    SetOtherBounds(*bounds);
    SetRenderBounds(*bounds);
    mHasRenderBounds = true;
  } else {
    mHasRenderBounds = false;
    const CVector3f pos = GetTranslation();
    const CAABox pointBounds(pos, pos);
    SetOtherBounds(pointBounds);
    SetRenderBounds(pointBounds);
  }
  UpdatePortalSystemState(mgr);
}
