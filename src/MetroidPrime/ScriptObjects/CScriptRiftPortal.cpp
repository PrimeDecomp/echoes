#include "MetroidPrime/ScriptObjects/CScriptRiftPortal.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRiftPortal.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "REL/REL_Setup.h"
#include "dolphin/mtx.h"
#include "dolphin/mtx/mtx44ext.h"
#include "rstl/math.hpp"

#include <float.h>
#include <math.h>

CScriptRiftPortal::CScriptRiftPortal(TUniqueId uid, const rstl::string& name,
                                     const CEntityInfo& info, const CModelData& model,
                                     const CModelData& backgroundModel,
                                     const CModelData& incandescentModel,
                                     const CModelData& lineModel, const CTransform4f& xf,
                                     const CVector3f& scale, bool ripPortal,
                                     int projectileAttraction, float projectileBoxWidth,
                                     float projectileAngle, float projectileDestructionRadius)
: CActor(uid, name, info, 0, xf, model, CMaterialList(kMT_NoStepLogic), CActorParameters::None(),
         kInvalidUniqueId)
, mScale(scale)
, mBackgroundModel(backgroundModel)
, mIncandescentModel(incandescentModel)
, mLineModel(lineModel)
, mProjectileBoxWidth(projectileBoxWidth)
, mProjectileAngle(projectileAngle)
, mProjectileAttraction(projectileAttraction)
, mProjectileDestructionRadius(projectileDestructionRadius)
, mBounds(CAABox::MakeMaxInvertedBox())
, mOpening(false)
, mOpen(false)
, mRipPortal(ripPortal)
, mDrawnOnce(false) {}

CScriptRiftPortal::~CScriptRiftPortal() {}

void CScriptRiftPortal::UpdateProjectiles(CStateManager& mgr, float dt) {
  if (mProjectileAttraction == 0) {
    return;
  }

  static CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Projectile));
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, mBounds, filter, nullptr);

  const CVector3f center =
      GetTranslation() +
      GetTransform().Rotate(CVector3f(0.5f * GetModelData()->GetScale().GetX(), 0.f,
                                      0.5f * GetModelData()->GetScale().GetZ()));
  for (int i = 0; i < nearList.size(); ++i) {
    const CRelAngle maxTurn = CRelAngle::FromRadians(mProjectileAngle * dt);
    CGameProjectile* projectile = TCastToPtr< CGameProjectile >(mgr.ObjectById(nearList[i]));
    if (projectile == nullptr) {
      continue;
    }

    CVector3f toCenter = center - projectile->GetTranslation();
    if (toCenter.Magnitude() < mProjectileDestructionRadius) {
      projectile->StopProjectile(mgr);
      projectile->Projectile().DeactivateProjectile();
    } else if (toCenter.CanBeNormalized()) {
      toCenter.Normalize();
      if (mProjectileAttraction == 2) {
        toCenter = -toCenter;
      }
      const CUnitVector3f heading(projectile->GetTranslation() - projectile->GetPreviousPos());
      if (CMath::AbsD(CVector3f::Dot(heading, toCenter)) < 0.99999f) {
        const CQuaternion turn = CQuaternion::LookAt(heading, CUnitVector3f(toCenter), maxTurn);
        projectile->Projectile().SetWorldSpaceOrientation(
            turn.BuildTransform4f() * projectile->Projectile().GetTransform().GetRotation());
      }
    }
  }
}

void CScriptRiftPortal::AdvanceModel(CModelData& model, CStateManager& mgr, float dt) {
  model.AdvanceAnimation(dt, mgr, GetAreaIdForPersistence(), true, 0.f);
  model.AdvanceParticles(GetTransform(), dt, mgr);
  if (CAnimData* animData = model.AnimationData()) {
    int count;
    const CParticlePOINode* nodes = animData->GetParticlePOIList(count);
    for (int i = 0; i < count; ++i) {
      if (nodes[i].GetCharacterIndex() == -1 ||
          nodes[i].GetCharacterIndex() == animData->GetCharacterIndex()) {
        animData->GetParticleDB().SetParticleEffectState(nodes[i].GetNameHash(), true, &mgr);
      }
    }
  }
}

