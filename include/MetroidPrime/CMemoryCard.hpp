#ifndef _CMEMORYCARD
#define _CMEMORYCARD

#include "types.h"

#include "Kyoto/CObjectReference.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/vector.hpp"
#include "rstl/pair.hpp"

class CMemoryCard {
public:
  typedef rstl::pair< CAssetId, uint > ScanState;

  ~CMemoryCard();
  // TODO

  // rstl::pair< CAssetId, TAreaId > GetAreaAndWorldIdForSaveId(int saveId) const;

  // typedef rstl::pair< CAssetId, CWorldSaveGameInfo::EScanCategory > ScanState;
  const rstl::vector< ScanState >& GetScanStates() const { return scanStates; }

private:
  uchar mPad[0x20];
  rstl::vector< ScanState > scanStates;
};

extern CMemoryCard* gpMemoryCard;

#endif // _CMEMORYCARD
