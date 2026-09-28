#ifndef _CWORLDLAYERSTATE
#define _CWORLDLAYERSTATE

#include "types.h"

#include "MetroidPrime/CWorldLayers.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/bit_vector.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CBitStreamReader;
class CBitStreamWriter;

// Name from CWorldState; interface adapted from Prime's CScriptLayerManager.
class CWorldLayerState {
public:
  CWorldLayerState();
  explicit CWorldLayerState(CBitStreamReader& in);

  void PutTo(CBitStreamWriter& out) const;
  void SetLayerActive(TAreaId area, TLayerId layer, bool active);
  bool IsLayerActive(TAreaId area, TLayerId layer) const;
  const rstl::string& GetLayerName(TAreaId area, TLayerId layer) const; // Guessed name
  void InitializeWorldLayers(const rstl::vector< CWorldLayers::Area >& areas,
                             const rstl::rc_ptr< rstl::vector< rstl::string > >& names,
                             const rstl::rc_ptr< rstl::vector< int > >& indices);
  int GetAreaLayerCount(TAreaId area) const;
  const rstl::vector< CWorldLayers::Area >& GetAreaLayers() const;
  const rstl::rc_ptr< rstl::vector< rstl::string > >& GetLayerNames() const; // Guessed name
  const rstl::rc_ptr< rstl::vector< int > >& GetLayerNameOffsets() const;    // Guessed name

private:
  rstl::vector< CWorldLayers::Area > mAreaLayers;
  rstl::bit_vector< rstl::rmemory_allocator > mSaveLayers;
  rstl::rc_ptr< rstl::vector< rstl::string > > mLayerNames;
  rstl::rc_ptr< rstl::vector< int > > mLayerNameOffsets;
};
CHECK_SIZEOF(CWorldLayerState, 0x34)

#endif // _CWORLDLAYERSTATE
