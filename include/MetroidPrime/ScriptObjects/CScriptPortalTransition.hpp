#ifndef _CSCRIPTPORTALTRANSITION
#define _CSCRIPTPORTALTRANSITION

#include "rstl/single_ptr.hpp"

class CPortalTransition;
class CStateManager;

// Pointer-only interface; recover the full class before constructing or embedding it.
class CScriptPortalTransition {
public:
  // Guessed name.
  rstl::single_ptr< CPortalTransition > CreateTransition(CStateManager& mgr) const;
};

#endif // _CSCRIPTPORTALTRANSITION
