#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetaRender/SModelRenderData.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CMapWorld.hpp"
#include "MetroidPrime/CMapWorldInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"
#include "MetroidPrime/TCastTo.hpp"

static const CColor skLockedColor = CColor::Grey();
static const CColor skResetColor(uchar(0), uchar(255), uchar(255), uchar(255));

int CScriptDoor::FindAnimation(const CPASAnimParmData& parms) const {
  return HasAnimation() ? GetAnimationData()->FindBestAnimation(parms) : -1;
}

CScriptDoor::CScriptDoor(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CTransform4f& xf, const CModelData& model,
                         const rstl::optional_object< CModelData >& shellModel,
                         const rstl::optional_object< CModelData >& blueShellModel,
                         const rstl::optional_object< TLockedToken< CTexture > >& burnTexture,
                         const CColor& shellColor, const CHealthInfo& health,
                         const CDamageVulnerability& vulnerability,
                         const CActorParameters& parameters, CAssetId alternateScan,
                         const CVector3f& orbitOffset, const CAABox& bounds, bool open, bool locked,
                         float openTime, float closeTime, float closeDelay, float shieldFadeOutTime,
                         float shieldFadeInTime, bool ballDoor, bool horizontal)
: CPhysicsActor(uid, name, info, 0, xf, model,
                open ? CMaterialList(kMT_Unknown59, kMT_Immovable, kMT_Orbit)
                     : CMaterialList(kMT_Immovable, kMT_Occluder, kMT_Unknown59, kMT_Orbit),
                bounds, SMoverData(1.f), parameters, StepData(0.3f, 0.3f, 0))
, mDoorState(open ? kDS_Open : kDS_Closed)
, mOpenTime(openTime)
, mCloseTime(closeTime)
, mCloseDelay(closeDelay)
, mShieldState(open ? kSS_Hidden : kSS_Visible)
, mShieldAlpha(open ? 0.f : 1.f)
, mShieldFadeOutTime(shieldFadeOutTime)
, mShieldFadeInTime(shieldFadeInTime)
, mShellModel(shellModel)
, mBlueShellModel(blueShellModel)
, mBurnTexture(burnTexture)
, mLockState(kLS_Unlocked)
, mLockActorId(kInvalidUniqueId)
, mLockTimer(0.f)
, mAnimationId(3)
, mBounds(bounds)
, mCurrentHealth(health)
, mInitialHealth(health)
, mBaseVulnerability(vulnerability)
, mCurrentVulnerability(vulnerability)
, mPartnerDoorId(kInvalidUniqueId)
, mOpeningSenderDoorId(kInvalidUniqueId)
, mDockId(kInvalidUniqueId)
, mOrbitOffset(orbitOffset)
, mOpenRequestCount(0)
, mBurnOrigin(CVector3f::Zero())
, mCurrentShellColor(CColor::Black())
, mShellColor(shellColor)
, mClosedAnimation(FindAnimation(CPASAnimParmData(pas::kAS_Generate, CPASAnimParm::FromEnum(1))))
, mOpeningAnimation(FindAnimation(CPASAnimParmData(pas::kAS_Generate, CPASAnimParm::FromEnum(3))))
, mClosingAnimation(FindAnimation(CPASAnimParmData(pas::kAS_Generate, CPASAnimParm::FromEnum(0))))
, mOpenAnimation(FindAnimation(CPASAnimParmData(pas::kAS_Generate, CPASAnimParm::FromEnum(4))))
, mAlternateScan(nullptr)
, mWasOpen(open)
, mIsOpen(open)
, mBallDoor(ballDoor)
, mInitiallyLocked(locked)
, mColorDirty(true)
, mResetPending(false)
, mHasReset(false)
, mHorizontal(horizontal) {
  SetDoorAnimation(open ? kDAT_Opening : kDAT_Closed);
  SetMass(0.f);

  if (alternateScan != kInvalidAssetId) {
    mAlternateScan = rs_new TCachedToken< CScannableObjectInfo >(
        gpSimplePool->GetObj(SObjectTag('SCAN', alternateScan)), true);
  }
}

