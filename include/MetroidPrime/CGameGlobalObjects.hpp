#ifndef _CGAMEGLOBALOBJECTS
#define _CGAMEGLOBALOBJECTS

#include "types.h"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

#include "Kyoto/Alloc/CMemorySys.hpp"
#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/CMemoryCardSys.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/TOneStatic.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#include "MetroidPrime/CRELFileManager.hpp"
#include "MetroidPrime/Factories/CCharacterFactoryBuilder.hpp"

class IRenderer;
class CStringTable;
class CGameState;
class CMemoryCard;
class CInGameTweakManager;

class CGameGlobalObjects : public TOneStatic< CGameGlobalObjects > {
public:
  CGameGlobalObjects(COsContext&, CMemorySys&);
  ~CGameGlobalObjects();

  void PostInitialize(COsContext&, CMemorySys&);
  void AddPaksAndFactories(COsContext& context);
  void LoadStringTable();

  rstl::single_ptr< CGameState >& GameState() { return mGameState; }
  rstl::single_ptr< CMemoryCard >& MemoryCard() { return mMemoryCard; }

private:
  CMemoryCardSys mMemoryCardSys;
  CResFactory mResFactory;
  CSimplePool mSimplePool;
  CCharacterFactoryBuilder mCharacterFactoryBuilder;
  rstl::single_ptr< CGameState > mGameState;
  rstl::single_ptr< CMemoryCard > mMemoryCard;
  rstl::optional_object< TLockedToken< CStringTable > > mStringTable;
  rstl::single_ptr< IRenderer > mRenderer;
  rstl::single_ptr< CInGameTweakManager > mInGameTweakManager;
  CRELFileManager mRelFileManager;
};
CHECK_SIZEOF(CGameGlobalObjects, 0x164)

class IController;

extern const TToken< CRasterFont >* gpDefaultFont;
extern IController* gpController;
extern bool sProgressiveModePrompt; // Prime name; cleared once the splash prompt is answered.

#endif // _CGAMEGLOBALOBJECTS
