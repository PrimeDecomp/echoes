#include "MetroidPrime/CModelData.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Animation/CCharacterInfo.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/PortalPlane.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetaRender/SModelRenderData.hpp"
#include "MetroidPrime/Factories/CCharacterFactory.hpp"
#include "MetroidPrime/Factories/CCharacterFactoryBuilder.hpp"
#include "rstl/math.hpp"

#include <dolphin/mtx.h>

// Guessed name.
struct SModelDataMultipassContext {
  SModelDataMultipassContext(const CSkinnedModel& model, const CModelFlags* flags, const u64* masks,
                             const CColor* colors, const CPlane* planes, int count)
  : mModel(model), mFlags(flags), mMasks(masks), mColors(colors), mPlanes(planes), mCount(count) {}

  const CSkinnedModel& mModel;
  const CModelFlags* mFlags;
  const u64* mMasks;
  const CColor* mColors;
  const CPlane* mPlanes;
  int mCount;
};
CHECK_SIZEOF(SModelDataMultipassContext, 0x18)

static const CAdvancementDeltas skNullAdvance(CVector3f::Zero(), CQuaternion::NoRotation());

CModelData::CModelData(const CStaticRes& res)
: mScale(res.GetScale())
, mRenderSorted(false)
, mTexturesLocked(false)
, mRenderUnsortedParts(true)
, mRenderFullEchoModel(false)
, mAmbientColor(CColor::White())
, mNormalModel(TLockedToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', res.GetId())))) {}

CModelData::CModelData()
: mScale(1.f, 1.f, 1.f)
, mRenderSorted(false)
, mTexturesLocked(false)
, mRenderUnsortedParts(true)
, mRenderFullEchoModel(false)
, mAmbientColor(CColor::White()) {}

CModelData::CModelData(const CAnimRes& res)
: mScale(res.GetScale())
, mRenderSorted(false)
, mTexturesLocked(false)
, mRenderUnsortedParts(true)
, mRenderFullEchoModel(false)
, mAmbientColor(CColor::White()) {
  TLockedToken< CCharacterFactory > factory(gpCharacterFactoryBuilder->GetFactory(res));
  int defaultAnim = res.GetDefaultAnim();
  if (defaultAnim < 0) {
    defaultAnim = factory->GetCharInfo(res.GetCharacterNodeId()).GetDefaultAnimation();
  }
  mAnimData = rstl::auto_ptr< CAnimData >(
      factory->CreateCharacter(res.GetCharacterNodeId(), res.CanLoop(), factory, defaultAnim)
          .release());
  mAnimData->SetModelScale(mScale);
}

CModelData::~CModelData() {}

void CModelData::Render(EWhichModel which, const CTransform4f& xf, const CActorLights* lights,
                        const CModelFlags& flags) const {
  if (which == kWM_Echo) {
    uchar destinationAlpha = 0;
    if (flags.GetTrans() == CModelFlags::kT_Two) {
      const CColor& color = flags.GetColorRef();
      const uchar intensity = rstl::max_val(
          rstl::max_val(color.GetRedu8(), color.GetGreenu8()), color.GetBlueu8());
      destinationAlpha = rstl::min_val< uint >(255, intensity * 2);
    }
    if (destinationAlpha != 0) {
      gpRender->SetDestinationAlpha(destinationAlpha);
    }
    const CModelFlags echoFlags(CModelFlags::kT_One, 0,
                                static_cast< CModelFlags::EFlags >(CModelFlags::kF_DepthCompare |
                                                                  CModelFlags::kF_DepthUpdate),
                                CColor::Black());
    RenderSolid(which, xf, !mRenderFullEchoModel, echoFlags);
    if (destinationAlpha != 0) {
      gpRender->SetDestinationAlpha(0);
    }
    return;
  }

  CTransform4f modelXf = xf;
  modelXf *= CTransform4f::Scale(mScale.GetX(), mScale.GetY(), mScale.GetZ());
  gpRender->SetModelMatrix(modelXf);
  if (lights != nullptr && which != kWM_Dark) {
    lights->ActivateLights();
  } else {
    CGraphics::DisableAllLights();
    gpRender->SetAmbientColor(mAmbientColor);
  }

  if (HasAnimation()) {
    mAnimData->Render(PickAnimatedModel(which), flags);
  } else if (mNormalModel) {
    const CModel& model = **PickStaticModel(which);
    if (mRenderSorted) {
      model.DrawSortedParts(flags);
    } else {
      model.Draw(flags);
    }
  }

  gpRender->SetAmbientColor(CColor::White());
  CGraphics::DisableAllLights();
  mRenderSorted = false;
}

void CModelData::RenderUnsortedParts(EWhichModel which, const CTransform4f& xf,
                                     const CActorLights* lights, const CModelFlags& flags) const {
  if (HasAnimation() || !mNormalModel || static_cast< char >(flags.GetTrans()) > 4 ||
      !mRenderUnsortedParts) {
    mRenderSorted = false;
    return;
  }

  const CTransform4f modelXf(xf *
                             CTransform4f::Scale(mScale.GetX(), mScale.GetY(), mScale.GetZ()));
  gpRender->SetModelMatrix(modelXf);
  if (lights != nullptr && which != kWM_Dark) {
    lights->ActivateLights();
  } else {
    CGraphics::DisableAllLights();
    gpRender->SetAmbientColor(mAmbientColor);
  }

  PickStaticModel(which)->DrawUnsortedParts(flags);
  gpRender->SetAmbientColor(CColor::White());
  CGraphics::DisableAllLights();
  mRenderSorted = true;
}

void CModelData::MultipassDrawCallback(const SSkinningWorkspace& workspace, void* data) {
  const SModelDataMultipassContext& context =
      *static_cast< const SModelDataMultipassContext* >(data);
  for (int i = 0; i < context.mCount; ++i) {
    gpRender->SetGXRegister1Color(context.mColors[i]);
    PortalPlane::SetCurrentPlane(context.mPlanes[i]);
    context.mModel.DolphinDrawFromWorkspace(workspace, 6, context.mFlags[i], context.mMasks[i]);
  }
}

void CModelData::DisintegrateDraw(EWhichModel which, const CTransform4f& xf,
                                  const CTexture& texture, const CColor& color,
                                  float amount) const {
  const CTransform4f modelXf = xf * CTransform4f::Scale(mScale);
  gpRender->SetModelMatrix(modelXf);
  CGraphics::DisableAllLights();

  if (HasAnimation()) {
    const CSkinnedModel& model = PickAnimatedModel(which);
    mAnimData->SetupRender();
    const SModelRenderData renderData(model, mAnimData->Pose());
    gpRender->DrawModelDisintegrate(renderData, texture, color, amount);
  } else {
    const SModelRenderData renderData(**PickStaticModel(which));
    gpRender->DrawModelDisintegrate(renderData, texture, color, amount);
  }
}

void CModelData::DisintegrateDraw(const CStateManager& mgr, const CTransform4f& xf,
                                  const CTexture& texture, const CColor& color,
                                  float amount) const {
  DisintegrateDraw(GetRenderingModel(mgr), xf, texture, color, amount);
}

void CModelData::RenderNoise(EWhichModel which, const CTransform4f& xf, const CColor& color,
                             bool additive) const {
  const CTransform4f modelXf = xf * CTransform4f::Scale(mScale);
  gpRender->SetModelMatrix(modelXf);
  CGraphics::DisableAllLights();

  if (HasAnimation()) {
    const CSkinnedModel& model = PickAnimatedModel(which);
    mAnimData->SetupRender();
    const SModelRenderData renderData(model, mAnimData->Pose());
    gpRender->DrawModelNoise(renderData, color, additive);
  } else {
    const SModelRenderData renderData(**PickStaticModel(which));
    gpRender->DrawModelNoise(renderData, color, additive);
  }
}

void CModelData::RenderNoise(const CStateManager& mgr, const CTransform4f& xf, const CColor& color,
                             bool additive) const {
  RenderNoise(GetRenderingModel(mgr), xf, color, additive);
}

void CModelData::RenderSolid(EWhichModel which, const CTransform4f& xf, bool unsortedOnly,
                             const CModelFlags& flags) const {
  const CTransform4f modelXf = xf * CTransform4f::Scale(mScale);
  gpRender->SetModelMatrix(modelXf);
  CGraphics::DisableAllLights();

  if (HasAnimation()) {
    const CSkinnedModel& model = PickAnimatedModel(which);
    mAnimData->SetupRender();
    const SModelRenderData renderData(model, mAnimData->Pose());
    gpRender->DrawModelFlat(renderData, flags, unsortedOnly);
  } else {
    const SModelRenderData renderData(**PickStaticModel(which));
    gpRender->DrawModelFlat(renderData, flags, unsortedOnly);
  }
}

void CModelData::RenderModelMultipleTimesWithFlags(EWhichModel which, const CTransform4f& xf,
                                                   const CActorLights* lights,
                                                   const CModelFlags* flags, const u64* masks,
                                                   const CColor* colors, const CPlane* planes,
                                                   int count) const {
  const CTransform4f modelXf = xf * CTransform4f::Scale(mScale);
  gpRender->SetModelMatrix(modelXf);
  if (lights != nullptr && which != kWM_Dark) {
    lights->ActivateLights();
  } else {
    CGraphics::DisableAllLights();
    gpRender->SetAmbientColor(mAmbientColor);
  }

  if (HasAnimation()) {
    const CSkinnedModel& model = PickAnimatedModel(which);
    mAnimData->SetupRender();
    SModelDataMultipassContext context(model, flags, masks, colors, planes, count);
    model.Draw(&mAnimData->Pose(), &MultipassDrawCallback, &context);
  } else {
    const CModel& model = **PickStaticModel(which);
    for (int i = 0; i < count; ++i) {
      gpRender->SetGXRegister1Color(colors[i]);
      model.Draw(masks[i], flags[i]);
    }
  }
}

void CModelData::Touch(const CStateManager& mgr, int shaderIdx) const {
  if (!mTexturesLocked) {
    return;
  }
  for (int which = kWM_Normal; which <= kWM_Echo; ++which) {
    Touch(static_cast< EWhichModel >(which), shaderIdx);
  }
}

void CModelData::Touch(EWhichModel which, int shaderIdx) const {
  if (!mTexturesLocked) {
    return;
  }
  if (HasAnimation()) {
    CAnimData::Touch(PickAnimatedModel(which), shaderIdx);
  } else {
    PickStaticModel(which)->Touch(shaderIdx);
  }
}

void CModelData::Touch() const {
  if (!mTexturesLocked) {
    return;
  }
  if (HasAnimation()) {
    for (int which = kWM_Normal; which <= kWM_Echo; ++which) {
      CAnimData::Touch(PickAnimatedModel(static_cast< EWhichModel >(which)));
    }
  } else {
    for (int which = kWM_Normal; which <= kWM_Echo; ++which) {
      const CModel& model = **PickStaticModel(static_cast< EWhichModel >(which));
      const int shaderCount = model.GetNumMaterialSets();
      for (int shader = 0; shader < shaderCount; ++shader) {
        model.Touch(shader);
      }
    }
  }
}

void CModelData::RenderParticles(const CFrustumPlanes& planes) const {
  if (HasAnimation()) {
    mAnimData->RenderAuxiliary(planes);
  }
}

bool CModelData::IsAnimating() const { return HasAnimation() && mAnimData->IsAnimating(); }

CAdvancementDeltas CModelData::AdvanceAnimation(float dt, CStateManager& mgr, TAreaId aid,
                                                bool advTree, float cameraDistance) {
  if (!HasAnimation()) {
    return skNullAdvance;
  }
  return mAnimData->Advance(dt, cameraDistance, GetScale(), &mgr, *mgr.Random(), aid, advTree);
}

CAdvancementDeltas CModelData::AdvanceAnimation(float dt, CRandom16& random, bool advTree) {
  if (!HasAnimation()) {
    return skNullAdvance;
  }
  static const TAreaId areaId = kInvalidAreaId;
  return mAnimData->Advance(dt, 0.f, GetScale(), nullptr, random, areaId, advTree);
}

CAdvancementDeltas CModelData::AdvanceAnimationIgnoreParticles(float dt, CRandom16& random,
                                                               bool advTree) {
  if (!HasAnimation()) {
    return skNullAdvance;
  }
  return mAnimData->AdvanceIgnoreParticles(dt, random, advTree);
}

CTransform4f CModelData::GetLocatorTransform(const rstl::string& name) const {
  if (!HasAnimation()) {
    return CTransform4f::Identity();
  }
  return mAnimData->GetLocatorTransform(name, nullptr);
}

CTransform4f CModelData::GetLocatorTransform(const CSegId& id) const {
  if (!HasAnimation()) {
    return CTransform4f::Identity();
  }
  return mAnimData->GetLocatorTransform(id, nullptr);
}

CTransform4f CModelData::GetLocatorTransformDynamic(const rstl::string& name,
                                                    const CCharAnimTime* time) const {
  if (!HasAnimation()) {
    return CTransform4f::Identity();
  }
  return mAnimData->GetLocatorTransform(name, time);
}

CTransform4f CModelData::GetLocatorTransformDynamic(const CSegId& id,
                                                    const CCharAnimTime* time) const {
  if (!HasAnimation()) {
    return CTransform4f::Identity();
  }
  return mAnimData->GetLocatorTransform(id, time);
}

CTransform4f CModelData::GetScaledLocatorTransform(const rstl::string& name) const {
  CTransform4f xf = GetLocatorTransform(name);
  xf.SetTranslation(CVector3f::ByElementMultiply(mScale, xf.GetTranslation()));
  return xf;
}

CTransform4f CModelData::GetScaledLocatorTransform(const CSegId& id) const {
  CTransform4f xf = GetLocatorTransform(id);
  xf.SetTranslation(CVector3f::ByElementMultiply(mScale, xf.GetTranslation()));
  return xf;
}

CTransform4f CModelData::GetScaledLocatorTransformDynamic(const rstl::string& name,
                                                          const CCharAnimTime* time) const {
  CTransform4f xf = GetLocatorTransformDynamic(name, time);
  xf.SetTranslation(CVector3f::ByElementMultiply(mScale, xf.GetTranslation()));
  return xf;
}

CTransform4f CModelData::GetScaledLocatorTransformDynamic(const CSegId& id,
                                                          const CCharAnimTime* time) const {
  CTransform4f xf = GetLocatorTransformDynamic(id, time);
  xf.SetTranslation(CVector3f::ByElementMultiply(mScale, xf.GetTranslation()));
  return xf;
}

CAABox CModelData::GetBounds(const CTransform4f& xf) const {
  const CTransform4f scaledXf = xf * CTransform4f::Scale(mScale.GetX(), mScale.GetY(), mScale.GetZ());
  if (HasAnimation()) {
    return mAnimData->GetBoundingBox(scaledXf);
  }

  CAABox bounds = (*mNormalModel)->GetAABB();
  if (mEchoModel) {
    bounds.Include((*mEchoModel)->GetAABB());
  }
  if (mDarkModel) {
    bounds.Include((*mDarkModel)->GetAABB());
  }
  return bounds.GetTransformedAABox(scaledXf);
}

CAABox CModelData::GetBounds() const {
  if (HasAnimation()) {
    return mAnimData->GetBoundingBox(CTransform4f::Scale(mScale.GetX(), mScale.GetY(), mScale.GetZ()));
  }

  CAABox bounds = (*mNormalModel)->GetAABB();
  if (mEchoModel) {
    bounds.Include((*mEchoModel)->GetAABB());
  }
  if (mDarkModel) {
    bounds.Include((*mDarkModel)->GetAABB());
  }
  return CAABox(CVector3f::ByElementMultiply(bounds.GetMinPoint(), mScale),
                CVector3f::ByElementMultiply(bounds.GetMaxPoint(), mScale));
}

void CModelData::AdvanceParticles(const CTransform4f& xf, float dt, CStateManager& mgr) {
  if (HasAnimation()) {
    mAnimData->AdvanceParticles(xf, dt, mScale, &mgr);
  }
}

void CModelData::EnableLooping(bool enable) {
  if (HasAnimation()) {
    mAnimData->EnableLooping(enable);
  }
}

float CModelData::GetAnimationDuration(int anim) const {
  return !HasAnimation() ? 0.f : mAnimData->GetAnimationDuration(anim);
}

bool CModelData::GetIsLoop() const { return HasAnimation() && mAnimData->GetIsLoop(); }

bool CModelData::IsDefinitelyOpaque(EWhichModel which) const {
  if (HasAnimation()) {
    return PickAnimatedModel(which).GetModel()->IsDefinitelyOpaque();
  }
  if (mNormalModel) {
    return PickStaticModel(which)->IsDefinitelyOpaque();
  }
  return false;
}

void CModelData::SetEchoModel(const rstl::pair< CAssetId, CAssetId >& assets) {
  if (assets.first == 0 || gpResourceFactory->GetResourceTypeById(assets.first) != 'CMDL') {
    return;
  }

  if (HasAnimation() && assets.second != 0 &&
      gpResourceFactory->GetResourceTypeById(assets.second) == 'CSKR') {
    mAnimData->SetXRayModel(
        TLockedToken< CModel >(
            TToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', assets.first)))),
        TLockedToken< CSkinRules >(
            TToken< CSkinRules >(gpSimplePool->GetObj(SObjectTag('CSKR', assets.second)))));
  } else {
    mEchoModel = TLockedToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', assets.first)));
  }
}

void CModelData::SetDarkModel(const rstl::pair< CAssetId, CAssetId >& assets) {
  if (assets.first == 0 || gpResourceFactory->GetResourceTypeById(assets.first) != 'CMDL') {
    return;
  }

  if (HasAnimation() && assets.second != 0 &&
      gpResourceFactory->GetResourceTypeById(assets.second) == 'CSKR') {
    mAnimData->SetInfraModel(
        TLockedToken< CModel >(
            TToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', assets.first)))),
        TLockedToken< CSkinRules >(
            TToken< CSkinRules >(gpSimplePool->GetObj(SObjectTag('CSKR', assets.second)))));
  } else {
    mDarkModel = TLockedToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', assets.first)));
  }
}

const TLockedToken< CModel >& CModelData::PickStaticModel(EWhichModel which) const {
  switch (which) {
  case kWM_Echo:
    if (mEchoModel) {
      return *mEchoModel;
    }
    break;
  case kWM_Dark:
    if (mDarkModel) {
      return *mDarkModel;
    }
    break;
  default:
    break;
  }
  return *mNormalModel;
}

CSkinnedModel& CModelData::PickAnimatedModel(EWhichModel which) const {
  CSkinnedModel* model = nullptr;
  switch (which) {
  case kWM_Echo:
    model = mAnimData->GetXRayModel();
    break;
  case kWM_Dark:
    model = mAnimData->GetInfraModel();
    break;
  default:
    break;
  }
  return model ? *model : **mAnimData->GetModelData();
}

CModelData::EWhichModel CModelData::GetRenderingModel(const CStateManager& mgr,
                                                      const CPlayerState& playerState) {
  switch (playerState.GetActiveVisor(mgr)) {
  case CPlayerState::kPV_Echo:
    return kWM_Echo;
  case CPlayerState::kPV_Dark:
    return kWM_Dark;
  default:
    return kWM_Normal;
  }
}

CModelData::EWhichModel CModelData::GetRenderingModel(const CStateManager& mgr) {
  return GetRenderingModel(mgr, *mgr.GetPlayerState());
}

void CModelData::Render(const CStateManager& mgr, const CTransform4f& xf,
                        const CActorLights* lights, const CModelFlags& flags) const {
  Render(GetRenderingModel(mgr), xf, lights, flags);
}

bool CModelData::IsLoaded(int shaderIdx) const {
  if (HasAnimation()) {
    const CAnimData* animData = mAnimData.get();
    if (!animData->GetModelData()->GetModel()->IsLoaded(shaderIdx)) {
      return false;
    }
    if (animData->GetXRayModel() && !animData->GetXRayModel()->GetModel()->IsLoaded(shaderIdx)) {
      return false;
    }
    if (animData->GetInfraModel() && !animData->GetInfraModel()->GetModel()->IsLoaded(shaderIdx)) {
      return false;
    }
  }
  if (mNormalModel && !(*mNormalModel)->IsLoaded(shaderIdx)) {
    return false;
  }
  if (mEchoModel && !(*mEchoModel)->IsLoaded(shaderIdx)) {
    return false;
  }
  if (mDarkModel && !(*mDarkModel)->IsLoaded(shaderIdx)) {
    return false;
  }
  return true;
}

int CModelData::GetNumShaders() const {
  if (HasAnimation()) {
    return mAnimData->GetModelData()->GetModel()->GetNumMaterialSets();
  }
  if (mNormalModel) {
    return (*mNormalModel)->GetNumMaterialSets();
  }
  return 1;
}

void CModelData::LockTextures() {
  if (mTexturesLocked) {
    return;
  }
  mTexturesLocked = true;
  const int shaderCount = GetNumShaders();
  if (HasAnimation()) {
    for (int which = kWM_Normal; which <= kWM_Echo; ++which) {
      CModel& model = **PickAnimatedModel(static_cast< EWhichModel >(which)).GetModel();
      for (int shader = 0; shader < shaderCount; ++shader) {
        model.UnlockTextures();
      }
    }
  } else {
    for (int which = kWM_Normal; which <= kWM_Echo; ++which) {
      CModel& model = **PickStaticModel(static_cast< EWhichModel >(which));
      for (int shader = 0; shader < shaderCount; ++shader) {
        model.UnlockTextures();
      }
    }
  }
}

void CModelData::SetScale(const CVector3f& scale) {
  mScale = scale;
  if (HasAnimation()) {
    mAnimData->SetModelScale(scale);
  }
}

void CModelData::SetupWorldSpacePortalPlane(const CTransform4f& xf, const CPlane& plane) const {
  const CTransform4f model = xf * CTransform4f::Scale(mScale);
  Mtx modelView;
  PSMTXConcat(CGraphics::GetCameraMtx(), model.GetCStyleMatrix(), modelView);
  // SDK Mtx and CTransform4f both store the same 3x4 float matrix.
  PortalPlane::SetWorldSpacePlane(plane, *reinterpret_cast< const CTransform4f* >(modelView),
                                  model);
}
