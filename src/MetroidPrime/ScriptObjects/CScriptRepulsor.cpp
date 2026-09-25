#include "MetroidPrime/ScriptObjects/CScriptRepulsor.hpp"

#include "MetroidPrime/CActorParameters.hpp"

CScriptRepulsor::CScriptRepulsor(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                                 const CTransform4f& transform, float radius, float strength,
                                 EShape shape, uint flags)
: CActor(uid, name, info, 0, transform, CModelData(), CMaterialList(kMT_Pillar), CActorParameters(),
         kInvalidUniqueId)
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
