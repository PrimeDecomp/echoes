#include "MetroidPrime/Player/CPlayer.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/Player/CGrappleArm.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "MetroidPrime/CAxisAngle.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CSwarmBasics.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayerCameraBob.hpp"
#include "MetroidPrime/Player/CPlayerTargeting.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPointOfInterest.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerControls.hpp"
#include "dolphin/os/OSCache.h"
#include <math.h>

namespace {
enum EOrbitValidationResult {
  kOVR_OK = 0,
  kOVR_InvalidTarget = 1,
  kOVR_PlayerNotReadyToTarget = 2,
  kOVR_NonTargetableTarget = 3,
  kOVR_ExtremeHorizonAngle = 4,
  kOVR_BrokenLookAngle = 5,
  kOVR_OccludedTarget = 6,
  kOVR_TargetingThroughDoor = 7,
};

const CMaterialList kLineOfSightIncludeList = CMaterialList(kMT_Unknown59);
const CMaterialList kLineOfSightExcludeList =
    CMaterialList(kMT_NoPlatformCollision, kMT_ScanPassthrough, kMT_Character);
const CMaterialFilter kLineOfSightFilter =
    CMaterialFilter::MakeIncludeExclude(kLineOfSightIncludeList, kLineOfSightExcludeList);
const CMaterialList kPlayerLineOfSightExcludeList =
    CMaterialList(kMT_NoPlatformCollision, kMT_ScanPassthrough, kMT_Character, kMT_Player);
const CMaterialFilter kPlayerLineOfSightFilter =
    CMaterialFilter::MakeIncludeExclude(kLineOfSightIncludeList, kPlayerLineOfSightExcludeList);
CAABox staticBox(CVector3f(0.f, 0.f, 0.f), CVector3f(1.f, 1.f, 1.f));

CAABox BuildNearListBox(bool cropBottom, const CTransform4f& xf, float x, float z, float y) {
  const CAABox bounds(-x, cropBottom ? 0.f : -y, -z, x, y, z);
  return bounds.GetTransformedAABox(xf);
}
} // namespace

// Definitions follow reverse target order for deferred inlining.

bool CPlayer::ValidateOrbitTargetIdAndPointer(TUniqueId target, const CStateManager& mgr) const {
  if (target.value == kInvalidUniqueId.value) {
    return false;
  }
  return TCastToConstPtr< CActor >(mgr.GetObjectById(target)) != nullptr;
}

int CPlayer::ValidateCurrentOrbitTargetId(CStateManager& mgr) {
  const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(GetOrbitTargetId()));
  if (!act || !(act->GetValidTargetPlayers() & (1 << mgr.MaskUIdNumPlayers(GetUniqueId()))) ||
      !act->GetActive()) {
    return kOVR_InvalidTarget;
  }
  if (!act->GetMaterialList().HasMaterial(kMT_Orbit)) {
    if (!act->GetMaterialList().HasMaterial(kMT_Scannable) ||
        mPlayerState->GetCurrentVisor() != CPlayerState::kPV_Scan) {
      return kOVR_NonTargetableTarget;
    }
  }
  const int result = ValidateOrbitTargetId(GetOrbitTargetId(), mgr);
  if (result != kOVR_OK) {
    return result;
  }
  if (mPlayerState->GetCurrentVisor() == CPlayerState::kPV_Scan &&
      act->GetCurrentAreaId() != GetCurrentAreaId()) {
    return kOVR_OccludedTarget;
  }

  const CVector3f orbitPosition = act->GetOrbitPosition(mgr);
  CVector3f eyeToOrbitFlat = orbitPosition - GetEyePosition();
  eyeToOrbitFlat.SetZ(0.f);
  if (eyeToOrbitFlat.CanBeNormalized()) {
    const float angle = acosf(CMath::Limit(
        CVector3f::Dot(eyeToOrbitFlat.AsNormalized(), GetTransform().GetForward()), 1.f));
    if (mOrbitLockEstablished) {
      if (angle >= GetTweakPlayer()->GetOrbitHorizAngle()) {
        return kOVR_BrokenLookAngle;
      }
    } else if (angle <= M_PIF / 180.f) {
      mOrbitLockEstablished = true;
    }
  }

  if ((mgr.GetUpdateFrameIdx() & 3) == GetPlayerIndex()) {
    const CVector3f eyePosition = GetEyePosition();
    const CVector3f eyeToOrbit = orbitPosition - eyePosition;
    if (eyeToOrbit.CanBeNormalized()) {
      if (mPlayerState->GetCurrentVisor() == CPlayerState::kPV_Scan) {
        mOrbitTargetLineOfSightClear = true;
      } else {
        mOrbitTargetLineOfSightClear = mgr.RayCollideWorld(eyePosition, eyePosition + eyeToOrbit,
                                                           kPlayerLineOfSightFilter, act);
      }
      if (!mOrbitTargetLineOfSightClear && act->GetCurrentAreaId() != GetCurrentAreaId()) {
        const rstl::list< CEntity* >& doors = mgr.GetDoorList();
        for (rstl::list< CEntity* >::const_iterator it = doors.begin(); it != doors.end(); ++it) {
          const CScriptDoor* door = static_cast< const CScriptDoor* >(*it);
          if (door && door->GetCurrentAreaId() == GetCurrentAreaId()) {
            const CScriptDock* dock =
                TCastToConstPtr< CScriptDock >(mgr.GetObjectById(door->GetConnectedDockID()));
            if (dock && dock->GetCurrentConnectedAreaId(mgr) == act->GetCurrentAreaId() &&
                !door->IsOpen()) {
              return kOVR_TargetingThroughDoor;
            }
          }
        }
      }
    }
  }
  return mOrbitTargetLineOfSightClear ? kOVR_OK : kOVR_OccludedTarget;
}

int CPlayer::ValidateOrbitTargetId(TUniqueId target, CStateManager& mgr) const {
  if (target.value == kInvalidUniqueId.value) {
    return kOVR_InvalidTarget;
  }
  const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(target));
  if (!act || !(act->GetValidTargetPlayers() & (1 << mgr.MaskUIdNumPlayers(GetUniqueId()))) ||
      !act->GetActive()) {
    return kOVR_InvalidTarget;
  }
  if (GetStaticTimer() != 0.f) {
    return kOVR_PlayerNotReadyToTarget;
  }

  const CVector3f orbitPosition = act->GetOrbitPosition(mgr);
  const CVector3f eyeToOrbit = orbitPosition - GetEyePosition();
  const CVector3f eyeToOrbitFlat(eyeToOrbit.ToVec2f(), 0.f);
  if (eyeToOrbitFlat.CanBeNormalized() && eyeToOrbitFlat.Magnitude() > 1.f) {
    const float angle = static_cast< float >(
        asin(CMath::Limit(CMath::AbsF(eyeToOrbit.GetZ()) / eyeToOrbit.Magnitude(), 1.f)));
    if ((eyeToOrbit.GetZ() >= 0.f && angle >= GetTweakPlayer()->GetOrbitUpperAngle()) ||
        (eyeToOrbit.GetZ() < 0.f && angle >= GetTweakPlayer()->GetOrbitLowerAngle())) {
      return kOVR_ExtremeHorizonAngle;
    }
  } else {
    return kOVR_ExtremeHorizonAngle;
  }
  const CScriptGrapplePoint* point =
      TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(target));
  if (point && eyeToOrbitFlat.Magnitude() > GetTweakPlayer()->GetGrappleDistance() + 5.f) {
    return kOVR_PlayerNotReadyToTarget;
  }

  const uchar flags = act->GetTargetableVisorFlags();
  switch (mPlayerState->GetCurrentVisor()) {
  case CPlayerState::kPV_Combat:
    if ((flags & 1) == 0) {
      return kOVR_PlayerNotReadyToTarget;
    }
    break;
  case CPlayerState::kPV_Scan:
    if ((flags & 2) == 0) {
      return kOVR_PlayerNotReadyToTarget;
    }
    if (GetCurrentAreaId() != act->GetCurrentAreaId()) {
      return kOVR_TargetingThroughDoor;
    }
    break;
  case CPlayerState::kPV_Dark:
    if ((flags & 4) == 0) {
      return kOVR_PlayerNotReadyToTarget;
    }
    break;
  case CPlayerState::kPV_Echo:
    if ((flags & 8) == 0) {
      return kOVR_PlayerNotReadyToTarget;
    }
    break;
  }

  const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(target));
  if (player) {
    if (player->GetPlayerState()->GetItemAmount(CPlayerState::kIT_Invisibility, true) != 0 &&
        mPlayerState->GetCurrentVisor() != CPlayerState::kPV_Dark) {
      return kOVR_PlayerNotReadyToTarget;
    }
    if (player->GetMorphballTransitionState() != kMS_Unmorphed) {
      if (gkMorphBallOrbitMode == 2 && mPlayerState->GetCurrentVisor() != CPlayerState::kPV_Echo) {
        return kOVR_PlayerNotReadyToTarget;
      }
      if (player->GetMorphBall()->InScrewAttackMode()) {
        return kOVR_PlayerNotReadyToTarget;
      }
    }
  } else if (mgr.IsMultiplayer() &&
             mPlayerState->GetItemAmount(CPlayerState::kIT_ScanVirus, true) != 0) {
    return kOVR_PlayerNotReadyToTarget;
  }
  return kOVR_OK;
}

float CPlayer::GetOrbitMaxTargetDistance() const {
  float distance = GetTweakPlayer()->GetOrbitMaxTargetDistance();
  if (mPlayerState->GetCurrentVisor() == CPlayerState::kPV_Scan) {
    distance = GetTweakPlayer()->GetScanMaxTargetDistance();
  }
  return distance;
}

float CPlayer::GetOrbitMaxLockDistance(CStateManager& mgr) const {
  float distance = GetTweakPlayer()->GetOrbitMaxLockDistance();
  if (mPlayerState->GetCurrentVisor() == CPlayerState::kPV_Scan) {
    distance = GetTweakPlayer()->GetScanMaxLockDistance();
  }
  return distance;
}