rstl::optional_object< CAABox > CScriptDoor::GetTouchBounds() const {
  if (GetActive() && GetMaterialList().HasMaterial(kMT_Unknown59)) {
    return GetBoundingBox();
  }
  return rstl::optional_object_null();
}

CVector3f CScriptDoor::GetOrbitPosition(const CStateManager& mgr) const {
  return GetTranslation() + mOrbitOffset;
}

void CScriptDoor::SetDoorAnimation(EDoorAnimType animation) {
  int animationId = 0;
  switch (animation) {
  case kDAT_Open:
    animationId = mOpenAnimation;
    break;
  case kDAT_Closing:
    animationId = mClosingAnimation;
    break;
  case kDAT_Opening:
    animationId = mOpeningAnimation;
    break;
  case kDAT_Closed:
    animationId = mClosedAnimation;
    break;
  }
  mAnimationId = animationId;
  if (HasAnimation()) {
    AnimationData()->SetAnimation(CAnimPlaybackParms(animationId, -1, 1.f, true), false);
  }
}

void CScriptDoor::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId sender = msg.GetSenderId();
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  if (!GetActive()) {
    return;
  }

  switch (message) {
  case kSM_Open:
    if (!mIsOpen) {
      const CScriptDoor* door = TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(sender));
      mOpeningSenderDoorId = door ? door->GetUniqueId() : kInvalidUniqueId;
      ++mOpenRequestCount;
      if (mOpeningSenderDoorId != kInvalidUniqueId && mLockState != kLS_Unlocked) {
        SetLockState(mgr, kLS_Unlocked);
      }
      if (mDoorState == kDS_Closed) {
        SetDoorState(mgr, kDS_WaitingForArea);
      }
    }
    break;
  case kSM_Close:
    --mOpenRequestCount;
    if (mOpenRequestCount < 0) {
      mOpenRequestCount = 0;
    }
    if (mOpenRequestCount == 0 || (sender == mOpeningSenderDoorId && mDoorState != kDS_Closed)) {
      if (mDoorState == kDS_WaitingForArea) {
        SetDoorState(mgr, kDS_Closed);
      } else if (mDoorState != kDS_Closed) {
        SetDoorState(mgr, kDS_Closing);
      }
    }
    break;
  case kSM_Increment:
    ++mOpenRequestCount;
    break;
  case kSM_Decrement:
    --mOpenRequestCount;
    break;
  case kSM_Lock:
    SetLockState(mgr, kLS_Pending);
    break;
  case kSM_Unlock:
    SetLockState(mgr, kLS_Unlocking);
    break;
  case kSM_Reset:
    if (!mResetPending) {
      if ((mDoorState == kDS_Open || mDoorState == kDS_Closed) && mLockState == kLS_Unlocked) {
        ResetDoor(mgr);
      } else {
        mResetPending = true;
      }
    }
    break;
  case kSM_AreaLoaded:
    mDockId = FindConnectedObject(mgr, kSS_InvalidState, kSM_Increment);
    mLockActorId = FindConnectedObject(mgr, kSS_Connect, kSM_Attach);
    if (mInitiallyLocked) {
      SetLockState(mgr, kLS_Locked);
    }
    break;
  }
}

void CScriptDoor::SetShieldAlpha(float alpha, CStateManager& mgr) {
  const rstl::vector< TUniqueId > slaves = FindConnectedObjects(mgr, kSS_Slave, kSM_Activate);
  for (rstl::vector< TUniqueId >::const_iterator it = slaves.begin(); it != slaves.end(); ++it) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(*it))) {
      actor->SetActive(alpha != 0.f);
      actor->SetModelFlags(CModelFlags::AlphaBlended(alpha));
    }
  }
}

void CScriptDoor::UpdateShield(float dt, CStateManager& mgr) {
  switch (mShieldState) {
  case kSS_FadingOut:
    mColorDirty = true;
    mShieldAlpha -= dt / mShieldFadeOutTime;
    if (mShieldAlpha <= 0.f) {
      mShieldAlpha = 0.f;
      mShieldState = kSS_Hidden;
    }
    SetShieldAlpha(mShieldAlpha, mgr);
    break;
  case kSS_FadingIn:
    mColorDirty = true;
    mShieldAlpha += dt / mShieldFadeInTime;
    if (mShieldAlpha >= 1.f) {
      mShieldAlpha = 1.f;
      mShieldState = kSS_Visible;
    }
    SetShieldAlpha(mShieldAlpha, mgr);
    break;
  }
}

