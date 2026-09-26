#ifndef _CSAVEWORLDMEMORY
#define _CSAVEWORLDMEMORY

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CWorldLayers.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CStringTable;

class CSaveWorldMemory {
  friend class CMemoryCard;

public:
  CSaveWorldMemory(CAssetId worldNameId = kInvalidAssetId,
                   CAssetId darkWorldNameId = kInvalidAssetId,
                   CAssetId saveWorldId = kInvalidAssetId)
  : mWorldNameId(worldNameId)
  , mDarkWorldNameId(darkWorldNameId)
  , mSaveWorldId(saveWorldId)
  , mAreaCount(0) {}

  int GetAreaCount() const { return mAreaCount; }
  CAssetId GetWorldNameId() const { return mWorldNameId; }
  CAssetId GetDarkWorldNameId() const { return mDarkWorldNameId; }
  CAssetId GetSaveWorldAssetId() const { return mSaveWorldId; }
  const rstl::vector< CWorldLayers::Area >& GetDefaultLayerStates() const {
    return mDefaultLayerStates;
  }
  const wchar_t* GetFrontEndName() const;
  const wchar_t* GetDarkFrontEndName() const; // Guessed name

private:
  CAssetId mWorldNameId;
  CAssetId mDarkWorldNameId;
  CAssetId mSaveWorldId;
  int mAreaCount;
  rstl::vector< uint > mAreaIds;
  rstl::vector< CWorldLayers::Area > mDefaultLayerStates;
  rstl::rc_ptr< rstl::vector< rstl::string > > mLayerNames;
  rstl::rc_ptr< rstl::vector< int > > mAreaLayerNameOffsets;
  rstl::optional_object< TCachedToken< CStringTable > > mWorldName;
  rstl::optional_object< TCachedToken< CStringTable > > mDarkWorldName;
  rstl::optional_object< CToken > mSaveWorld;
};
CHECK_SIZEOF(CSaveWorldMemory, 0x6c)

#endif // _CSAVEWORLDMEMORY
