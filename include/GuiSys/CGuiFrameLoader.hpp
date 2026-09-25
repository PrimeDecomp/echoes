#ifndef _CGUIFRAMELOADER
#define _CGUIFRAMELOADER
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/single_ptr.hpp"
class CGuiFrame;
class CSimplePool;
class CResFactory;
// Guessed name
class CGuiFrameLoader {
public:
  CGuiFrameLoader(CAssetId asset, CResFactory& factory, CSimplePool& pool);
  ~CGuiFrameLoader();
  CGuiFrame* CreateFrame();

private:
  SObjectTag mTag;
  CSimplePool* mPool;
  uint mBufferLength;
  rstl::auto_ptr< uchar > mBuffer;
  rstl::single_ptr< CDvdRequest > mRequest;
};
CHECK_SIZEOF(CGuiFrameLoader, 0x1c)
#endif // _CGUIFRAMELOADER
