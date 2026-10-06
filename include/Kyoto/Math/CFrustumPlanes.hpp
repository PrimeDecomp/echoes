#ifndef _CFRUSTUMPLANES
#define _CFRUSTUMPLANES

#include "types.h"

#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"

class CAABox;
class CSphere;
class CTransform4f;

class CFrustumPlanes {
public:
  CFrustumPlanes() {}
  explicit CFrustumPlanes(const rstl::reserved_vector< CPlane, 6 >& planes) : mPlanes(planes) {}
  CFrustumPlanes(const CTransform4f&, float, float, float, bool, float);

  const rstl::reserved_vector< CPlane, 6 >& GetPlanes() const { return mPlanes; }

  // Guessed name; callers must respect the six-plane capacity.
  void AddPlane(const CPlane& plane) { mPlanes.push_back(plane); }

  bool BoxInFrustumPlanes(const CAABox& box) const;
  bool BoxInFrustumPlanes(const rstl::optional_object< CAABox >& box) const;
  int BoxFrustumPlanesCheck(const CAABox& box) const;
  bool SphereInFrustumPlanes(const CSphere& sphere) const;
  bool PointInFrustumPlanes(const CVector3f& point) const;

private:
  rstl::reserved_vector< CPlane, 6 > mPlanes;
};

CHECK_SIZEOF(CFrustumPlanes, 0x64)

#endif // _CFRUSTUMPLANES
