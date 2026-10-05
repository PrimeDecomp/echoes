#include "MetroidPrime/Weapons/CHomingBlob.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "WorldFormat/CCollisionCache.hpp"

// Guessed names for the target's writable rendering parameters.
static float sDistanceConstant = -5.f;
static float sAngleConstant = 5.f;
static float sAmbientRed = 1.f;
static float sAmbientGreen = 0.8f;
static float sAmbientBlue = 1.f;
static float sLightRed = 1.f;
static float sLightGreen = 1.f;
static float sLightBlue = 1.f;
static float sDistanceLinear;
static float sDistanceQuadratic;
static float sAngleLinear;
static float sAngleQuadratic;

CHomingBlob::CHomingBlob(const TToken< CGenDescription >& particle, TUniqueId uid, TAreaId areaId,
                         TUniqueId owner, bool active, const CAABox& bounds,
                         const CDamageInfo& damage, int playerIndex, const rstl::string& name,
                         const CTransform4f& xf, int modeFlags, float generatorRate,
                         float collisionRadius, float nearTargetDistance, float escapeDistance,
                         float targetSearchRadius, float homingAcceleration)
: CWeapon(uid, areaId, active, owner, kWT_Dark, name, xf,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59),
                                              CMaterialList(kMT_Character, kMT_Player)),
          CMaterialList(kMT_Projectile), damage, kPA_None, CModelData())
, mCollisionBounds(bounds)
, mParticleGen(rs_new CElementGen(particle, CElementGen::kMOT_One, CElementGen::kOSF_One))
, mCollisionCache(rs_new CCollisionCache(mCollisionBounds, 2, 2, uid.Value() & 0x3ff))
, mLightId(kInvalidUniqueId)
, mParticleAssetId(CToken(particle).GetTag().GetId())
, mTargetIds()
, mNextParticleTarget(0)
, mParticleUpdatePhase(0)
, mElapsedTime(0.f)
, x220_(6.f)
, mGeneratorRate(generatorRate)
, mCollisionRadius(collisionRadius)
, mNearTargetDistance(nearTargetDistance)
, mEscapeDistanceSquared(escapeDistance * escapeDistance)
, mTargetSearchRadius(targetSearchRadius)
, mHomingAcceleration(homingAcceleration)
, mPlayerIndex(playerIndex)
, mModeFlags(modeFlags)
, mFollowPlayerArea((modeFlags & kMF_FollowPlayerArea) != 0)
, mHasRenderBounds(false) {
  mParticleGen->SetOrientation(GetTransform().GetRotation());
  mParticleGen->SetTranslation(GetTranslation());
  mParticleGen->SetGeneratorRate(generatorRate);
}

CHomingBlob::~CHomingBlob() {}

void CHomingBlob::PreRenderAllViewports(CStateManager& mgr) {
  rstl::optional_object< CAABox > bounds = mParticleGen->GetBounds();
  if (bounds) {
    mHasRenderBounds = true;
    SetOtherBounds(*bounds);
    SetRenderBounds(*bounds);
  } else {
    mHasRenderBounds = false;
    const CVector3f pos = GetTranslation();
    const CAABox pointBounds(pos, pos);
    SetOtherBounds(pointBounds);
    SetRenderBounds(pointBounds);
  }
  UpdatePortalSystemState(mgr);
}

void CHomingBlob::PreRender(CStateManager& mgr) {
  SetPreRenderClipped(!mHasRenderBounds || !mgr.fn_800366e4(this));
  if (!GetPreRenderClipped() && mPlayerIndex == mgr.GetCurrentRenderPlayerIndex()) {
    SetPreRenderClipped(true);
  }
}

void CHomingBlob::AddToRenderer(const CStateManager& mgr) const {
  if (!GetPreRenderClipped()) {
    const CAABox& bounds = GetRenderBoundsCached();
    EnsureRendered(mgr, bounds.GetCenterPoint(), bounds);
  }
}

