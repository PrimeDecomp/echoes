#include "MetroidPrime/Player/CSamusFaceReflection.hpp"

#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

#include "Kyoto/CDependencyGroupToken.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CRelAngle.hpp"

static const char* const skFaceAssetIdName = "ACS_SamusFace";
static const CTransform4f skFaceModelViewAdjust =
    CTransform4f::Scale(0.3f) * CTransform4f::Translate(CVector3f(0.f, 0.5f, 0.f));

static CModelData* BuildFaceModel() {
  CModelData* model =
      rs_new CModelData(CAnimRes(gpResourceFactory->GetResourceIdByName(skFaceAssetIdName)->GetId(),
                                 CAnimRes::kDefaultCharIdx, CVector3f(1.f, 1.f, 1.f), 0, true));
  CAnimPlaybackParms parms(3, -1, 1.f, true);
  model->AnimationData()->SetAnimation(parms, false);
  model->LockTextures();
  return model;
}

CSamusFaceReflection::CSamusFaceReflection(const CStateManager& mgr, int playerIndex)
: mPlayerIndex(playerIndex)
, mDependencies(rs_new CDependencyGroupToken(gpSimplePool->GetObj("SamusFace_DGRP"), *gpSimplePool))
, mModelData(nullptr)
, mLights(nullptr)
, mLookRot(CQuaternion::NoRotation())
, mLookDir(CVector3f::Forward())
, x2c_(0)
, mHidden(true) {
  mDependencies->Lock();
}

static inline float FaceLookBlend(float dt, float lookDot, float freeLookSpeed) {
  const float lookAng = acosf(CMath::Limit(lookDot, 1.f));
  const float f = lookAng > 0.f ? freeLookSpeed / lookAng : 0.f;
  return CMath::Clamp(0.f, dt * 18.f * f, 1.f);
}

void CSamusFaceReflection::Update(float dt, const CStateManager& mgr, CRandom16& rand) {
  if (mDependencies.get() != nullptr) {
    if (mDependencies->IsLoaded()) {
      mModelData = BuildFaceModel();
      mDependencies = nullptr;
      mLights = rs_new CActorLights(8, CVector3f::Zero(), 4, 4);
    } else {
      return;
    }
  }

  if (const CGameCamera* const fpCam = CCameraManager::CastGameCameratoFirstPersonCamera(
          mgr.GetCameraManager(mPlayerIndex)->GetCurrentCamera(mgr, true))) {
    CVector3f camTrans = fpCam->GetTranslation();
    const CPlayer* player = mgr.GetPlayer(mPlayerIndex);
    mModelData->AdvanceAnimationIgnoreParticles(dt, rand, true);

    CActorLights& lights = *mLights;
    TAreaId areaId = mgr.GetPlayer(mPlayerIndex)->GetCurrentAreaId();
    if (areaId == kInvalidAreaId)
      return;

    const CVector3f offset(0.125f, 0.125f, 0.125f);
    const CAABox aabb(camTrans - offset, camTrans + offset);

    const CGameArea& area = mgr.GetWorld()->GetAreaAlways(areaId);
    lights.BuildFaceLightList(mgr, area, aabb, mPlayerIndex);

    const CMatrix3f matrix(fpCam->GetTransform().BuildMatrix3f());
    const CUnitVector3f lookDir(matrix.GetColumn(kDY));

    const CQuaternion xfLook1 = CQuaternion::LookAt(CUnitVector3f(lookDir), CVector3f::Forward(),
                                                    CRelAngle::FromRadians(M_2PIF));
    CQuaternion xfLook2 =
        CQuaternion::LookAt(CVector3f::Forward(), CUnitVector3f(xfLook1.Transform(mLookDir)),
                            CRelAngle::FromRadians(M_2PIF));
    xfLook2 *= xfLook2;

    const CVector3f lookCenter = xfLook2.BuildTransform().GetColumn(kDY);
    const CVector3f lookRotCenter = CVector3f(mLookRot.BuildTransform().GetColumn(kDY));
    xfLook2 = CQuaternion::SlerpLocal(
        mLookRot, xfLook2,
        FaceLookBlend(dt, CVector3f::Dot(lookRotCenter, lookCenter),
                      dt * player->GetTweakPlayer()->GetFreeLookSpeed() * 0.5f));
    mLookRot = xfLook2;
    mLookDir = lookDir;
  }
}

