#ifndef _CSANDWORM
#define _CSANDWORM

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CActor.hpp"

// Original class names from the Wii SEL exports. Both live in the Sandworm REL; the DOL only
// proves the CActor base through the entity type casts, so the layouts are left opaque.
class CSandworm : public CActor {};
class CSandwormEye : public CActor {};

// Guessed names. DOL queries forward through the Sandworm REL's registered callbacks;
// they do not require the concrete enemy layout in the DOL.
int GetSandwormRadarPointCount(const CSandworm* sandworm);
CVector3f GetSandwormRadarPointPosition(const CSandworm* sandworm, int index);

#endif // _CSANDWORM