void CPlayer::UpdateOrbitTarget(CStateManager& mgr) {
  if (!ValidateOrbitTargetIdAndPointer(GetOrbitTargetId(), mgr)) {
    SetOrbitTargetId(kInvalidUniqueId, mgr);
  }
  if (!ValidateOrbitTargetIdAndPointer(GetOrbitNextTargetId(), mgr)) {
    SetOrbitNextTargetId(kInvalidUniqueId);
  }
  CVector3f playerToPoint = mOrbitPoint - GetTranslation();
  playerToPoint.SetZ(0.f);
  const float distance = playerToPoint.Magnitude();
  switch (mOrbitState) {
  case kOS_OrbitObject: {
    const CActor* const act = static_cast< const CActor* >(mgr.GetObjectById(GetOrbitTargetId()));
    if (act && act->GetDoTargetDistanceTest() &&
        (distance >= GetOrbitMaxLockDistance(mgr) || distance < .5f)) {
      if (distance < .5f) {
        SetOrbitRequest(kOR_BadVerticalAngle, mgr);
      } else {
        ActivateOrbitSource(mgr);
      }
    } else {
      UpdateOrbitPosition(GetTweakPlayer()->GetOrbitNormalDistance(mOrbitType), mgr);
    }
    break;
  }
  case kOS_OrbitPoint: {
    if (GetTweakPlayerControls()->GetOrbitPointFixedOffset() &&
        CMath::AbsF(mOrbitVector.GetZ()) > GetTweakPlayer()->GetOrbitFixedOffsetZDiff()) {
      UpdateOrbitFixedPosition();
      return;
    }
    if (distance < CalculateOrbitMinDistance(mOrbitType)) {
      UpdateOrbitPosition(CalculateOrbitMinDistance(mOrbitType), mgr);
    }
    if (distance > GetTweakPlayer()->GetOrbitMaxDistance(mOrbitType)) {
      UpdateOrbitPosition(GetTweakPlayer()->GetOrbitMaxDistance(mOrbitType), mgr);
    }
    if (mLookButtonHeld) {
      SetOrbitPosition(GetTweakPlayer()->GetOrbitNormalDistance(mOrbitType));
    }
    const CVector3f eyeToPoint = mOrbitPoint - GetEyePosition();
    const float angle = static_cast< float >(
        asin(CMath::Limit(CMath::AbsF(eyeToPoint.GetZ()) / eyeToPoint.Magnitude(), 1.f)));
    if ((eyeToPoint.GetZ() >= 0.f && angle >= GetTweakPlayer()->GetOrbitUpperAngle()) ||
        (eyeToPoint.GetZ() < 0.f && angle >= GetTweakPlayer()->GetOrbitLowerAngle())) {
      SetOrbitRequest(kOR_BadVerticalAngle, mgr);
    }
    break;
  }
  case kOS_OrbitCarcass: {
    if (mLookButtonHeld) {
      SetOrbitPosition(mOrbitPointDistance);
    }
    if (distance < CalculateOrbitMinDistance(mOrbitType)) {
      UpdateOrbitPosition(CalculateOrbitMinDistance(mOrbitType), mgr);
      mOrbitPointDistance = CalculateOrbitMinDistance(mOrbitType);
    }
    if (distance > GetTweakPlayer()->GetOrbitMaxDistance(mOrbitType)) {
      UpdateOrbitPosition(GetTweakPlayer()->GetOrbitMaxDistance(mOrbitType), mgr);
      mOrbitPointDistance = GetTweakPlayer()->GetOrbitMaxDistance(mOrbitType);
    }
    break;
  }
  case kOS_NoOrbit:
    SetOrbitTargetId(kInvalidUniqueId, mgr);
    break;
  case kOS_ForcedOrbitObject:
  case kOS_Grapple:
  default:
    break;
  }
  UpdateOrbitZPosition();
}

void CPlayer::UpdateOrbitOrientation(CStateManager& mgr) {
  if (mMorphBallState != kMS_Unmorphed) {
    return;
  }
  switch (mOrbitState) {
  case kOS_NoOrbit:
    return;
  case kOS_OrbitPoint:
    if (mInFreeLook) {
      return;
    }
  case kOS_OrbitObject:
  case kOS_OrbitCarcass:
  case kOS_ForcedOrbitObject: {
    CVector3f playerToPoint = mOrbitPoint - GetTranslation();
    if (!mOrbitLockEstablished) {
      playerToPoint = mCameraManager->GetFirstPersonCamera()->GetTransform().GetForward();
    }
    playerToPoint.SetZ(0.f);
    if (playerToPoint.CanBeNormalized()) {
      CTransform4f xf = CTransform4f::LookAt(CVector3f::Zero(), playerToPoint);
      xf.SetTranslation(GetTranslation());
      SetTransform(xf);
    }
    break;
  }
  case kOS_Grapple:
  default:
    break;
  }
}

void CPlayer::UpdateOrbitSelection(const CFinalInput& input, CStateManager& mgr) {
  const CScriptGrapplePoint* const curPoint =
      TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(mOrbitTargetId));
  const CScriptGrapplePoint* const nextPoint =
      TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(mOrbitNextTargetId));
  if (curPoint || (mOrbitState == kOS_Grapple && !nextPoint)) {
    mOrbitNextTargetId = kInvalidUniqueId;
    return;
  }
  if (mControlMapper.GetPressInput(CControlMapper::kC_OrbitObject, input) &&
      mOrbitNextTargetId != kInvalidUniqueId) {
    SetOrbitTargetId(mOrbitNextTargetId, mgr);
    if (ValidateAimTargetId(GetOrbitTargetId(), mgr)) {
      SetAimTarget(GetOrbitTargetId());
    }
    SetOrbitState(kOS_OrbitObject, mgr);
    UpdateOrbitPosition(GetTweakPlayer()->GetOrbitNormalDistance(mOrbitType), mgr);
  }
}

void CPlayer::ActivateOrbitSource(CStateManager& mgr) {
  switch (mOrbitSource) {
  case 0:
  default:
    OrbitCarcass(mgr);
    break;
  case 1:
    SetOrbitRequest(kOR_InvalidateTarget, mgr);
    break;
  case 2:
    OrbitPoint(kOT_Far, mgr);
    break;
  }
}

void CPlayer::UpdateOrbitInput(const CFinalInput& input, float dt, CStateManager& mgr) {
  if (mMorphBallState != kMS_Unmorphed) {
    return;
  }
  if (mOrbitPreventionTimer > 0.f) {
    return;
  }
  if (mInFreeLook) {
    mOrbitRequest = kOR_InvalidateTarget;
  }
  UpdateOrbitableObjects(mgr);
  if (mControlMapper.GetDigitalInput(CControlMapper::kC_OrbitClose, input) ||
      mControlMapper.GetDigitalInput(CControlMapper::kC_OrbitFar, input) ||
      mControlMapper.GetDigitalInput(CControlMapper::kC_OrbitObject, input)) {
    switch (mOrbitState) {
    case kOS_NoOrbit:
      if (mControlMapper.GetPressInput(CControlMapper::kC_OrbitObject, input)) {
        SetOrbitTargetId(GetOrbitNextTargetId(), mgr);
        if (mOrbitTargetId != kInvalidUniqueId) {
          if (ValidateAimTargetId(GetOrbitTargetId(), mgr)) {
            SetAimTarget(GetOrbitTargetId());
          }
          SetOrbitState(kOS_OrbitObject, mgr);
          UpdateOrbitPosition(GetTweakPlayer()->GetOrbitNormalDistance(mOrbitType), mgr);
        }
      } else {
        if (mControlMapper.GetPressInput(CControlMapper::kC_OrbitFar, input)) {
          OrbitPoint(kOT_Far, mgr);
        }
        if (mControlMapper.GetPressInput(CControlMapper::kC_OrbitClose, input)) {
          OrbitPoint(kOT_Close, mgr);
        }
      }
      break;
    case kOS_Grapple:
      if (mOrbitTargetId == kInvalidUniqueId) {
        BreakGrapple(kOR_StopOrbit, mgr);
      }
      break;
    case kOS_OrbitObject:
      if (TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(GetOrbitTargetId()))) {
        if (ValidateCurrentOrbitTargetId(mgr) == kOVR_OK) {
          UpdateOrbitPosition(GetTweakPlayer()->GetOrbitNormalDistance(mOrbitType), mgr);
        } else {
          BreakGrapple(kOR_InvalidateTarget, mgr);
        }
      } else {
        const int result = ValidateCurrentOrbitTargetId(mgr);
        if (result == kOVR_OK) {
          UpdateOrbitPosition(GetTweakPlayer()->GetOrbitNormalDistance(mOrbitType), mgr);
          mOrbitOcclusionTimer = 0.f;
        } else {
          switch (result) {
          case kOVR_OccludedTarget: {
            const float breakTime = GetTweakPlayer()->GetOrbitBreakOnOccludedTime();
            if (breakTime != 0.f) {
              mOrbitOcclusionTimer += dt;
              if (mOrbitOcclusionTimer >= breakTime) {
                ActivateOrbitSource(mgr);
              }
            }
            break;
          }
          case kOVR_BrokenLookAngle:
            OrbitPoint(kOT_Far, mgr);
            mOrbitOcclusionTimer = 0.f;
            break;
          case kOVR_ExtremeHorizonAngle:
            SetOrbitRequest(kOR_BadVerticalAngle, mgr);
            mOrbitOcclusionTimer = 0.f;
            break;
          case kOVR_TargetingThroughDoor:
            SetOrbitRequest(kOR_TargetingThroughDoor, mgr);
            break;
          default:
            ActivateOrbitSource(mgr);
            mOrbitOcclusionTimer = 0.f;
            break;
          }
        }
      }
      UpdateOrbitSelection(input, mgr);
      break;
    case kOS_OrbitPoint:
      if (mControlMapper.GetPressInput(CControlMapper::kC_OrbitObject, input)) {
        SetOrbitTargetId(GetOrbitNextTargetId(), mgr);
        if (mOrbitTargetId != kInvalidUniqueId) {
          if (ValidateAimTargetId(GetOrbitTargetId(), mgr)) {
            SetAimTarget(GetOrbitTargetId());
          }
          SetOrbitState(kOS_OrbitObject, mgr);
          UpdateOrbitPosition(GetTweakPlayer()->GetOrbitNormalDistance(mOrbitType), mgr);
        }
      } else {
        switch (mOrbitType) {
        case kOT_Default:
          break;
        case kOT_Far:
          if (mControlMapper.GetDigitalInput(CControlMapper::kC_OrbitClose, input)) {
            mOrbitType = kOT_Close;
            SetOrbitPosition(GetTweakPlayer()->GetOrbitNormalDistance(mOrbitType));
          }
          break;
        case kOT_Close:
          if (mControlMapper.GetDigitalInput(CControlMapper::kC_OrbitFar, input) &&
              !mControlMapper.GetDigitalInput(CControlMapper::kC_OrbitClose, input)) {
            mOrbitType = kOT_Far;
            SetOrbitPosition(GetTweakPlayer()->GetOrbitNormalDistance(mOrbitType));
          }
          break;
        default:
          break;
        }
      }
      UpdateOrbitPosition(GetTweakPlayer()->GetOrbitNormalDistance(mOrbitType), mgr);
      break;
    case kOS_OrbitCarcass:
      if (mControlMapper.GetPressInput(CControlMapper::kC_OrbitObject, input)) {
        SetOrbitTargetId(GetOrbitNextTargetId(), mgr);
        if (mOrbitTargetId != kInvalidUniqueId) {
          if (ValidateAimTargetId(GetOrbitTargetId(), mgr)) {
            SetAimTarget(GetOrbitTargetId());
          }
          SetOrbitState(kOS_OrbitObject, mgr);
          UpdateOrbitPosition(GetTweakPlayer()->GetOrbitNormalDistance(mOrbitType), mgr);
        }
      }
      UpdateOrbitSelection(input, mgr);
      break;
    case kOS_ForcedOrbitObject:
      UpdateOrbitPosition(GetTweakPlayer()->GetOrbitNormalDistance(mOrbitType), mgr);
      UpdateOrbitSelection(input, mgr);
      break;
    }
    if (mOrbitState == kOS_Grapple) {
      if (mOrbitNextTargetId == mOrbitTargetId) {
        mOrbitNextTargetId = kInvalidUniqueId;
      }
    }
  } else {
    switch (mOrbitState) {
    case kOS_NoOrbit:
      break;
    case kOS_OrbitObject:
      if (TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(GetOrbitTargetId()))) {
        BreakGrapple(kOR_Default, mgr);
      } else {
        SetOrbitRequest(kOR_StopOrbit, mgr);
      }
      break;
    case kOS_Grapple:
      if (!GetTweakPlayer()->GetOrbitReleaseBreaksGrapple()) {
        if (mOrbitNextTargetId == mOrbitTargetId) {
          mOrbitNextTargetId = kInvalidUniqueId;
        }
      } else {
        BreakGrapple(kOR_StopOrbit, mgr);
      }
      break;
    case kOS_OrbitPoint:
    case kOS_OrbitCarcass:
      SetOrbitRequest(kOR_StopOrbit, mgr);
      break;
    case kOS_ForcedOrbitObject:
      UpdateOrbitPosition(GetTweakPlayer()->GetOrbitNormalDistance(mOrbitType), mgr);
      UpdateOrbitSelection(input, mgr);
      break;
    default:
      SetOrbitRequest(kOR_StopOrbit, mgr);
      break;
    }
  }
}

