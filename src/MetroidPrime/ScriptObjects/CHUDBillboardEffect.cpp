#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"

#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

#include "MetaRender/CCubeRenderer.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"

#include "rstl/math.hpp"

int CHUDBillboardEffect::g_BillboardCount = 0;
int CHUDBillboardEffect::g_IndirectTexturedBillboardCount = 0;

void CHUDBillboardEffect::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    mgr.SetActorAreaId(*this, mgr.GetWorld()->GetCurrentAreaId());
    float oldGenRate = mGenerator->GetGeneratorRate();
    mGenerator->SetGeneratorRate(oldGenRate * CalcGenRate());
    mGenerator->Update(dt);
    mGenerator->SetGeneratorRate(oldGenRate);
    if (!mRunIndefinitely) {
      mTimeoutTimer += dt;
      if (mTimeoutTimer > 30.f) {
        mgr.DeleteObjectRequest(GetUniqueId());
        return;
      }
    }
    if (mGenerator->IsSystemDeletable() || (mFinishing && mGenerator->GetParticleCount() == 0))
      mgr.DeleteObjectRequest(GetUniqueId());
  }
}

void CHUDBillboardEffect::Render(const CStateManager& mgr) const {
  if (mPlayerIndex == mgr.GetCurrentRenderPlayerIndex() && mEnableRender && !mRenderAsParticleGen) {
    float near = 0.f;
    float far = near;
    if (mAdjustDepthRange) {
      near = CGraphics::GetDepthNear();
      far = CGraphics::GetDepthFar();
      CGraphics::SetDepthRange(1.f / 256.f, 1.f / 64.f);
    }
    mGenerator->Render();
    if (mAdjustDepthRange) {
      CGraphics::SetDepthRange(near, far);
    }
  }
}

void CHUDBillboardEffect::PreRender(CStateManager& mgr) {
  if (mPlayerIndex != mgr.GetCurrentRenderPlayerIndex()) {
    mEnableRender = false;
    return;
  }

  if (mgr.GetCurrentRenderPlayer()->GetCameraState() == CPlayer::kCS_FirstPerson) {
    CTransform4f camXf = mgr.GetCurrentRenderCameraManager()->GetCurrentCameraTransform(mgr, true);
    mGenerator->SetGlobalTranslation(camXf * mTranslation);
    mGenerator->SetGlobalOrientation(camXf);
    mEnableRender = true;
  } else {
    mEnableRender = false;
  }

  if (mAdjustDepthRange) {
    mRenderAsParticleGen = !mgr.RenderLastOverlay(GetUniqueId());
  } else {
    mRenderAsParticleGen = !mgr.RenderLastHUD(GetUniqueId());
  }
}

void CHUDBillboardEffect::AddToRenderer(const CStateManager& mgr) const {
  if (mEnableRender && mRenderAsParticleGen) {
    gpRender->AddParticleGen(*mGenerator);
  }
}

CHUDBillboardEffect::~CHUDBillboardEffect() {
  --g_BillboardCount;
  if (mGenerator->Get4CharId() == 'PART')
    if (static_cast< CElementGen& >(*mGenerator).IsIndirectTextured())
      --g_IndirectTexturedBillboardCount;
}

CHUDBillboardEffect::CHUDBillboardEffect(
    const rstl::optional_object< TToken< CGenDescription > >& particle,
    const rstl::optional_object< TToken< CElectricDescription > >& electric, TUniqueId uid,
    bool active, const rstl::string& name, float dist, const CVector3f& scale0, int playerIndex,
    const CColor& color, const CVector3f& scale1, const CVector3f& translation,
    bool adjustDepthRange)
: CEffect(uid, CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList, active, kInvalidEditorId),
          name, CTransform4f::Identity())
, mPlayerIndex(playerIndex)
, mTranslation(translation.GetX(), translation.GetY() + dist, translation.GetZ())
, mLocalScale(CVector3f::ByElementMultiply(scale1, scale0))
, mRenderAsParticleGen(true)
, mEnableRender(false)
, mIsElementGen(false)
, mRunIndefinitely(false)
, mAdjustDepthRange(adjustDepthRange)
, mFinishing(false)
, mTimeoutTimer(0.f) {
  if (particle) {
    mIsElementGen = true;
    mGenerator = rs_new CElementGen(*particle);
    if (static_cast< CElementGen& >(*mGenerator).IsIndirectTextured())
      ++g_IndirectTexturedBillboardCount;
  } else {
    mGenerator = rs_new CParticleElectric(TToken< CElectricDescription >(*electric));
  }
  ++g_BillboardCount;
  mGenerator->SetModulationColor(color);
  mGenerator->SetLocalScale(mLocalScale);
}

float CHUDBillboardEffect::GetNearClipDistance(const CStateManager& mgr, int playerIndex) {
  return 0.01f +
         mgr.GetCameraManager(playerIndex)->GetCurrentCamera(mgr, true)->GetNearClipDistance();
}

const CVector3f& CHUDBillboardEffect::GetScaleForPOV(const CStateManager& mgr) {
  static CVector3f result(0.155f, 1.f, 0.155f);
  return result;
}

float CHUDBillboardEffect::CalcGenRate() {
  float f1 = (g_BillboardCount + g_IndirectTexturedBillboardCount <= 4)
                 ? 0.f
                 : g_BillboardCount * 0.2f + g_IndirectTexturedBillboardCount * 0.1f;
  return 1.f - rstl::min_val(0.8f, f1);
}
