#ifndef _CSCRIPTOBJECTLOADERHELPER
#define _CSCRIPTOBJECTLOADERHELPER

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/map.hpp"
#include "rstl/vector.hpp"

class CEntity;
class CStateManager;

// Class name corroborated by Echoes Wii exports.
class CScriptObjectLoaderHelper {
public:
  // Guessed name: serialized generated-object metadata, correlated with Prime.
  struct SScriptObjectStream {
    FourCC mType;
    uint mPosition;
    uint mLength;
  };

  // Guessed name: result returned when a Generate connection creates an object.
  struct SGeneratedObject {
    SGeneratedObject(TEditorId editorId, TUniqueId uniqueId, CEntity* entity)
    : mEditorId(editorId), mUniqueId(uniqueId), mEntity(entity) {}

    TEditorId mEditorId;
    TUniqueId mUniqueId;
    CEntity* mEntity;
  };

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

  CScriptObjectLoaderHelper();
  ~CScriptObjectLoaderHelper();

  // Guessed method names.
  rstl::pair< const SScriptObjectStream*, TEditorId > GetBuildForScript(TEditorId editorId) const;
  SGeneratedObject LoadScriptObject(TAreaId area, FourCC type, uint length, CInputStream& in,
                                    CStateManager& mgr);
  void LoadScriptObjects(TAreaId aid, CInputStream& in, rstl::vector< TEditorId >& ids,
                         CStateManager& mgr);
  void InitScriptObjects(rstl::vector< TEditorId >& ids, CStateManager& mgr);
  void RemoveLayerObjects(TAreaId area, TLayerId layer, CStateManager& mgr);
  void FreeScriptObjects(TAreaId area, CStateManager& mgr);
  void BeginLayerLoad(SLoadContext& context, rstl::auto_ptr< CInputStream > in,
                      rstl::vector< TEditorId >& ids);
  bool ContinueLayerLoad(SLoadContext& context, uint timeBudget, CStateManager& mgr);
  void LoadGeneratedScriptObjects(TAreaId area, CInputStream& in);
  void RegisterScriptObjects(rstl::vector< CEntity* > objects, CStateManager& mgr);
  // Name and parameters corroborated by the Echoes Wii SEL export.
  SGeneratedObject GenerateScriptObject(const TEditorId& editorId, CStateManager& mgr);
  bool IsGeneratingObject() const { return mGeneratingObject; }

private:
  typedef rstl::map< TEditorId, SScriptObjectStream > TScriptObjectMap;

  // Guessed names.
  TScriptObjectMap mGeneratedScriptObjects;
  bool mGeneratingObject : 1;
};
CHECK_SIZEOF(CScriptObjectLoaderHelper, 0x18)
NESTED_CHECK_SIZEOF(CScriptObjectLoaderHelper, SScriptObjectStream, 0xc)
NESTED_CHECK_SIZEOF(CScriptObjectLoaderHelper, SGeneratedObject, 0xc)
NESTED_CHECK_SIZEOF(CScriptObjectLoaderHelper, SLoadContext, 0x2c)

#endif // _CSCRIPTOBJECTLOADERHELPER
