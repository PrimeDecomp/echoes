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
class CWorldSaveGameInfo;

// Name from CWorldState; interface adapted from Prime's CScriptLayerManager.
// GetLayerName, GetLayerCount and the two name lists are named by the Corruption prototype's
// (G2MEAB) range-check messages ("Invalid AreaId %d passed to GetLayerName", "Invalid index into
// mpLayerNameStartIndexList").
class CWorldLayerState {
public:
  CWorldLayerState();
  CWorldLayerState(CBitStreamReader& in, const CWorldSaveGameInfo& saveWorld);

  void PutTo(CBitStreamWriter& out, const CWorldSaveGameInfo& saveWorld) const;
  void SetLayerActive(TAreaId area, TLayerId layer, bool active);
  bool IsLayerActive(TAreaId area, TLayerId layer) const;
  const rstl::string& GetLayerName(TAreaId area, TLayerId layer) const;
  void InitializeWorldLayers(const rstl::vector< CWorldLayers::Area >& areas,
                             const rstl::rc_ptr< rstl::vector< rstl::string > >& names,
                             const rstl::rc_ptr< rstl::vector< int > >& indices);
  int GetLayerCount(TAreaId area) const;
  const rstl::vector< CWorldLayers::Area >& GetAreaLayers() const;
  const rstl::rc_ptr< rstl::vector< rstl::string > >& GetLayerNames() const; // Guessed name
  const rstl::rc_ptr< rstl::vector< int > >& GetLayerNameOffsets() const;    // Guessed name

private:
  rstl::vector< CWorldLayers::Area > mAreaLayers;
  rstl::bit_vector< rstl::rmemory_allocator > mSaveLayers;
  rstl::rc_ptr< rstl::vector< rstl::string > > mpLayerNameList;
  rstl::rc_ptr< rstl::vector< int > > mpLayerNameStartIndexList;
};
CHECK_SIZEOF(CWorldLayerState, 0x34)

#endif // _CWORLDLAYERSTATE
