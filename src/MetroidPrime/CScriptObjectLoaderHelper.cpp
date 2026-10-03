#include "MetroidPrime/CScriptObjectLoaderHelper.hpp"

#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/ScriptLoader.hpp"

CScriptObjectLoaderHelper::SLoadContext::SLoadContext(TAreaId area)
: mAreaId(area)
, mStream()
, mRemainingObjects(0)
, x10_(0)
, mLayerIndex(0)
, mObjects()
, mEditorIds(nullptr) {}

CScriptObjectLoaderHelper::CScriptObjectLoaderHelper()
: mGeneratedScriptObjects(), mGeneratingObject(false) {}

CScriptObjectLoaderHelper::~CScriptObjectLoaderHelper() {}

// Guessed name; the discarded header fields' meanings remain unresolved.
static int ReadScriptLayerHeader(CInputStream& in) {
  in.ReadInt32();
  in.ReadUint8();
  in.ReadInt32();
  in.ReadUint8();
  return in.ReadInt32();
}

void CScriptObjectLoaderHelper::LoadScriptObjects(TAreaId area, CInputStream& in,
                                                  rstl::vector< TEditorId >& ids,
                                                  CStateManager& mgr) {
  int remaining = ReadScriptLayerHeader(in);
  ids.reserve(remaining + ids.size());
  while (remaining--) {
    const FourCC type = in.Get< FourCC >();
    const uint length = in.ReadUint16();
    const SGeneratedObject loaded = LoadScriptObject(area, type, length, in, mgr);
    if (loaded.mEditorId != kInvalidEditorId) {
      ids.push_back_unsafe(loaded.mEditorId);
      mgr.AddObject(loaded.mEntity);
    }
  }
}

CScriptObjectLoaderHelper::SGeneratedObject
CScriptObjectLoaderHelper::LoadScriptObject(TAreaId area, FourCC type, uint length,
                                            CInputStream& in, CStateManager& mgr) {
  const TEditorId editorId = in.Get< uint >();
  const uint connectionCount = in.ReadUint16();
  uint bytesLeft = length - 6;
  rstl::vector< SConnection > connections;
  connections.reserve(connectionCount);
  for (uint i = 0; i < connectionCount; ++i) {
    const EScriptObjectState state = static_cast< EScriptObjectState >(in.ReadInt32());
    const EScriptObjectMessage message = static_cast< EScriptObjectMessage >(in.ReadInt32());
    const TEditorId target = in.Get< uint >();
    connections.push_back_unsafe(SConnection(state, message, target));
    bytesLeft -= 12;
  }

  const uint readPosition = in.GetReadPosition();
  in.ReadInt32();
  in.ReadUint16();
  CEntity* entity = nullptr;
  FScriptLoader loader = GetScriptLoaderForType(type);
  if (loader != nullptr) {
    CEntityInfo info(area, connections, true, editorId);
    entity = loader(mgr, in, info);
  }

  bytesLeft -= in.GetReadPosition() - readPosition;
  while (bytesLeft--) {
    in.ReadUint8();
  }

  if (entity == nullptr) {
    return SGeneratedObject(kInvalidEditorId, kInvalidUniqueId, nullptr);
  }
  return SGeneratedObject(editorId, entity->GetUniqueId(), entity);
}

CScriptObjectLoaderHelper::SGeneratedObject
CScriptObjectLoaderHelper::GenerateScriptObject(const TEditorId& editorId, CStateManager& mgr) {
  const bool wasGenerating = mGeneratingObject;
  mGeneratingObject = true;
  const rstl::pair< const SScriptObjectStream*, TEditorId > build = GetBuildForScript(editorId);
  const TAreaId areaId(build.second.AreaNum());
  if (build.first != nullptr && mgr.World()->GetArea(areaId)->IsLoaded()) {
    const rstl::pair< const uchar*, int > buffer =
        mgr.World()->GetArea(areaId)->GetGeneratedScriptBuffer();
    CMemoryInStream in(buffer.first + build.first->mPosition, build.first->mLength);
    const SGeneratedObject generated =
        LoadScriptObject(areaId, build.first->mType, build.first->mLength, in, mgr);
    if (generated.mEntity != nullptr) {
      mgr.AddObject(generated.mEntity);
    }
    mGeneratingObject = wasGenerating;
    return generated;
  }

  mGeneratingObject = wasGenerating;
  return SGeneratedObject(kInvalidEditorId, kInvalidUniqueId, nullptr);
}