void CPlayer::UpdateOrbitZone() {
  if (mPlayerState->GetCurrentVisor() != CPlayerState::kPV_Scan) {
    mOrbitZoneType = kZT_Ellipse;
    mOrbitScreenBoxType = 1;
    mOrbitZoneMode = kZI_Targeting;
  } else {
    mOrbitZoneType = kZT_Box;
    mOrbitScreenBoxType = 2;
    mOrbitZoneMode = kZI_Scan;
  }
}

void CPlayer::UpdateOrbitModeTimer(float dt) {
  if (mOrbitState == kOS_NoOrbit && mOrbitModeTimer > 0.f) {
    mOrbitModeTimer -= dt;
  } else {
    mOrbitModeTimer = 0.f;
  }
}

void CPlayer::UpdateOrbitPreventionTimer(float dt) {
  if (mOrbitPreventionTimer > 0.f) {
    mOrbitPreventionTimer -= dt;
  }
}

void CPlayer::AddOrbitDisableSource(CStateManager& mgr, TUniqueId id) {
  if (mOrbitDisableSources.size() >= 5) {
    return;
  }
  for (rstl::reserved_vector< TUniqueId, 5 >::iterator it = mOrbitDisableSources.begin();
       it != mOrbitDisableSources.end(); ++it) {
    if (*it == id) {
      return;
    }
  }
  mOrbitDisableSources.push_back(id);
  SetAimTarget(kInvalidUniqueId);
  const TUniqueId orbitTarget = GetOrbitTargetId();
  if (!TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(orbitTarget))) {
    SetOrbitTargetId(kInvalidUniqueId, mgr);
  }
}

void CPlayer::RemoveOrbitDisableSource(TUniqueId id) {
  for (rstl::reserved_vector< TUniqueId, 5 >::iterator it = mOrbitDisableSources.begin();
       it != mOrbitDisableSources.end(); ++it) {
    if (*it == id) {
      mOrbitDisableSources.erase(it);
      break;
    }
  }
}

bool CPlayer::CheckOrbitDisableSourceList() const { return !mOrbitDisableSources.empty(); }

bool CPlayer::CheckOrbitDisableSourceList(const CStateManager& mgr) {
  for (rstl::reserved_vector< TUniqueId, 5 >::iterator it = mOrbitDisableSources.begin();
       it != mOrbitDisableSources.end();) {
    if (!mgr.GetObjectById(*it)) {
      mOrbitDisableSources.erase(it);
      it = mOrbitDisableSources.begin();
    } else {
      ++it;
    }
  }
  return !mOrbitDisableSources.empty();
}

bool CPlayer::WithinOrbitScreenEllipse(const CVector3f& screenPosition,
                                       EPlayerZoneInfo zone) const {
  if (screenPosition.GetZ() >= 1.f) {
    return false;
  }
  const float x =
      CMath::AbsF(screenPosition.GetX() - CCast::LtoF(GetTweakPlayer()->GetOrbitZoneCentreX(zone)));
  const float y =
      CMath::AbsF(screenPosition.GetY() - CCast::LtoF(GetTweakPlayer()->GetOrbitZoneCentreY(zone)));
  const float heXSq = CCast::LtoF(GetTweakPlayer()->GetOrbitZoneWidth(zone) *
                                  GetTweakPlayer()->GetOrbitZoneWidth(zone));
  const float heYSq = CCast::LtoF(GetTweakPlayer()->GetOrbitZoneHeight(zone) *
                                  GetTweakPlayer()->GetOrbitZoneHeight(zone));
  return x * x <= (1.f - y * y / heYSq) * heXSq;
}

bool CPlayer::WithinOrbitScreenBox(const CVector3f& screenPosition, EPlayerZoneInfo zone,
                                   EPlayerZoneType type) const {
  if (screenPosition.GetZ() >= 1.f) {
    return false;
  }
  switch (type) {
  case kZT_Box:
    if (CMath::AbsF(screenPosition.GetX() - float(GetTweakPlayer()->GetOrbitZoneCentreX(zone))) <=
            float(GetTweakPlayer()->GetOrbitZoneWidth(zone)) &&
        CMath::AbsF(screenPosition.GetY() - float(GetTweakPlayer()->GetOrbitZoneCentreY(zone))) <=
            float(GetTweakPlayer()->GetOrbitZoneHeight(zone)) &&
        screenPosition.GetZ() < 1.f) {
      return true;
    }
    break;
  case kZT_Ellipse:
    return WithinOrbitScreenEllipse(screenPosition, zone);
  default:
    return true;
  }
  return false;
}

void CPlayer::FindOrbitableObjects(const rstl::reserved_vector< TUniqueId, 1024 >& candidates,
                                   rstl::reserved_vector< TUniqueId, 64 >& objects,
                                   EPlayerZoneInfo zone, EPlayerZoneType type, CStateManager& mgr,
                                   bool onScreenTest) {
  const CVector3f eyePosition = GetEyePosition();
  CVector3f forward = GetTransform().GetForward();
  forward.Normalize();
  const CFirstPersonCamera* const fpCamera = mCameraManager->GetFirstPersonCamera();
  for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = candidates.begin();
       it != candidates.end(); ++it) {
    const CActor* const act = TCastToConstPtr< CActor >(mgr.GetObjectById(*it));
    if (act) {
      if (act->GetUniqueId() == GetUniqueId()) {
        continue;
      }
      if (ValidateOrbitTargetId(act->GetUniqueId(), mgr) != kOVR_OK) {
        continue;
      }
      const CVector3f orbitPosition = act->GetOrbitPosition(mgr);
      CVector3f screenPosition = fpCamera->ConvertToScreenSpace(orbitPosition);
      screenPosition.SetX(screenPosition.GetX() * CCast::LtoF(CGraphics::GetViewport().mWidth) /
                              2.f +
                          CCast::LtoF(CGraphics::GetViewport().mWidth) / 2.f);
      screenPosition.SetY(screenPosition.GetY() * CCast::LtoF(CGraphics::GetViewport().mHeight) /
                              2.f +
                          CCast::LtoF(CGraphics::GetViewport().mHeight) / 2.f);
      bool pass = false;
      if (onScreenTest && WithinOrbitScreenBox(screenPosition, zone, type)) {
        pass = true;
      } else if (!onScreenTest && !WithinOrbitScreenBox(screenPosition, zone, type)) {
        pass = true;
      }
      if (pass) {
        const CVector3f eyeToOrbit = orbitPosition - eyePosition;
        const float distance = eyeToOrbit.Magnitude();
        if (!act->GetDoTargetDistanceTest() || distance <= GetOrbitMaxTargetDistance()) {
          if (objects.size() != objects.capacity()) {
            objects.push_back(act->GetUniqueId());
          }
        }
      }
    }
  }
}

TUniqueId CPlayer::FindBestOrbitableObject(const rstl::reserved_vector< TUniqueId, 64 >& objects,
                                           EPlayerZoneInfo zone, CStateManager& mgr) {
  const CVector3f eyePosition = GetEyePosition();
  CVector3f forward = GetTransform().GetForward();
  forward.Normalize();
  const float halfWidth = CCast::LtoF(CGraphics::GetViewport().mWidth / 2);
  const float halfHeight = CCast::LtoF(CGraphics::GetViewport().mHeight / 2);
  const float idealX =
      (CCast::LtoF(GetTweakPlayer()->GetOrbitZoneIdealX(zone)) - halfWidth) / halfWidth;
  const float idealY =
      (CCast::LtoF(GetTweakPlayer()->GetOrbitZoneIdealY(zone)) - halfHeight) / halfHeight;
  TUniqueId bestId = mOrbitNextTargetId;
  const CFirstPersonCamera* fpCamera = mCameraManager->GetFirstPersonCamera();
  const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(objects[mOrbitCandidateIndex]));
  bool pass = false;
  if (act) {
    const CVector3f orbitPosition = act->GetOrbitPosition(mgr);
    const CVector3f eyeToOrbit = orbitPosition - eyePosition;
    const CVector3f playerToOrbit = orbitPosition - GetTranslation();
    const float distance = playerToOrbit.Magnitude();
    const float flatDistance = playerToOrbit.ToVec2f().Magnitude();
    const CVector3f screenPosition = fpCamera->ConvertToScreenSpace(orbitPosition);
    if (screenPosition.GetZ() < 1.f && objects[mOrbitCandidateIndex] != mOrbitTargetId &&
        flatDistance > 2.f) {
      const CScriptGrapplePoint* point = TCastToConstPtr< CScriptGrapplePoint >(act);
      if (point) {
        if (act->GetUniqueId() == mOrbitNextTargetId) {
          if (mgr.RayCollideWorld(eyePosition, eyePosition + eyeToOrbit, kPlayerLineOfSightFilter,
                                  act)) {
            pass = true;
          }
        } else if (point->GetUniqueId() != mOrbitTargetId &&
                   point->GetCurrentAreaId() == GetCurrentAreaId() &&
                   mPlayerState->HasPowerUp(CPlayerState::kIT_GrappleBeam) &&
                   distance < mOrbitTargetDistance &&
                   distance < GetTweakPlayer()->GetGrappleDistance()) {
          if (mgr.RayCollideWorld(eyePosition, eyePosition + eyeToOrbit, kPlayerLineOfSightFilter,
                                  act)) {
            if (point->GetGrappleParameters().GetConstrainToAxis()) {
              CVector3f pointToPlayer = GetTranslation() - point->GetTranslation();
              if (pointToPlayer.CanBeNormalized()) {
                const CVector3f pointForward = point->GetTransform().GetForward().AsNormalized();
                pointToPlayer.SetZ(0.f);
                if (CMath::AbsF(CVector3f::Dot(pointForward, pointToPlayer.AsNormalized())) <=
                    0.70710677f) {
                  return bestId;
                }
              }
            }
            bestId = act->GetUniqueId();
            pass = true;
            const float screenX = screenPosition.GetX() - idealX;
            const float screenY = screenPosition.GetY() - idealY;
            mOrbitTargetScreenDistance = screenX * screenX + screenY * screenY;
            mOrbitTargetDistance = distance;
          }
        }
      } else if (act->GetUniqueId() == mOrbitNextTargetId) {
        if (mgr.RayCollideWorld(eyePosition, eyePosition + eyeToOrbit, kPlayerLineOfSightFilter,
                                act)) {
          pass = true;
        }
      } else if (mOrbitTargetDistance - distance > GetTweakPlayer()->GetOrbitDistanceThreshold() &&
                 mPlayerState->GetCurrentVisor() != CPlayerState::kPV_Scan) {
        if (mgr.RayCollideWorld(eyePosition, eyePosition + eyeToOrbit, kPlayerLineOfSightFilter,
                                act)) {
          bestId = act->GetUniqueId();
          pass = true;
          const float screenX = screenPosition.GetX() - idealX;
          const float screenY = screenPosition.GetY() - idealY;
          mOrbitTargetScreenDistance = screenX * screenX + screenY * screenY;
          mOrbitTargetDistance = distance;
        }
      } else if (CMath::AbsF(distance - mOrbitTargetDistance) <
                     GetTweakPlayer()->GetOrbitDistanceThreshold() ||
                 mPlayerState->GetCurrentVisor() == CPlayerState::kPV_Scan) {
        const float screenX = screenPosition.GetX() - idealX;
        const float screenY = screenPosition.GetY() - idealY;
        const float screenDistance = screenX * screenX + screenY * screenY;
        if (screenDistance < mOrbitTargetScreenDistance &&
            mgr.RayCollideWorld(eyePosition, eyePosition + eyeToOrbit, kPlayerLineOfSightFilter,
                                act)) {
          bestId = act->GetUniqueId();
          pass = true;
          mOrbitTargetScreenDistance = screenDistance;
          mOrbitTargetDistance = distance;
        }
      }
    }
    if (!pass && act->GetUniqueId() == mOrbitNextTargetId) {
      return kInvalidUniqueId;
    }
  }
  return bestId;
}

