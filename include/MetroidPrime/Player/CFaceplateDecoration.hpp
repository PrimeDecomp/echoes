#ifndef _CFACEPLATEDECORATION
#define _CFACEPLATEDECORATION

#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"

class CTexture;
class CStateManager;

class CFaceplateDecoration {
public:
  CFaceplateDecoration(const CStateManager& mgr, int playerIndex);
  void Update(const CStateManager& mgr);
  void Draw(const CStateManager& mgr) const;

private:
  int mPlayerIndex;
  CAssetId mTextureId;
  rstl::optional_object< TToken< CTexture > > mTexture;
};
CHECK_SIZEOF(CFaceplateDecoration, 0x14)

#endif // _CFACEPLATEDECORATION
