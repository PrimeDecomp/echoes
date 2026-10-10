#include "MetroidPrime/Enemies/CSplitterBeamEffect.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/PVS/CPVSVisSet.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "WorldFormat/CPVSAreaSet.hpp"

#include <dolphin/gx.h>
#include <dolphin/os.h>
#include <math.h>

static EMaterialTypes skMaterial = kMT_NoStepLogic;                // Guessed name
static float skMinAngle = CRelAngle::FromDegrees(1.f).AsRadians(); // Guessed name

CSplitterBeamEffect::CSplitterBeamEffect(TUniqueId uid, const CEntityInfo& info,
                                         const rstl::string& name, const CTransform4f& xf,
                                         int textureHeight, const CAbsAngle& angle, float range,
                                         const CColor& cloudColor1, const CColor& cloudColor2,
                                         const CColor& addColor1, const CColor& addColor2,
                                         float cloudScale, float fadeOffSize, float openSpeed)
: CActor(uid, name, info, 0, xf, CModelData::None(), CMaterialList(skMaterial),
         CActorParameters::None(), kInvalidUniqueId)
, mMaxAngle(angle.AsRadians())
, mRange(range)
, mCloudColor1(cloudColor1)
, mCloudColor2(cloudColor2)
, mAddColor1(addColor1)
, mAddColor2(addColor2)
, mCloudScale(cloudScale)
, mFadeOffSize(fadeOffSize)
, mOpenSpeed(openSpeed)
, mTexture(rs_new CTexture(kTF_IA8, 4, textureHeight, 1))
, mCurrentAngle(skMinAngle)
, mPreRenderCount(0)
, mOpenFraction(0.f)
, mRandom(99)
, mAddColorLerp(0.f)
, mCloudColorLerp(0.f)
, mOpening(true) {}

CSplitterBeamEffect::~CSplitterBeamEffect() { mTexture->ScheduleDeletion(); }

void CSplitterBeamEffect::Think(float dt, CStateManager& mgr) {
  CActor::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }

  if (mOpening) {
    mOpenFraction = CMath::Min(1.f, mOpenFraction + dt * mOpenSpeed);
  } else {
    mOpenFraction = rstl::max_val(0.f, mOpenFraction - dt * mOpenSpeed);
  }
  const float delta = mMaxAngle - skMinAngle;
  mCurrentAngle = CMath::ClampRadians(delta * mOpenFraction + skMinAngle);
}

