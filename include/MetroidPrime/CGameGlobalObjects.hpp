#ifndef _CGAMEGLOBALOBJECTS
#define _CGAMEGLOBALOBJECTS

#include "types.h"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/CMemoryCardSys.hpp"
#include "Kyoto/Alloc/CMemorySys.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Graphics/CGraphicsSys.hpp"
#include "Kyoto/TOneStatic.hpp"
#include "Kyoto/Text/CRasterFont.hpp"

class IRenderer;
class CStringTable;
class CGameState;
class CMemoryCard;
class CInGameTweakManager;

class CGameGlobalObjects {
public:
  CGameGlobalObjects(COsContext&, CMemorySys&);

  void PostInitialize(COsContext&, CMemorySys&);
  void AddPaksAndFactories();
  void LoadStringTable();

  rstl::single_ptr< CGameState >& GameState() { return gameState; }

  static CRasterFont* LoadDefaultFont();

private:
  char pad0[4];
  CResFactory resFactory;
  CSimplePool simplePool;
  // CCharacterFactoryBuilder characterFactoryBuilder;
  rstl::single_ptr< CGameState > gameState;
  rstl::single_ptr< CMemoryCard > memoryCard;
  rstl::optional_object< TLockedToken< CStringTable > > stringTable;
  rstl::single_ptr< IRenderer > renderer;
  rstl::single_ptr< CInGameTweakManager > inGameTweakManager;
};

#endif // _CGAMEGLOBALOBJECTS
