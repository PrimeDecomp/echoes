#include "MetroidPrime/CVisorFlare.hpp"

#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "dolphin/os/OSCache.h"
#include "rstl/math.hpp"

CVisorFlare::CFlareDef::CFlareDef(const TToken< CTexture >& tex, float pos, float scale, uint color)
: mTex(tex), mPos(pos), mScale(scale), mColor(color) {
  mTex.Lock();
}

CVisorFlare::CVisorFlare(EBlendMode blendMode, bool distanceScaled, float fadeTime,
                         float angularFalloff, float rotationScale, uint darkVisorMode,
                         uint combatVisorMode, const rstl::vector< CFlareDef >& flares,
                         bool smallOcclusionTest, bool noOcclusionTest)
: mBlendMode(blendMode)
, mFlareDefs(flares)
, mAngularFalloff(angularFalloff)
, mRotationScale(rotationScale)
, mIntensity(0.f)
, mDarkVisorMode(darkVisorMode)
, mCombatVisorMode(combatVisorMode)
, mDistanceScaled(distanceScaled)
, mSmallOcclusionTest(smallOcclusionTest)
, mOutsideFrustum(true)
, mNoOcclusionTest(noOcclusionTest)
, mOcclusionAverage(smallOcclusionTest ? 2 : 10)
, mOcclusionWarmupFrames(4)
, mSavedFramebuffer(kTF_RGBA8, smallOcclusionTest ? 8 : 64, smallOcclusionTest ? 4 : 64, 1)
, mOcclusionTexture(kTF_I8, smallOcclusionTest ? 8 : 32, smallOcclusionTest ? 4 : 32, 1) {}

CVisorFlare::~CVisorFlare() {
  mSavedFramebuffer.ScheduleDeletion();
  mOcclusionTexture.ScheduleDeletion();
}

void CVisorFlare::Update(float dt, const CVector3f& pos, const CActor* actor, CStateManager& mgr) {
  const CPlayerState::EPlayerVisor visor = mgr.GetPlayerState(0)->GetCurrentVisor();
  if ((visor == CPlayerState::kPV_Combat ||
       (mDarkVisorMode != 1 && visor == CPlayerState::kPV_Dark)) &&
      mgr.GetPlayer(0)->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
    const rstl::optional_object< float > average = mOcclusionAverage.GetAverage();
    const float occlusion = average ? *average : 1.f;
    const CGameCamera& camera = *mgr.GetCameraManager(0)->GetCurrentCamera(mgr, true);
    const CVector3f direction = pos - camera.GetTransform().GetTranslation();
    mIntensity = 1.f - occlusion;
    const float dot = CVector3f::Dot(direction.AsNormalized(), camera.GetTransform().GetForward());
    mIntensity *= rstl::max_val(0.f, 1.f - 4.f * mAngularFalloff * (1.f - dot));
  }
}

void CVisorFlare::UpdateFrustum(const CStateManager& mgr, const CVector3f& pos) {
  mOutsideFrustum = !mgr.GetFrustumPlanes().PointInFrustumPlanes(pos);
}

void CVisorFlare::Render(const CVector3f& pos, const CActor& actor, const CStateManager& mgr) {
  if (mgr.GetWorld()->GetAreaAlways(actor.GetCurrentAreaId()).GetOcclusionState() !=
      CGameArea::kOS_Visible) {
    return;
  }
  switch (mgr.GetPlayerState()->GetActiveVisor(mgr)) {
  case CPlayerState::kPV_Combat:
    if (mCombatVisorMode != 0) {
      return;
    }
    break;
  case CPlayerState::kPV_Dark:
    if (mDarkVisorMode == 1) {
      return;
    }
    break;
  default:
    return;
  }
  const int playerIndex = mgr.GetCurrentRenderPlayerIndex();
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  if (mgr.GetCurrentRenderPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
    UpdateOcclusion(pos, mgr, playerIndex);
    RenderFlares(pos, mgr);
  }
}

