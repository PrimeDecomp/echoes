#ifndef _CMEMORYCARD
#define _CMEMORYCARD

#include "types.h"

#include "Kyoto/CObjectReference.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/vector.hpp"

class CMemoryCard {
public:
  struct ScanState {};

  ~CMemoryCard();
  // TODO

  // rstl::pair< CAssetId, TAreaId > GetAreaAndWorldIdForSaveId(int saveId) const;

  // typedef rstl::pair< CAssetId, CWorldSaveGameInfo::EScanCategory > ScanState;
  const rstl::vector< ScanState >& GetScanStates() const { return scanStates; }

private:
  uchar x0_pad[0x20];
  rstl::vector< ScanState > scanStates;
};

extern CMemoryCard* gpMemoryCard;

#endif // _CMEMORYCARD
