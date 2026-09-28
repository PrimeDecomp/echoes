#ifndef _CPERSISTENTOPTIONS
#define _CPERSISTENTOPTIONS

#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CEnvironmentVariable;

class CPersistentOptions {
public:
  bool GetCinematicState(rstl::pair< CAssetId, TEditorId > cinematicId) const;
  void SetCinematicState(rstl::pair< CAssetId, TEditorId > cinematicId, bool state);
  CEnvironmentVariable* FindEnvironmentVariable(const char* name); // Guessed name
  void SetSaveIdx(int idx) { mSaveIdx = idx; }                     // Guessed name

private:
  char x0_[0x28];
  int mSaveIdx;
};

#endif // _CPERSISTENTOPTIONS
