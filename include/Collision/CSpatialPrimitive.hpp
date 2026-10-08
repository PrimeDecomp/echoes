#ifndef _CSPATIALPRIMITIVE
#define _CSPATIALPRIMITIVE

#include "Collision/CMaterialList.hpp"
#include "Collision/COBBox.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CSphere.hpp"

#include "rstl/vector.hpp"

// Guessed name: the CSPP resource, retained from the existing annotation.
class CSpatialPrimitive {
public:
  // Guessed record names: geometry associated with two serialized segment IDs.
  struct SSphere {
    explicit SSphere(CInputStream& in)
    : mFirstSegment(in), mSecondSegment(in), x8_(in), mSphere(in) {}

    CSegId mFirstSegment;
    CSegId mSecondSegment;
    // Guessed type: aligned 64-bit stream value; runtime material use is unverified.
    CMaterialList x8_;
    CSphere mSphere;
  };

  struct SBox {
    explicit SBox(CInputStream& in) : mFirstSegment(in), mSecondSegment(in), x8_(in), mBox(in) {}

    CSegId mFirstSegment;
    CSegId mSecondSegment;
    // Guessed type: the same unresolved value as in SSphere.
    CMaterialList x8_;
    COBBox mBox;
  };

  explicit CSpatialPrimitive(CInputStream& in);

  const rstl::vector< SSphere >& GetSpheres() const { return mSpheres; }
  const rstl::vector< SBox >& GetBoxes() const { return mBoxes; }

private:
  rstl::vector< SSphere > mSpheres;
  rstl::vector< SBox > mBoxes;
};

NESTED_CHECK_SIZEOF(CSpatialPrimitive, SSphere, 0x20)
NESTED_CHECK_SIZEOF(CSpatialPrimitive, SBox, 0x50)
CHECK_SIZEOF(CSpatialPrimitive, 0x20)

#endif // _CSPATIALPRIMITIVE
