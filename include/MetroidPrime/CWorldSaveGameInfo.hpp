#ifndef _CWORLDSAVEGAMEINFO
#define _CWORLDSAVEGAMEINFO

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CWorldSaveGameInfo {
public:
  struct SLayerState {
    TAreaId mArea;
    uint mLayer;
  };

  // Guessed name. SAVW records shared by the system and game environment-variable lists.
  struct SEnvironmentVariable {
    explicit SEnvironmentVariable(CInputStream& in)
    : mName(in)
    , mMinimum(in.ReadInt32())
    , mMaximum(in.ReadInt32())
    , mDefaultValue(in.ReadInt32()) {}

    bool operator==(const SEnvironmentVariable& other) const { return mName == other.mName; }

    rstl::string mName;
    int mMinimum;
    int mMaximum;
    int mDefaultValue;
  };

  typedef rstl::pair< CAssetId, uint > ScanState;

  explicit CWorldSaveGameInfo(CInputStream& in);

  uint GetAreaCount() const { return mAreaCount; }
  const rstl::vector< ScanState >& GetScans() const { return mScans; }
  const rstl::vector< SEnvironmentVariable >& GetSystemVariables() const {
    return mSystemVariables;
  }
  const rstl::vector< SEnvironmentVariable >& GetGameVariables() const { return mGameVariables; }

private:
  uint mAreaCount;
  rstl::vector< TEditorId > mCinematics;
  rstl::vector< TEditorId > mRelays;
  rstl::vector< SLayerState > mLayers;
  rstl::vector< TEditorId > mDoors;
  rstl::vector< TEditorId > mUnmappableObjects;
  rstl::vector< ScanState > mScans;
  rstl::vector< SEnvironmentVariable > mSystemVariables;
  rstl::vector< SEnvironmentVariable > mGameVariables;
};
CHECK_SIZEOF(CWorldSaveGameInfo, 0x84)
NESTED_CHECK_SIZEOF(CWorldSaveGameInfo, SLayerState, 0x8)
NESTED_CHECK_SIZEOF(CWorldSaveGameInfo, SEnvironmentVariable, 0x1c)

#endif // _CWORLDSAVEGAMEINFO
