#ifndef _CMETROIDALPHA
#define _CMETROIDALPHA

class CStateManager;

// Pointer-only interface to the REL-backed enemy; its layout is not yet recovered.
class CMetroidAlpha {
public:
  // Guessed name. The DOL wrapper forwards this notification to the loaded module.
  void OnDockTouch(CStateManager& mgr);
};

#endif // _CMETROIDALPHA