void CScriptDoor::SetLockAnimation(CStateManager& mgr, int animation) {
  if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(mLockActorId))) {
    if (actor->HasAnimation()) {
      actor->AnimationData()->SetAnimation(CAnimPlaybackParms(animation, -1, 1.f, true), false);
    }
  }
}

void CScriptDoor::SetLockState(CStateManager& mgr, ELockState state) {
  switch (state) {
  case kLS_Unlocked:
    SetLockAnimation(mgr, 0);
    mCurrentVulnerability = mBaseVulnerability;
    mLockState = kLS_Unlocked;
    mColorDirty = true;
    mLockTimer = 0.f;
    if (mResetPending) {
      ResetDoor(mgr);
      mResetPending = false;
    }
    break;
  case kLS_Pending:
    if (mLockState == kLS_Unlocked || mLockState == kLS_Unlocking) {
      mLockState = kLS_Pending;
    }
    break;
  case kLS_Locking:
    SetLockAnimation(mgr, 1);
    mCurrentVulnerability = CDamageVulnerability::ReflectVulnerabilty();
    mLockState = kLS_Locking;
    break;
  case kLS_Locked:
    SetLockAnimation(mgr, 3);
    mCurrentVulnerability = CDamageVulnerability::ReflectVulnerabilty();
    mLockState = kLS_Locked;
    mColorDirty = true;
    mLockTimer = 0.f;
    break;
  case kLS_Unlocking:
    if (mLockState != kLS_Unlocked) {
      SetLockAnimation(mgr, 2);
      mLockState = kLS_Unlocking;
      if (mResetPending && !mHasReset) {
        mShellColor = skResetColor;
        if (mBlueShellModel) {
          mShellModel = mBlueShellModel;
        }
      }
    }
    break;
  }
}

void CScriptDoor::UpdateLock(float dt, CStateManager& mgr) {
  switch (mLockState) {
  case kLS_Pending:
    if (mgr.GetPlayer(0)->GetCurrentAreaId() == GetCurrentAreaId()) {
      switch (mDoorState) {
      case kDS_Closed:
        SetLockState(mgr, kLS_Locking);
        mColorDirty = true;
        break;
      case kDS_WaitingForArea:
        SetDoorState(mgr, kDS_Closed);
        SetLockState(mgr, kLS_Locking);
        mColorDirty = true;
        break;
      case kDS_Opening:
      case kDS_Open:
      case kDS_CloseDelay:
        SetDoorState(mgr, kDS_Closing);
        break;
      }
    }
    break;
  case kLS_Locking:
  case kLS_Unlocking: {
    const CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(mLockActorId));
    if (mLockTimer >= 1.f && (!actor || !actor->GetModelData()->IsAnimating())) {
      SetLockState(mgr, mLockState == kLS_Locking ? kLS_Locked : kLS_Unlocked);
    }
    mLockTimer += dt;
    mColorDirty = true;
    break;
  }
  }
}

