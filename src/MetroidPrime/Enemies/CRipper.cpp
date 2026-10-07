#include "MetroidPrime/Enemies/CRipper.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRipper.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"PathOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CRipper::PathOver)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CRipper::Patrol)},
};

static EMaterialTypes skIncludeMaterial = kMT_Unknown59;
static EMaterialTypes skExcludeMaterial1 = kMT_NoStaticCollision;
static EMaterialTypes skExcludeMaterial2 = kMT_Unknown60;
static EMaterialTypes skExcludeMaterial3 = kMT_Platform;

CRipper::CRipper(TUniqueId uid, const rstl::string& name, EFlavorType flavor,
                 const CEntityInfo& info, const CTransform4f& xf, const CModelData& modelData,
                 const CPatternedInfo& patternedInfo, const CActorParameters& actorParams,
                 const CGrappleParameters& grappleParams)
: CPatterned(kPAI_Ripper, uid, name, flavor, info, xf, modelData, patternedInfo, kMT_Flyer, kCT_One,
             kBT_Flyer, actorParams)
, mGrappleParams(grappleParams)
, mGrapplePoint(kInvalidUniqueId)
, mMuted(false) {
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(skIncludeMaterial),
      CMaterialList(skExcludeMaterial1, skExcludeMaterial2, skExcludeMaterial3)));
  KnockBackController().EnableKnockBackPhysics(false);
  KnockBackController().SetAnimReactionRange(CKnockBackMgr::kAR_Flinch,
                                             CKnockBackMgr::kAR_KnockBack);
}

CRipper::~CRipper() {}

void CRipper::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  ProcessGrapplePoint(mgr);

  bool stopped = false;
  bool firing = false;
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    const CPlayer* player = mgr.GetPlayer(i);
    if (mGrapplePoint != kInvalidUniqueId && player->GetOrbitTargetId() == mGrapplePoint &&
        player->GetGrappleState() != CPlayer::kGS_None) {
      if (player->GetGrappleState() == CPlayer::kGS_Firing) {
        firing = true;
      } else {
        stopped = true;
      }
    }
  }

  if (stopped) {
    Stop();
    if (!mMuted) {
      SetMuted(true);
      mMuted = true;
    }
  } else if (firing) {
    CPatterned::Think(dt, mgr);
  } else {
    CPatterned::Think(dt, mgr);
    if (mMuted) {
      SetMuted(false);
      mMuted = false;
    }
  }
}

void CRipper::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CPatterned::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_Create:
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    AddMaterial(kMT_Immovable, mgr);
    RemoveMaterial(kMT_Unknown59, mgr);
    if (GetFlavorType() == kFT_One) {
      AddGrapplePoint(mgr);
      RemoveMaterial(kMT_Orbit, mgr);
    }
    break;
  case kSM_Delete:
    RemoveGrapplePoint(mgr);
    break;
  case kSM_Activate:
    AddGrapplePoint(mgr);
    break;
  case kSM_Deactivate:
    RemoveGrapplePoint(mgr);
    break;
  default:
    break;
  }
}

EWeaponCollisionResponseTypes CRipper::GetCollisionResponseType(const CVector3f& position,
                                                                const CVector3f& direction,
                                                                const CWeaponMode& mode,
                                                                int attributes) const {
  EWeaponCollisionResponseTypes ret = kWCR_Unknown32;
  if (!GetDamageVulnerability()->WeaponHurts(mode)) {
    ret = static_cast< EWeaponCollisionResponseTypes >(92);
  }
  return ret;
}

void CRipper::KnockBack(CStateManager& mgr, const CKnockBackInfo& info) {
  CPatterned::KnockBack(mgr, info);
  BodyController()->CommandMgr().DeliverCmd(CBCKnockBackCmd(-info.GetDirection(), pas::kS_One));
}

void CRipper::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

bool CRipper::PathOver(CStateManager& mgr, const CTriggerData& data) const { return false; }

void CRipper::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
  BodyController()->CommandMgr().SetSteeringSpeedRange(1.f, 1.f);
  CPatterned::Patrol(mgr, msg, dt);
}

void CRipper::AddGrapplePoint(CStateManager& mgr) {
  if (mGrapplePoint != kInvalidUniqueId) {
    return;
  }

  mGrapplePoint = mgr.AllocateUniqueId();
  mgr.AddObject(rs_new CScriptGrapplePoint(
      mGrapplePoint, rstl::string_l("RipperGrapplePoint"),
      CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true), GetTransform(),
      mGrappleParams));
}

void CRipper::RemoveGrapplePoint(CStateManager& mgr) {
  if (mGrapplePoint == kInvalidUniqueId) {
    return;
  }

  mgr.DeleteObjectRequest(mGrapplePoint);
  mGrapplePoint = kInvalidUniqueId;
}

void CRipper::ProcessGrapplePoint(CStateManager& mgr) {
  if (GetFlavorType() == kFT_One && mGrapplePoint != kInvalidUniqueId) {
    if (CScriptGrapplePoint* gp =
            TCastToPtr< CScriptGrapplePoint >(mgr.ObjectById(mGrapplePoint))) {
      gp->SetTransform(GetTransform());
    }
  }
}

CEntity* LoadRipper(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrRipper sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrRipper.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CRipper(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                        static_cast< CPatterned::EFlavorType >(sldrThis.flavor),
                        LdrToEntityInfo(info, sldrThis.editorProperties),
                        LdrToTransform4f(sldrThis.editorProperties), *modelData,
                        LdrToPatternedInfo(sldrThis.patterned, nullptr),
                        LdrToActorParameters(sldrThis.actorInformation),
                        LdrToGrappleParameters(sldrThis.grappleInfo));
}

static void SetFuncPtrs() {
  static SRipper_FuncPtrs funcPtrs;
  funcPtrs.mLoadRipper = &LoadRipper;
  SetSRipper_FuncPtrs(&funcPtrs);
}

void RELMain() { SetFuncPtrs(); }

void RELExit() { SetSRipper_FuncPtrs(nullptr); }
