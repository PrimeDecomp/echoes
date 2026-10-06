#include "MetroidPrime/ScriptObjects/CScriptRepulsor.hpp"

#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRepulsor.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"

CScriptRepulsor::CScriptRepulsor(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                                 const CTransform4f& transform, float radius, float strength,
                                 EShape shape, uint flags)
: CActor(uid, name, info, 0, transform, CModelData::CModelDataNull(), CMaterialList(kMT_Pillar),
         CActorParameters::None(), kInvalidUniqueId)
, mRadius(radius)
, mStrength(strength)
, mShape(shape)
, mFlags(flags) {
  SetCallTouch(false);
}

rstl::optional_object< CAABox > CScriptRepulsor::GetTouchBounds() const {
  return CAABox(GetTranslation(), GetTranslation());
}

CScriptRepulsor::~CScriptRepulsor() {}

float CScriptRepulsor::GetRadius() const { return mRadius; }

float CScriptRepulsor::GetStrength() const { return mStrength; }

CScriptRepulsor::EShape CScriptRepulsor::GetShape() const { return mShape; }

CEntity* LoadRepulsor(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrRepulsor sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrRepulsor.inc"

  CScriptRepulsor::EShape shape = CScriptRepulsor::kS_Invalid;
  switch (sldrThis.shape) {
  case 0:
    shape = CScriptRepulsor::kS_Sphere;
    break;
  case 1:
    shape = CScriptRepulsor::kS_Cylinder;
    break;
  }

  return rs_new CScriptRepulsor(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                LdrToEntityInfo(info, sldrThis.editorProperties),
                                LdrToTransform4f(sldrThis.editorProperties), sldrThis.radius,
                                sldrThis.value, shape, sldrThis.flagsRepulsor);
}
