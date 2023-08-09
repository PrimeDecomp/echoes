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

    SLdrEditorProperties editor_properties;
    CAssetId name_string_table;
    rstl::string name_string_name;
    SLdrInventorySlot inventory_slot;
    SLdrScannableParameters scannable_parameters;
};

void LoadTypedefSLdrScanTreeInventory(SLdrScanTreeInventory&, CInputStream&);

#endif // _SLDRSCANTREEINVENTORY
