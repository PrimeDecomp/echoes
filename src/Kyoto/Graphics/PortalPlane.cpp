#include "Kyoto/Graphics/PortalPlane.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

namespace {
CTransform4f sTextureTransform = CTransform4f::Identity();
CPlane sWorldSpacePlane(0.f, CVector3f::Forward());
bool sHasPlane = false;
} // namespace

const CTransform4f& PortalPlane::GetTextureTransform() { return sTextureTransform; }

void PortalPlane::SetCurrentPlane(const CPlane& plane) {
  SetWorldSpacePlane(plane, CGraphics::GetGXModelView(), CGraphics::GetModelMatrix());
}

void PortalPlane::SetWorldSpacePlane(const CPlane& plane, const CTransform4f& modelView,
                                     const CTransform4f& model) {
  sWorldSpacePlane = plane;
  sHasPlane = true;

  const CTransform4f inverseModelView = modelView.GetInverse();
  const CTransform4f inverseModel = model.GetInverse();
  const CUnitVector3f localNormal(inverseModel.Rotate(plane.GetNormal()));
  const CVector3f localPoint = inverseModel * (plane.GetConstant() * plane.GetNormal());
  const float localDistance = CVector3f::Dot(localPoint, localNormal);

  const CTransform4f projection =
      CTransform4f::FromRows(CVector3f::Zero(), localNormal, CVector3f::Zero(),
                             CVector3f(0.5f, 0.5f - localDistance, 1.f));
  sTextureTransform = projection * inverseModelView;
}