void CScriptDoor::SetDoorState(CStateManager& mgr, EDoorState state) {
  mDoorState = state;
  switch (state) {
  case kDS_Closed:
    if (mShieldState != kSS_Visible) {
      mShieldState = kSS_FadingIn;
    }
    mWasOpen = false;
    mOpeningSenderDoorId = kInvalidUniqueId;
    for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
      mgr.CameraManager(i)->BallCamera()->DoorClosed(GetUniqueId());
    }
    SendScriptMsgs(kSS_Closed, mgr, kInvalidUniqueId, kSM_None);
    mCurrentHealth = mInitialHealth;
    break;
  case kDS_WaitingForArea:
    mShieldState = kSS_FadingOut;
    break;
  case kDS_Opening:
    mIsOpen = true;
    mgr.MapWorldInfo()->SetDoorVisited(mgr.GetEditorIdForUniqueId(GetUniqueId()), true);
    mWasOpen = true;
    SendScriptMsgs(kSS_Opened, mgr, kInvalidUniqueId, kSM_None);
    mPartnerDoorId = kInvalidUniqueId;
    if (mOpeningSenderDoorId == kInvalidUniqueId || mgr.GetNextAreaId() == GetCurrentAreaId()) {
      SetDoorAnimation(kDAT_Opening);
      if (const CScriptDock* dock = TCastToConstPtr< CScriptDock >(mgr.GetObjectById(mDockId))) {
        const CScriptDock* connectedDock =
            TCastToConstPtr< CScriptDock >(mgr.GetObjectById(dock->GetConnectedScriptDockId(mgr)));
        if (connectedDock) {
          const rstl::list< CEntity* >& doors = mgr.GetDoorList();
          for (rstl::list< CEntity* >::const_iterator it = doors.begin(); it != doors.end(); ++it) {
            CScriptDoor* door = static_cast< CScriptDoor* >(*it);
            if (door && door->GetUniqueId() != mOpeningSenderDoorId &&
                door->mDockId == connectedDock->GetUniqueId()) {
              mPartnerDoorId = door->GetUniqueId();
              mgr.SendScriptMsg(door, GetUniqueId(), kSM_Open, kInvalidUniqueId);
              return;
            }
          }
        }
      }
    } else {
      SetDoorAnimation(kDAT_Open);
      SetDoorState(mgr, kDS_Open);
    }
    break;
  case kDS_Open:
    RemoveMaterial(kMT_Unknown59, kMT_Occluder, kMT_Orbit, kMT_Scannable, mgr);
    ResetBurnOrigin();
    if (mResetPending) {
      ResetDoor(mgr);
      mResetPending = false;
    }
    break;
  case kDS_CloseDelay:
    mCloseTimer = mCloseDelay;
    break;
  case kDS_Closing:
    mIsOpen = false;
    SetDoorAnimation(kDAT_Closing);
    if (GetScannableObjectInfo()) {
      AddMaterial(kMT_Unknown59, kMT_Metal, kMT_Occluder, kMT_Orbit, kMT_Scannable, mgr);
    } else {
      AddMaterial(kMT_Unknown59, kMT_Metal, kMT_Occluder, kMT_Orbit, mgr);
    }
    for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
      mgr.CameraManager(i)->BallCamera()->DoorClosing(GetUniqueId());
    }
    if (mPartnerDoorId != kInvalidUniqueId) {
      if (CEntity* partner = mgr.ObjectById(mPartnerDoorId)) {
        mgr.SendScriptMsg(partner, GetUniqueId(), kSM_Close, kInvalidUniqueId);
      }
      mPartnerDoorId = kInvalidUniqueId;
    }
    break;
  }
}