void CSplitterBeamEffect::PreRender(CStateManager& mgr) {
  ++mPreRenderCount;
  const CTransform4f& xf = GetTransform();

  const CGraphics::CProjectionState oldProjection = CGraphics::GetProjectionState();
  const CTransform4f oldView = CGraphics::GetViewMatrix();
  const CViewport oldViewport = CGraphics::GetViewport();
  const float oldNear = CGraphics::GetDepthNear();
  const float oldFar = CGraphics::GetDepthFar();
  const bool useVideoFilter = CGraphics::GetUseVideoFilter();
  CGraphics::SetUseVideoFilter(false);

  gpRender->SetViewport(0, CGraphics::GetRenderMode().efbHeight - mTexture->GetHeight(),
                        mTexture->GetWidth(), mTexture->GetHeight());
  CGraphics::SetDepthRange(0.f, 1.f);
  gpRender->SetWorldViewpoint(xf);
  CGraphics::SetModelMatrix(CTransform4f::Identity());

  const CVector3f center = xf.GetTranslation();
  const CVector3f extent = mRange * CVector3f::One();
  const CAABox bounds(center - extent, center + extent);
  CGX::SetNumChans(0);
  CGraphics::DisableAllLights();

  CPVSVisSet pvs(kVSS_OutOfBounds);
  const CGameArea* area = mgr.GetWorld()->GetArea(GetCurrentAreaId());
  const CPVSAreaSet* areaSet = area->GetPostConstructed()->mPvs.get();
  if (areaSet != nullptr && gkPVSEnabled == 1) {
    pvs = areaSet->GetVisOctree().GetVisSet(area->GetPostConstructed()->mInverseTransform *
                                            GetTranslation());
  }

  const float aspect = CGraphics::GetPixelAspectRatio() *
                       (float(mTexture->GetWidth()) / float(mTexture->GetHeight()));
  const CFrustumPlanes frustum(xf, mCurrentAngle, aspect, 0.2f, true, mRange);
  gpRender->SetPerspective(57.295776f * mCurrentAngle, aspect, 0.2f, mRange);

  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(1);
  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetZMode(true, GX_LEQUAL, true);

  gpRender->DrawVisibleAreaGeometry(GetCurrentAreaId().Value(), pvs, frustum, GetOtherBounds());

  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, bounds, CMaterialFilter::GetPassEverything(), nullptr);
  for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = nearList.begin();
       it != nearList.end(); ++it) {
    CActor* actor = static_cast< CActor* >(const_cast< CEntity* >(mgr.GetObjectById(*it)));
    if (actor != nullptr && actor->CanDrawStatic() && actor->GetTakesProjectedShadow()) {
      const CAABox& actorBounds = actor->GetOtherBounds();
      if (frustum.BoxInFrustumPlanes(actorBounds) && bounds.DoBoundsOverlap(actorBounds)) {
        const CModelData& modelData = *actor->GetModelData();
        const CTransform4f modelXf =
            actor->GetTransform() * CTransform4f::Scale(modelData.GetScale());
        gpRender->SetModelMatrix(modelXf);
        (*modelData.PickStaticModel(CModelData::kWM_Normal))->DolphinDrawFlat(CModel::kDF_All);
      }
    }
  }

  GXSetTexCopySrc(0, 0, mTexture->GetWidth(), mTexture->GetHeight());
  GXSetTexCopyDst(mTexture->GetWidth(), mTexture->GetHeight(), GX_TF_Z16, false);
  GXCopyTex(mTexture->Lock(), true);
  mTexture->UnLock();
  GXPixModeSync();

  CGraphics::SetDepthRange(oldNear, oldFar);
  CGraphics::SetUseVideoFilter(useVideoFilter);
  CGraphics::SetProjectionState(oldProjection);
  gpRender->SetWorldViewpoint(oldView);
  gpRender->SetViewport(oldViewport.mLeft, oldViewport.mTop, oldViewport.mWidth,
                        oldViewport.mHeight);

  SetPreRenderClipped(!mgr.IsActorVisible(*this));
  mAddColorLerp = mRandom.Float();
  mCloudColorLerp = mRandom.Float();
}

void CSplitterBeamEffect::AddToRenderer(const CStateManager& mgr) const {
  if (!GetPreRenderClipped()) {
    EnsureRendered(mgr);
  }
}

