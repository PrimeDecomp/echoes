#ifndef _SLDRSCANTREEINVENTORY
#define _SLDRSCANTREEINVENTORY

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrInventorySlot.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScannableParameters.hpp"
#include "rstl/string.hpp"

struct SLdrScanTreeInventory {
    SLdrScanTreeInventory();
    ~SLdrScanTreeInventory();

    SLdrEditorProperties editorProperties;
    CAssetId nameStringTable;
    rstl::string nameStringName;
    SLdrInventorySlot inventorySlot;
    SLdrScannableParameters scannableParameters;
};

void LoadTypedefSLdrScanTreeInventory(SLdrScanTreeInventory&, CInputStream&);

#endif // _SLDRSCANTREEINVENTORY