void CScriptObjectLoaderHelper::LoadGeneratedScriptObjects(TAreaId area, CInputStream& in) {
  in.ReadUint8();
  int remaining = in.ReadInt32();
  while (remaining--) {
    const FourCC type = in.Get< FourCC >();
    const uint length = in.ReadUint16();
    const uint position = in.GetReadPosition();
    const TEditorId editorId = in.Get< uint >();
    if (editorId != kInvalidEditorId && GetBuildForScript(editorId).first == nullptr) {
      const SScriptObjectStream stream = {type, position, length};
      mGeneratedScriptObjects.insert(
          rstl::pair< TEditorId, SScriptObjectStream >(editorId, stream));
    }
    in.ReadBytes(nullptr, static_cast< ushort >(length - 4));
  }
}

void CScriptObjectLoaderHelper::InitScriptObjects(rstl::vector< TEditorId >& ids,
                                                  CStateManager& mgr) {
  const int count = ids.size();
  const TEditorId* editorIds = ids.data();
  for (int i = 0; i < count; ++i) {
    if (editorIds[i] != kInvalidEditorId) {
      const CScriptMsg message(kInvalidUniqueId, kInvalidUniqueId, mgr.GetIdForScript(editorIds[i]),
                               kSM_XALD, kSS_InvalidState);
      mgr.DeliverScriptMsg(message);
    }
  }
}

void CScriptObjectLoaderHelper::RegisterScriptObjects(rstl::vector< CEntity* > objects,
                                                      CStateManager& mgr) {
  for (int i = 0; i < objects.size(); ++i) {
    if (objects[i] != nullptr) {
      mgr.AddObject(objects[i]);
    }
  }

  for (int i = 0; i < objects.size(); ++i) {
    if (objects[i] != nullptr) {
      const CScriptMsg message(kInvalidUniqueId, kInvalidUniqueId, objects[i]->GetUniqueId(),
                               kSM_XALD, kSS_InvalidState);
      mgr.DeliverScriptMsg(message);
    }
  }
}

rstl::pair< const CScriptObjectLoaderHelper::SScriptObjectStream*, TEditorId >
CScriptObjectLoaderHelper::GetBuildForScript(TEditorId editorId) const {
  TScriptObjectMap::const_iterator it = mGeneratedScriptObjects.find(editorId);
  if (it != mGeneratedScriptObjects.end()) {
    return rstl::pair< const SScriptObjectStream*, TEditorId >(&it->second, it->first);
  }
  return rstl::pair< const SScriptObjectStream*, TEditorId >(nullptr, kInvalidEditorId);
}

void CScriptObjectLoaderHelper::FreeScriptObjects(TAreaId area, CStateManager& mgr) {
  // TODO: area-object deletion and message-queue draining; nonfunctional scaffold.
}

void CScriptObjectLoaderHelper::RemoveLayerObjects(TAreaId area, TLayerId layer,
                                                   CStateManager& mgr) {
  // TODO: collect matching instances, delete them and drain messages; nonfunctional scaffold.
}

void CScriptObjectLoaderHelper::BeginLayerLoad(SLoadContext& context,
                                               rstl::auto_ptr< CInputStream > in,
                                               rstl::vector< TEditorId >& ids) {
  context.mStream = in;
  context.mRemainingObjects = ReadScriptLayerHeader(*in);
  context.mEditorIds = &ids;
  ids.reserve(context.mRemainingObjects);
  context.mObjects.reserve(context.mRemainingObjects + context.mObjects.size());
}

bool CScriptObjectLoaderHelper::ContinueLayerLoad(SLoadContext& context, uint timeBudget,
                                                  CStateManager& mgr) {
  CStopwatch timer;
  while (context.mRemainingObjects != 0) {
    const FourCC type = context.mStream->Get< FourCC >();
    const uint length = context.mStream->ReadUint16();
    const SGeneratedObject loaded =
        LoadScriptObject(context.mAreaId, type, length, *context.mStream, mgr);
    if (loaded.mEditorId != kInvalidEditorId) {
      context.mEditorIds->push_back_unsafe(loaded.mEditorId);
      context.mObjects.push_back_unsafe(loaded.mEntity);
    }
    --context.mRemainingObjects;
    if (context.mRemainingObjects != 0 && timer.GetElapsedMicros() > timeBudget) {
      return false;
    }
  }
  return true;
}
