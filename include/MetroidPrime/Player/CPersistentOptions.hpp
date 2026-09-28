#ifndef _CPERSISTENTOPTIONS
#define _CPERSISTENTOPTIONS

#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/Player/CGameStateEnvVarManager.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

// Guessed name. The system-wide options include cinematics and the selected save slot.
class CPersistentOptions : public CGameStateEnvVarManager {
public:
  CPersistentOptions();
  explicit CPersistentOptions(CBitStreamReader& in);
  void InitializeMemoryState();
  void PutTo(CBitStreamWriter& out) const;
  bool GetCinematicState(rstl::pair< CAssetId, TEditorId > cinematicId) const;
  void SetCinematicState(rstl::pair< CAssetId, TEditorId > cinematicId, bool state);
  void SetSaveIdx(int idx) { mSaveIdx = idx; } // Guessed name
  int GetSaveIdx() const { return mSaveIdx; }

private:
  rstl::vector< rstl::pair< CAssetId, TEditorId > > mCinematicStates;
  int mSaveIdx;
};
CHECK_SIZEOF(CPersistentOptions, 0x2c)

#endif // _CPERSISTENTOPTIONS
