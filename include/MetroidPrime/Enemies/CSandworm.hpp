#ifndef _CSANDWORM
#define _CSANDWORM

#include "Kyoto/Math/CVector3f.hpp"

class CSandworm;
class CSandwormEye;

// Guessed names. DOL queries forward through the Sandworm REL's registered callbacks;
// they do not require the concrete enemy layout in the DOL.
int GetSandwormRadarPointCount(const CSandworm* sandworm);
CVector3f GetSandwormRadarPointPosition(const CSandworm* sandworm, int index);

#endif // _CSANDWORM