void CHomingBlob::Render(const CStateManager& mgr) const {
  const CTransform4f& view = CGraphics::GetViewMatrix();
  const CLight light = CLight::BuildCustom(view.GetTranslation(), view.GetForward(),
                                           CColor(sLightRed, sLightGreen, sLightBlue, 1.f),
                                           sDistanceConstant, sDistanceLinear, sDistanceQuadratic,
                                           sAngleConstant, sAngleLinear, sAngleQuadratic);
  CGraphics::SetAmbientColor(CColor(sAmbientRed, sAmbientGreen, sAmbientBlue, 1.f));
  CGraphics::SetLightState(1);
  CGraphics::LoadLight(kLight0, light);
  mParticleGen->SetLeaveLightsEnabledForModelRender(true);
  mParticleGen->Render();
  CGraphics::SetLightState(0);
}

void CHomingBlob::Think(float dt, CStateManager& mgr) {
  mElapsedTime += dt;
  // TODO: update the packed cache against the native near-list adapter.
  mParticleGen->Update(dt);
  UpdateParticles(mgr);

  if (mLightId != kInvalidUniqueId) {
    CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId));
    if (light && GetActive()) {
      light->SetLight(mParticleGen->GetLight());
    }
  }
  if (mFollowPlayerArea) {
    mgr.SetActorAreaId(*this, mgr.GetPlayer(0)->GetCurrentAreaId());
  }
  if (mParticleGen->IsSystemDeletable()) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

bool CHomingBlob::FindNearestTriangle(float radius, const CVector3f& position, CVector3f& closest,
                                      const CCachedCollisionSurface*& surface,
                                      CVector3f& barycentric) {
  // TODO: nonfunctional scaffold until the native plane-refreshing cache iterator is exposed.
  return false;
}

// Guessed TU-local identity; native calls pass two floats and no blob instance.
static CVector3f TangentVelocity(float normalDot, float speed, const CVector3f& velocity,
                                 const CVector3f& normal, float& resultSpeed) {
  const CVector3f tangent = velocity - normal * normalDot;
  const float magnitude = tangent.Magnitude();
  if (magnitude < 0.00011920929f) {
    resultSpeed = 0.f;
    return CVector3f::Zero();
  }
  resultSpeed = speed;
  return tangent * (speed / magnitude);
}

void CHomingBlob::UpdateParticles(CStateManager& mgr) {
  // TODO: nonfunctional scaffold for phased steering, cached-triangle collision and damage.
  // The existing NextTriangle does not perform the target's required plane refresh.
}

void CHomingBlob::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  const TUniqueId sender = msg.GetSenderId();

  switch (message) {
  case kSM_Create:
    if (mParticleGen->SystemHasLight()) {
      mLightId = mgr.AllocateUniqueId();
      mgr.AddObject(rs_new CGameLight(
          mLightId, GetCurrentAreaId(), GetActive(), rstl::string_l("HomingBlobLight"),
          GetTransform(), GetUniqueId(), mParticleGen->GetLight(), mParticleAssetId, 1, 0.f));
    }
    // TODO: initialize the packed world/dynamic cache through its native adapter.
    if (!(mModeFlags & kMF_SkipInitialTargets)) {
      // TODO: distance-sorted near-list filtering and the distinct type-102 callback.
    }
    break;
  case kSM_Delete:
    if (mLightId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mLightId);
      mLightId = kInvalidUniqueId;
    }
    break;
  default:
    break;
  }

  CActor::AcceptScriptMsg(mgr, msg);
  if (mLightId != kInvalidUniqueId) {
    mgr.SendScriptMsg(mLightId, sender, message, kInvalidUniqueId);
  }
}

rstl::optional_object< CAABox > CHomingBlob::GetTouchBounds() const {
  return rstl::optional_object_null();
}

void CHomingBlob::Touch(CActor& actor, CStateManager& mgr) {}