void CScriptRiftPortal::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (mOpening) {
    if (mLineModel.HasAnimation()) {
      mLineModel.AdvanceAnimation(dt, mgr, GetAreaIdForPersistence(), true, 0.f);
      AdvanceModel(mLineModel, mgr, dt);
    }
    if (!mLineModel.AnimationData()->IsAnimTimeRemaining(0.01f, "Whole Body")) {
      mOpening = false;
      mOpen = true;
      mDrawnOnce = false;
      if (GetModelData()->HasAnimation()) {
        AnimationData()->EnableLooping(true);
        AnimationData()->SetPhase(0.f);
      }
      if (mIncandescentModel.HasAnimation()) {
        mIncandescentModel.AnimationData()->EnableLooping(true);
        mIncandescentModel.AnimationData()->SetPhase(0.f);
      }
    }
  }

  if (GetModelData()->HasAnimation()) {
    UpdateAnimation(dt, mgr, true);
  }
  if (mIncandescentModel.HasAnimation()) {
    AdvanceModel(mIncandescentModel, mgr, dt);
  }
  UpdateProjectiles(mgr, dt);
}

void CScriptRiftPortal::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case 'XALD': { // Guessed message
    if (GetModelData()->HasAnimation()) {
      AnimationData()->SetIsAnimating(false);
      AnimationData()->EnableLooping(false);
    }
    if (mIncandescentModel.HasAnimation()) {
      mIncandescentModel.AnimationData()->SetIsAnimating(false);
      mIncandescentModel.AnimationData()->EnableLooping(false);
    }
    if (mLineModel.HasAnimation()) {
      mLineModel.AnimationData()->SetIsAnimating(false);
      mLineModel.AnimationData()->EnableLooping(false);
    }

    const CVector3f center =
        GetTranslation() +
        GetTransform().Rotate(CVector3f(0.5f * GetModelData()->GetScale().GetX(), 0.f,
                                        0.5f * GetModelData()->GetScale().GetZ()));
    const float halfWidth = mProjectileBoxWidth / 2.f;
    mBounds.AccumulateBounds(center +
                             GetTransform().Rotate(CVector3f(halfWidth, halfWidth, halfWidth)));
    const float negHalfWidth = -mProjectileBoxWidth * 0.5f;
    mBounds.AccumulateBounds(
        center + GetTransform().Rotate(CVector3f(negHalfWidth, negHalfWidth, negHalfWidth)));
    mDrawnOnce = false;
    mOpening = false;
    break;
  }
  case kSM_Deactivate:
    SetActive(false);
    mOpening = false;
    mOpen = false;
    break;
  case kSM_Activate:
    SetActive(true);
    mDrawnOnce = false;
    if (GetModelData()->HasAnimation()) {
      AnimationData()->SetPhase(0.f);
    }
    if (mIncandescentModel.HasAnimation()) {
      mIncandescentModel.AnimationData()->SetPhase(0.f);
    }
    if (mLineModel.HasAnimation()) {
      mLineModel.AnimationData()->SetPhase(0.f);
    }
    mOpening = false;
    mOpen = false;
    break;
  case kSM_Increment:
    if (mLineModel.HasAnimation()) {
      mLineModel.AnimationData()->SetIsAnimating(true);
      mOpening = true;
    }
    break;
  case kSM_Decrement:
    if (GetModelData()->HasAnimation()) {
      AnimationData()->SetIsAnimating(false);
    }
    if (mIncandescentModel.HasAnimation()) {
      mIncandescentModel.AnimationData()->SetIsAnimating(false);
    }
    if (mLineModel.HasAnimation()) {
      mLineModel.AnimationData()->SetIsAnimating(false);
    }
    break;
  }

  CActor::AcceptScriptMsg(mgr, msg);
}

void CScriptRiftPortal::AddToRenderer(const CStateManager& mgr) const {
  if (mgr.IsActorVisible(*this)) {
    EnsureRendered(mgr);
  } else if (GetActive() && mRipPortal) {
    gpRender->SetDestinationAlpha(0);
    gpRender->DisableDestinationAlpha();
  }
}

