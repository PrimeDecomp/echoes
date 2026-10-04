#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAIHint.hpp"

CScriptAIHint::CScriptAIHint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, EHintType hintType, float radius,
                           float valueParm, float valueParm2, float valueParm3)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
, mHintType(hintType)
, mRadius(radius)
, mValueParm(valueParm)
, mValueParm2(valueParm2)
, mValueParm3(valueParm3)
, mInUse(false)
, mOccupant(kInvalidUniqueId)
, mTimeRemaining(0.f) {}

void CScriptAIHint::AddToRenderer(const CStateManager& mgr) const {}

void CScriptAIHint::PreRender(CStateManager& mgr) {}

void CScriptAIHint::Render(const CStateManager& mgr) const {}

void CScriptAIHint::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CActor::AcceptScriptMsg(mgr, msg);
}

void CScriptAIHint::Think(float dt, CStateManager& mgr) {
  mTimeRemaining -= dt;
  if (mTimeRemaining < 0.f) {
    mTimeRemaining = 0.f;
  }
}

bool CScriptAIHint::GetInUse(TUniqueId uid) const {
  return (mOccupant != kInvalidUniqueId && uid != kInvalidUniqueId && uid != mOccupant) ||
         mInUse || mTimeRemaining > 0.f;
}

bool CScriptAIHint::GetInUseIgnoreLock(TUniqueId uid) const {
  return (mOccupant != kInvalidUniqueId && uid != kInvalidUniqueId && uid != mOccupant) ||
         mInUse;
}

void CScriptAIHint::SetInUse(bool inUse) {
  mInUse = inUse;
  if (!mInUse) {
    mTimeRemaining = 2.f;
  }
}

float CScriptAIHint::GetValueParm() const { return mValueParm; }

float CScriptAIHint::GetValueParm2() const { return mValueParm2; }

CEntity* LoadAIHint(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrAIHint sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrAIHint.inc"
  return rs_new CScriptAIHint(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      static_cast< CScriptAIHint::EHintType >(sldrThis.hintType), sldrThis.radius,
      sldrThis.valueParm, sldrThis.valueParm2, sldrThis.valueParm3);
}

CScriptAIHint::~CScriptAIHint() {}
