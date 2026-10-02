#include "MetroidPrime/ScriptObjects/CScriptSpiderBallAttractionSurface.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSpiderBallAttractionSurface.hpp"

CScriptSpiderBallAttractionSurface::CScriptSpiderBallAttractionSurface(TUniqueId uid,
                                                                       const rstl::string& name,
                                                                       const CEntityInfo& info,
                                                                       const CTransform4f& xf,
                                                                       const CVector3f& scale)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mScale(scale)
, mAabb(CAABox(CVector3f(-(scale.GetX() * 0.5f), -(scale.GetY() * 0.5f), -(scale.GetZ() * 0.5f)),
               CVector3f(scale.GetX() * 0.5f, scale.GetY() * 0.5f, scale.GetZ() * 0.5f))
            .GetTransformedAABox(xf.GetRotation())) {}

CScriptSpiderBallAttractionSurface::~CScriptSpiderBallAttractionSurface() {}

void CScriptSpiderBallAttractionSurface::Touch(CActor&, CStateManager&) {}

rstl::optional_object< CAABox > CScriptSpiderBallAttractionSurface::GetTouchBounds() const {
  if (GetActive()) {
    return rstl::optional_object< CAABox >(
        CAABox(mAabb.GetMinPoint() + GetTranslation(), mAabb.GetMaxPoint() + GetTranslation()));
  }
  return rstl::optional_object_null();
}

void CScriptSpiderBallAttractionSurface::Think(float, CStateManager&) {}

CEntity* LoadSpiderBallAttractionSurface(CStateManager& mgr, CInputStream& input,
                                         CEntityInfo& info) {
  SLdrSpiderBallAttractionSurface sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSpiderBallAttractionSurface.inc"
  return rs_new CScriptSpiderBallAttractionSurface(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      sldrThis.editorProperties.transform.scale);
}
