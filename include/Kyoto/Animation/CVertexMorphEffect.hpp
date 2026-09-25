#ifndef _CVERTEXMORPHEFFECT
#define _CVERTEXMORPHEFFECT

#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/vector.hpp"

class CRandom16;

class CVertexMorphEffect {
public:
  ~CVertexMorphEffect() {}

private:
  CUnitVector3f mDir;
  CVector3f mPos;
  float mDuration;
  float mElapsed;
  float mDiagExtent;
  CRandom16& mRandom;
  rstl::vector< uint > mIndices;
  rstl::vector< float > mFloats;
};

#endif // _CVERTEXMORPHEFFECT