// Guessed name: copies a region of the frame buffer into a texture.
static void CopyFrameBufferRegion(bool halfSize, void* dest, GXTexFmt format, bool clear, uint left,
                                  uint top, uint width, uint height) {
  const bool videoFilter = CGraphics::GetUseVideoFilter();
  CGraphics::SetUseVideoFilter(false);
  GXSetTexCopySrc(CGraphics::GetViewport().mLeft + left, CGraphics::GetViewport().mTop + top, width,
                  height);
  GXSetTexCopyDst(halfSize ? width >> 1 : width, halfSize ? height >> 1 : height, format, halfSize);
  GXSetColorUpdate(false);
  GXCopyTex(dest != nullptr ? dest : CGraphics::GetDolphinSpareBuffer(), clear);
  GXSetColorUpdate(true);
  GXPixModeSync();
  CGraphics::SetUseVideoFilter(videoFilter);
}

void CScriptRiftPortal::Render(const CStateManager& mgr) const {
  if (GetModelData()->IsNull() || !GetActive() || !mRipPortal || (!mOpening && !mOpen)) {
    return;
  }

  IRenderer* renderer = gpRender;
  CTransform4f portalXf(GetTransform());
  const CVector3f origin = portalXf.GetTranslation();
  portalXf.RotateLocalX(CRelAngle::FromRadians(-M_PIF / 2.f));
  portalXf.SetTranslation(origin + portalXf.Rotate(CVector3f(0.f, -mScale.GetX(), 0.f)));

  // Project the corners of the portal to find the part of the screen that it covers.
  const CTransform4f quadXf = portalXf * CTransform4f::Scale(GetModelData()->GetScale());
  const CVector3f corners[4] = {
      quadXf * CVector3f(0.f, 0.f, 1.f), quadXf * CVector3f(1.f, 0.f, 1.f),
      quadXf * CVector3f(0.f, 0.f, 0.f), quadXf * CVector3f(0.f, 1.f, 0.f)};
  const CGameCamera* camera = mgr.GetCameraManager(0)->GetCurrentCamera(mgr, true);
  float minX = camera->ConvertToScreenSpace(corners[0]).GetX();
  float maxY = camera->ConvertToScreenSpace(corners[0]).GetY();
  for (int i = 1; i < 4; ++i) {
    const CVector3f screen = camera->ConvertToScreenSpace(corners[i]);
    minX = CMath::Min(minX, screen.GetX());
    maxY = rstl::max_val(screen.GetY(), maxY);
  }

  const float clampedX = CMath::Clamp(0.f, 1.f + minX, 2.f);
  const float clampedY = CMath::Clamp(0.f, 1.f + maxY, 2.f);
  const int viewportWidth = CGraphics::GetViewport().mWidth;
  const int viewportHeight = CGraphics::GetViewport().mHeight;
  const uint copyX = CCast::ToUint32(0.5f * clampedX * viewportWidth) & ~1u;
  const uint copyY = CCast::ToUint32((1.f - 0.5f * clampedY) * viewportHeight) & ~1u;
  const float offsetY = 2.f * (1.f - static_cast< float >(copyY) / viewportHeight) - 1.f;
  const float offsetX = 2.f * (static_cast< float >(copyX) / viewportWidth) - 1.f;
  CopyFrameBufferRegion(false, CGraphics::GetDolphinSpareBuffer(), GX_TF_RGB565, false, copyX,
                        copyY, viewportWidth / 2, viewportHeight / 2);

  CGraphics::SetCullMode(kCM_None);
  const CTransform4f modelXf =
      portalXf * CTransform4f::RotateX(CRelAngle::FromRadians(M_PIF / 2.f)) *
      CTransform4f::Scale(GetModelData()->GetScale()) * CTransform4f::Translate(0.f, 0.f, -1.f);
  CGraphics::SetModelMatrix(modelXf);

  // Build the texture matrix that maps the copied region back onto the portal.
  static float skFrustumScaleX = 1.f; // Guessed name
  static float skFrustumScaleY = 1.f; // Guessed name
  const CGraphics::CProjectionState& projection = CGraphics::GetProjectionState();
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(1);
  CGX::SetNumTevStages(1);
  CGX::SetNumIndStages(0);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);

  Mtx44 modelView;
  Mtx44 frustum;
  Mtx44 viewProjection;
  Mtx44 translation;
  PSMTX44Identity(modelView);
  PSMTXCopy(CGraphics::GetGXModelView().GetCStyleMatrix(), modelView);
  C_MTXFrustum(frustum, skFrustumScaleY * projection.GetBottom(),
               skFrustumScaleY * projection.GetTop(), skFrustumScaleX * projection.GetLeft(),
               skFrustumScaleX * projection.GetRight(), projection.GetNear(), projection.GetFar());
  PSMTX44Concat(frustum, modelView, viewProjection);
  PSMTX44Trans(translation, -CMath::Clamp(-1.f, offsetX, 1.f), CMath::Clamp(-1.f, offsetY, 1.f),
               0.f);
  PSMTX44Concat(translation, viewProjection, viewProjection);
  Mtx texMtx;
  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 4; ++col) {
      texMtx[row][col] = viewProjection[row][col];
    }
  }
  GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX3x4);

  static const GXVtxDescList skVtxDesc[] = {
      {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}}; // Guessed name
  CGX::SetVtxDescv(skVtxDesc);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTIDENTITY);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_1);
  CGraphics::SetFog(kRFM_None, 0.f, 0.f, CColor::Black());
  CGraphics::SetAlphaCompare(kAF_Never, 0, kAO_And, kAF_Always, 0);
  renderer->SetDestinationAlpha(255);
  CGraphics::SetDepthWriteMode(true, kE_Always, false);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  CGraphics::SetBlendMode(kBM_Blend, kBF_Zero, kBF_One, kLO_Clear);

  // Stamp the portal area into the destination alpha.
  CGX::Begin(GX_QUADS, GX_VTXFMT0, 4);
  GXPosition3f32(0.01f, 0.99f, 1.f);
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(0.99f, 0.99f, 1.f);
  GXTexCoord2f32(1.f, 1.f);
  GXPosition3f32(0.99f, 0.01f, 1.f);
  GXTexCoord2f32(1.f, 0.f);
  GXPosition3f32(0.01f, 0.01f, 1.f);
  GXTexCoord2f32(0.f, 0.f);
  CGX::End();
  renderer->DisableDestinationAlpha();

  if (mOpen && !mDrawnOnce) {
    mDrawnOnce = true;
    return;
  }

  // Draw the swirling background, incandescent glow and line effect over the copied frame.
  CGraphics::DisableAllLights();
  renderer->SetAmbientColor(CColor::White());
  const CModelFlags flags(CModelFlags::kT_Opaque, 0, static_cast< CModelFlags::EFlags >(0),
                          CColor::White());
  const CModel& backgroundModel = **mBackgroundModel.PickStaticModel(CModelData::kWM_Normal);
  const CCubeModel* cubeModel = backgroundModel.GetModelInstance();
  if (cubeModel != nullptr && !mOpening) {
    mBackgroundModel.Touch(mgr, 0);
    backgroundModel.PreDrawModel(flags);
    CGraphics::SetModelMatrix(
        portalXf *
        CTransform4f::Scale(CVector3f(GetModelData()->GetScale().GetX(),
                                      GetModelData()->GetScale().GetY() * 0.5f,
                                      GetModelData()->GetScale().GetZ())) *
        CTransform4f::Translate(0.5f, 0.f, 0.5f));
    CCubeMaterial::KillCachedViewDepState();
    cubeModel->SetArraysCurrent();
    cubeModel->TryLockTextures();
    for (CCubeSurface surface = cubeModel->GetNormalSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      cubeModel->GetMaterial(surface).SetCurrent(flags, surface, *cubeModel);
      CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
      CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
    }
  }
  if (cubeModel != nullptr && !mOpening) {
    mIncandescentModel.Touch(mgr, 0);
    if (mIncandescentModel.HasAnimation()) {
      mIncandescentModel.Render(
          mgr, modelXf, nullptr,
          CModelFlags(CModelFlags::kT_Opaque, 0, CModelFlags::kF_DepthCompare, CColor::White()));
    }
  }

  CGraphics::SetBlendMode(kBM_None, kBF_One, kBF_Zero, kLO_Clear);
  CGraphics::SetModelMatrix(modelXf);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetVtxDescv(skVtxDesc);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTIDENTITY);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_1);
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);

  GXFogType fogType;
  float fogStartZ;
  float fogEndZ;
  float fogNearZ;
  float fogFarZ;
  GXColor fogColor;
  CGX::GetFog(&fogType, &fogStartZ, &fogEndZ, &fogNearZ, &fogFarZ, &fogColor);
  CGraphics::SetFog(kRFM_None, 0.f, 0.f, CColor::Black());
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX3x4);
  CGraphics::SetCullMode(kCM_Front);
  if (mOpening) {
    mLineModel.Touch(mgr, 0);
    if (mLineModel.HasAnimation()) {
      mLineModel.GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnLast();
    }
  } else {
    if (GetModelData()->HasAnimation()) {
      CGraphics::LoadDolphinSpareTexture(viewportWidth / 2, viewportHeight / 2, GX_TF_RGB565,
                                         nullptr, GX_TEXMAP0);
      const CAnimData* animData = GetAnimationData();
      animData->SetupRender();
      animData->GetModelData()->DolphinDrawWithFlags(&animData->Pose(), 14,
                                                     CModelFlags(CModelFlags::kT_Opaque, 1.f));
      animData->GetParticleDB().RenderSystemsToBeDrawnLast();
    }
    if (mIncandescentModel.HasAnimation()) {
      mIncandescentModel.GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnLast();
    }
  }
  CGX::SetFog(fogType, fogStartZ, fogEndZ, fogNearZ, fogFarZ, fogColor);
}

