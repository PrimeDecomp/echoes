#ifndef _RENDERGEOMETRYRAYCAST
#define _RENDERGEOMETRYRAYCAST

#include "Collision/CRayCastResult.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/pair.hpp"

class CCubeMaterial;
class CCubeSurface;
class CLine;
class CMaterialFilter;
class CStateManager;

// Guessed namespace and function names for the render-geometry ray queries.
namespace RenderGeometryRayCast {

CRayCastResult RayWorldIntersection(const CStateManager& mgr, const CVector3f& origin,
                                    const CVector3f& direction, float length,
                                    const CMaterialFilter& filter,
                                    rstl::pair< TAreaId, int >* modelOut);
int RaySurfaceIntersection(const CCubeSurface& surface, const CCubeMaterial& material,
                           const CVector3f* positions, const CLine& line, CRayCastResult& result,
                           float& nearest);

} // namespace RenderGeometryRayCast

#endif // _RENDERGEOMETRYRAYCAST