void CScriptDoor::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  UpdateShield(dt, mgr);
  UpdateLock(dt, mgr);
  if (GetModelData()->IsAnimating()) {
    const float duration = GetModelData()->GetAnimationDuration(mAnimationId);
    float speed = 1.f;
    if (mDoorState == kDS_Opening) {
      speed = duration / mOpenTime;
    } else if (mDoorState == kDS_Closing) {
      speed = duration / mCloseTime;
    }
    UpdateAnimation(speed * dt, mgr, true);
  }

  switch (mDoorState) {
  case kDS_Closed:
    if (mCurrentHealth.GetHP() <= 0.f && mgr.GetNextAreaId() == GetCurrentAreaId()) {
      SetDoorState(mgr, kDS_WaitingForArea);
      SendScriptMsgs(kSS_Dead, mgr, kInvalidUniqueId, kSM_None);
    }
    break;
  case kDS_WaitingForArea: {
    CScriptDock* dock = TCastToPtr< CScriptDock >(mgr.ObjectById(mDockId));
    if (!dock) {
      SetDoorState(mgr, kDS_Opening);
      break;
    }
    CWorld* world = mgr.World();
    if (!world->DoesAreaExist(dock->GetAreaId())) {
      SetDoorState(mgr, kDS_Closed);
      break;
    }
    const CGameArea::Dock& areaDock =
        world->GetAreaAlways(dock->GetAreaId()).GetDock(dock->GetDockId());
    const TAreaId connectedArea = areaDock.GetConnectedAreaId(dock->GetDockReference(mgr));
    if (!world->DoesAreaExist(connectedArea)) {
      SetDoorState(mgr, kDS_Closed);
      break;
    }

    bool canOpen = true;
    const rstl::list< CEntity* >& doors = mgr.GetDoorList();
    for (rstl::list< CEntity* >::const_iterator it = doors.begin(); it != doors.end(); ++it) {
      const CScriptDoor* door = static_cast< const CScriptDoor* >(*it);
      if (door && door->GetUniqueId() != GetUniqueId() &&
          door->GetCurrentAreaId() == GetCurrentAreaId() && door->mWasOpen &&
          door->mDockId != kInvalidUniqueId) {
        canOpen = false;
      }
    }
    if (!canOpen || mOpenRequestCount == 0) {
      break;
    }
    CGameArea* area = world->Area(connectedArea);
    if (!area->IsLoaded()) {
      mgr.SendScriptMsg(dock, GetUniqueId(), kSM_SetToMax, kInvalidUniqueId);
      break;
    }
    if (area->GetPostConstructed()->x190_ != 0 || !world->IsAreaValid(dock->GetAreaId()) ||
        !world->AreSkyNeedsMet()) {
      break;
    }
    bool finishedOccluding = true;
    for (CGameArea::CConstChainIterator it = world->GetChainHead(CWorld::kC_Alive);
         it != CWorld::skGlobalEnd; ++it) {
      if (it->GetId() != area->GetId() && !it->IsFinishedOccluding()) {
        finishedOccluding = false;
      }
    }
    if (finishedOccluding && area->TryTakingOutOfARAM() &&
        !world->GetMapWorld()->IsMapAreasStreaming()) {
      SetDoorState(mgr, kDS_Opening);
    }
    break;
  }
  case kDS_Opening:
    if (!GetModelData()->IsAnimating() && mShieldAlpha == 0.f) {
      SetDoorState(mgr, kDS_Open);
    }
    break;
  case kDS_Open:
    if (mOpenRequestCount == 0) {
      SetDoorState(mgr, kDS_CloseDelay);
    }
    break;
  case kDS_CloseDelay:
    mCloseTimer -= dt;
    if (mOpenRequestCount == 0) {
      if (mCloseTimer <= 0.f) {
        SetDoorState(mgr, kDS_Closing);
      }
    } else {
      SetDoorState(mgr, kDS_Open);
    }
    break;
  case kDS_Closing:
    if (!GetModelData()->IsAnimating()) {
      SetDoorState(mgr, kDS_Closed);
    }
    break;
  }

  UpdateShellColor(dt);
  for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
    SetValidTarget(i, mgr.GetPlayerState(i)->GetCurrentVisor() == CPlayerState::kPV_Scan);
  }
}

bool CScriptDoor::IsConnectedToArea(const CStateManager& mgr, TAreaId area) const {
  const CScriptDock* dock = TCastToConstPtr< CScriptDock >(mgr.GetObjectById(mDockId));
  if (dock) {
    if (dock->GetAreaId() == area) {
      return true;
    }
    const CGameArea::Dock& areaDock =
        mgr.GetWorld()->GetAreaAlways(dock->GetAreaId()).GetDock(dock->GetDockId());
    return areaDock.GetConnectedAreaId(dock->GetDockReference(mgr)) == area;
  }
  return false;
}

void CScriptDoor::ForceClosed(CStateManager& mgr) {
  if (mIsOpen) {
    SetDoorState(mgr, kDS_Closing);
  }
}

void CScriptDoor::UpdateShellColor(float dt) {
  if (!mColorDirty) {
    return;
  }
  mColorDirty = false;

  switch (mLockState) {
  case kLS_Locked:
    mCurrentShellColor = skLockedColor;
    break;
  case kLS_Locking:
    mCurrentShellColor =
        CColor::Lerp(mShellColor, skLockedColor, CMath::Clamp(0.f, mLockTimer, 1.f));
    break;
  case kLS_Unlocking:
    mCurrentShellColor =
        CColor::Lerp(skLockedColor, mShellColor, CMath::Clamp(0.f, mLockTimer, 1.f));
    break;
  default:
    mCurrentShellColor = mShellColor;
    break;
  }
}

