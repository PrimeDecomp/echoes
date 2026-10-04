#ifndef _CSPLITTERMAINCHASSIS
#define _CSPLITTERMAINCHASSIS

// Pointer-only interface to the REL-backed enemy; its layout is not yet recovered.
class CSplitterMainChassis {
public:
  // The Wii export corroborates this interface, not the DOL facade's original spelling.
  void AutoDestruct(float time);
};

#endif // _CSPLITTERMAINCHASSIS
