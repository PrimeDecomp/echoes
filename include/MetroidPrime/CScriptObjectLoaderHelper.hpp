#ifndef _CSCRIPTOBJECTLOADERHELPER
#define _CSCRIPTOBJECTLOADERHELPER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/vector.hpp"

class CEntity;
class CStateManager;

// Partial interface; class name corroborated by Echoes Wii exports.
class CScriptObjectLoaderHelper {
public:
  // Guessed name. Shared context for incremental loading of an area's layers.
  struct SLoadContext {
    explicit SLoadContext(TAreaId area);

    TAreaId mAreaId;
    rstl::auto_ptr< CInputStream > mStream;
    int mRemainingObjects;
    int x10_;
    int mLayerIndex;
    rstl::vector< CEntity* > mObjects;
    rstl::vector< TEditorId >* mEditorIds;
  };

  // Guessed method names.
  void LoadScriptObjects(TAreaId aid, CInputStream& in, rstl::vector< TEditorId >& ids,
                         CStateManager& mgr);
  void InitScriptObjects(rstl::vector< TEditorId >& ids, CStateManager& mgr);
  void RemoveLayerObjects(TAreaId area, TLayerId layer, CStateManager& mgr);
  void BeginLayerLoad(SLoadContext& context, rstl::auto_ptr< CInputStream > in,
                      rstl::vector< TEditorId >& ids);
  bool ContinueLayerLoad(SLoadContext& context, uint timeBudget, CStateManager& mgr);
  void LoadGeneratedScriptObjects(TAreaId area, CInputStream& in);
  void RegisterScriptObjects(rstl::vector< CEntity* > objects, CStateManager& mgr);
};
NESTED_CHECK_SIZEOF(CScriptObjectLoaderHelper, SLoadContext, 0x2c)

#endif // _CSCRIPTOBJECTLOADERHELPER
