#ifndef _CGUIFACTORIES
#define _CGUIFACTORIES

#include "Kyoto/SObjectTag.hpp"

class CGuiFrame;
class CGuiWidget;
class CInputStream;
class CSimplePool;

// Reconstructed function/file spelling correlated with Prime; the five-argument ABI is
// established by the Echoes widget reader and dispatcher.
CGuiWidget* FGuiWidgetFactoryInGame(FourCC type, CGuiFrame* frame, CInputStream& in,
                                  CSimplePool* pool, uint version);

#endif // _CGUIFACTORIES
