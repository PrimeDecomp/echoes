#ifndef _CGEOMBLOBEFFECT
#define _CGEOMBLOBEFFECT

#include "types.h"

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CEffect.hpp"

class CGenDescription;

// Guessed class: the blob effect created by the GeomBlobV2 REL (module 25). Only the constructor
// and one scalar setter are used from other RELs; the layout beyond CEffect is not reconstructed.
class CGeomBlobEffect : public CEffect {
public:
  CGeomBlobEffect(const TToken< CGenDescription >& desc, TUniqueId uid, TAreaId area, bool active,
                  const rstl::string& name, const CTransform4f& xf, TUniqueId owner, float f,
                  int g);

  // CEntity
  ~CGeomBlobEffect() override;

  // CGeomBlobEffect
  void SetBlobIntensity(float intensity); // Guessed name

private:
  uchar mUnported[0x1e0 - 0x158]; // Guessed layout; defined by the GeomBlobV2 REL.
};
CHECK_SIZEOF(CGeomBlobEffect, 0x1e0)

#endif // _CGEOMBLOBEFFECT
