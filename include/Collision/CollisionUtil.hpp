#ifndef _COLLISIONUTIL
#define _COLLISIONUTIL

class CAABox;
class CMaterialList;
class CCollisionInfoList;
class CVector3f;
class CSphere;
class CMRay;
class CPlane;

namespace CollisionUtil {

void AddAverageToFront(const CCollisionInfoList& in, CCollisionInfoList& out);
void FilterOutBackfaces(const CVector3f& relVel, const CCollisionInfoList& in,
                        CCollisionInfoList& out);
void FilterByClosestNormal(const CVector3f& norm, const CCollisionInfoList& in,
                           CCollisionInfoList& out);
bool TriBoxOverlap(const CVector3f& boxcenter, const CVector3f& boxhalfsize,
                   const CVector3f& trivert0, const CVector3f& trivert1, const CVector3f& trivert2);
bool BoxLineTest(const CAABox& box, const CVector3f& point, const CVector3f& direction, float& tMin,
                 float& tMax, int& axis, bool& sign);
bool LineCircleIntersection2d(const CVector3f& point, const CVector3f& direction,
                              const CSphere& sphere, int axis1, int axis2, float& distance);
bool TriSphereOverlap(const CSphere& sphere, const CVector3f& a, const CVector3f& b,
                      const CVector3f& c);
bool TriSphereIntersection(const CSphere& sphere, const CVector3f& a, const CVector3f& b,
                           const CVector3f& c, CVector3f& point, CVector3f& normal);
double TriPointSqrDist(const CVector3f& point, const CVector3f& a, const CVector3f& b,
                       const CVector3f& c, float* baryX, float* baryY);
// Guessed name for the existing separate single-precision implementation.
float TriPointSqrDist_Float(const CVector3f& point, const CVector3f& a, const CVector3f& b,
                           const CVector3f& c, float* baryX, float* baryY);

bool AABoxAABoxIntersection(const CAABox& left, const CAABox& right);
bool AABoxAABoxIntersection(const CAABox& left, const CMaterialList& leftFilter,
                            const CAABox& right, const CMaterialList& rightFilter,
                            CCollisionInfoList& list);
bool AABoxSphereIntersection(const CAABox& box, const CSphere& sphere);
float AABoxSphereIntersectionRadius(const CAABox& box, const CSphere& sphere);
bool RayTriangleIntersection(const CVector3f& point, const CVector3f& direction,
                             const CVector3f* vertices, float& distance);
bool RayTriangleIntersection_Double(const CVector3f& point, const CVector3f& direction,
                                    const CVector3f* vertices, double& distance);
int RayAABoxIntersection(const CMRay& ray, const CAABox& box, float& tMin, float& tMax);
int RayAABoxIntersection_Double(const CMRay& ray, const CAABox& box, CVector3f& normal,
                                double& penetration);
int RayAABoxIntersection(const CMRay& ray, const CAABox& box, CVector3f& normal,
                         float& penetration);
bool AABox_ABBox_Moving(const CAABox& left, const CAABox& right, const CVector3f& dir, double& d,
                        CVector3f& point, CVector3f& normal);

bool MovingSphereAABox(const CSphere& sphere, const CAABox& aabb, const CVector3f& dir,
                       double& dOut, CVector3f& point, CVector3f& normal);
bool RaySphereIntersection_Double(const CSphere& sphere, const CVector3f& pos, const CVector3f& dir,
                                  double& T);
bool RaySphereIntersection(const CSphere& sphere, const CVector3f& pos, const CVector3f& dir,
                           float mag, float& T, CVector3f& point);
bool RayPlaneIntersection(const CVector3f& from, const CVector3f& to, const CPlane& plane,
                          CVector3f& point);

// Guessed name for the additional single-precision implementation.
float TriPointSqrDist_Float(const CVector3f& point, const CVector3f& a, const CVector3f& b,
                            const CVector3f& c, float* baryX, float* baryY);
// AABoxPointSqrDist is an original MP2 Wii export; the GC body corroborates this interface.
float AABoxPointSqrDist(const CVector3f& point, const CAABox& box, CVector3f* closestPoint);
// Guessed names/signatures for the other target-derived additions.
float AABoxPointDist(const CVector3f& point, const CAABox& box, CVector3f* closestPoint);
bool SphereAABoxIntersection(const CSphere& sphere, const CAABox& box);
int RayAABoxIntersection(const CVector3f& start, const CVector3f& direction, float length,
                         const CAABox& box);

} // namespace CollisionUtil

#endif // _COLLISIONUTIL
