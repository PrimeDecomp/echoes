#ifndef _CLINE
#define _CLINE

#include "types.h"

#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CLine {
public:
  CLine(const CVector3f& origin, const CUnitVector3f& dir) : mOrigin(origin), mDir(dir) {}

  const CVector3f& GetRefPoint() const { return mOrigin; }
  const CUnitVector3f& GetNormal() const { return mDir; }
  // Guessed name (target-derived: projects a point onto the line).
  CVector3f GetClosestPoint(const CVector3f& point) const;

private:
  CVector3f mOrigin;
  CUnitVector3f mDir;
};

#endif // _CLINE
