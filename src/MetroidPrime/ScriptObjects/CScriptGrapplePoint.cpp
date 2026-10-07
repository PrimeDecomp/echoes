#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"

CScriptGrapplePoint::CScriptGrapplePoint(TUniqueId uid, const rstl::string& name,
                                         const CEntityInfo& info, const CTransform4f& xf,
                                         const CGrappleParameters& parameters)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(kMT_Orbit),
         CActorParameters::None(), kInvalidUniqueId)
, mParameters(parameters)
, mPreviousPosition(xf.GetTranslation())
, mActivationFrame(0) {}

CScriptGrapplePoint::~CScriptGrapplePoint() {}

void CScriptGrapplePoint::AddToRenderer(const CStateManager&) const {}

rstl::optional_object< CAABox > CScriptGrapplePoint::GetTouchBounds() const {
  const CVector3f extent(0.5f, 0.5f, 0.5f);
  return CAABox(GetTranslation() - extent, GetTranslation() + extent);
}

void CScriptGrapplePoint::Render(const CStateManager&) const {}

void CScriptGrapplePoint::Think(float, CStateManager& mgr) {
  if (GetActive() && !close_enough(GetTranslation(), mPreviousPosition, 0.0001f)) {
    if (mgr.GetUpdateFrameIdx() - mActivationFrame > 1 &&
        mgr.GetCameraManager(0)->IsInCinematicCamera()) {
      // Native evaluates this cinematic-camera check but discards the result.
    }
    mPreviousPosition = GetTranslation();
  }
}

void CScriptGrapplePoint::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  switch (message) {
  case kSM_AreaLoaded:
    for (uint player = 0; player < mgr.GetNumPlayers(); ++player) {
      SetValidTarget(player, true);
    }
    mActivationFrame = mgr.GetUpdateFrameIdx();
    break;
  case kSM_Increment:
  case kSM_Decrement:
    if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(msg.GetOriginator()))) {
      SetValidTarget(mgr.MaskUIdNumPlayers(msg.GetOriginator()), message == kSM_Increment);
    }
    break;
  case kSM_Activate:
    if (!GetActive()) {
      AddMaterial(kMT_Orbit, mgr);
      SetActive(true);
      mPreviousPosition = GetTranslation();
      mActivationFrame = mgr.GetUpdateFrameIdx();
    }
    break;
  case kSM_Deactivate:
    if (GetActive()) {
      RemoveMaterial(kMT_Orbit, mgr);
      SetActive(false);
    }
    break;
  default:
    break;
  }
}
