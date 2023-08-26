#ifndef _CWORLD
#define _CWORLD

#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/vector.hpp"


class CGameArea;

class CWorld {
public:
  const CGameArea& GetAreaAlways(TAreaId id) const { return *m_areas[id.Value()]; }
  CGameArea* Area(TAreaId id) { return m_areas[id.Value()].get(); }
  const CGameArea* GetArea(TAreaId id) const { return m_areas[id.Value()].get(); }

  void SetLoadPauseState(bool);

private:
  char pad1[0x18];
  rstl::vector< rstl::auto_ptr< CGameArea > > m_areas; // x18
};

#endif // _CWORLD
