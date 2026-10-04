#ifndef _PORTALPLANE
#define _PORTALPLANE

class CPlane;
class CTransform4f;

// Guessed namespace and function names for the portal alpha-mask projection utility.
namespace PortalPlane {
void SetWorldSpacePlane(const CPlane& plane, const CTransform4f& modelView,
                        const CTransform4f& model);
void SetCurrentPlane(const CPlane& plane);
const CTransform4f& GetTextureTransform();
} // namespace PortalPlane

#endif // _PORTALPLANE