void CVisorFlare::UpdateOcclusion(const CVector3f& pos, const CStateManager& mgr, int playerIndex) {
  const CViewport& viewport = CGraphics::GetViewport();
  if (mOutsideFrustum) {
    mOcclusionAverage.AddValue(1.f);
    return;
  }
  if (mNoOcclusionTest) {
    mOcclusionAverage.AddValue(0.f);
    return;
  }

  const short width = mSmallOcclusionTest ? 8 : 64;
  const short height = mSmallOcclusionTest ? 4 : 64;
  const int framebufferWidth = CGraphics::GetRenderMode().fbWidth;
  const int framebufferHeight = CGraphics::GetRenderMode().xfbHeight;
  CGraphics::DisableAllLights();
  const CGameCamera& camera = *mgr.GetCameraManager(playerIndex)->GetCurrentCamera(mgr, true);
  const CVector3f right = CGraphics::GetViewMatrix().GetRight();
  const CVector3f up = CGraphics::GetViewMatrix().GetUp();
  const CVector3f screenPos = camera.ConvertToScreenSpace(pos);
  camera.ConvertToWorldSpace(screenPos);
  const float screenX = 0.5f * screenPos.GetX() + 0.5f;
  const float screenY = 0.5f * screenPos.GetY() + 0.5f;
  const int left = (int(framebufferWidth * screenX) - width / 2) & ~1;
  const int top = (int(framebufferHeight * screenY) + height / 2) & ~1;
  const int viewLeft = (int(viewport.mWidth * screenX) - width / 2) & ~1;
  const int viewTop = (int(viewport.mHeight * screenY) + height / 2) & ~1;
  if (left < 0 || left + width > framebufferWidth || top - height < viewport.mTop ||
      top > framebufferHeight - viewport.mTop) {
    mOcclusionAverage.AddValue(1.f);
    return;
  }

  const bool videoFilter = CGraphics::GetUseVideoFilter();
  CGraphics::SetUseVideoFilter(false);
  gpRender->SetDepthReadWrite(true, false);
  const CVector3f corner = camera.ConvertToWorldSpace(
      CVector3f(2.f * (float(viewLeft) / viewport.mWidth) - 1.f,
                2.f * (float(viewTop) / viewport.mHeight) - 1.f, screenPos.GetZ()));
  const CVector3f opposite = camera.ConvertToWorldSpace(
      CVector3f(2.f * (float(viewLeft + width) / viewport.mWidth) - 1.f,
                2.f * (float(viewTop + height) / viewport.mHeight) - 1.f, screenPos.GetZ()));
  const float worldWidth = CVector3f::Dot(opposite - corner, right);
  const float worldHeight = CVector3f::Dot(opposite - corner, up);

  GXSetTexCopySrc(left, framebufferHeight - top, width, height);
  GXSetTexCopyDst(mSavedFramebuffer.GetWidth(), mSavedFramebuffer.GetHeight(), GX_TF_RGBA8, false);
  GXCopyTex(mSavedFramebuffer.Lock(), true);
  mSavedFramebuffer.UnLock();

  GXFogType fogType;
  float fogStart, fogEnd, fogNear, fogFar;
  GXColor fogColor;
  CGX::GetFog(&fogType, &fogStart, &fogEnd, &fogNear, &fogFar, &fogColor);
  CGX::SetFog(GX_FOG_NONE, 0.f, 1.f, 0.f, 1.f, fogColor);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  CGraphics::SetBlendMode(::kBM_Blend, kBF_One, kBF_One, kLO_Clear);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
  CGraphics::StreamBegin(kP_TriangleFan);
  CGraphics::StreamColor(CColor::White());
  CGraphics::StreamVertex(corner);
  CGraphics::StreamVertex(corner - worldHeight * up);
  CGraphics::StreamVertex(corner - worldHeight * up + worldWidth * right);
  CGraphics::StreamVertex(corner + worldWidth * right);
  CGraphics::StreamEnd();
  CGX::SetFog(fogType, fogStart, fogEnd, fogNear, fogFar, fogColor);

  GXSetTexCopySrc(left, framebufferHeight - top, width, height);
  if (mSmallOcclusionTest) {
    GXSetTexCopyDst(width, height, GX_CTF_R8, false);
  } else {
    GXSetTexCopyDst(width / 2, height / 2, GX_CTF_R8, true);
  }
  GXCopyTex(mOcclusionTexture.Lock(), false);
  mOcclusionTexture.UnLock();
  GXPixModeSync();
  CGraphics::SetUseVideoFilter(videoFilter);
  gpRender->SetDepthReadWrite(false, false);
  CGraphics::SetBlendMode(::kBM_Blend, kBF_One, kBF_Zero, kLO_Clear);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  CGraphics::Render2D(mSavedFramebuffer, left, top - viewport.mTop, width, -height,
                      CColor::White());

  if (mOcclusionWarmupFrames != 0) {
    --mOcclusionWarmupFrames;
    return;
  }

  const void* pixels = mOcclusionTexture.Lock();
  DCInvalidateRange(const_cast< void* >(pixels),
                    mOcclusionTexture.GetWidth() * mOcclusionTexture.GetHeight());
  int visibleSamples = 0;
  int sampleCount;
  if (mSmallOcclusionTest) {
    sampleCount = width * height;
    const uchar* sample = static_cast< const uchar* >(pixels);
    for (int i = 0; i < sampleCount; ++i) {
      if (sample[i] != 0) {
        ++visibleSamples;
      }
    }
  } else {
    sampleCount = width * height / 4 / 8;
    // The original tests each eight-byte block as a double against zero.
    const double* sample = static_cast< const double* >(pixels);
    for (int i = 0; i < sampleCount; ++i) {
      if (sample[i] != 0.0) {
        ++visibleSamples;
      }
    }
  }
  const float occlusion = 1.f - float(visibleSamples) / sampleCount;
  mOcclusionTexture.UnLock();
  mOcclusionAverage.AddValue(0.5f * (occlusion * occlusion + occlusion));
}