void CScriptRiftPortal::PreRender(CStateManager& mgr) {
  CActor::PreRender(mgr);
  if (!mOpening && mOpen) {
    if (mIncandescentModel.HasAnimation()) {
      mIncandescentModel.AnimationData()->PreRender();
    }
  }
}

bool CScriptRiftPortal::CanRenderUnsorted(const CStateManager& mgr) const { return false; }

void CScriptRiftPortal::PreRenderAllViewports(CStateManager& mgr) {
  const CAABox bounds(GetTranslation(), GetTranslation());
  SetOtherBounds(bounds);
  SetRenderBounds(bounds);
  UpdatePortalSystemState(mgr);
}

void CScriptRiftPortal::SetActive(bool active) {
  CActor::SetActive(active);
  SetDrawEnabled(true);
}

CEntity* REL_LoadRiftPortal(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrRiftPortal sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrRiftPortal.inc"

  rstl::optional_object< CModelData > model(
      LdrToModelData(sldrThis.editorProperties.transform.scale, sldrThis.model,
                     sldrThis.animationInformation, true));
  rstl::optional_object< CModelData > backgroundModel(
      LdrToModelData(sldrThis.editorProperties.transform.scale, sldrThis.backgroundModel,
                     sldrThis.backgroundAnimation, true));
  rstl::optional_object< CModelData > incandescentModel(LdrToModelData(
      CVector3f::One(), sldrThis.incandescentModel, sldrThis.incandescentAnimation, true));
  rstl::optional_object< CModelData > lineModel(
      LdrToModelData(CVector3f::One(), sldrThis.lineModel, sldrThis.lineAnimation, true));
  if (!model || !backgroundModel || !incandescentModel || !lineModel) {
    return nullptr;
  }

  return rs_new CScriptRiftPortal(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), *model, *backgroundModel,
      *incandescentModel, *lineModel, LdrToTransform4f(sldrThis.editorProperties),
      sldrThis.editorProperties.transform.scale, sldrThis.ripPortal, sldrThis.projectileAttraction,
      sldrThis.projectileBoxWidth, 0.017453292f * sldrThis.projectileAngle,
      sldrThis.projectileDestructionRadius);
}

static void SetFuncPtrs() {
  static SRiftPortal_FuncPtrs funcPtrs;
  funcPtrs.mLoadRiftPortal = &REL_LoadRiftPortal;
  SetSRiftPortal_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSRiftPortal_FuncPtrs(nullptr); }
