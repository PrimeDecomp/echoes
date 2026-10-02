#include "MetroidPrime/CEffect.hpp"

CEffect::CEffect(TUniqueId uid, const CEntityInfo& info, const rstl::string& name,
                 const CTransform4f& xf)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None().WithAlphaSorting(true), kInvalidUniqueId) {}

void CEffect::Render(const CStateManager&) const {}

void CEffect::AddToRenderer(const CStateManager&) const {}
