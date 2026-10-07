#include "MetroidPrime/HUD/CHudRadarInterface.hpp"

#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiWidget.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CEulerAngles.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Enemies/CSandworm.hpp"
#include "MetroidPrime/Enemies/CSwarmBasics.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"

#include "rstl/math.hpp"

static const char skRadarRootName[] = "basewidget_radar";
static const char skRadarModelName[] = "model_radar";
static const char skRadarPaintName[] = "TXTR_RadarPaint";
static const char skBigRingName[] = "TXTR_BigRing";

static const float skRadarViewportParameters[3][4] = {
    {0.25f, 4.f, 240.f, -132.f},
    {0.18f, 4.f, 250.f, -46.f},
    {0.125f, 6.f, 120.f, -56.f},
};

CHudRadarInterface::CHudRadarInterface(CGuiFrame& frame, const CStateManager& mgr, int playerIndex,
                                       const CColor& color)
: mRadarPaint(gpSimplePool->GetObj(skRadarPaintName))
, mBigRing(gpSimplePool->GetObj(skBigRingName))
, mRadarTransform(CTransform4f::Identity())
, mVisibleGame(true)
, mVisibleDebug(true)
, mPlayerIndex(playerIndex) {
  mRadarModel = frame.FindWidget(skRadarModelName);
  mRadarRoot = frame.FindWidget(skRadarRootName);
  mCamera = frame.GetFrameCamera();
  mRadarTransform = mRadarRoot->GetO2PTransform();

  if (mRadarModel) {
    mRadarModel->SetColor(color);
  }
  if (mRadarRoot) {
    mRadarRoot->SetIsVisible(false);
  }
  mRadarPaint.Lock();
  mBigRing.Lock();
}

void CHudRadarInterface::SetColor(const CColor& color) {
  if (mRadarModel) {
    mRadarModel->SetColor(color);
  }
}

void CHudRadarInterface::Update(float dt, const CStateManager& mgr) {
  mRadarPaint.IsLoaded();
  mBigRing.IsLoaded();
}

