#ifndef _CSCANTREEINVENTORY
#define _CSCANTREEINVENTORY

#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeScan.hpp"

class CInputStream;
struct SLdrTransform;

// Class, method and member names are guessed.
class CScanTreeInventory : public CScanTreeScan {
public:
  CScanTreeInventory(int id, const SLdrTransform& transform, CAssetId nameStringTable,
                     CAssetId scannableInfo, CPlayerState::EItemType inventoryItem,
                     const rstl::string& nameStringName);

  // CScanTreeNode
  ~CScanTreeInventory();
  ENodeType GetNodeType() const;

  CPlayerState::EItemType GetInventoryItem() const;

private:
  CPlayerState::EItemType mInventoryItem;
};
CHECK_SIZEOF(CScanTreeInventory, 0x6C)

// Guessed loader name.
CScanTreeInventory* LoadScanTreeInventory(int* id, CInputStream& input);

#endif // _CSCANTREEINVENTORY
