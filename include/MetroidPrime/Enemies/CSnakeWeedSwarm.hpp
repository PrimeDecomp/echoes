#ifndef _CSNAKEWEEDSWARM
#define _CSNAKEWEEDSWARM

#include "MetroidPrime/CActor.hpp"

// Original class name from the Wii SEL exports. The class lives in the SnakeWeedSwarm REL; the DOL
// only proves the CActor base through its TypesMatch parent, so the layout is left opaque.
class CSnakeWeedSwarm : public CActor {
public:
  // CEntity
  ~CSnakeWeedSwarm() override;
  CEntity* TypesMatch(int typeId) const override;
};

#endif // _CSNAKEWEEDSWARM
