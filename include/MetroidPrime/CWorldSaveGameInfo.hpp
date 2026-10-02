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
    explicit SLayerState(CInputStream& in);

    int mArea;
    uint mLayer;
  };

  // Guessed name. SAVW records shared by the system and game environment-variable lists.
  struct SEnvironmentVariable {
    explicit SEnvironmentVariable(CInputStream& in);

    bool operator==(const SEnvironmentVariable& other) const;

    rstl::string mName;
    int mMinimum;
    int mMaximum;
    int mDefaultValue;
  };

  typedef rstl::pair< CAssetId, uint > ScanState;

  explicit CWorldSaveGameInfo(CInputStream& in);

  uint GetAreaCount() const { return mAreaCount; }
  int GetCinematicCount() const { return mCinematics.size(); }
  const rstl::vector< TEditorId >& GetCinematics() const { return mCinematics; }
  const rstl::vector< TEditorId >& GetRelays() const { return mRelays; }
  uint CalculateHash() const; // Guessed name
  int GetRelayIndex(const TEditorId& id) const;
  const rstl::vector< TEditorId >& GetDoors() const { return mDoors; }
  const rstl::vector< TEditorId >& GetUnmappableObjects() const { return mUnmappableObjects; }
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