void CHudRadarInterface::Draw(const CStateManager& mgr, float alpha) const {
  float visorAlpha = 1.f;
  {
    const CPlayerState& playerState = *mgr.GetPlayerState(mPlayerIndex);
    if (playerState.GetCurrentVisor() == CPlayerState::kPV_Scan) {
      visorAlpha = 0.f;
    } else if (playerState.GetTransitioningVisor() == CPlayerState::kPV_Scan) {
      visorAlpha = playerState.GetVisorTransitionFactor();
    }
  }
  const float radarAlpha = alpha * visorAlpha * gpGameState->GameOptions().GetHudAlpha();
  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  const float xyRadius = player.IsOverrideRadarRadius() ? player.GetRadarXYRadiusOverride()
                                                        : gpTweakGui->GetRadarWorldRadius();
  const float zRadius = player.IsOverrideRadarRadius() ? player.GetRadarZRadiusOverride()
                                                       : gpTweakGui->GetRadarWorldHalfHeight();
  const float zCloseRadius =
      player.IsOverrideRadarRadius() ? 0.667f * zRadius : gpTweakGui->GetRadarZCloseRadius();
  const float scopeRadius = gpTweakGui->GetRadarScopeCoordRadius();
  const float scopeScalar = scopeRadius / xyRadius;
  const float paintRadius = gpTweakGui->GetRadarPlayerPaintRadius();
  const CEulerAngles angles = CEulerAngles::FromQuaternion(
      player.GetCameraManager()->GetCurrentCamera(mgr, true)->GetRotation());
  const CTransform4f preTranslate =
      CTransform4f::RotateY(CAbsAngle::FromRadians(angles.GetYaw()) - CAbsAngle::FromRadians(0.f));
  const CVector3f playerPos = player.GetTranslation();
  CTransform4f postTranslate(CTransform4f::Identity());
  const float* viewportParameters = skRadarViewportParameters[mgr.GetViewportLayoutIndex()];
  mCamera->Draw(CGuiWidgetDrawParms(0.f, CVector3f::Zero()));
  postTranslate = mRadarRoot->GetWorldTransform();
  gpRender->SetBlendMode_AdditiveAlpha();
  gpRender->SetModelMatrix(postTranslate);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  gpRender->SetDepthReadWrite(false, false);

  rstl::reserved_vector< const CPlayer*, 4 > otherPlayers;
  for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
    if (i != mPlayerIndex &&
        mgr.GetPlayerState(i)->GetItemAmount(CPlayerState::kIT_Invisibility, true) == 0) {
      otherPlayers.push_back(mgr.GetPlayer(i));
    }
  }
  const SRadarPaintDrawParms parms(playerPos, preTranslate, postTranslate, scopeRadius, scopeScalar,
                                   radarAlpha, xyRadius, zRadius, zCloseRadius);
  const float playerRadius = paintRadius * viewportParameters[1];
  const uint teamIndex = mgr.GetPlayerState(mPlayerIndex)->GetTeamIndex();
  DrawRadarPaint(playerPos, playerRadius, 1.f, parms, gpTweakGuiColors->GetRadarPlayerPaintColor());
  if (player.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Echo) {
    DrawEchoPulse(player, parms);
  }

  if (mgr.IsMultiplayer()) {
    for (AUTO(it, otherPlayers.begin()); it != otherPlayers.end(); ++it) {
      const CPlayer& other = **it;
      const CColor& color = teamIndex == other.GetPlayerState()->GetTeamIndex()
                                ? gpTweakGuiColors->GetRadarEnemyTeamPaintColor()
                                : gpTweakGuiColors->GetRadarFriendTeamPaintColor();
      DrawRadarPaint(other.GetTranslation(), playerRadius, 1.f, parms, color);
    }
  } else {
    const CMaterialFilter filter(CMaterialList(kMT_Target, kMT_RadarObject),
                                 CMaterialList(kMT_ExcludeFromRadar, kMT_Player),
                                 CMaterialFilter::kFT_IncludeExclude);
    CAABox bounds(CAABox::MakeMaxInvertedBox());
    const CVector3f extent(xyRadius, xyRadius, zRadius);
    bounds.AccumulateBounds(playerPos + -extent);
    bounds.AccumulateBounds(playerPos + extent);
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    mgr.BuildNearList(nearList, bounds, filter, nullptr);
    int count = 0;
    for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
      if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(*it))) {
        if (actor->GetActive()) {
          float enemyRadius = paintRadius * viewportParameters[1];
          if (const CSwarmBasics* swarm = TCastToConstPtr< CSwarmBasics >(actor)) {
            const int boidCount = swarm->GetBoidCount();
            enemyRadius *= 0.5f;
            for (int i = 0; i < boidCount; ++i) {
              if (swarm->GetLockOnLocationValid(i)) {
                const CVector3f position = swarm->GetLockOnLocation(i);
                DrawRadarPaint(position, enemyRadius, 0.5f, parms,
                               gpTweakGuiColors->GetRadarEnemyPaintColor());
              }
            }
          } else if (const CSandworm* sandworm = TCastToConstPtr< CSandworm >(actor)) {
            enemyRadius *= 0.7f;
            for (int i = 0; i < GetSandwormRadarPointCount(sandworm); ++i) {
              const CColor& color = gpTweakGuiColors->GetRadarEnemyPaintColor();
              DrawRadarPaint(GetSandwormRadarPointPosition(sandworm, i), enemyRadius, 0.5f, parms,
                             color);
            }
          } else if (!TCastToConstPtr< CSandwormEye >(actor)) {
            DrawRadarPaint(actor->GetTranslation(), enemyRadius, 1.f, parms,
                           gpTweakGuiColors->GetRadarEnemyPaintColor());
          }
          if (++count >= 128) {
            break;
          }
        }
      }
    }
  }
  gpRender->SetDepthReadWrite(true, true);
}