void CPlayer::UpdateOrbitableObjects(CStateManager& mgr) {
  if (mPlayerState->GetCurrentVisor() == CPlayerState::kPV_Scan && !mgr.IsMultiplayer()) {
    if (mPlayerState->GetVisorTransitionFactor() > 0.5f) {
      mOrbitNextTargetId = FindScanTargetId(mgr);
    }
    return;
  }
  ++mOrbitCandidateIndex;
  if (mOrbitCandidateIndex >= mOnScreenOrbitObjects.size()) {
    mOrbitCandidateIndex = 0;
  }
  --mOrbitCandidateRefreshFrames;
  if (mOrbitCandidateRefreshFrames <= 0) {
    mOnScreenOrbitObjects.clear();
    mNearbyOrbitObjects.clear();
    mOffScreenOrbitObjects.clear();
    if (!CheckOrbitDisableSourceList(mgr)) {
      const CTransform4f& cameraXf = GetFirstPersonCameraTransform();
      float distance = GetOrbitMaxTargetDistance();
      if (mExtendTargetDistance) {
        distance *= 5.f;
      }
      const CAABox bounds = BuildNearListBox(true, cameraXf, GetTweakPlayer()->GetOrbitBoxWidth(),
                                             GetTweakPlayer()->GetOrbitBoxHeight(), distance);
      staticBox = bounds;
      CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Orbit));
      if (mPlayerState->GetCurrentVisor() == CPlayerState::kPV_Scan) {
        filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Scannable));
      }
      rstl::reserved_vector< TUniqueId, 1024 > nearList;
      mgr.BuildNearList(nearList, bounds, filter, nullptr);
      FindOrbitableObjects(nearList, mNearbyOrbitObjects, mOrbitZoneMode, kZT_Always, mgr, true);
      FindOrbitableObjects(nearList, mOnScreenOrbitObjects, mOrbitZoneMode, mOrbitZoneType, mgr,
                           true);
    }
    mOrbitCandidateIndex = 0;
    mOrbitCandidateRefreshFrames = 20;
  }
  mOrbitNextTargetId = FindOrbitTargetId(mgr);
}

TUniqueId CPlayer::FindOrbitTargetId(CStateManager& mgr) {
  const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(GetOrbitNextTargetId()));
  if (act) {
    const CVector3f eyePosition = GetEyePosition();
    const int width = CGraphics::GetViewport().mWidth;
    const float idealX =
        (float(GetTweakPlayer()->GetOrbitZoneIdealX(mOrbitZoneMode)) - float(width / 2)) /
        float(CGraphics::GetViewport().mWidth / 2);
    const int height = CGraphics::GetViewport().mHeight;
    const float idealY =
        (float(GetTweakPlayer()->GetOrbitZoneIdealY(mOrbitZoneMode)) - float(height / 2)) /
        float(CGraphics::GetViewport().mHeight / 2);
    const CVector3f orbitPosition = act->GetOrbitPosition(mgr);
    const CVector3f screenPosition =
        mCameraManager->GetFirstPersonCamera()->ConvertToScreenSpace(orbitPosition);
    CVector3f positionInBox = screenPosition;
    positionInBox.SetX(screenPosition.GetX() * CCast::LtoF(CGraphics::GetViewport().mWidth) / 2.f +
                       CCast::LtoF(CGraphics::GetViewport().mWidth) / 2.f);
    positionInBox.SetY(screenPosition.GetY() * CCast::LtoF(CGraphics::GetViewport().mHeight) / 2.f +
                       CCast::LtoF(CGraphics::GetViewport().mHeight) / 2.f);
    if (ValidateOrbitTargetId(GetOrbitNextTargetId(), mgr) != kOVR_OK ||
        !WithinOrbitScreenBox(positionInBox, mOrbitZoneMode, mOrbitZoneType)) {
      mOrbitTargetDistance = 10000.f;
      mOrbitTargetScreenDistance = 10000.f;
      SetOrbitNextTargetId(kInvalidUniqueId);
      return kInvalidUniqueId;
    }
    const float screenX = screenPosition.GetX() - idealX;
    const float screenY = screenPosition.GetY() - idealY;
    mOrbitTargetScreenDistance = screenX * screenX + screenY * screenY;
    mOrbitTargetDistance = (orbitPosition - GetTranslation()).Magnitude();
  } else {
    mOrbitTargetDistance = 10000.f;
    mOrbitTargetScreenDistance = 10000.f;
  }
  if (mOnScreenOrbitObjects.empty()) {
    return kInvalidUniqueId;
  }
  return FindBestOrbitableObject(mOnScreenOrbitObjects, mOrbitZoneMode, mgr);
}

// Guessed name
TUniqueId CPlayer::CheckEnemyAgainstOrbitZone(TUniqueId target, EPlayerZoneInfo zone,
                                              EPlayerZoneType type, CStateManager& mgr) {
  const CVector3f eyePosition = GetEyePosition();
  CVector3f forward = GetTransform().GetForward();
  forward.Normalize();
  const float halfWidth = CCast::LtoF(CGraphics::GetViewport().mWidth / 2);
  const float halfHeight = CCast::LtoF(CGraphics::GetViewport().mHeight / 2);
  const float idealX =
      (CCast::LtoF(GetTweakPlayer()->GetOrbitZoneIdealX(zone)) - halfWidth) / halfWidth;
  const float idealY =
      (CCast::LtoF(GetTweakPlayer()->GetOrbitZoneIdealY(zone)) - halfHeight) / halfHeight;
  const CFirstPersonCamera* fpCamera = mCameraManager->GetFirstPersonCamera();
  const CActor* act = static_cast< const CActor* >(mgr.GetObjectById(target));
  if (act && act->GetUniqueId() != GetUniqueId() &&
      ValidateObjectForMode(act->GetUniqueId(), mgr)) {
    const CVector3f aimPosition = act->GetAimPosition(mgr, 0.f);
    const CVector3f screenPosition = fpCamera->ConvertToScreenSpace(aimPosition);
    CVector3f positionInBox = screenPosition;
    positionInBox.SetX(screenPosition.GetX() * CCast::LtoF(CGraphics::GetViewport().mWidth) / 2.f +
                       CCast::LtoF(CGraphics::GetViewport().mWidth) / 2.f);
    positionInBox.SetY(screenPosition.GetY() * CCast::LtoF(CGraphics::GetViewport().mHeight) / 2.f +
                       CCast::LtoF(CGraphics::GetViewport().mHeight) / 2.f);
    if (WithinOrbitScreenBox(positionInBox, zone, type)) {
      const CVector3f eyeToAim = aimPosition - eyePosition;
      const float distance = eyeToAim.Magnitude();
      if (distance <= GetTweakPlayer()->GetAimMaxDistance()) {
        if (mAimTargetDistance - distance > GetTweakPlayer()->GetAimThresholdDistance()) {
          if (mgr.RayCollideWorld(eyePosition, eyePosition + eyeToAim, kPlayerLineOfSightFilter,
                                  act)) {
            const float screenX = positionInBox.GetX() - idealX;
            const float screenY = positionInBox.GetY() - idealY;
            mAimTargetScreenDistance = screenX * screenX + screenY * screenY;
            mAimTargetDistance = distance;
            return target;
          }
        } else if (CMath::AbsF(distance - mAimTargetDistance) <
                   GetTweakPlayer()->GetAimThresholdDistance()) {
          const float screenX = positionInBox.GetX() - idealX;
          const float screenY = positionInBox.GetY() - idealY;
          const float screenDistance = screenX * screenX + screenY * screenY;
          if (screenDistance < mAimTargetScreenDistance &&
              mgr.RayCollideWorld(eyePosition, eyeToAim, kPlayerLineOfSightFilter, act)) {
            mAimTargetScreenDistance = screenDistance;
            mAimTargetDistance = distance;
            return target;
          }
        }
      }
    }
  }
  return kInvalidUniqueId;
}

