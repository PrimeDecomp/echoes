#include "GuiSys/CGuiFrame.hpp"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CVParamTransfer.hpp"

CFactoryFnReturn RGuiFrameFactoryInGame(const SObjectTag&, CInputStream& in,
                                        const CVParamTransfer& params) {
  const CVParamTransfer paramCopy(params);
  CSimplePool* pool = static_cast< const TObjOwnerParam< CSimplePool* >& >(*paramCopy).GetData();
  return rs_new CGuiFrame(in, pool);
}
