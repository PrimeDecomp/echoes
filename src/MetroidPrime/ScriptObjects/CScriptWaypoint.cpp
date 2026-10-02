#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"

// Guessed name
class CValidWaypointPredicate : public CValidEntityPredicate {
public:
  // CValidEntityPredicate
  ~CValidWaypointPredicate() override {}
  bool IsValid(const CStateManager& mgr, TUniqueId id) const override;
};

bool CValidWaypointPredicate::IsValid(const CStateManager& mgr, TUniqueId id) const {
  return TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id)) != nullptr;
}

CScriptWaypoint::CScriptWaypoint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                                 const CTransform4f& xf)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId) {
  SetUseInSortedLists(false);
  SetCallTouch(false);
}

CScriptWaypoint::~CScriptWaypoint() {}

void CScriptWaypoint::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);

  if (GetActive()) {
    switch (message) {
    case static_cast< EScriptObjectMessage >('ARRV'):
      SendScriptMsgs(kSS_Arrived, mgr, msg.GetOriginator(), kSM_None);
      break;
    default:
      break;
    }
  }
}

TUniqueId CScriptWaypoint::NextWaypoint(CStateManager& mgr) const {
  rstl::vector< TUniqueId > ids =
      FindConnectedObjects_if(mgr, kSS_Arrived, kSM_Next, CValidWaypointPredicate());
  if (ids.empty()) {
    return kInvalidUniqueId;
  }
  return ids[int(mgr.Random()->Float() * ids.size() * 0.99f)];
}

TUniqueId CScriptWaypoint::FollowWaypoint(CStateManager& mgr) const {
  return CheckConnectedObject(mgr, kSS_Arrived, kSM_Follow);
}

void CScriptWaypoint::AddToRenderer(const CStateManager& mgr) const {}

void CScriptWaypoint::Render(const CStateManager& mgr) const {}

CTransform4f LoadEditorTransform(const SLdrEditorProperties&);

CEntity* LoadWaypoint(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  SLdrWaypoint sldrThis;

  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    const int propertyId = input.Get< int >();
    const u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x255a4580:
      LoadTypedefSLdrEditorProperties(sldrThis.editorProperties, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }

  return rs_new CScriptWaypoint(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                LdrToEntityInfo(info, sldrThis.editorProperties),
                                LoadEditorTransform(sldrThis.editorProperties));
}