void CVisorFlare::RenderFlares(const CVector3f& pos, const CStateManager& mgr) const {
  CGraphics::SetDepthWriteMode(false, kE_LEqual, false);
  const CGameCamera& camera = *mgr.GetCurrentRenderCameraManager()->GetCurrentCamera(mgr, true);
  const CVector3f cameraPos = camera.GetTransform().GetTranslation();
  const CVector3f flarePos = pos;
  const CTransform4f viewMatrix = CGraphics::GetViewMatrix();
  CVector3f reflectedPos = viewMatrix.GetInverse() * flarePos;
  reflectedPos = CVector3f(-reflectedPos.GetX(), reflectedPos.GetY(), -reflectedPos.GetZ());
  const CVector3f reflectedWorldPos = viewMatrix * reflectedPos;
  const CVector3f cameraForward = camera.GetTransform().GetForward();
  const CVector3f toFlare = flarePos - cameraPos;
  if (close_enough(mIntensity, 0.f)) {
    return;
  }

  float angle = 0.f;
  if (!close_enough(mRotationScale, 0.f)) {
    const CVector3f flareDir = toFlare.DropZ().AsNormalized();
    const CVector3f cameraDir = cameraForward.DropZ().AsNormalized();
    float relativeAngle = CMath::ArcCosineR(CVector3f::Dot(flareDir, cameraDir));
    if (CVector3f::Cross(flareDir, cameraDir).GetZ() < 0.f) {
      relativeAngle = -relativeAngle;
    }
    angle = mRotationScale * relativeAngle;
  }

  SetupRenderState(mgr);
  for (int i = 0; i < mFlareDefs.size(); ++i) {
    const CFlareDef& flare = mFlareDefs[i];
    const CVector3f origin = CVector3f::Lerp(flarePos, reflectedWorldPos, flare.GetPosition());
    const CTransform4f modelMatrix = CTransform4f::LookAt(origin, cameraPos);
    gpRender->SetModelMatrix(modelMatrix);
    float scale = 0.5f * mIntensity * flare.GetScale();
    if (mDistanceScaled) {
      const CVector3f distance = origin - cameraPos;
      if (distance.CanBeNormalized()) {
        scale *= distance.Magnitude();
      }
    }
    TLockedToken< CTexture > texture(flare.GetTexture());
    if (flare.GetTexture().IsLoaded()) {
      texture->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
      float sinScale = 0.f;
      float cosScale = scale;
      if (!close_enough(angle, 0.f)) {
        sinScale = scale * sine(CRelAngle::FromRadians(angle));
        cosScale = scale * cosine(CRelAngle::FromRadians(angle));
      }
      DrawStreamed(flare.GetColor(), sinScale, cosScale);
    }
  }
  ResetRenderState(mgr);
}

void CVisorFlare::DrawStreamed(const CColor& color, float sinScale, float cosScale) const {
  CGraphics::StreamBegin(kP_TriangleStrip);
  CGraphics::StreamColor(color.WithAlphaModulatedBy(mIntensity));
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(sinScale - cosScale, 0.f, cosScale + sinScale);
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(sinScale + cosScale, 0.f, cosScale - sinScale);
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(-(sinScale + cosScale), 0.f, -(cosScale - sinScale));
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(-sinScale + cosScale, 0.f, -cosScale - sinScale);
  CGraphics::StreamEnd();
}

void CVisorFlare::SetupRenderState(const CStateManager& mgr) const {
  if (mBlendMode == kBM_Additive ||
      (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Dark &&
       mDarkVisorMode == 2)) {
    gpRender->SetBlendMode_AdditiveAlpha();
  } else {
    gpRender->SetBlendMode_AlphaBlended();
  }
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  CGraphics::SetCullMode(kCM_None);
}

void CVisorFlare::ResetRenderState(const CStateManager& mgr) const {
  CGraphics::SetCullMode(kCM_Front);
}