TUniqueId CPlayer::FindAimTargetId(CStateManager& mgr) {
  const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(GetAimTarget()));
  if (act) {
    const CVector3f eyePosition = GetEyePosition();
    CVector3f forward = GetTransform().GetForward();
    forward.Normalize();
    const int width = CGraphics::GetViewport().mWidth;
    const float idealX =
        (float(GetTweakPlayer()->GetOrbitZoneIdealX(kZI_Targeting)) - float(width / 2)) /
        float(CGraphics::GetViewport().mWidth / 2);
    const int height = CGraphics::GetViewport().mHeight;
    const float idealY =
        (float(GetTweakPlayer()->GetOrbitZoneIdealY(kZI_Targeting)) - float(height / 2)) /
        float(CGraphics::GetViewport().mHeight / 2);
    if (ValidateObjectForMode(act->GetUniqueId(), mgr)) {
      const CVector3f aimPosition = act->GetAimPosition(mgr, 0.f);
      const CVector3f screenPosition =
          mCameraManager->GetFirstPersonCamera()->ConvertToScreenSpace(aimPosition);
      CVector3f positionInBox = screenPosition;
      positionInBox.SetX(screenPosition.GetX() * CCast::LtoF(CGraphics::GetViewport().mWidth) /
                             2.f +
                         CCast::LtoF(CGraphics::GetViewport().mWidth) / 2.f);
      positionInBox.SetY(screenPosition.GetY() * CCast::LtoF(CGraphics::GetViewport().mHeight) /
                             2.f +
                         CCast::LtoF(CGraphics::GetViewport().mHeight) / 2.f);
      if (WithinOrbitScreenBox(positionInBox, kZI_Targeting, kZT_Ellipse)) {
        const float distance = (aimPosition - eyePosition).Magnitude();
        const float screenX = positionInBox.GetX() - idealX;
        const float screenY = positionInBox.GetY() - idealY;
        mAimTargetScreenDistance = screenX * screenX + screenY * screenY;
        mAimTargetDistance = distance;
      } else {
        SetAimTarget(kInvalidUniqueId);
        mAimTargetDistance = 10000.f;
        mAimTargetScreenDistance = 10000.f;
      }
    } else {
      SetAimTarget(kInvalidUniqueId);
      mAimTargetDistance = 10000.f;
      mAimTargetScreenDistance = 10000.f;
    }
  } else {
    mAimTargetDistance = 10000.f;
    mAimTargetScreenDistance = 10000.f;
  }
  if (mAimCandidates.empty()) {
    return kInvalidUniqueId;
  }
  return CheckEnemyAgainstOrbitZone(mAimCandidates[mAimCandidateIndex], kZI_Targeting, kZT_Ellipse,
                                    mgr);
}

// Guessed name
void CPlayer::UpdateAimCandidates(CStateManager& mgr) {
  ++mAimCandidateIndex;
  if (mAimCandidateIndex == mAimCandidates.size()) {
    mAimCandidateIndex = 0;
  }
  --mAimCandidateRefreshFrames;
  if (mAimCandidateRefreshFrames == 0) {
    const CTransform4f& cameraXf = GetFirstPersonCameraTransform();
    float distance = GetTweakPlayer()->GetAimMaxDistance();
    if (mExtendTargetDistance) {
      distance *= 5.f;
    }
    const CAABox bounds = BuildNearListBox(true, cameraXf, GetTweakPlayer()->GetAimBoxWidth(),
                                           GetTweakPlayer()->GetAimBoxHeight(), distance);
    const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Target));
    mAimCandidates.clear();
    mAimCandidateIndex = 0;
    mAimCandidateRefreshFrames = 20;
    mgr.BuildNearList(mAimCandidates, bounds, filter, this);
  }
  const TUniqueId target = FindAimTargetId(mgr);
  if (target != kInvalidUniqueId) {
    SetAimTarget(target);
  }
}

bool CPlayer::ValidateObjectForMode(TUniqueId target, CStateManager& mgr) const {
  const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(target));
  if (!act || target.value == kInvalidUniqueId.value) {
    return false;
  }
  if (TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(target))) {
    return true;
  }
  if (GetCombatMode()) {
    if (act->GetHealthInfo()) {
      if (act->GetHealthInfo()->GetHP() > 0.f) {
        return true;
      }
    } else {
      if (act->GetMaterialList().HasMaterial(kMT_Projectile) ||
          act->GetMaterialList().HasMaterial(kMT_Scannable)) {
        return true;
      }
      if (const CScriptGrapplePoint* point =
              TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(target))) {
        const CVector3f playerToPoint = point->GetTranslation() - GetTranslation();
        if (playerToPoint.CanBeNormalized() &&
            playerToPoint.Magnitude() < GetTweakPlayer()->GetGrappleDistance()) {
          return true;
        }
      }
    }
  }
  if (GetExplorationMode()) {
    if (!act->GetHealthInfo()) {
      if (const CScriptGrapplePoint* point =
              TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(target))) {
        const CVector3f playerToPoint = point->GetTranslation() - GetTranslation();
        if (playerToPoint.CanBeNormalized() &&
            playerToPoint.Magnitude() < GetTweakPlayer()->GetGrappleDistance()) {
          return true;
        }
      } else {
        return true;
      }
    } else {
      return true;
    }
  }
  return false;
}

bool CPlayer::ValidateAimTargetId(TUniqueId target, CStateManager& mgr) {
  if (target.value == kInvalidUniqueId.value) {
    mAimTargetAverage.clear();
    mAimTargetTimer = 0.f;
    return false;
  }
  const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(target));
  if (!act || !act->GetMaterialList().HasMaterial(kMT_Target) ||
      !(act->GetValidTargetPlayers() & (1 << mgr.MaskUIdNumPlayers(GetUniqueId())))) {
    return false;
  }
  if (mOrbitState == kOS_OrbitObject || mOrbitState == kOS_ForcedOrbitObject) {
    if (ValidateOrbitTargetId(GetOrbitTargetId(), mgr) != kOVR_OK) {
      SetAimTarget(kInvalidUniqueId);
      mAimTargetTimer = 0.f;
      return false;
    }
    return true;
  }
  if (act->GetMaterialList().HasMaterial(kMT_Target) && target != kInvalidUniqueId &&
      ValidateObjectForMode(target, mgr)) {
    const CVector3f aimPosition = act->GetAimPosition(mgr, 0.f);
    const CVector3f eyePosition = GetEyePosition();
    CVector3f eyeToAim = aimPosition - eyePosition;
    const CVector3f screenPosition =
        mCameraManager->GetFirstPersonCamera()->ConvertToScreenSpace(aimPosition);
    CVector3f positionInBox = screenPosition;
    positionInBox.SetX(positionInBox.GetX() * CCast::LtoF(CGraphics::GetViewport().mWidth) / 2.f +
                       CCast::LtoF(CGraphics::GetViewport().mWidth) / 2.f);
    positionInBox.SetY(positionInBox.GetY() * CCast::LtoF(CGraphics::GetViewport().mHeight) / 2.f +
                       CCast::LtoF(CGraphics::GetViewport().mHeight) / 2.f);
    if (WithinOrbitScreenBox(positionInBox, mOrbitZoneMode, mOrbitZoneType) ||
        (mOrbitZoneMode != kZI_Targeting &&
         WithinOrbitScreenBox(positionInBox, kZI_Targeting, mOrbitZoneType))) {
      const float distance = eyeToAim.Magnitude();
      if (distance <= GetTweakPlayer()->GetAimMaxDistance()) {
        mAimTargetTimer = GetTweakPlayer()->GetAimTargetTimer();
        return true;
      }
    }
    if (mAimTargetTimer > 0.f) {
      return true;
    }
  }
  SetAimTarget(kInvalidUniqueId);
  mAimTargetTimer = 0.f;
  return false;
}

void CPlayer::UpdateAimTargetTimer(float dt) {
  if (mAimTarget != kInvalidUniqueId && mAimTargetTimer > 0.f) {
    mAimTargetTimer -= dt;
  }
}

void CPlayer::UpdateAimTarget(CStateManager& mgr) {
  UpdateAimCandidates(mgr);
  if (!GetCombatMode()) {
    SetAimTarget(kInvalidUniqueId);
    mAimTargetTimer = 0.f;
  } else if (!gkAutoAim && gkAutoAimAtOrbitedObject) {
    if (mOrbitState == kOS_OrbitObject || mOrbitState == kOS_ForcedOrbitObject) {
      if (ValidateOrbitTargetId(GetOrbitTargetId(), mgr) == kOVR_OK) {
        SetAimTarget(GetOrbitTargetId());
      }
    }
  } else {
    const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(GetAimTarget()));
    if (GetTweakPlayerControls()->GetAimWhenOrbitingPoint()) {
      switch (mOrbitState) {
      case kOS_OrbitObject:
      case kOS_ForcedOrbitObject:
        if (ValidateOrbitTargetId(GetOrbitTargetId(), mgr) == kOVR_OK) {
          SetAimTarget(GetOrbitTargetId());
        }
        break;
      default:
        break;
      }
    }
  }
}

void CPlayer::SetOrbitPosition(float distance) {
  CTransform4f cameraXf = GetFirstPersonCameraTransform();
  if (mOrbitState == kOS_OrbitPoint && mOrbitRequest == kOR_BadVerticalAngle) {
    cameraXf = GetTransform();
    cameraXf.SetTranslation(GetEyePosition());
  }
  CVector3f flatForward = cameraXf.GetForward();
  flatForward.SetZ(0.f);
  float dot = CVector3f::Dot(flatForward.AsNormalized(), cameraXf.GetForward());
  dot = CMath::Limit(dot, 1.f);
  const CVector3f orbitVector(0.f, distance / dot, 0.f);
  mOrbitPoint = cameraXf.GetTranslation() + cameraXf.Rotate(orbitVector);
  mOrbitVector = CVector3f(0.f, distance, mOrbitPoint.GetZ() - cameraXf.GetTranslation().GetZ());
}

void CPlayer::UpdateOrbitFixedPosition() {
  const CVector3f eyePosition = GetEyePosition();
  mOrbitPoint = eyePosition + GetTransform().Rotate(mOrbitVector);
}

void CPlayer::UpdateOrbitZPosition() {
  switch (mOrbitState) {
  case kOS_OrbitPoint:
    if (CMath::AbsF(mOrbitVector.GetZ()) < GetTweakPlayer()->GetOrbitZRange()) {
      mOrbitPoint.SetZ(mOrbitVector[kDZ] + (GetTranslation().GetZ() + GetEyeHeight()));
    }
    break;
  default:
    break;
  }
}

void CPlayer::UpdateOrbitPosition(float distance, const CStateManager& mgr) {
  switch (mOrbitState) {
  case kOS_NoOrbit:
    break;
  case kOS_OrbitPoint:
  case kOS_OrbitCarcass:
    SetOrbitPosition(distance);
    break;
  case kOS_ForcedOrbitObject:
  case kOS_Grapple:
  case kOS_OrbitObject:
    if (const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(GetOrbitTargetId()))) {
      if (mOrbitTargetId != kInvalidUniqueId) {
        mOrbitPoint = act->GetOrbitPosition(mgr);
      }
    }
    break;
  default:
    break;
  }
}

void CPlayer::SetOrbitTargetId(TUniqueId target, const CStateManager& mgr) {
  if (target != kInvalidUniqueId) {
    const CPatterned* patterned = TCastToConstPtr< CPatterned >(mgr.GetObjectById(target));
    const CSwarmBasics* swarm = TCastToConstPtr< CSwarmBasics >(mgr.GetObjectById(target));
    if (patterned || swarm) {
      mOrbitingEnemy = true;
    } else {
      mOrbitingEnemy = false;
    }
    mOrbitTargetLineOfSightClear = true;
  }
  mOrbitTargetId = target;
  if (mOrbitTargetId == kInvalidUniqueId) {
    mOrbitLockEstablished = false;
    mOrbitTargetLineOfSightClear = false;
  }
}

