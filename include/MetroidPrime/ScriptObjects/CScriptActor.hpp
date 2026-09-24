#ifndef _CSCRIPTACTOR
#define _CSCRIPTACTOR

#include "MetroidPrime/CPhysicsActor.hpp"

class CScriptActor : public CPhysicsActor {
public:
  CEntity* TypesMatch(int typeId) const override;
  bool CheckActorRenderOnly() const;
  // TODO
};

#endif // _CSCRIPTACTOR
