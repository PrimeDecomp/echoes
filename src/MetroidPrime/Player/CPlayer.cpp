#include "MetroidPrime/Player/CPlayer.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "dolphin/types.h"

CPlayer::CPlayer(TUniqueId uid, const CTransform4f& xf, const CAABox& aabb, CAssetId resId,
                 const CVector3f& playerScale, float mass, float stepUp, float stepDown,
                 float ballRadius, const CMaterialList& ml, CPlayerState* playerState,
                 CCameraManager*, bool, int playerIndex, int gunParam, int charIdx)
: CPhysicsActor(uid, CBasics::Stringize("CPlayer (%d)", playerIndex),
                CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList, true), 0, xf,
                CAnimRes(resId, charIdx, CVector3f(1.8f, 1.8f, 1.8f), 0, true), ml, aabb,
                SMoverData(mass), CActorParameters::None(), StepData(stepUp, stepDown, 0.0f))

, m_movementState(NPlayer::kMS_OnGround)
, m_ballTransitionsRes()
, m_attachedActor(kInvalidUniqueId)
, m_attachedActorTime(0.f)
, m_energyDrain(4)
, m_startingJumpTimeout(0.f)
, m_sjTimer(0.f)
, m_minJumpTimeout(0.f)
, m_jumpCameraTimer(0.f)
, m_jumpPresses(0)
, m_fallCameraTimer(0.f)
, m_cancelCameraPitch(false)
, m_timeSinceJump(1000.f)
, x324_unk(1000.0f)
, x328_unk(0.f)
, x32c_unk(CVector3f::Zero())
, m_lastSpaceJumpPosition(CVector3f::Zero())
, m_surfaceRestraint(kSR_Normal)
, m_accelerationTable()
, m_curAcceleration(1)
, m_accelerationChangeTimer(0.f)
, m_fpBounds(aabb)
, m_ballTransHeight(1.f)
, m_cameraState(kCS_FirstPerson)
, m_morphBallState(kMS_Unmorphed)
, m_spawnedMorphBallState(kMS_Unmorphed)
, x394_unk_sa(false)
, x398_unk_sa_state(kMS_Unmorphed)
, x39c_unk(kMS_Unmorphed)
, m_fallingTime(0.f)
, m_orbitState(kOS_NoOrbit)
, m_orbitType(kOT_Close)
, m_orbitRequest(3)
, m_orbitTargetId(kInvalidUniqueId)
, m_orbitPoint(0.f, 0.f, 0.f)
, m_orbitVector(0.f, 0.f, 0.f)
, m_orbitModeTimer(0.f)
, m_orbitZoneMode(kZI_Targeting)
, m_orbitZoneType(kZT_Ellipse)
, x3d8_unk(1)
, m_orbitNextTargetId(kInvalidUniqueId)
, x3e0_unk(0.f)
, m_nearbyOrbitObjects()
, m_onScreenOrbitObjects()
, m_offScreenOrbitObjects()
, m_orbitLockEstablished(false)
, m_orbitPreventionTimer(0.f)
, m_sidewaysDashing(false)
, m_strafeInputAtDash(0.f)
, m_dashTimer(0.f)
, m_dashButtonHoldTime(0.f)
, m_doneSidewaysDashing(false)
, m_orbitSource(2)
, m_orbitingEnemy(false)
, x591_unk(false)
, x594_unk(0.f)
, x598_unk(0)
, x59c_unk(0x14)
, m_dashSpeedMultiplier(1.5f)
, m_noStrafeDashBlend(false)
, m_dashDuration(0.5f)
, m_strafeDashBlendDuration(0.449f)
, m_scanState(kSS_NotScanning)
, m_scanningTime(0.f)
, m_curScanTime(0.f)
, m_scanningObject(kInvalidUniqueId)
, m_scanningObjectId(kInvalidAssetId)
, m_grappleState(kGS_None)
, m_grappleSwingTimer(0.f)
, m_grappleSwingAxis(0.f, 1.f, 0.f)
, x5e0_unk(0.f)
, x5e4_unk(0.f)
, x5e8_unk(0.f)
, m_grappleJumpTimeout(0.f)
, x5f0_unk(0xff)
, m_inFreeLook(false)
, m_lookButtonHeld(false)
, m_lookAnalogHeld(false)
, x5f4_unk(false)
, x5f5_unk(false)
, m_curFreeLookCenteredTime(0.f)
, m_freeLookYawAngle(0.f)
, m_horizFreeLookAngleVel(0.f)
, m_freeLookPitchAngle(0.f)
, m_vertFreeLookAngleVel(0.f)
, m_aimTarget(kInvalidUniqueId)
, m_targetAimPosition(CVector3f::Zero())
, m_aimTargetAverage()
, m_assistedTargetAim(CVector3f::Zero())
, m_aimTargetTimer(0.f)
, x6a8_unk(0)
, xeac_unk(0)
, xeb0_unk(0x14)
, xeb4_unk(10000.0f)
, xeb8_unk(10000.0f)
, m_gun(new CPlayerGun(uid, gunParam))
, m_gunAlpha(1.f)
, xec4_unk()
, xec8_unk(false)
, xec9_unk(false)
, xeca_unk(3)
, xecc_unk(1.0f, 1.0f, 1.0f, 1.0f)
, xed0_unk(0)
, xed4_unk(new UnkStructA(uid))
, xed8_unk(nullptr)
, xedc_unk()
, xf6c_unk(0)
, xf70_unk(new UnkStructC)
// , m_gunHolsterState(kGH_Drawn)
// , m_gunHolsterRemTime(gpTweakPlayerGun->x40_gunNotFiringTime)
// , m_playerStuckTracker(rs_new CPlayerStuckTracker())
, m_moveSpeedAvg()
, m_moveSpeed(0.f)
, m_flatMoveSpeed(0.f) {}
