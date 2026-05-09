#ifndef _CSCANTREEINVENTORY
#define _CSCANTREEINVENTORY

#include "rstl/string.hpp"
#include "Kyoto/SObjectTag.hpp"

#include "MetroidPrime/Player/CPlayerState.hpp"

class CInputStream;
class SLdrTransform;

class CScanTreeInventory {
public:
CScanTreeInventory(int id, const SLdrTransform& transform, CAssetId nameStringTable, CAssetId scannableInfo,
                     CPlayerState::EItemType inventorySlotId, const rstl::string& nameStringName);
};

CScanTreeInventory* LoadScanTreeInventory(int* id, CInputStream& input);

#endif // _CSCANTREEINVENTORY
