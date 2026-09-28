#ifndef _CGUIFRAMEMODELDATABASE
#define _CGUIFRAMEMODELDATABASE

#include "Kyoto/TToken.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CCubeModel;
class CInputStream;
class CSimplePool;
class CTexture;

// Guessed name: owns the frame's embedded model data, not animation tracks.
class CGuiFrameModelDatabase {
public:
  CGuiFrameModelDatabase(CInputStream& in, CSimplePool* pool);
  ~CGuiFrameModelDatabase();

private:
  uint mBufferSize;
  rstl::single_ptr< uchar > mBuffer;
  rstl::vector< TCachedToken< CTexture > > mTextures;
  rstl::vector< rstl::auto_ptr< CCubeModel > > mModels;
  rstl::vector< rstl::vector< void* > > mSurfaces;
};
CHECK_SIZEOF(CGuiFrameModelDatabase, 0x38)

#endif // _CGUIFRAMEMODELDATABASE
