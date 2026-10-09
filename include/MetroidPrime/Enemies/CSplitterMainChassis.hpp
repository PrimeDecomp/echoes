#ifndef _CSPLITTERMAINCHASSIS
#define _CSPLITTERMAINCHASSIS

#include "REL/REL_Setup.h"
// Pointer-only interface to the REL-backed enemy; its layout is not yet recovered.
class REL_EXPORT CSplitterMainChassis {
public:
  // The Wii export corroborates this interface, not the DOL facade's original spelling.
  void AutoDestruct(float time);
};

#endif // _CSPLITTERMAINCHASSIS