void CSplitterBeamEffect::Render(const CStateManager& mgr) const {
  if (mPreRenderCount < 2) {
    return;
  }

  CGraphics::SetModelMatrix(GetTransform());

  float lastRow = mTexture->GetHeight();
  const float tanHalfAngle = tan(0.5f * mCurrentAngle);
  lastRow -= 1.f;
  const float step = 2.f * tanHalfAngle / lastRow;
  float tanAngle = tanHalfAngle;

  CGX::SetTevKColor(GX_KCOLOR0,
                    CColor::Lerp(mCloudColor1, mCloudColor2, mCloudColorLerp).GetGXColor());
  CGX::SetTevKColor(GX_KCOLOR1, CColor::Lerp(mAddColor1, mAddColor2, mAddColorLerp).GetGXColor());

  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_KONST);
  CGX::SetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
  CGX::SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE2);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD2, GX_TEXMAP1, GX_COLOR_NULL);
  CGX::SetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetNumTevStages(3);
  CGX::SetNumChans(0);

  float fadeMtx[2][4] = {{0.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 0.f}};
  fadeMtx[0][1] = 1.f / mFadeOffSize;
  fadeMtx[0][3] = 1.f + -mRange / mFadeOffSize;

  const float time = CGraphics::GetSecondsMod900();
  const float invCloudScale = 1.f / mCloudScale;
  float cloudMtx1[2][4] = {{0.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 0.f}};
  float cloudMtx2[2][4] = {{0.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 0.f}};
  cloudMtx1[0][1] = invCloudScale;
  cloudMtx1[0][3] = time;
  cloudMtx1[1][2] = invCloudScale;
  cloudMtx1[1][3] = 0.5f * time;
  cloudMtx2[0][2] = 1.4f * invCloudScale;
  cloudMtx2[0][3] = 0.3f * time;
  cloudMtx2[1][1] = 1.5f * invCloudScale;
  cloudMtx2[1][3] = 1.15f * time;

  GXLoadTexMtxImm(reinterpret_cast< MtxPtr >(fadeMtx), GX_TEXMTX0, GX_MTX2x4);
  GXLoadTexMtxImm(reinterpret_cast< MtxPtr >(cloudMtx1), GX_TEXMTX1, GX_MTX2x4);
  GXLoadTexMtxImm(reinterpret_cast< MtxPtr >(cloudMtx2), GX_TEXMTX2, GX_MTX2x4);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX1, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX2, false, GX_PTIDENTITY);
  CGX::SetNumTexGens(3);

  static const uchar skNoisePattern[32] = {0xFF, 0xDB, 0xB7, 0x93, 0x6F, 0x4B, 0x27, 0x00,
                                           0xFF, 0xDB, 0xB7, 0x93, 0x6F, 0x4B, 0x27, 0x00,
                                           0xFF, 0xDB, 0xB7, 0x93, 0x6F, 0x4B, 0x27, 0x00,
                                           0xFF, 0xDB, 0xB7, 0x93, 0x6F, 0x4B, 0x27, 0x00};
  CTexture::InvalidateTexmap(GX_TEXMAP0);
  GXTexObj texObj;
  GXInitTexObj(&texObj, const_cast< uchar* >(skNoisePattern), 8, 4, GX_TF_I8, GX_CLAMP, GX_CLAMP,
               false);
  GXInitTexObjLOD(&texObj, GX_LINEAR, GX_LINEAR, 0.f, 0.f, 0.f, true, false, GX_ANISO_1);
  GXLoadTexObj(&texObj, GX_TEXMAP0);
  CCubeRenderer::That()->GetDarkWorldCloud()->Load(GX_TEXMAP1, CTexture::kCM_Mirror);

  CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_Or, kAF_Always, 0);
  gpRender->SetDepthReadWrite(true, false);
  CGraphics::SetCullMode(kCM_None);
  CGX::ResetVtxDescv();
  CGX::SetVtxDesc(GX_VA_POS, GX_DIRECT);
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, mTexture->GetHeight() + 1);
  GXPosition3f32(0.f, 0.f, 0.f);

  const void* depthData = mTexture->GetConstBitMapData(0);
  const float nearZ = 0.2f;
  const float range = mRange;
  const int stride = mTexture->GetWidth();
  const int column = stride / 2;
  int rowOffset = 0;
  for (int i = 0; i < mTexture->GetHeight(); ++i, rowOffset += stride) {
    const ushort depth = __lhbrx(const_cast< void* >(depthData), (column + rowOffset) * 2);
    const float eyeDepth = (nearZ * -range) / (depth / 65535.f * (range - nearZ) - range);
    GXPosition3f32(0.f, eyeDepth, tanAngle * eyeDepth);
    tanAngle -= step;
  }
  CGX::End();
  DCInvalidateRange(const_cast< void* >(depthData),
                    OSRoundUp32B(mTexture->GetWidth() * mTexture->GetHeight() * 2));
  CGraphics::SetCullMode(kCM_Front);
}

void CSplitterBeamEffect::PreRenderAllViewports(CStateManager& mgr) {
  const CTransform4f& xf = GetTransform();
  const CVector3f origin = GetTranslation();
  CAABox bounds(origin, origin);
  const float tanHalfAngle = tan(0.5f * mCurrentAngle);
  const float halfWidth = tanHalfAngle * mRange;
  bounds.AccumulateBounds(xf * CVector3f(0.f, mRange, halfWidth));
  bounds.AccumulateBounds(xf * CVector3f(0.f, mRange, -halfWidth));
  SetOtherBounds(bounds);
  SetRenderBounds(bounds);
  if (GetRenderBoundsDirty()) {
    UpdatePortalSystemState(mgr);
    SetRenderBoundsDirty(false);
  }
}