EWeaponCollisionResponseTypes CScriptDoor::GetCollisionResponseType(const CVector3f& point,
                                                                    const CVector3f& normal,
                                                                    const CWeaponMode& mode,
                                                                    int attribs) const {
  return GetDamageVulnerability()->GetEffect(mode) == CWeaponTypeVulnerability::kE_Reflect
             ? kWCR_Unknown15
             : kWCR_OtherProjectile;
}

void CScriptDoor::AddToRenderer(const CStateManager& mgr) const {
  if (GetPreRenderClipped()) {
    return;
  }
  CPhysicsActor::Render(mgr);
  if (!mShellModel || !(mShieldAlpha > 0.f)) {
    return;
  }

  if (mShieldAlpha < 1.f && mBurnTexture) {
    const CModel& model = **mShellModel->PickStaticModel(CModelData::kWM_Normal);
    model.Touch(0);
    if (!model.IsLoaded(0)) {
      return;
    }
    gpRender->SetModelMatrix(GetTransform() * CTransform4f::Scale(mShellModel->GetScale()));
    gpRender->DrawModelWithTextureMask(SModelRenderData(model), ***mBurnTexture, mBurnOrigin,
                                       mCurrentShellColor.WithAlphaOf(mShieldAlpha),
                                       10.f * (mShieldAlpha * mShieldAlpha));
  } else {
    mShellModel->Render(mgr, GetTransform(), nullptr,
                        CModelFlags::AlphaBlended(mCurrentShellColor.WithAlphaOf(mShieldAlpha)));
  }
}

void CScriptDoor::Render(const CStateManager& mgr) const {}

CHealthInfo* CScriptDoor::HealthInfo() { return &mCurrentHealth; }

const CDamageVulnerability* CScriptDoor::GetDamageVulnerability() const {
  return &mCurrentVulnerability;
}

void CScriptDoor::SetBurnOrigin(const CVector3f& position) {
  mBurnOrigin = GetTransform().GetInverse() * position;
}

void CScriptDoor::ResetBurnOrigin() { mBurnOrigin = GetModelData()->GetBounds().GetCenterPoint(); }

bool CScriptDoor::CanRenderUnsorted(const CStateManager& mgr) const {
  if (mDoorState != kDS_Closed) {
    return false;
  }
  return CActor::CanRenderUnsorted(mgr);
}

CScannableObjectInfo* CScriptDoor::GetScannableObjectInfo() const {
  if (mLockState != kLS_Unlocked) {
    return nullptr;
  }
  if (!mHasReset && !mAlternateScan.null()) {
    return mAlternateScan->GetObject();
  }
  return CActor::GetScannableObjectInfo();
}

void CScriptDoor::ResetDoor(CStateManager& mgr) {
  static const CDamageVulnerability::TWeaponVulnerability overrides[] = {
      CDamageVulnerability::TWeaponVulnerability(
          kWT_BoostBall, CWeaponTypeVulnerability(0.f, CWeaponTypeVulnerability::kE_Normal, false)),
      CDamageVulnerability::TWeaponVulnerability(
          kWT_Bomb, CWeaponTypeVulnerability(1.f, CWeaponTypeVulnerability::kE_Normal, false)),
      CDamageVulnerability::TWeaponVulnerability(
          kWT_PowerBomb, CWeaponTypeVulnerability(1.f, CWeaponTypeVulnerability::kE_Normal, false)),
      CDamageVulnerability::TWeaponVulnerability(
          kWT_ScrewAttack,
          CWeaponTypeVulnerability(0.f, CWeaponTypeVulnerability::kE_Normal, false)),
  };
  static const CDamageVulnerability vulnerability(
      CDamageVulnerability::NormalIgnoreRadiusVulnerability(), overrides, 4,
      CDamageVulnerability::kOF_Normal);
  mCurrentVulnerability = mBaseVulnerability = vulnerability;
  mShellColor = skResetColor;
  if (mBlueShellModel) {
    mShellModel = mBlueShellModel;
  }
  mColorDirty = true;
  mHasReset = true;
  mgr.MapWorldInfo()->SetDoorVisited(mgr.GetEditorIdForUniqueId(GetUniqueId()), true);
}

CScriptDoor::~CScriptDoor() {}