void CSamusFaceReflection::PreDraw(const CStateManager& mgr) {
  if (mModelData.get() != nullptr) {
    if (mPlayerIndex != mgr.GetCurrentRenderPlayerIndex() || x2c_ == 2 ||
        (mLights->GetActiveLightCount() < 1 && (x2c_ == 0 || x2c_ == 3)) ||
        CCameraManager::CastGameCameratoFirstPersonCamera(
            mgr.GetCurrentRenderCameraManager()->GetCurrentCamera(mgr, true)) == nullptr) {
      mHidden = true;
    } else {
      mHidden = false;
      mModelData->AnimationData()->PreRender();
    }
  }
}

void CSamusFaceReflection::Draw(const CStateManager& mgr) const {
  if (mModelData.get() != nullptr && !mHidden) {
    if (const CGameCamera* fpCam = CCameraManager::CastGameCameratoFirstPersonCamera(
            mgr.GetCurrentRenderCameraManager()->GetCurrentCamera(mgr, true))) {
      const CVector3f camTranslation = fpCam->GetTranslation();
      const CVector3f camYcol = fpCam->GetTransform().GetColumn(kDY);
      const CVector3f camZcol = fpCam->GetTransform().GetColumn(kDZ);

      CQuaternion camRot = CQuaternion::FromMatrix(fpCam->GetTransform());

      float dist = CTweakGui::FaceReflectionDistanceDebugValueToActualValue(
          gpTweakGui->GetFaceReflectionDistanceDebugValue());
      float height = CTweakGui::FaceReflectionHeightDebugValueToActualValue(
          gpTweakGui->GetFaceReflectionHeightDebugValue());
      float aspect = CTweakGui::FaceReflectionAspectDebugValueToActualValue(
          gpTweakGui->GetFaceReflectionAspectDebugValue());
      float orthoWidth = CTweakGui::FaceReflectionOrthoWidthDebugValueToActualValue(
          gpTweakGui->GetFaceReflectionOrthoWidthDebugValue());
      float orthoHeight = CTweakGui::FaceReflectionOrthoHeightDebugValueToActualValue(
          gpTweakGui->GetFaceReflectionOrthoHeightDebugValue());

      CTransform4f modelXf = CTransform4f((camRot * mLookRot).BuildTransform(),
                                          camTranslation + (dist * camYcol) + (height * camZcol)) *
                             skFaceModelViewAdjust;

      CGraphics::SetViewPointMatrix(fpCam->GetTransform());
      CGraphics::SetOrtho(aspect * -orthoWidth, aspect * orthoWidth, orthoHeight, -orthoHeight,
                          -10.f, 10.f);

      CActorLights* lights = x2c_ == 1 ? nullptr : mLights.get();
      if (x2c_ == 3) {
        mModelData->Render(mgr, modelXf, lights, CModelFlags::Normal());
      } else {
        const CPlayerState* playerState = mgr.GetCurrentRenderPlayerState();
        float transFactor = playerState->GetActiveVisor(mgr) == CPlayerState::kPV_Combat
                                ? playerState->GetVisorTransitionFactor()
                                : 0.f;
        if (transFactor > 0.f) {
          mModelData->Render(mgr, modelXf, nullptr,
                             CModelFlags::Additive(CColor::Black()).DepthCompareUpdate(true, true));
          mModelData->Render(mgr, modelXf, lights,
                             CModelFlags::Additive(transFactor).DepthCompareUpdate(true, false));
        }
      }
    }
  }
}