void CHudRadarInterface::DrawRadarPaint(const CVector3f& position, float radius, float alpha,
                                        const SRadarPaintDrawParms& parms,
                                        const CColor& color) const {
  const CTexture* texture = mRadarPaint.GetObject();
  if (!texture) {
    return;
  }

  const CVector2f playerPos(parms.mPlayerPos.GetX(), parms.mPlayerPos.GetY());
  const CVector2f delta(position.GetX() - parms.mPlayerPos.GetX(),
                        position.GetY() - parms.mPlayerPos.GetY());
  const float zDelta = CMath::AbsF(position.GetZ() - parms.mPlayerPos.GetZ());
  const float distance = delta.Magnitude();
  if (distance <= parms.mXyRadius && zDelta <= parms.mZRadius) {
    const float zClose = parms.mZCloseRadius;
    if (zDelta > zClose) {
      alpha *= 1.f - (zDelta - zClose) / (parms.mZRadius - zClose);
    }
    const CVector2f xyPosition(position.GetX(), position.GetY());
    const CVector2f scaled = (xyPosition - playerPos) * parms.mScopeScalar;
    const CVector3f paintPosition =
        parms.mPreTranslate * CVector3f(scaled.GetX(), 0.f, scaled.GetY());

    texture->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
    gpRender->SetModelMatrix(parms.mPostTranslate * CTransform4f::Translate(paintPosition));
    CGraphics::StreamColor(color.WithAlphaModulatedBy(alpha * parms.mAlpha));
    DoDrawRadarPaint(radius);
  } else {
    const CVector2f direction = delta / distance;
    const CVector2f scaled = direction * parms.mXyRadius * parms.mScopeScalar;
    const CVector3f edgePosition =
        parms.mPreTranslate * CVector3f(scaled.GetX(), 0.f, scaled.GetY());
    const CVector3f forward =
        parms.mPreTranslate * CVector3f(direction.GetX(), 0.f, direction.GetY());
    const CVector3f up(0.f, 1.f, 0.f);
    const CVector3f right = CVector3f::Cross(up, forward);
    const CTransform4f edgeTransform(right.GetX(), up.GetX(), forward.GetX(), edgePosition.GetX(),
                                     right.GetY(), up.GetY(), forward.GetY(), edgePosition.GetY(),
                                     right.GetZ(), up.GetZ(), forward.GetZ(), edgePosition.GetZ());

    texture->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
    gpRender->SetModelMatrix(parms.mPostTranslate * edgeTransform);
    CGraphics::StreamColor(color.WithAlphaModulatedBy(alpha * parms.mAlpha));
    DoDrawRadarPaint(radius);
  }
}

void CHudRadarInterface::DoDrawRadarPaint(float radius) const {
  radius = 4.f * radius;
  CGraphics::StreamBegin(kP_TriangleStrip);
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(CVector3f(-radius, 0.f, radius));
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(CVector3f(-radius, 0.f, -radius));
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(CVector3f(radius, 0.f, radius));
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(CVector3f(radius, 0.f, -radius));
  CGraphics::StreamEnd();
}

void CHudRadarInterface::DrawEchoPulse(const CPlayer& player,
                                       const SRadarPaintDrawParms& parms) const {
  gpRender->SetModelMatrix(parms.mPostTranslate);
  const CTexture* texture = mBigRing.GetObject();
  if (!texture) {
    return;
  }

  texture->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  const float scopeRadius =
      player.GetEchoPulsePhase() * gpTweakGui->GetEchoPulseRadiusScale() / parms.mXyRadius;
  const float clampedRadius = rstl::min_val(1.3f, scopeRadius);
  CGraphics::StreamColor(gpTweakGuiColors->GetRadarEchoPulseColor().WithAlphaOf(
      parms.mAlpha * (1.f - rstl::min_val(1.f, rstl::max_val(0.f, scopeRadius) / 1.2f))));

  const float radius = 1.6f * clampedRadius;
  CGraphics::StreamBegin(kP_TriangleStrip);
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(CVector3f(-radius, 0.f, radius));
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(CVector3f(-radius, 0.f, -radius));
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(CVector3f(radius, 0.f, radius));
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(CVector3f(radius, 0.f, -radius));
  CGraphics::StreamEnd();
}