void CPlayer::SetOrbitState(EPlayerOrbitState state, const CStateManager& mgr) {
  mOrbitState = state;
  mOrbitOcclusionTimer = 0.f;
  CFirstPersonCamera* camera = mCameraManager->FirstPersonCamera();
  switch (mOrbitState) {
  case kOS_OrbitObject:
    camera->SetLockCamera(false);
    break;
  case kOS_OrbitCarcass: {
    camera->SetLockCamera(true);
    CVector3f playerToPoint = mOrbitPoint - GetTranslation();
    playerToPoint.SetZ(0.f);
    if (playerToPoint.CanBeNormalized()) {
      mOrbitPointDistance = playerToPoint.Magnitude();
    } else {
      mOrbitPointDistance = 0.f;
    }
    SetOrbitTargetId(kInvalidUniqueId, mgr);
    SetOrbitNextTargetId(kInvalidUniqueId);
    break;
  }
  case kOS_NoOrbit:
    mOrbitModeTimer = GetTweakPlayer()->GetOrbitModeTimer();
    mOrbitModeTimer = 0.28f;
    SetOrbitTargetId(kInvalidUniqueId, mgr);
    break;
  case kOS_OrbitPoint:
    SetOrbitTargetId(kInvalidUniqueId, mgr);
    break;
  default:
    break;
  }
}

CVector3f CPlayer::GetHUDOrbitTargetPosition() const {
  return mOrbitPoint + mCameraBob->GetCameraBobTransformation().GetTranslation();
}

float CPlayer::CalculateOrbitMinDistance(EPlayerOrbitType type) const {
  float distance = GetTweakPlayer()->GetOrbitMinDistance(type);
  distance *=
      CMath::Clamp(1.f, CMath::AbsF(mOrbitPoint.GetZ() - GetTranslation().GetZ()) / 20.f, 4.f);
  return distance;
}

void CPlayer::OrbitPoint(EPlayerOrbitType type, CStateManager& mgr) {
  mOrbitType = type;
  SetOrbitState(kOS_OrbitPoint, mgr);
  SetOrbitPosition(GetTweakPlayer()->GetOrbitNormalDistance(mOrbitType));
}

void CPlayer::OrbitCarcass(CStateManager& mgr) {
  if (mOrbitState == kOS_OrbitObject) {
    mOrbitType = kOT_Default;
    SetOrbitState(kOS_OrbitCarcass, mgr);
  }
}

void CPlayer::PreventFallingCameraPitch() {
  mJumpCameraTimer = 0.f;
  mFallCameraTimer = 0.01f;
  mCancelCameraPitch = true;
}

bool CPlayer::InGrappleJumpCooldown() const {
  if (mMovementState != NPlayer::kMS_OnGround &&
      (mGrappleJumpTimeout > 0.f || (mJumpCameraTimer == 0.f && mOrbitState == kOS_NoOrbit))) {
    return true;
  }
  return false;
}

void CPlayer::SetOrbitRequestForOtherPlayers(EPlayerOrbitRequest request, CStateManager& mgr) {
  for (int i = 0; i < static_cast< uint >(mgr.GetNumPlayers()); ++i) {
    CPlayer& player = *mgr.Player(i);
    if (player.GetUniqueId() != GetUniqueId() && player.GetOrbitTargetId() == GetUniqueId()) {
      player.SetOrbitRequestForTarget(GetUniqueId(), request, mgr);
    }
  }
}

void CPlayer::SetOrbitRequestForTarget(TUniqueId target, EPlayerOrbitRequest request,
                                       CStateManager& mgr) {
  if ((mOrbitState == kOS_OrbitObject || mOrbitState == kOS_Grapple ||
       mOrbitState == kOS_ForcedOrbitObject) &&
      target == mOrbitTargetId) {
    SetOrbitRequest(request, mgr);
  }
}

void CPlayer::SetOrbitRequest(EPlayerOrbitRequest request, CStateManager& mgr) {
  mOrbitRequest = request;
  const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mOrbitTargetId));
  if (player && mTurretState == kTS_None) {
    ActivateOrbitSource(mgr);
    return;
  }
  switch (request) {
  case kOR_ActivateOrbitSource:
    ActivateOrbitSource(mgr);
    break;
  case kOR_BadVerticalAngle:
    SetOrbitState(kOS_OrbitPoint, mgr);
    mOrbitPoint = GetEyePosition() + GetTweakPlayer()->GetOrbitNormalDistance(mOrbitType) *
                                         GetTransform().GetForward();
    break;
  default:
    SetOrbitState(kOS_NoOrbit, mgr);
    break;
  }
}

void CPlayer::BreakGrapple(EPlayerOrbitRequest request, CStateManager& mgr) {
  mJumpCameraTimer = 0.f;
  mFallCameraTimer = 0.f;
  if (GetTweakPlayer()->GetGrappleJumpMode() == 2 && mGrappleState == kGS_Swinging) {
    ApplyGrappleJump(mgr);
    PreventFallingCameraPitch();
  }
  SetOrbitRequest(request, mgr);
  mGrappleState = kGS_None;
  AddMaterial(kMT_GroundCollider, mgr);
  if (CGrappleArm* arm = mGun->GrappleArm()) {
    if (arm->GetAnimState() != CGrappleArm::kAS_Done) {
      arm->SetAnimState(CGrappleArm::kAS_OutOfGrapple);
    }
  }
  if (!InGrappleJumpCooldown() && mGrappleState != kGS_JumpOff && mTurretState == kTS_None) {
    mGun->DrawGun(mgr);
  }
}

void CPlayer::BeginGrapple(CVector3f& direction, CStateManager& mgr) {
  direction.SetZ(0.f);
  if (direction.CanBeNormalized()) {
    mGrappleSwingAxis.SetX(direction.GetY());
    mGrappleSwingAxis.SetY(-direction.GetX());
    mGrappleSwingAxis.Normalize();
    mGrappleSwingTimer = 0.f;
    SetOrbitState(kOS_Grapple, mgr);
    mGrappleState = kGS_Pull;
    RemoveMaterial(kMT_GroundCollider, mgr);
  }
}

void CPlayer::ApplyGrappleJump(CStateManager& mgr) {
  const CScriptGrapplePoint* point =
      TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(GetOrbitTargetId()));
  if (!point) {
    return;
  }
  CVector3f swingAxis = mGrappleSwingAxis;
  if (mGrappleSwingTimer < 0.5f * GetTweakPlayer()->GetGrappleSwingPeriod()) {
    swingAxis *= -1.f;
  }
  const CVector3f pointToPlayer = GetTranslation() - point->GetTranslation();
  const CVector3f cross = CVector3f::Cross(pointToPlayer.AsNormalized(), swingAxis);
  CVector3f pointToPlayerFlat = pointToPlayer;
  pointToPlayerFlat.SetZ(0.f);
  float dot = 1.f;
  if (pointToPlayerFlat.CanBeNormalized() && cross.CanBeNormalized()) {
    float cosAngle =
        CMath::AbsF(CVector3f::Dot(cross.AsNormalized(), pointToPlayerFlat.AsNormalized()));
    cosAngle = CMath::Limit(cosAngle, 1.f);
    dot = cosAngle;
  }
  const CVector3f force = dot * (10000.f * (GetTweakPlayer()->GetGrappleJumpForce() * cross));
  ApplyForceWR(force, CAxisAngle::Identity());
}

void CPlayer::UpdateGrappleState(const CFinalInput& input, CStateManager& mgr) {
  if (!mPlayerState->HasPowerUp(CPlayerState::kIT_GrappleBeam) || mMorphBallState == kMS_Morphed ||
      mPlayerState->GetCurrentVisor() == CPlayerState::kPV_Scan ||
      mPlayerState->GetTransitioningVisor() == CPlayerState::kPV_Scan) {
    return;
  }
  if (GetOrbitTargetId() == kInvalidUniqueId) {
    mGrappleState = kGS_None;
    AddMaterial(kMT_GroundCollider, mgr);
    return;
  }
  const CScriptGrapplePoint* point =
      TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(GetOrbitTargetId()));
  if (point) {
    const CVector3f eyePosition = GetEyePosition();
    CVector3f playerToPoint = point->GetTranslation() - eyePosition;
    CVector3f playerToPointFlat = playerToPoint;
    playerToPointFlat.SetZ(0.f);
    if (playerToPoint.CanBeNormalized() && playerToPointFlat.CanBeNormalized() &&
        playerToPointFlat.Magnitude() > 2.f) {
      switch (mOrbitState) {
      case kOS_Grapple:
        switch (static_cast< int >(GetTweakPlayer()->GetGrappleJumpMode())) {
        case 0:
        case 1:
          if (FireBeamPressed(input)) {
            if (const CScriptGrapplePoint* nextPoint =
                    TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(mOrbitNextTargetId))) {
              playerToPoint = nextPoint->GetTranslation() - eyePosition;
              playerToPoint.SetZ(0.f);
              if (playerToPoint.CanBeNormalized()) {
                if (mGun->GrappleArm()) {
                  mGun->GrappleArm()->GrappleBeamDisconnected();
                }
                mGrappleSwingAxis.SetX(playerToPoint.GetY());
                mGrappleSwingAxis.SetY(-playerToPoint.GetX());
                mGrappleSwingAxis.Normalize();
                mGrappleSwingTimer = 0.f;
                SetOrbitTargetId(mOrbitNextTargetId, mgr);
                SetOrbitNextTargetId(kInvalidUniqueId);
                mGrappleState = kGS_Pull;
                if (mGun->GrappleArm()) {
                  mGun->GrappleArm()->GrappleBeamConnected(mgr);
                }
              }
            } else {
              if (static_cast< int >(GetTweakPlayer()->GetGrappleJumpMode()) == 0 &&
                  mGrappleJumpTimeout <= 0.f) {
                ApplyGrappleJump(mgr);
              }
              BreakGrapple(kOR_StopOrbit, mgr);
            }
          }
          break;
        case 2:
          break;
        default:
          break;
        }
        break;
      case kOS_OrbitObject:
        if (playerToPoint.CanBeNormalized()) {
          const CRayCastResult result =
              mgr.RayStaticIntersection(eyePosition, playerToPoint.AsNormalized(),
                                        playerToPoint.Magnitude(), kLineOfSightFilter);
          if (!result.IsValid()) {
            switch (mGrappleState) {
            case kGS_Firing:
            case kGS_Swinging:
              switch (static_cast< int >(GetTweakPlayer()->GetGrappleJumpMode())) {
              case 0:
                if (mGun->GrappleArm()) {
                  switch (mGun->GrappleArm()->GetAnimState()) {
                  case CGrappleArm::kAS_IntoGrappleIdle:
                    if (FireBeamHeld(input)) {
                      mGun->GrappleArm()->SetAnimState(CGrappleArm::kAS_FireGrapple);
                    }
                    break;
                  case CGrappleArm::kAS_Connected:
                    BeginGrapple(playerToPoint, mgr);
                    break;
                  default:
                    break;
                  }
                }
                break;
              case 1:
                if (FireBeamHeld(input) && mGun->GrappleArm()) {
                  switch (mGun->GrappleArm()->GetAnimState()) {
                  case CGrappleArm::kAS_IntoGrappleIdle:
                    mGun->GrappleArm()->SetAnimState(CGrappleArm::kAS_FireGrapple);
                    break;
                  case CGrappleArm::kAS_Connected:
                    BeginGrapple(playerToPoint, mgr);
                    break;
                  default:
                    break;
                  }
                }
                break;
              case 2:
                if (mGun->GrappleArm()) {
                  switch (mGun->GrappleArm()->GetAnimState()) {
                  case CGrappleArm::kAS_IntoGrappleIdle:
                    mGun->GrappleArm()->SetAnimState(CGrappleArm::kAS_FireGrapple);
                    break;
                  case CGrappleArm::kAS_Connected:
                    BeginGrapple(playerToPoint, mgr);
                    break;
                  default:
                    break;
                  }
                }
                break;
              default:
                break;
              }
              break;
            case kGS_None:
              mGrappleState = kGS_Firing;
              if (mGun->GrappleArm()) {
                mGun->GrappleArm()->Activate(true);
              }
              break;
            default:
              break;
            }
          }
        }
        break;
      default:
        break;
      }
    }
  }
  const int jumpMode = static_cast< int >(GetTweakPlayer()->GetGrappleJumpMode());
  switch (mOrbitState) {
  case kOS_Grapple: {
    if (!point) {
      BreakGrapple(kOR_Default, mgr);
      return;
    }
    switch (jumpMode) {
    case 0:
    case 2:
      switch (mGrappleState) {
      case kGS_JumpOff:
        mGrappleJumpTimeout -= input.DeltaTime();
        if (mGrappleJumpTimeout <= 0.f) {
          BreakGrapple(kOR_StopOrbit, mgr);
          SetMoveState(NPlayer::kMS_ApplyJump, mgr);
          ComputeMovement(input, mgr, input.DeltaTime());
          PreventFallingCameraPitch();
        }
        break;
      default:
        break;
      }
      break;
    case 1:
      switch (mGrappleState) {
      case kGS_Swinging:
        if (!FireBeamHeld(input) && mGrappleJumpTimeout <= 0.f) {
          mGrappleJumpTimeout = GetTweakPlayer()->GetGrappleReleaseTime();
          mGrappleState = kGS_JumpOff;
          ApplyGrappleJump(mgr);
        }
        break;
      case kGS_JumpOff:
        mGrappleJumpTimeout -= input.DeltaTime();
        if (mGrappleJumpTimeout <= 0.f) {
          SetMoveState(NPlayer::kMS_ApplyJump, mgr);
          ComputeMovement(input, mgr, input.DeltaTime());
          BreakGrapple(kOR_StopOrbit, mgr);
          PreventFallingCameraPitch();
        }
        break;
      case kGS_Firing:
      case kGS_Pull:
        if (!FireBeamHeld(input)) {
          BreakGrapple(kOR_StopOrbit, mgr);
        }
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
    const CVector3f eyePosition = GetEyePosition();
    const CVector3f playerToPoint = point->GetTranslation() - eyePosition;
    if (playerToPoint.CanBeNormalized()) {
      const CRayCastResult result = mgr.RayStaticIntersection(
          eyePosition, playerToPoint.AsNormalized(), playerToPoint.Magnitude(), kLineOfSightFilter);
      if (result.IsValid()) {
        BreakGrapple(kOR_LostGrappleLineOfSight, mgr);
      }
    }
    break;
  }
  case kOS_OrbitObject:
    if (mGun->GrappleArm() && mGun->GrappleArm()->IsGrappleBeamActive() &&
        GetTweakPlayer()->GetGrappleJumpMode() == 1 && !FireBeamHeld(input)) {
      BreakGrapple(kOR_StopOrbit, mgr);
    }
    break;
  default:
    break;
  }
}

bool CPlayer::ValidateFPPosition(CVector3f position, CStateManager& mgr) {
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Unknown59));
  const CVector3f margin(1.f, 1.f, 1.f);
  mgr.BuildColliderList(nearList, *this,
                        CAABox(mFpBounds.GetMinPoint() - margin + position,
                               mFpBounds.GetMaxPoint() + margin + position));
  const CAABox& baseBounds = GetBaseBoundingBox();
  const CCollidableAABox collisionBounds(
      CAABox(baseBounds.GetMinPoint() + position, baseBounds.GetMaxPoint() + position),
      CMaterialList());
  if (!CGameCollision::DetectCollisionBoolean(mgr, collisionBounds, CTransform4f::Identity(),
                                              filter, nearList)) {
    return true;
  }
  return false;
}

void CPlayer::ApplyGrappleForces(const CFinalInput& input, CStateManager& mgr, float dt) {
  const CVector3f playerPosition = GetTranslation();
  if (const CScriptGrapplePoint* point =
          TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(GetOrbitTargetId()))) {
    const CVector3f pointPosition = point->GetTranslation();
    switch (mGrappleState) {
    case kGS_Pull: {
      const CVector3f swingLow =
          pointPosition + CVector3f(0.f, 0.f, -GetTweakPlayer()->GetGrappleSwingLength());
      const CVector3f playerToPoint = pointPosition - GetTranslation();
      if (playerToPoint.CanBeNormalized()) {
        CVector3f playerToSwingLow = swingLow - playerPosition;
        if (playerToSwingLow.CanBeNormalized()) {
          const float distanceToLow = playerToSwingLow.Magnitude();
          playerToSwingLow.Normalize();
          const float timeToLow =
              CMath::Limit(distanceToLow / GetTweakPlayer()->GetGrapplePullDampenDistance(), 1.f);
          const float pullSpeed = timeToLow * (GetTweakPlayer()->GetGrapplePullSpeedMax() -
                                               GetTweakPlayer()->GetGrappleMaxVelocity()) +
                                  GetTweakPlayer()->GetGrappleMaxVelocity();
          const CVector3f pullVelocity = pullSpeed * playerToSwingLow;
          SetVelocityWR(pullVelocity);
          if (distanceToLow < GetTweakPlayer()->GetGrapplePullCloseDistance()) {
            mGrappleState = kGS_Swinging;
            mGrappleSwingTimer = 0.25f * GetTweakPlayer()->GetGrappleSwingPeriod();
            mGrappleJumpTimeout = 0.f;
            SetAligningGrappleSwingTurn(point->GetGrappleParameters().GetConstrainToAxis());
          } else {
            const CMotionState& motion = PredictMotion(dt);
            CVector3f lookDirectionFlat = GetTransform().GetForward();
            CVector3f newPlayerToPoint =
                pointPosition - (GetTranslation() + motion.GetTranslation());
            lookDirectionFlat.SetZ(0.f);
            if (lookDirectionFlat.CanBeNormalized()) {
              lookDirectionFlat.Normalize();
            }
            newPlayerToPoint.SetZ(0.f);
            if (newPlayerToPoint.CanBeNormalized()) {
              newPlayerToPoint.Normalize();
              float cosAngle = CVector3f::Dot(lookDirectionFlat, newPlayerToPoint);
              cosAngle = CMath::Limit(cosAngle, 1.f);
              const double lookToPointAngle = acosf(cosAngle);
              if (lookToPointAngle > 0.001) {
                float deltaAngle = dt * GetTweakPlayer()->GetGrapplePullCameraSpeed();
                if (lookToPointAngle >= deltaAngle) {
                  CVector3f leftDirection(lookDirectionFlat.GetY(), -lookDirectionFlat.GetX(), 0.f);
                  if (leftDirection.CanBeNormalized()) {
                    leftDirection.Normalize();
                  }
                  if (CVector3f::Dot(newPlayerToPoint, leftDirection) >= 0.f) {
                    deltaAngle = -deltaAngle;
                  }
                  RotateToOR(
                      CQuaternion::AxisAngle(CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes),
                                             CRelAngle::FromRadians(deltaAngle)),
                      dt);
                } else if (fabs(lookToPointAngle - M_PI) > 0.001) {
                  RotateToOR(CQuaternion::ShortestRotationArc(lookDirectionFlat, newPlayerToPoint),
                             dt);
                }
              } else {
                SetAngularVelocityWR(CAxisAngle::Identity());
                SetTorqueWR(CAxisAngle::Identity());
              }
            }
          }
        } else {
          mGrappleState = kGS_Swinging;
          mGrappleSwingTimer = 0.25f * GetTweakPlayer()->GetGrappleSwingPeriod();
          mGrappleJumpTimeout = 0.f;
        }
      }
      break;
    }
    case kGS_Swinging: {
      float turnAngleSpeed = (M_PIF / 180.f) * GetTweakPlayer()->GetMaxGrappleTurnSpeed();
      if (GetTweakPlayer()->GetInvertGrappleTurn()) {
        turnAngleSpeed *= -1.f;
      }
      const CVector3f pointToPlayer = playerPosition - pointPosition;
      const float pointToPlayerZProjection =
          CMath::Limit(CMath::AbsF(pointToPlayer.GetZ() / pointToPlayer.Magnitude()), 1.f);
      bool enableTurn = false;
      if (!point->GetGrappleParameters().GetConstrainToAxis()) {
        if (mControlMapper.GetAnalogInput(CControlMapper::kC_TurnLeft, input) > 0.05f) {
          enableTurn = true;
          turnAngleSpeed *= -mControlMapper.GetAnalogInput(CControlMapper::kC_TurnLeft, input);
        }
        if (mControlMapper.GetAnalogInput(CControlMapper::kC_TurnRight, input) > 0.05f) {
          enableTurn = true;
          turnAngleSpeed *= mControlMapper.GetAnalogInput(CControlMapper::kC_TurnRight, input);
        }
      } else if (IsAligningGrappleSwingTurn()) {
        enableTurn = true;
      }
      mGrappleSwingTimer += dt;
      if (mGrappleSwingTimer > GetTweakPlayer()->GetGrappleSwingPeriod()) {
        mGrappleSwingTimer -= GetTweakPlayer()->GetGrappleSwingPeriod();
      }
      CVector3f swingAxis = mGrappleSwingAxis;
      if (mGrappleSwingTimer < 0.5f * GetTweakPlayer()->GetGrappleSwingPeriod()) {
        swingAxis *= -1.f;
      }
      float swingCos =
          cosf(2.f * M_PIF * (mGrappleSwingTimer / GetTweakPlayer()->GetGrappleSwingPeriod()) +
               M_PIF / 2.f);
      swingCos = CMath::Limit(swingCos, 1.f);
      const float pullSpeed = CMath::AbsF(swingCos) * GetTweakPlayer()->GetGrappleMaxVelocity();
      CVector3f pullVector = pullSpeed * CVector3f::Cross(pointToPlayer.AsNormalized(), swingAxis);
      const float lengthError =
          pointToPlayer.Magnitude() - GetTweakPlayer()->GetGrappleSwingLength();
      const float lengthScale =
          CMath::Limit(lengthError / GetTweakPlayer()->GetGrappleSwingLength(), 1.f);
      pullVector += pointToPlayerZProjection * (-32.f * (lengthScale * pointToPlayer));
      const CVector3f backupVelocity = GetVelocityWR();
      SetVelocityWR(pullVector);
      const CTransform4f backupTransform = GetTransform();
      const CMotionState& predictedMotion = PredictMotion(dt);
      const CVector3f translation = GetTranslation();
      if (ValidateFPPosition(translation + predictedMotion.GetTranslation(), mgr)) {
        if (enableTurn) {
          CQuaternion turnRotation =
              CQuaternion::ZRotation(CRelAngle::FromRadians(turnAngleSpeed * dt));
          if (point->GetGrappleParameters().GetConstrainToAxis() && IsAligningGrappleSwingTurn()) {
            CVector3f playerDirection = GetTransform().GetForward();
            CVector3f pointDirection = point->GetTransform().GetForward().AsNormalized();
            float playerPointProjection =
                CVector3f::Dot(playerDirection.AsNormalized(), pointDirection);
            playerPointProjection = CMath::Limit(playerPointProjection, 1.f);
            if (CMath::AbsF(playerPointProjection) == 1.f) {
              SetAligningGrappleSwingTurn(false);
            }
            if (playerPointProjection < 0.f) {
              playerPointProjection = -playerPointProjection;
              pointDirection = -pointDirection;
            }
            float turnAngle = acosf(playerPointProjection);
            playerDirection.SetZ(0.f);
            turnAngle *= dt;
            turnRotation = CQuaternion::LookAt(playerDirection.AsNormalized(), pointDirection,
                                               CRelAngle::FromRadians(turnAngle));
          }
          if (pointToPlayer.MagSquared() > 0.2f * 0.2f) {
            const CVector3f pointAtPlayerHeight(pointPosition.GetX(), pointPosition.GetY(),
                                                playerPosition.GetZ());
            const CVector3f pointToPlayerFlat = playerPosition - pointAtPlayerHeight;
            const CVector3f playerToGrapplePlane =
                pointAtPlayerHeight + turnRotation.Transform(pointToPlayerFlat) - playerPosition;
            if (playerToGrapplePlane.CanBeNormalized()) {
              pullVector += (1.f / dt) * playerToGrapplePlane;
            }
          }
          const CVector3f backupSwingAxis = mGrappleSwingAxis;
          mGrappleSwingAxis = turnRotation.Transform(mGrappleSwingAxis);
          mGrappleSwingAxis.Normalize();
          const CVector3f swingForward(-mGrappleSwingAxis.GetY(), mGrappleSwingAxis.GetX(), 0.f);
          SetTransform(CTransform4f::FromColumns(mGrappleSwingAxis, swingForward,
                                                 CVector3f(0.f, 0.f, 1.f), GetTranslation()));
          SetVelocityWR(pullVector);
          if (!ValidateFPPosition(GetTranslation(), mgr)) {
            mGrappleSwingAxis = backupSwingAxis;
            SetTransform(backupTransform);
            SetVelocityWR(backupVelocity);
          }
        }
      } else {
        BreakGrapple(kOR_InvalidateTarget, mgr);
      }
      break;
    }
    case kGS_JumpOff: {
      ApplyForceOR(CVector3f(0.f, 0.f, GetGravity() * GetMass()), CAxisAngle::Identity());
      break;
    }
    default:
      break;
    }
  }
  SetAngularVelocityOR(
      CAxisAngle(CVector3f(0.f, 0.f, 0.9f * GetAngularVelocityOR().GetVector().GetZ())));
}

void CPlayer::UpdateGrappleArmTransform(const CVector3f& offset, CStateManager& mgr, float dt) {
  if (!mGun->GrappleArm()) {
    return;
  }
  CTransform4f armXf = GetTransform();
  const CVector3f armPosition = GetTransform().Rotate(offset) + GetTranslation();
  armXf.SetTranslation(armPosition);
  if (mMorphBallState != kMS_Unmorphed) {
    mGun->GrappleArm()->SetTransform(armXf);
  } else if (mGun->GrappleArm()->GetStateFlags() & CGrappleArm::kSF_Grappling) {
    CVector3f lookDirection = GetTransform().GetForward();
    CVector3f armToTarget = mGun->GrappleArm()->GetTransform().GetForward();
    if (lookDirection.CanBeNormalized()) {
      lookDirection.Normalize();
      if (mGrappleState != kGS_None) {
        if (const CActor* target =
                TCastToConstPtr< CActor >(mgr.GetObjectById(GetOrbitTargetId()))) {
          armToTarget = target->GetTranslation() - armPosition;
          CVector3f armToTargetFlat = armToTarget;
          armToTargetFlat.SetZ(0.f);
          if (armToTarget.CanBeNormalized()) {
            armToTarget.Normalize();
          }
          if (armToTargetFlat.CanBeNormalized() && mGrappleState != kGS_Firing) {
            const CQuaternion adjustment = CQuaternion::LookAt(
                armToTargetFlat.AsNormalized(), lookDirection, CRelAngle::FromRadians(2.f * M_PIF));
            armToTarget = adjustment.Transform(armToTarget);
            if (mGrappleSwingTimer >= 0.25f * GetTweakPlayer()->GetGrappleSwingPeriod() &&
                mGrappleSwingTimer < 0.75f * GetTweakPlayer()->GetGrappleSwingPeriod()) {
              armToTarget = mGun->GrappleArm()->GetTransform().GetForward();
            }
          }
        }
      }
      armXf = CTransform4f::LookAt(CVector3f::Zero(), armToTarget, CVector3f::Up());
      armXf.SetTranslation(armPosition);
      mGun->GrappleArm()->SetTransform(armXf);
    }
  }
}

CVector3f CPlayer::GetCameraForwardPoint() const {
  const CVector3f eyePosition = GetEyePosition();
  float distance;
  if (mOrbitState == kOS_OrbitObject) {
    distance = (mOrbitPoint - eyePosition).Magnitude();
  } else {
    distance = 10.f;
  }
  return eyePosition + distance * GetFirstPersonCameraTransform().GetForward();
}

void CPlayer::UpdateScreenSpaceMotion() {
  const CVector3f cameraPoint = GetCameraForwardPoint();
  const CVector3f eyePosition = GetEyePosition();
  const CVector3f previousPoint =
      mPreviousCameraForwardPoint + 0.5f * (eyePosition - mPreviousEyePosition);
  mPreviousCameraForwardPoint = cameraPoint;
  mPreviousEyePosition = eyePosition;

  if (mOrbitState == kOS_OrbitObject) {
    mScreenPosition = CVector2i(0, 0);
    return;
  }

  const CFirstPersonCamera* camera = mCameraManager->GetFirstPersonCamera();
  const CVector3f previousScreen = camera->ConvertToScreenSpace(previousPoint);
  const CVector3f currentScreen = camera->ConvertToScreenSpace(cameraPoint);
  if (!isfinite(previousScreen.GetX()) || !isfinite(currentScreen.GetX()) ||
      !isfinite(previousScreen.GetY()) || !isfinite(currentScreen.GetY()) ||
      !isfinite(previousScreen.GetZ()) || !isfinite(currentScreen.GetZ()) ||
      CMath::AbsF(previousScreen.GetX()) > 1.f || CMath::AbsF(currentScreen.GetX()) > 1.f ||
      CMath::AbsF(previousScreen.GetY()) > 1.f || CMath::AbsF(currentScreen.GetY()) > 1.f ||
      previousScreen.GetZ() >= 1.f || currentScreen.GetZ() >= 1.f) {
    mScreenPosition = CVector2i(0, 0);
    return;
  }

  mScreenPosition =
      CVector2i(static_cast< int >(0.5f * (currentScreen.GetX() - previousScreen.GetX()) *
                                   CGraphics::GetViewport().mWidth),
                static_cast< int >(0.5f * (currentScreen.GetY() - previousScreen.GetY()) *
                                   CGraphics::GetViewport().mHeight));
}

// Guessed name
void* CPlayer::GetScanTargetIdTextureData() const { return mScanTargetIdTextureData.get(); }

void* CPlayer::GetDepthHighTextureData() const { return mDepthHighTextureData.get(); }

void* CPlayer::GetDepthLowTextureData() const { return mDepthLowTextureData.get(); }

TUniqueId CPlayer::FindScanTargetId(const CStateManager& mgr) const {
  uint height = kScanTargetTextureHeight;
  uint width = kScanTargetTextureWidth;
  TUniqueId selectedId = kInvalidUniqueId;
  uint selectedPalette = 0;

  if (mScanTargetIdTextureData.get() && mTargeting.get()) {
    if (mgr.IsMultiplayer()) {
      height >>= 1;
    }
    if (mgr.GetNumPlayers() >= 3u) {
      width >>= 1;
    }

    rstl::reserved_vector< uint, 62 > histogram;
    histogram.resize(62, 0u);
    const uchar* paletteData = static_cast< const uchar* >(mScanTargetIdTextureData.get());
    for (uint i = 0; i < height * width; ++i) {
      const uint palette = paletteData[i] >> 2;
      if (palette < 62) {
        ++histogram[palette];
      }
    }

    uint bestCount = 0;
    for (int palette = 1; palette < 62; ++palette) {
      const uint count = histogram[palette];
      if (count > bestCount && count - bestCount > 16u) {
        const TUniqueId id = mTargeting->GetScanTargetId(mgr, palette);
        const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(id));
        if (actor && actor->GetMaterialList().HasMaterial(kMT_Scannable)) {
          bestCount = count;
          selectedId = id;
          selectedPalette = palette;
        }
      }
    }
  }

  const uchar* paletteData = static_cast< const uchar* >(mScanTargetIdTextureData.get());
  const ushort* depthData = static_cast< const ushort* >(mDepthHighTextureData.get());
  const uchar* lowDepthData = static_cast< const uchar* >(mDepthLowTextureData.get());
  double totalDepth = 0.;
  uint sampleCount = 0;
  for (uint i = 0; i < height * width; ++i) {
    if (paletteData[i] >> 2 == selectedPalette) {
      const ushort depth = ushort((depthData[i] << 8) | (depthData[i] >> 8));
      totalDepth += 256. * depth + lowDepthData[i];
      ++sampleCount;
    }
  }

  const double meanDepth = totalDepth / sampleCount;
  const double nearClip = mCameraManager->GetCurrentCamera(mgr, false)->GetNearClipDistance();
  const double farClip = mCameraManager->GetCurrentCamera(mgr, false)->GetFarClipDistance();
  const double distance =
      (-farClip * nearClip) / ((meanDepth / 16777215.) * (farClip - nearClip) - farClip);
  if (selectedId != mOrbitTargetId) {
    const CScriptPointOfInterest* point =
        TCastToConstPtr< CScriptPointOfInterest >(mgr.GetObjectById(selectedId));
    if (point && !point->GetLookAt()) {
      CVector3f position = mCameraManager->GetCurrentCamera(mgr, false)->GetTranslation();
      position += float(distance) *
                  mCameraManager->GetCurrentCamera(mgr, false)->GetTransform().GetForward();
      // The const lookup borrows a view of a mutable registered game entity.
      const_cast< CScriptPointOfInterest* >(point)->SetTranslation(position);
    }
  }

  DCInvalidateRange(mScanTargetIdTextureData.get(), kScanTargetTextureSize);
  DCInvalidateRange(mDepthHighTextureData.get(), kScanTargetTextureSize * 2);
  DCInvalidateRange(mDepthLowTextureData.get(), kScanTargetTextureSize);
  return selectedId;
}
