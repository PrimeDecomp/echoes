#include "MetroidPrime/CGameArea.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CARAMManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/CLZOSupport.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Streams/CMemoryStreamOut.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CPortalArea.hpp"
#include "MetroidPrime/CRELFileManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CStaticGeometryMap.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/CWorldLayerState.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAreaProperties.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "WorldFormat/CAreaBspTree.hpp"
#include "WorldFormat/CAreaOctTree.hpp"
#include "WorldFormat/CPVSAreaSet.hpp"
#include "rstl/algorithm.hpp"

#include <alloca.h>
#include <dolphin/dvd.h>
#include <dolphin/os/OSCache.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>

namespace {
// Guessed name: sorts dependency indices by their resource offsets.
class CTextureDependencySorter {
public:
  explicit CTextureDependencySorter(const rstl::vector< uint >& offsets) : mOffsets(offsets) {}

  bool operator()(const int& a, const int& b) const { return mOffsets[a] < mOffsets[b]; }

private:
  const rstl::vector< uint >& mOffsets;
};
} // namespace

float CGameArea::skEntityThinkDisableDelayOnOcclusion = 5.f;

// The complex loading paths below remain scaffolds. This TU is NonMatching.

rstl::string CGameArea::IGetInternalAreaName() const { return mInternalAreaName; }

IGameArea::~IGameArea() {}

int CGameArea::VerifyHeader() const {
  if (!mPostConst->GetSectionBuffers().empty()) {
    const int* header =
        reinterpret_cast< const int* >(mPostConst->GetSectionBuffers().front().first.get());
    if (header[0] == 0xdeadbeef && header[1] >= 23 && header[1] <= 25) {
      return header[1];
    }
  }
  return 0;
}

int CGameArea::GetSectionIndex(int section) const {
  const int version = VerifyHeader();
  const int* header =
      reinterpret_cast< const int* >(mPostConst->GetSectionBuffers().front().first.get());
  if (version >= 11) {
    switch (section) {
    case 0:
      return header[17] + 2;
    case 1:
      return header[18] + 2;
    case 2:
      return header[19] + 2;
    case 3:
      return header[20] + 2;
    case 4:
      return header[21] + 2;
    case 5:
      return header[22] + 2;
    case 6:
      return header[23] + 2;
    case 7:
      return header[24] + 2;
    case 8:
      return header[26] + 2;
    case 9:
      return header[27] + 2;
    }
  }
  return -1;
}

CGameArea::CPostConstructed::CPostConstructed(const CGameArea& area)
: mMreaVersion(-1)
, mCollisionSize(0)
, mBspTree(nullptr)
, mPvs(nullptr)
, mPvsVersion(0)
, mPathArea(nullptr)
, mStaticGeometryMap(nullptr)
, mPortalArea(nullptr)
, mAreaObjectList(nullptr)
, mVisibleActorList(nullptr)
, mAreaFog(nullptr)
, mGeneratedScriptSize(0)
, mScriptLoadState(nullptr)
, mFirstMaterial(nullptr)
, mAreaAttributes(nullptr)
, mOcclusionState(kOS_Occluded)
, mOcclusionFrameCount(0)
, mOccludedTime(skEntityThinkDisableDelayOnOcclusion)
, mFirstAramSection(-1)
, mFirstMaterialSection(0)
, mAramBytes(0)
, x174_(0)
, mModelsInMram(false)
, mModelsConstructed(false)
, x178_2_(false)
, mOcclusionPinged(false)
, mPvsHasActors(false)
, mPvsHasLights(false)
, mScriptObjectsInitialized(false)
, mStreamingDelay(0)
, mDocksDisabled(false)
, mWorldLightingLevel(1.f)
, mXraySpeed(0.f)
, mXrayTarget(1.f)
, mWeaponWorldLightingSpeed(0.f)
, mWeaponWorldLightingTarget(1.f)
, mPlayerActorsLoading(0)
, mInverseTransform(area.GetTM().GetInverse())
, mMreaSize(0)
, mLoadedSectionCount(0)
, mLoadedBlockCount(0)
, mMreaDataOffset(0)
, mFirstScriptSection(-1)
, mDependencyDmaHandle(CARAMManager::GetInvalidDMAHandle())
, mSerializedDependencies(nullptr) {}

CGameArea::CPostConstructed::~CPostConstructed() {
  CARAMManager::WaitForDMACompletion(mDependencyDmaHandle);
}

CGameArea::CGameArea(CInputStream& in, int index, int mlvlVersion)
: mSelfIdx(index)
, mNameSTRG(in.ReadInt32())
, mTransform(in)
, mBounds(in)
, mAreaAssetId(in.ReadInt32())
, mAreaSaveId(in.ReadInt32())
, mAttachedAreaIndices(in)
, mDependencies1(in)
, mDependencies2(in)
, mSerializedDependencySize(0)
, mDependenciesInAram(nullptr)
, mPhase(kP_Allocate)
, mNext(nullptr)
, mPrev(nullptr)
, mCurrentChain(-1)
, mPostConst(nullptr)
, mLoadPaused(false)
, mValidationPaused(false)
, mActive(true)
, mUnloading(false) {
  mBounds = mBounds.GetTransformedAABox(mTransform);
  mLayerDependencyOffsets = rstl::vector< uint >(in);

  const int dockCount = in.ReadInt32();
  mDocks.reserve(dockCount);
  for (int i = 0; i < dockCount; ++i) {
    mDocks.push_back_unsafe(Dock(in, mTransform));
  }

  if (mlvlVersion > 18) {
    mRelModules = rstl::vector< rstl::string >(in);
    if (mlvlVersion > 20) {
      mRelOffsets = rstl::vector< int >(in);
    }
  }
  if (mlvlVersion >= 20) {
    const rstl::string name(in);
    if (mNameSTRG == kInvalidAssetId) {
      mInternalAreaName = name;
    }
  }

  SortTextureDependencies();
  mSerializedDependencySize = CalculateDependencyListByteCount();
  mDependenciesInAram = CARAMManager::Alloc(mSerializedDependencySize);
  {
    void* buffer = CMemory::Alloc(mSerializedDependencySize, IAllocator::kHI_RoundUpLen);
    {
      CMemoryStreamOut out(buffer, mSerializedDependencySize, CMemoryStreamOut::kOS_NotOwned, 64);
      mDependencies2.PutTo(out);
    }

    const uint handle = CARAMManager::DMAToARAM(
        buffer, mDependenciesInAram, mSerializedDependencySize, CARAMManager::kDMAPrio_One);
    mDependencies2 = rstl::vector< rstl::pair< CAssetId, uint > >();
    CARAMManager::WaitForDMACompletion(handle);
    CMemory::Free(buffer);
  }

  ClearTokenList();
  fn_80054F74();
}

CGameArea::~CGameArea() {
  if (IsLoaded()) {
    RemoveStaticGeometry();
  } else {
    while (!Invalidate(nullptr)) {
    }
  }
  CARAMManager::Free(mDependenciesInAram);
}

void CGameArea::ClearTokenList() {
  mLayerPhases.clear();
  if (mPostConst.get()) {
    mPostConst->GetLayerTokens().clear();
    mPostConst->GetLayerRelTokens().clear();
    mPostConst->mSortedRelTokens.clear();
  }
}

void CGameArea::AddLayerTokens(int layer, rstl::vector< CToken >& tokens) {
  if (mDependencies2.empty()) {
    return;
  }

  const int first = mLayerDependencyOffsets[layer];
  const int depCount = mDependencies2.size();
  const int last =
      layer + 1 < mLayerDependencyOffsets.size() ? mLayerDependencyOffsets[layer + 1] : depCount;
  for (int i = first; i < last; ++i) {
    const SObjectTag tag(mDependencies2[i].second, mDependencies2[i].first);
    if (tag.type == 'AGSC' && !gpSimplePool->HasObject(SObjectTag(tag))) {
      tokens.push_back_unsafe(TToken< int >(rs_new int));
    } else {
      tokens.push_back_unsafe(gpSimplePool->GetObj(tag));
    }
  }
  mLayerPhases[layer] = kLP_Loading;
}

bool CGameArea::UpdateDependencyLoading(CStateManager& mgr) {
  bool finished = true;
  for (int layer = 0; layer < mLayerDependencyOffsets.size(); ++layer) {
    if (mLayerPhases[layer] == kLP_Loading) {
      finished = false;
      break;
    }
  }
  if (!finished) {
    const bool loadTexturesToAram = mgr.IsFullyInitialized();
    int pending = 0;
    for (int i = 0; i < mPostConst->GetLayerTokens().size(); ++i) {
      const int layer = i != 0 ? i - 1 : mPostConst->GetLayerTokens().size() - 1;
      const int priorPending = pending;
      if (mLayerPhases[layer] == kLP_Loading) {
        rstl::vector< CToken >& tokens = mPostConst->GetLayerTokens()[layer];
        for (int j = 0; j < tokens.size(); ++j) {
          CToken& token = tokens[j];
          if (token.IsLoaded()) {
            token.Lock();
            if (token.GetReferenceType() == 'TXTR') {
              TToken< CTexture > texture(token);
              CTexture* resource = texture.GetT();
              resource->MakeSwappable();
              if (loadTexturesToAram) {
                resource->LoadToARAM();
              }
            }
          } else {
            if (!token.HasLock()) {
              const SObjectTag tag('MREA', mAreaAssetId);
              gpResourceFactory->GetResLoader().FindResource(tag);
              token.Lock();
            }
            ++pending;
          }
        }
        if (pending > 80) {
          return false;
        }

        for (rstl::list< rstl::pair< int, rstl::auto_ptr< CDvdRequest > > >::const_iterator it =
                 mPostConst->GetLayerLoadTransactions().begin();
             it != mPostConst->GetLayerLoadTransactions().end(); ++it) {
          const rstl::pair< int, rstl::auto_ptr< CDvdRequest > >& request = *it;
          if (request.first == layer) {
            ++pending;
            break;
          }
        }
        if (layer < mPostConst->GetLayerRelTokens().size()) {
          rstl::vector< CRELFileToken >& rels = mPostConst->GetLayerRelTokens()[layer];
          for (int j = 0; j < rels.size(); ++j) {
            if (!rels[j].IsLoaded()) {
              ++pending;
            }
          }
        }
        if (pending == priorPending) {
          mLayerPhases[layer] = kLP_Ready;
        }
      }
    }

    for (int i = 0; i < mPostConst->mSortedRelTokens.size(); ++i) {
      mPostConst->mSortedRelTokens[i]->Load();
      if (!mPostConst->mSortedRelTokens[i]->IsLoaded()) {
        ++pending;
      }
    }
    if (pending != 0) {
      return false;
    }
  }
  return true;
}

void CGameArea::VerifyTokenList(CStateManager& mgr) {
  const CWorldLayerState& layers = *mgr.mCurrentWorldLayerState;
  if (GetTokenCount() == 0) {
    ClearTokenList();
    mLayerPhases.resize(mLayerDependencyOffsets.size(), kLP_Inactive);
    mPostConst->mActiveLayers.resize(layers.GetLayerCount(mSelfIdx), false);
    mPostConst->GetLayerTokens().resize(mLayerDependencyOffsets.size(),
                                              rstl::vector< CToken >());

    for (int layer = 0; layer < mLayerDependencyOffsets.size(); ++layer) {
      rstl::vector< CToken >& tokens = mPostConst->GetLayerTokens()[layer];
      const int first = mLayerDependencyOffsets[layer];
      const int last = layer + 1 < mLayerDependencyOffsets.size()
                           ? mLayerDependencyOffsets[layer + 1]
                           : mDependencies2.size();
      tokens.clear();
      tokens.reserve(last - first);
    }

    if (!mDependencies2.empty()) {
      for (int layer = mLayerDependencyOffsets.size() - 1; layer >= 0; --layer) {
        if (layers.IsLayerActive(mSelfIdx, TLayerId(layer))) {
          AddLayerTokens(layer, mPostConst->GetLayerTokens()[layer]);
        }
      }
      for (uint layer = 0; layer < layers.GetLayerCount(mSelfIdx); ++layer) {
        mPostConst->GetActiveLayers()[layer] =
            layers.IsLayerActive(mSelfIdx, TLayerId(layer));
      }
    }
  }

  const int relLayerCount = mRelOffsets.size() / 2;
  if (relLayerCount != mPostConst->GetLayerRelTokens().size()) {
    mPostConst->GetLayerRelTokens().resize(relLayerCount, rstl::vector< CRELFileToken >());
  }
  if (!mRelModules.empty() && mPostConst->mSortedRelTokens.empty()) {
    for (int layer = 0; layer < mPostConst->GetActiveLayers().size(); ++layer) {
      if (mPostConst->GetActiveLayers()[layer]) {
        LoadLayerRelModules(mgr, TLayerId(layer));
      }
    }
    SortRelTokens(layers);
  }
}

void CGameArea::SortRelTokens(const CWorldLayerState& layers) {
  if (mRelModules.empty()) {
    return;
  }

  const int layerCount = mPostConst->GetLayerRelTokens().size();
  int count = 0;
  for (int layer = 0; layer < layerCount; ++layer) {
    if (mLayerPhases[layer] == kLP_Loading) {
      count += mPostConst->GetLayerRelTokens()[layer].size();
    }
  }

  typedef rstl::pair< uint, CRELFileToken* > SRelLocation;
  SRelLocation* locations = static_cast< SRelLocation* >(alloca(count * sizeof(SRelLocation)));
  int cursor = 0;
  for (int layer = 0; layer < layerCount; ++layer) {
    if (mLayerPhases[layer] == kLP_Loading) {
      rstl::vector< CRELFileToken >& tokens = mPostConst->GetLayerRelTokens()[layer];
      for (int i = 0; i < tokens.size(); ++i, ++cursor) {
        CRELFileToken& token = tokens[i];
        uint offset = 0;
        DVDFileInfo info;
        if (DVDOpen(const_cast< char* >(token.GetFileName().data()), &info)) {
          offset = info.startAddr;
          DVDClose(&info);
        }
        locations[cursor] = SRelLocation(offset, &token);
      }
    }
  }
  rstl::sort(locations, locations + count,
             rstl::pair_sorter_finder< SRelLocation, rstl::less< uint > >(rstl::less< uint >()));

  mPostConst->mSortedRelTokens.clear();
  mPostConst->mSortedRelTokens.reserve(count);
  for (int i = 0; i < count; ++i) {
    mPostConst->mSortedRelTokens.push_back_unsafe(locations[i].second);
  }
}

void CGameArea::FillInStaticGeometry() {
  rstl::vector< rstl::pair< rstl::auto_ptr< char >, int > >::const_iterator section =
      mPostConst->GetSectionBuffers().begin() + mPostConst->mFirstMaterialSection;
  mPostConst->mFirstMaterial = reinterpret_cast< const uchar* >(section->first.get());
  mPostConst->mModelInstances.clear();
  ++section;
  const int modelCount = mPostConst->mModelInstances.capacity();
  rstl::vector< void* > surfaces;
  for (int model = 0; model < modelCount; ++model) {
    const void* header = section->first.get();
    const void* positions = (++section)->first.get();
    const void* normals = (++section)->first.get();
    const void* colors = (++section)->first.get();
    const void* texCoords = (++section)->first.get();
    const void* packedTexCoords = (++section)->first.get();
    const uint surfaceCount =
        CBasics::SwapBytes(*reinterpret_cast< const uint* >((++section)->first.get()));
    ++section;
    if (surfaceCount != 0) {
      surfaces.reserve(surfaceCount);
      for (uint surface = 0; surface < surfaceCount; ++surface) {
        surfaces.push_back_unsafe(section->first.get());
        ++section;
      }
      const void* section1 = section->first.get();
      const void* section2 = (++section)->first.get();
      ++section;
      mPostConst->mModelInstances.push_back_unsafe(
          CMetroidModelInstance(header, mPostConst->mFirstMaterial, positions, normals,
                                colors, texCoords, packedTexCoords, surfaces, section1, section2));
      surfaces.clear();
    }
  }

  {
    CMemoryInStream in((section + 1)->first.get(), (section + 1)->second);
    mPostConst->mSurfaces = rstl::vector< SAreaSurface >(in);
  }
  if (mPostConst->mMreaVersion >= 22) {
    CMemoryInStream in((section + 2)->first.get(), (section + 2)->second);
    mPostConst->mAmbientLightIds = rstl::vector< uint >(in);
    mPostConst->mAmbientLightIndices = rstl::vector< signed char >(in);
  }
  mPostConst->mModelsConstructed = true;
}

// The header prefix used during post-construction; later version fields follow it.
struct SMreaHeader {
  uint magic;
  uint version;
  CTransform4f transform;
  int modelCount;
  int layerCount;
  int sectionCount;
  int sectionIndices[8];
  int renderOctreeSection;
};

static inline CVector3f SwapVectorBytes(CVector3f vector) {
  return CVector3f(CBasics::SwapBytes(vector.GetX()), CBasics::SwapBytes(vector.GetY()),
                   CBasics::SwapBytes(vector.GetZ()));
}

void CGameArea::PostConstructArea() {
  mPostConst->mMreaVersion = VerifyHeader();
  rstl::vector< rstl::pair< rstl::auto_ptr< char >, int > >::const_iterator section =
      mPostConst->GetSectionBuffers().begin();
  const SMreaHeader* header = reinterpret_cast< const SMreaHeader* >(section->first.get());
  for (int i = 0; i < 3; ++i) {
    CVector3f row = SwapVectorBytes(header->transform.GetRow(i));
    close_enough(mTransform.GetRow(i), row, 0.001f);
  }
  CVector3f translation = SwapVectorBytes(header->transform.GetTranslation());
  close_enough(mTransform.GetTranslation(), translation, 0.001f);

  const int modelCount = CBasics::SwapBytes(header->modelCount);
  section += 2;
  if (header->version >= 24) {
    ++section;
  }
  int firstGeometry = section - mPostConst->GetSectionBuffers().begin();
  mPostConst->mFirstMaterialSection = firstGeometry;
  ++section;
  mPostConst->mModelInstances.reserve(modelCount);
  for (int i = 0; i < modelCount; ++i) {
    int surfaces = *reinterpret_cast< const int* >((section + 6)->first.get());
    section += 7;
    section += surfaces;
    section += 2;
  }
  long geometryEnd = section - mPostConst->GetSectionBuffers().begin();
  if (header->renderOctreeSection != -1) {
    rstl::auto_ptr< const uchar > buffer(reinterpret_cast< const uchar* >(section->first.get()));
    buffer.release();
    mPostConst->mRenderOctTree = CAreaRenderOctTree(buffer);
    ++section;
  }
  ++section;
  if (mPostConst->mMreaVersion >= 22) {
    ++section;
  }

  const int layerCount = header->layerCount;
  mPostConst->mLayerScriptBuffers.reserve(layerCount);
  mPostConst->mLayerScriptSizes.reserve(layerCount);
  for (int i = 0; i < layerCount; ++i) {
    mPostConst->mLayerScriptBuffers.push_back_unsafe(section->first.get());
    mPostConst->mLayerScriptBuffers.back().release();
    mPostConst->mLayerScriptSizes.push_back_unsafe(section->second);
    ++section;
  }
  mPostConst->mGeneratedScriptBuffer = section->first.get();
  mPostConst->mGeneratedScriptBuffer.release();
  mPostConst->mGeneratedScriptSize = section->second;
  ++section;

  char* collisionData = section->first.get();
  ++collisionData;
  while (reinterpret_cast< uintptr_t >(collisionData) & 3) {
    ++collisionData;
  }
  uint collisionSize = *reinterpret_cast< const uint* >(collisionData);
  collisionData += 4;
  CAreaOctTree* collision = nullptr;
  bool collisionOwned = false;
  CAreaOctTree::MakeFromMemory(collisionData, collisionSize, &collision, &collisionOwned);
  mPostConst->mCollision = collision;
  if (!collisionOwned) {
    mPostConst->mCollision.release();
  }
  mPostConst->mCollisionSize = collisionSize;
  collisionData += collisionSize;
  if (mPostConst->mMreaVersion < 25) {
    const uint count = *reinterpret_cast< const uint* >(collisionData);
    CMemoryInStream stream(collisionData + 4, count * sizeof(CAABox) + 8);
    stream.ReadFloat();
    stream.ReadFloat();
    for (uint i = 0; i < count; ++i) {
      stream.Get< CAABox >();
    }
  }
  ++section;
  {
    CMemoryInStream stream(section->first.get(), section->second);
    mPostConst->mBspTree = rs_new CAreaBspTree(stream, mTransform);
  }

  ++section;
  {
    CMemoryInStream stream(section->first.get(), section->second);
    const uint magic = stream.ReadInt32();
    const bool twoLayers = magic == 0xbabedead;
    int count = twoLayers ? stream.ReadInt32() : magic;
    mPostConst->mLightsA.clear();
    mPostConst->mLightsA.reserve(count);
    mPostConst->mGfxLightsA.clear();
    mPostConst->mGfxLightsA.reserve(count);
    for (int i = 0; i < count; ++i) {
      mPostConst->mLightsA.push_back_unsafe(CWorldLight(stream));
      mPostConst->mGfxLightsA.push_back_unsafe(
          mPostConst->mLightsA[i].GetAsCGraphicsLight());
    }
    if (twoLayers) {
      const int countB = stream.Get< int >();
      if (countB != 0) {
        mPostConst->mLightsB.reserve(countB);
        mPostConst->mGfxLightsB.reserve(countB);
        for (int i = 0; i < countB; ++i) {
          mPostConst->mLightsB.push_back_unsafe(CWorldLight(stream));
          mPostConst->mGfxLightsB.push_back_unsafe(
              mPostConst->mLightsB[i].GetAsCGraphicsLight());
        }
      }
    }
    const CPostConstructed* post = mPostConst.get();
    if (post->mLightsB.size() == 0) {
      mPostConst->mLightsB = mPostConst->mLightsA;
      mPostConst->mGfxLightsB = mPostConst->mGfxLightsA;
    }
  }

  ++section;
  {
    const int size = section->second;
    if (size > 64) {
      const char* const buffer = section->first.get();
      CMemoryInStream stream(buffer, size);
      if (stream.ReadInt32() == 'VISI') {
        int pvsVersion = stream.ReadInt32();
        mPostConst->mPvsVersion = pvsVersion;
        if (mPostConst->mPvsVersion == 2) {
          mPostConst->mPvsHasActors = stream.ReadBool();
          mPostConst->mPvsHasLights = stream.ReadBool();
          mPostConst->mPvs = CPVSAreaSet::MakeAreaSet(buffer + stream.GetReadPosition(),
                                                            size - stream.GetReadPosition())
                                       .release();
        }
      }
    }
  }
  ++section;
  {
    CMemoryInStream stream(section->first.get(), section->second);
    CAssetId pathId = stream.ReadInt32();
    if (pathId != kInvalidAssetId) {
      mPostConst->mPathToken =
          TLockedToken< CPFArea >(gpSimplePool->GetObj(SObjectTag('PATH', pathId)));
      mPostConst->mPathArea = **mPostConst->mPathToken;
      mPostConst->mPathArea->SetTransform(mTransform);
    }
  }
  ++section;
  {
    CMemoryInStream stream(section->first.get(), section->second);
    CAssetId portalId = stream.ReadInt32();
    if (portalId != kInvalidAssetId) {
      mPostConst->mPortalArea = rs_new CPortalArea(
          TLockedToken< CPortalAreaData >(gpSimplePool->GetObj(SObjectTag('PTLA', portalId))));
    }
  }
  ++section;
  {
    CMemoryInStream stream(section->first.get(), section->second);
    CAssetId mapId = stream.ReadInt32();
    if (mapId != kInvalidAssetId) {
      mPostConst->mStaticGeometryMap = rs_new CStaticGeometryMap(
          TLockedToken< CStaticGeometryMapData >(gpSimplePool->GetObj(SObjectTag('EGMC', mapId))));
    }
  }

  int firstAram = firstGeometry;
  for (; firstAram < mPostConst->GetSectionBuffers().size(); ++firstAram) {
    if (mPostConst->GetSectionBuffers()[firstAram].first.owner()) {
      break;
    }
  }
  int lastAram = geometryEnd;
  for (; firstAram < lastAram; --lastAram) {
    if (mPostConst->GetSectionBuffers()[lastAram].first.owner()) {
      break;
    }
  }
  if (firstAram < lastAram) {
    mPostConst->mFirstAramSection = firstAram;
    int bufferCount = 0;
    for (int i = firstAram; i < lastAram; ++i) {
      if (mPostConst->GetSectionBuffers()[i].first.owner()) {
        ++bufferCount;
      }
    }
    mPostConst->mAramTokens.reserve(bufferCount);
    for (int part = firstAram; part < lastAram;) {
      int start = part;
      int size = mPostConst->GetSectionBuffers()[part++].second;
      for (; part < lastAram && !mPostConst->GetSectionBuffers()[part].first.owner();
           ++part) {
        size += mPostConst->GetSectionBuffers()[part].second;
      }
      mPostConst->mAramTokens.push_back_unsafe(rstl::pair< CARAMToken, int >(
          CARAMToken(mPostConst->GetSectionBuffers()[start].first.release(), size, 1),
          part - start));
      if (GetOcclusionState() == kOS_Occluded) {
        CARAMToken& token = mPostConst->mAramTokens.back().first;
        token.LoadToARAM();
        if (token.GetStatus() != CARAMToken::kS_One) {
          mPostConst->mAramBytes += size;
        }
      }
    }
    mPostConst->mModelsInMram = GetOcclusionState() != kOS_Occluded;
  }

  mPostConst->mAreaObjectList = rs_new CAreaObjectList(mSelfIdx);
  mPostConst->mVisibleActorList = rs_new CAreaObjectList(mSelfIdx);
  mPostConst->mAreaFog = rs_new CAreaFog;
  fn_80054F74();
}

void CGameArea::FinishDependencyLoading(CStateManager& mgr) {
  if (IsLoaded()) {
    return;
  }

  for (rstl::list< rstl::auto_ptr< CDvdRequest > >::iterator it =
           mPostConst->mLoadTransactions.begin();
       it != mPostConst->mLoadTransactions.end(); ++it) {
    if (it->get()) {
      if (!(*it)->IsComplete()) {
        (*it)->WaitUntilComplete();
      }
      ClearDecompressionRequest(it->get());
    }
  }
  for (rstl::list< rstl::pair< int, rstl::auto_ptr< CDvdRequest > > >::iterator it =
           mPostConst->GetLayerLoadTransactions().begin();
       it != mPostConst->GetLayerLoadTransactions().end(); ++it) {
    if (it->second.get()) {
      if (!it->second->IsComplete()) {
        it->second->WaitUntilComplete();
      }
      ClearDecompressionRequest(it->second.get());
    }
  }
  while (!mPostConst->mDecompressionRequests.empty()) {
    DecompressAreaData();
  }

  if (!mPostConst->GetLayerRelTokens().empty()) {
    for (int layer = 0; layer < mPostConst->GetLayerRelTokens().size(); ++layer) {
      for (int i = 0; i < mPostConst->GetLayerRelTokens()[layer].size(); ++i) {
        mPostConst->GetLayerRelTokens()[layer][i].Load();
        while (!mPostConst->GetLayerRelTokens()[layer][i].IsLoaded()) {
          gpRelFileManager->Update();
        }
      }
    }
  }

  if (GetTokenCount() == 0) {
    VerifyTokenList(mgr);
    for (int layer = 0; layer < mPostConst->GetLayerTokens().size(); ++layer) {
      for (rstl::vector< CToken >::iterator it = mPostConst->GetLayerTokens()[layer].begin();
           it != mPostConst->GetLayerTokens()[layer].end(); ++it) {
        it->Lock();
      }
    }
    for (int layer = 0; layer < mPostConst->GetLayerTokens().size(); ++layer) {
      for (rstl::vector< CToken >::iterator it = mPostConst->GetLayerTokens()[layer].begin();
           it != mPostConst->GetLayerTokens()[layer].end(); ++it) {
        it->GetObj();
      }
    }
    for (int layer = 0; layer < mPostConst->GetActiveLayers().size(); ++layer) {
      if (mPostConst->GetActiveLayers()[layer]) {
        mLayerPhases[layer] = kLP_Ready;
      }
    }
    mLayerPhases[mLayerDependencyOffsets.size() - 1] = kLP_Ready;
  }
  mPostConst->mLoadTransactions.clear();
  mPostConst->GetLayerLoadTransactions().clear();
}

void CGameArea::PrepareScriptObjects(CStateManager& mgr) {
  const CWorldLayerState& layers = *mgr.mCurrentWorldLayerState;
  const int count = layers.GetLayerCount(mSelfIdx);
  mPostConst->mLayerEditorIds.clear();
  mPostConst->mLayerEditorIds.resize(count);
  mPostConst->mScriptLoadState = rs_new CScriptObjectLoaderHelper::SLoadContext(mSelfIdx);
}

bool CGameArea::LoadScriptObjects(CStateManager& mgr) {
  CScriptObjectLoaderHelper& loader = mgr.ScriptObjectLoaderHelper();
  CScriptObjectLoaderHelper::SLoadContext& context = *mPostConst->mScriptLoadState;
  for (int i = 0; i < mPostConst->GetActiveLayers().size(); ++i) {
    const TLayerId layer(i);
    if (context.mLayerIndex != layer.Value()) {
      continue;
    }
    if (mPostConst->GetActiveLayers()[i]) {
      if (context.mRemainingObjects == 0) {
        const rstl::pair< const uchar*, int > buffer = GetLayerScriptBuffer(layer);
        rstl::auto_ptr< CInputStream > stream(rs_new CMemoryInStream(buffer.first, buffer.second));
        rstl::vector< TEditorId >& ids = mPostConst->mLayerEditorIds[i];
        loader.BeginLayerLoad(context, stream, ids);
      }
      uint timeBudget = 4000;
      if (gpMain->GetAverageDrawTime() + gpMain->GetAverageTickTime() > 0.8f) {
        timeBudget = 1000;
      }
      if (loader.ContinueLayerLoad(context, timeBudget, mgr)) {
        mLayerPhases[i] = kLP_Active;
        ++context.mLayerIndex;
        return false;
      }
    } else {
      ++context.mLayerIndex;
    }
  }
  const bool complete = context.mLayerIndex == mPostConst->GetActiveLayers().size();
  return complete;
}

void CGameArea::FinishScriptObjects(CStateManager& mgr) {
  CScriptObjectLoaderHelper& loader = mgr.ScriptObjectLoaderHelper();
  for (int i = 0; i < mPostConst->GetActiveLayers().size(); ++i) {
    mPostConst->GetSectionBuffers()[i + mPostConst->mFirstScriptSection].first =
        rstl::auto_ptr< char >();
    mPostConst->mLayerScriptBuffers[i] = rstl::auto_ptr< char >();
  }
  mPostConst->mScriptObjectsInitialized = false;

  const rstl::pair< const uchar*, int > buffer = GetGeneratedScriptBuffer();
  CMemoryInStream stream(buffer.first, buffer.second);
  if (stream.Get< uint >() == 'SCGN') {
    stream.ReadUint8();
    loader.LoadGeneratedScriptObjects(GetId(), stream);
  }
  loader.RegisterScriptObjects(mPostConst->mScriptLoadState->mObjects, mgr);
  InitializeDocks(mgr);

  if (mPostConst->mPvs.get() && mPostConst->mPvsHasActors) {
    for (int i = 0; i < mPostConst->mPvs->GetNumActors(); ++i) {
      const CPostConstructed* post = mPostConst.get();
      const TEditorId editorId(post->mPvs->GetEntityIdByIndex(i) | (mSelfIdx.Value() << 16));
      const TUniqueId id = mgr.GetIdForScript(editorId);
      if (id != kInvalidUniqueId) {
        const CPVSAreaSet* pvs = mPostConst->mPvs.get();
        const int index = i + (pvs->GetNumFeatures() - pvs->GetNumActors());
        if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id))) {
          actor->SetPvsIndex(index);
        }
      }
    }
  }
  mPostConst->mScriptObjectsInitialized = true;
  mPostConst->mScriptLoadState = nullptr;
}

bool CGameArea::StartStreamIn(CStateManager& mgr) {
  if (IsLoaded()) {
    return true;
  }
  if (mPostConst.get() && mPostConst->mStreamingDelay) {
    --mPostConst->mStreamingDelay;
    return false;
  }
  if (mLoadPaused) {
    return false;
  }
  const bool loaded = StartStreamingMainArea(mgr);
  return loaded;
}

void CGameArea::Validate(CStateManager& mgr) {
  while (!StartStreamIn(mgr)) {
    gpResourceFactory->AsyncIdle(5000, false);
  }
}

void CGameArea::CullDeadAreaRequests() {
  while (!mPostConst->mLoadTransactions.empty() &&
         mPostConst->mLoadTransactions.front()->IsComplete()) {
    ClearDecompressionRequest(mPostConst->mLoadTransactions.front().get());
    mPostConst->mLoadTransactions.pop_front();
  }
}

bool CGameArea::Invalidate(CStateManager* mgr) {
  mUnloading = true;
  if (mPhase == kP_Allocate) {
    ClearTokenList();
  } else if (mPhase < kP_LoadScriptObjects) {
    if (mPostConst->mDependencyDmaHandle != CARAMManager::GetInvalidDMAHandle()) {
      if (CARAMManager::IsDMACompleted(mPostConst->mDependencyDmaHandle)) {
        mPostConst->mDependencyDmaHandle = CARAMManager::GetInvalidDMAHandle();
        mPostConst->mSerializedDependencies = nullptr;
      } else {
        return false;
      }
    }

    ClearTokenList();
    for (rstl::list< rstl::auto_ptr< CDvdRequest > >::iterator it =
             mPostConst->mLoadTransactions.begin();
         it != mPostConst->mLoadTransactions.end();) {
      rstl::list< rstl::auto_ptr< CDvdRequest > >::iterator cur = it;
      ++it;
      if (!(*cur)->IsComplete()) {
        (*cur)->PostCancelRequest();
      } else {
        mPostConst->mLoadTransactions.erase(cur);
      }
    }
    if (!mPostConst->mLoadTransactions.empty()) {
      return false;
    }

    for (rstl::list< rstl::pair< int, rstl::auto_ptr< CDvdRequest > > >::iterator it =
             mPostConst->GetLayerLoadTransactions().begin();
         it != mPostConst->GetLayerLoadTransactions().end();) {
      rstl::list< rstl::pair< int, rstl::auto_ptr< CDvdRequest > > >::iterator cur = it;
      ++it;
      if (!cur->second->IsComplete()) {
        cur->second->PostCancelRequest();
      } else {
        mPostConst->GetLayerLoadTransactions().erase(cur);
      }
    }
    if (!mPostConst->GetLayerLoadTransactions().empty()) {
      return false;
    }

    mPostConst = nullptr;
    mPhase = kP_Allocate;
    ResetLayerData();
  } else {
    if (mgr != nullptr) {
      if (mPhase != kP_Loaded && !StartStreamIn(*mgr)) {
        return false;
      }
      mgr->PrepareAreaUnload(GetId());
    }
    RemoveStaticGeometry();
    mPostConst = nullptr;
    mPhase = kP_Allocate;
    ResetLayerData();
    ClearTokenList();
    if (mgr != nullptr) {
      mgr->AreaUnloaded(GetId());
    }
    fn_80054F74();
  }
  mUnloading = false;
  return true;
}

void CGameArea::ResetLayerData() {
  mLayerPhases = rstl::vector< ELayerPhase >();
  mDependencies2 = rstl::vector< rstl::pair< CAssetId, uint > >();
}

char* CGameArea::AllocNewAreaData(int offset, int size) {
  char* buffer = static_cast< char* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen));
  rstl::pair< rstl::auto_ptr< char >, int > section(buffer, size);
  mPostConst->GetSectionBuffers().push_back_unsafe(section);

  const SObjectTag tag('MREA', mAreaAssetId);
  mPostConst->mLoadTransactions.push_back(
      gpResourceFactory->GetResLoader().LoadResourcePartAsync(tag, offset, size, buffer));
  return buffer;
}

uint CGameArea::CalculateDependencyListByteCount() const {
  return ALIGN_UP(4 + mDependencies2.size() * 8, 32);
}

int CGameArea::GetNumPartSizes() const {
  const uint* header =
      reinterpret_cast< const uint* >(mPostConst->GetSectionBuffers().front().first.get());
  return header[16];
}

int CGameArea::GetNumCompressedBlocks() const {
  const uint* header =
      reinterpret_cast< const uint* >(mPostConst->GetSectionBuffers().front().first.get());
  return header[1] < 24 ? 0 : header[28];
}

bool CGameArea::ReloadAllUnloadedTextures() {
  bool finished = true;

  for (int layer = 0; layer < mPostConst->GetLayerTokens().size(); ++layer) {
    rstl::vector< CToken >& tokens = mPostConst->GetLayerTokens()[layer];
    for (int i = 0; i < tokens.size(); ++i) {
      CToken& token = tokens[i];
      if (token.GetReferenceType() == 'TXTR' && token.IsLoaded() && token.HasLock()) {
        TToken< CTexture > texture(token);
        CTexture* resource = texture.GetT();
        resource->MakeSwappable();
        if (!resource->LoadToMRAM()) {
          finished = false;
        }
      }
    }
  }
  return finished;
}

bool CGameArea::UnloadAllloadedTextures() {
  bool finished = true;

  for (int layer = 0; layer < mPostConst->GetLayerTokens().size(); ++layer) {
    rstl::vector< CToken >& tokens = mPostConst->GetLayerTokens()[layer];
    for (int i = 0; i < tokens.size(); ++i) {
      CToken& token = tokens[i];
      if (token.GetReferenceType() == 'TXTR' && token.IsLoaded() && token.HasLock()) {
        TToken< CTexture > texture(token);
        CTexture* resource = texture.GetT();
        resource->MakeSwappable();
        resource->LoadToARAM();
        if (resource->IsARAMTransferInProgress()) {
          finished = false;
        }
      }
    }
  }
  return finished;
}

bool CGameArea::StartStreamingMainArea(CStateManager& mgr) {
  switch (mPhase) {
  case kP_Allocate:
    mPostConst = rs_new CPostConstructed(*this);
    mPhase = kP_ReadDependencies;
    mPostConst->mSerializedDependencies = static_cast< uchar* >(
        CMemory::Alloc(mSerializedDependencySize, IAllocator::kHI_RoundUpLen));
    mPostConst->mDependencyDmaHandle = CARAMManager::DMAToMRAM(
        mDependenciesInAram, mPostConst->mSerializedDependencies.get(),
        mSerializedDependencySize, CARAMManager::kDMAPrio_One);
    break;
  case kP_ReadDependencies:
    if (!CARAMManager::IsDMACompleted(mPostConst->mDependencyDmaHandle)) {
      break;
    }
    {
      CMemoryInStream in(mPostConst->mSerializedDependencies.get(), mSerializedDependencySize,
                         CMemoryInStream::kOS_NotOwned);
      mDependencies2 = rstl::vector< rstl::pair< CAssetId, uint > >(in);
    }
    mPostConst->mSerializedDependencies = nullptr;
    mPostConst->mDependencyDmaHandle = CARAMManager::GetInvalidDMAHandle();
    mPhase = kP_PrepareDependencies;
    // Fall through.
  case kP_PrepareDependencies:
    VerifyTokenList(mgr);
    mPhase = kP_WaitForDependencies;
    mPostConst->mStreamingDelay = 4;
    break;
  case kP_WaitForDependencies:
    if (!UpdateDependencyLoading(mgr)) {
      break;
    }
    mPhase = kP_LoadHeader;
    // Fall through.
  case kP_LoadHeader:
    mPostConst->GetSectionBuffers().reserve(3);
    AllocNewAreaData(0, 0x80);
    mPhase = kP_LoadSectionSizes;
    // Fall through.
  case kP_LoadSectionSizes: {
    CullDeadAreaRequests();
    if (!mPostConst->mLoadTransactions.empty()) {
      break;
    }
    mPostConst->mMreaVersion = VerifyHeader();
    const int sectionBytes = ALIGN_UP(GetNumPartSizes() * 4, 32);
    const int headerBytes = mPostConst->GetSectionBuffers()[0].second;
    AllocNewAreaData(headerBytes, sectionBytes);
    if (mPostConst->mMreaVersion >= 24) {
      AllocNewAreaData(headerBytes + sectionBytes, ALIGN_UP(GetNumCompressedBlocks() * 16, 32));
    }
    mPhase = kP_ReserveSections;
    break;
  }
  case kP_ReserveSections: {
    CullDeadAreaRequests();
    if (!mPostConst->mLoadTransactions.empty()) {
      break;
    }
    const int partCount = GetNumPartSizes();
    mPostConst->GetSectionBuffers().reserve(partCount + 3);
    int offset = mPostConst->GetSectionBuffers()[0].second;
    offset += mPostConst->GetSectionBuffers()[1].second;
    if (mPostConst->mMreaVersion >= 24) {
      offset += mPostConst->GetSectionBuffers()[2].second;
    }
    mPostConst->mLoadedSectionCount = 0;
    mPostConst->mLoadedBlockCount = 0;
    mPostConst->mMreaDataOffset = offset;
    mPhase = kP_LoadDataSections;
    break;
  }
  case kP_LoadDataSections: {
    CullDeadAreaRequests();
    if (mPostConst->mMreaVersion < 24) {
      const int firstSection = mPostConst->mLoadedSectionCount;
      int totalSize = 0;
      const int partCount = GetNumPartSizes();
      const int* sizes =
          reinterpret_cast< const int* >(mPostConst->GetSectionBuffers()[1].first.get());
      const SObjectTag tag('MREA', mAreaAssetId);
      bool load = true;
      const int scriptStart = GetSectionIndex(1) - 2;
      const int scriptCount =
          reinterpret_cast< const int* >(mPostConst->GetSectionBuffers()[0].first.get())[15];
      int endSection = firstSection;
      if (firstSection >= scriptStart && firstSection < scriptStart + scriptCount) {
        const int layer = firstSection - scriptStart;
        if (mPostConst->mFirstScriptSection == -1) {
          mPostConst->mFirstScriptSection = mPostConst->GetSectionBuffers().size();
        }
        if (!mPostConst->GetActiveLayers()[layer]) {
          load = false;
        }
        totalSize = sizes[firstSection];
        endSection = firstSection + 1;
        if (scriptCount != mPostConst->mLayerFileOffsets.capacity()) {
          mPostConst->mLayerFileOffsets.reserve(scriptCount);
        }
        mPostConst->mLayerFileOffsets.push_back_unsafe(mPostConst->mMreaDataOffset);
      } else {
        for (endSection = firstSection; endSection < partCount; ++endSection) {
          const int size = sizes[endSection];
          const bool isScript = endSection >= scriptStart && endSection < scriptStart + scriptCount;
          if (endSection != firstSection && (isScript || size + totalSize > 0x20000)) {
            break;
          }
          totalSize += size;
        }
      }

      rstl::auto_ptr< char > buffer(
          load ? static_cast< char* >(CMemory::Alloc(totalSize, IAllocator::kHI_RoundUpLen))
               : nullptr);
      if (load) {
        mPostConst->mLoadTransactions.push_back(
            gpResourceFactory->GetResLoader().LoadResourcePartAsync(
                tag, mPostConst->mMreaDataOffset, totalSize, buffer.get()));
      }
      mPostConst->mMreaDataOffset += totalSize;
      const int firstSize = sizes[firstSection];
      int offset = firstSize;
      mPostConst->GetSectionBuffers().push_back_unsafe(
          rstl::pair< rstl::auto_ptr< char >, int >(buffer, firstSize));
      for (int i = firstSection + 1; i < endSection; ++i) {
        rstl::auto_ptr< char > section(buffer.get() + offset);
        section.release();
        const int size = sizes[i];
        mPostConst->GetSectionBuffers().push_back_unsafe(
            rstl::pair< rstl::auto_ptr< char >, int >(section, size));
        offset += size;
      }
      mPostConst->mLoadedSectionCount = endSection;
      if (endSection == partCount) {
        mPostConst->mMreaSize = mPostConst->mMreaDataOffset;
        mPhase = kP_WaitForData;
      }
    } else {
      const int* sizes =
          reinterpret_cast< const int* >(mPostConst->GetSectionBuffers()[1].first.get());
      const SObjectTag tag('MREA', mAreaAssetId);
      const SMreaCompressedBlock& block = reinterpret_cast< const SMreaCompressedBlock* >(
          mPostConst->GetSectionBuffers()[2]
              .first.get())[mPostConst->mLoadedBlockCount];
      bool load = true;
      if (block.mSectionCount == 1) {
        const uint scriptStart = GetSectionIndex(1) - 2;
        const uint section = mPostConst->mLoadedSectionCount;
        const int scriptCount = reinterpret_cast< const int* >(
            mPostConst->GetSectionBuffers()[0].first.get())[15];
        if (section >= scriptStart && section < scriptStart + scriptCount) {
          const int layer = section - scriptStart;
          if (mPostConst->mFirstScriptSection == -1) {
            mPostConst->mFirstScriptSection = mPostConst->GetSectionBuffers().size();
          }
          if (scriptCount != mPostConst->mLayerFileOffsets.size()) {
            mPostConst->mLayerFileOffsets.resize(scriptCount, 0u);
          }
          if (!mPostConst->GetActiveLayers()[layer]) {
            load = false;
          }
          mPostConst->mLayerFileOffsets[layer] = mPostConst->mMreaDataOffset;
        }
      }

      char* memory;
      if (load) {
        const int bufferSize = block.mBufferSize;
        memory = static_cast< char* >(CMemory::Alloc(bufferSize, IAllocator::kHI_RoundUpLen));
      } else {
        memory = nullptr;
      }
      rstl::auto_ptr< char > buffer(memory);
      const int compressedSize = ALIGN_UP(block.mCompressedSize, 32);
      const int readSize = compressedSize ? compressedSize : ALIGN_UP(block.mDecompressedSize, 32);
      if (load) {
        mPostConst->mLoadTransactions.push_back(
            gpResourceFactory->GetResLoader().LoadResourcePartAsync(
                tag, mPostConst->mMreaDataOffset, readSize,
                buffer.get() + block.mBufferSize - readSize));
        if (compressedSize != 0) {
          mPostConst->mDecompressionRequests.push_back(
              SDecompressionRequest(mPostConst->mLoadTransactions.back().get(),
                                    reinterpret_cast< uchar* >(buffer.get()),
                                    reinterpret_cast< const uchar* >(
                                        buffer.get() + block.mBufferSize - block.mCompressedSize),
                                    block.mCompressedSize, block.mDecompressedSize));
        }
      }
      mPostConst->mMreaDataOffset += readSize;
      const int firstSize = sizes[mPostConst->mLoadedSectionCount];
      int offset = firstSize;
      mPostConst->GetSectionBuffers().push_back_unsafe(
          rstl::pair< rstl::auto_ptr< char >, int >(buffer, firstSize));
      for (int i = 1; i < block.mSectionCount; ++i) {
        rstl::auto_ptr< char > section(buffer.get() + offset);
        section.release();
        const int size = sizes[mPostConst->mLoadedSectionCount + i];
        mPostConst->GetSectionBuffers().push_back_unsafe(
            rstl::pair< rstl::auto_ptr< char >, int >(section, size));
        offset += size;
      }
      mPostConst->mLoadedSectionCount += block.mSectionCount;
      ++mPostConst->mLoadedBlockCount;
      if (mPostConst->mLoadedSectionCount == GetNumPartSizes()) {
        mPostConst->mMreaSize = mPostConst->mMreaDataOffset;
        mPhase = kP_WaitForData;
      }
    }
    break;
  }
  case kP_WaitForData:
    CullDeadAreaRequests();
    DecompressAreaData();
    if (mPostConst->mLoadTransactions.empty() &&
        mPostConst->mDecompressionRequests.empty()) {
      mPhase = kP_WaitForValidation;
    }
    break;
  case kP_WaitForValidation:
    if (!mValidationPaused) {
      mPhase = kP_FinishDependencies;
    }
    break;
  case kP_FinishDependencies:
    FinishDependencyLoading(mgr);
    mPhase = kP_PostConstruct;
    break;
  case kP_PostConstruct:
    PostConstructArea();
    PrepareScriptObjects(mgr);
    mPhase = kP_LoadScriptObjects;
    break;
  case kP_LoadScriptObjects:
    if (LoadScriptObjects(mgr)) {
      mPhase = kP_FinishScriptObjects;
    }
    break;
  case kP_FinishScriptObjects:
    mPhase = kP_Loaded;
    FinishScriptObjects(mgr);
    if (mSelfIdx != kInvalidAreaId) {
      mgr.World()->MoveAreaToChain3(mSelfIdx);
    }
    mgr.AreaLoaded(GetId());
    return true;
  case kP_Unknown14:
    mPhase = kP_Unknown15;
    break;
  case kP_Loaded:
    return true;
  }
  return false;
}

void CGameArea::DecompressAreaData() {
  if (mPostConst->mDecompressionRequests.empty()) {
    return;
  }

  uint decompressed = 0;
  while (!mPostConst->mDecompressionRequests.empty() && decompressed + 0x4000 <= 0x18000) {
    SDecompressionRequest& request = mPostConst->mDecompressionRequests.front();
    if (request.mRequest) {
      break;
    }

    const short chunkSize = *reinterpret_cast< const short* >(request.mInput);
    uint outputSize = request.mRemainingSize;
    if (chunkSize < 0) {
      const int uncompressedSize = -chunkSize;
      memcpy(request.mOutput, request.mInput + 2, uncompressedSize);
      outputSize = uncompressedSize;
    } else {
      CLZOSupport::Inflate(request.mInput + 2, chunkSize, request.mOutput, outputSize);
    }
    DCFlushRange(request.mOutput, outputSize);

    request.mOutput += outputSize;
    request.mRemainingSize -= outputSize;
    request.mInput += abs(chunkSize) + 2;
    decompressed += outputSize;
    if (request.mRemainingSize == 0) {
      mPostConst->mDecompressionRequests.pop_front();
    }
  }
}

int CGameArea::SetChain(CGameArea* next, int chain) {
  if (mCurrentChain == chain) {
    return mCurrentChain;
  }

  if (mPrev) {
    mPrev->mNext = mNext;
  }
  if (mNext) {
    mNext->mPrev = mPrev;
  }
  mPrev = nullptr;
  mNext = next;
  if (next) {
    next->mPrev = this;
  }
  const int oldChain = mCurrentChain;
  mCurrentChain = chain;
  return oldChain;
}

bool CGameArea::TransferARAMTokensOver(EARAMTransfer mode) {
  if (mPostConst->mModelsInMram) {
    return true;
  }

  bool finished = true;
  int part = mPostConst->mFirstAramSection;
  for (rstl::vector< rstl::pair< CARAMToken, int > >::iterator it =
           mPostConst->mAramTokens.begin();
       it != mPostConst->mAramTokens.end(); ++it) {
    if (it->first.GetStatus() != CARAMToken::kS_One) {
      mPostConst->mAramBytes -= it->first.GetSize();
    }
    if (mode == kAT_Async && !it->first.LoadToMRAM()) {
      finished = false;
    } else if (finished) {
      char* buffer = static_cast< char* >(it->first.GetMRAMSafe());
      int offset = 0;
      for (int j = 0; j < it->second; ++j) {
        rstl::auto_ptr< char > section(buffer + offset);
        section.release();
        offset += mPostConst->GetSectionBuffers()[part].second;
        mPostConst->GetSectionBuffers()[part].first = section;
        ++part;
      }
    }
  }
  mPostConst->mModelsInMram = finished;
  return finished;
}

bool CGameArea::TransferTokensToARAM() {
  bool finished = true;
  int part = mPostConst->mFirstAramSection;
  rstl::vector< rstl::pair< CARAMToken, int > >::iterator it =
      mPostConst->mAramTokens.begin();
  rstl::auto_ptr< char > empty;
  for (; it != mPostConst->mAramTokens.end(); ++it) {
    rstl::pair< CARAMToken, int >& entry = *it;
    for (int j = 0; j < entry.second; ++j) {
      mPostConst->GetSectionBuffers()[part].first = empty;
      ++part;
    }
    const CARAMToken::EStatus oldStatus = entry.first.GetStatus();
    entry.first.LoadToARAM();
    if (oldStatus == CARAMToken::kS_One && entry.first.GetStatus() != CARAMToken::kS_One) {
      mPostConst->mAramBytes += entry.first.GetSize();
    }
    if (entry.first.GetStatus() >= CARAMToken::kS_Two &&
        entry.first.GetStatus() <= CARAMToken::kS_Five) {
      finished = false;
    }
  }
  mPostConst->mModelsInMram = false;
  mPostConst->mModelsConstructed = false;
  return finished;
}

void CGameArea::AddStaticGeometry() {
  if (mPostConst->mOcclusionState != kOS_Visible) {
    mPostConst->mOcclusionFrameCount = 0;
    mPostConst->mOcclusionState = kOS_Visible;
    TransferARAMTokensOver(kAT_Blocking);
    if (!mPostConst->mModelsConstructed) {
      FillInStaticGeometry();
    }
    CPostConstructed& post = *mPostConst;
    const int areaIdx = mSelfIdx.Value();
    const CAreaRenderOctTree* tree =
        post.mRenderOctTree.valid() ? post.mRenderOctTree.get_ptr() : nullptr;
    gpRender->AddStaticGeometry(&post.mModelInstances, tree, &post.mSurfaces,
                                &post.mAmbientLightIds, &post.mAmbientLightIndices, areaIdx);
  }
}

void CGameArea::RemoveStaticGeometry() {
  if (IsLoaded() && mPostConst.get() && mPostConst->mOcclusionState != kOS_Occluded) {
    mPostConst->mOcclusionFrameCount = 0;
    mPostConst->mOcclusionState = kOS_Occluded;
    gpRender->RemoveStaticGeometry(&mPostConst->mModelInstances);
  }
}

void CGameArea::SetOcclusionState(EOcclusionState state) {
  if (IsLoaded() && state != mPostConst->mOcclusionState) {
    if (state == kOS_Occluded) {
      mPostConst->x178_2_ = true;
      mPostConst->mFinishedOccluding = false;
      RemoveStaticGeometry();
    } else {
      ReloadAllUnloadedTextures();
      AddStaticGeometry();
    }
  }
}

void CGameArea::AliveUpdate(float dt) {
  if (mPostConst->mOcclusionState == kOS_Occluded) {
    mPostConst->mOccludedTime += dt;
  } else {
    mPostConst->mOccludedTime = 0.f;
  }
  UpdateFog(dt);
  UpdateWeaponWorldLighting(dt);
  fn_80054F74();
}

void CGameArea::UpdateDynamicLayers(CStateManager& mgr) {
  typedef rstl::list< rstl::pair< int, rstl::auto_ptr< CDvdRequest > > > TRequests;
  if (!mPostConst->GetLayerLoadTransactions().empty()) {
    for (TRequests::iterator it = mPostConst->GetLayerLoadTransactions().begin();
         it != mPostConst->GetLayerLoadTransactions().end();) {
      if (it->second->IsComplete()) {
        ClearDecompressionRequest(it->second.get());
        it = mPostConst->GetLayerLoadTransactions().erase(it);
      } else {
        ++it;
      }
    }
  }
  DecompressAreaData();
  if (mPostConst->mDecompressionRequests.empty()) {
    for (int layer = 0; layer < mPostConst->GetLayerTokens().size(); ++layer) {
      UpdateLayerLoading(mgr, TLayerId(layer));
    }
  }
}

bool CGameArea::HasPendingLayerLoads() const {
  if (!mPostConst->mDecompressionRequests.empty()) {
    return true;
  }
  for (int i = 0; i < mPostConst->GetLayerTokens().size(); ++i) {
    switch (mLayerPhases[i]) {
    case kLP_Inactive:
    case kLP_Ready:
    case kLP_Active:
      break;
    default:
      return true;
    }
  }
  return false;
}

int CGameArea::GetLayerRequestCount(const TLayerId layer) const {
  int count = 0;
  typedef rstl::list< rstl::pair< int, rstl::auto_ptr< CDvdRequest > > > TRequests;
  for (TRequests::iterator it = mPostConst->GetLayerLoadTransactions().begin();
       it != mPostConst->GetLayerLoadTransactions().end(); ++it) {
    if (it->first == layer.Value()) {
      ++count;
    }
  }
  return count;
}

void CGameArea::UpdateLayerLoading(CStateManager& mgr, const TLayerId layer) {
  const int layerIdx = layer.Value();
  const int requestCount = GetLayerRequestCount(layer);
  switch (mLayerPhases[layerIdx]) {
  case kLP_RestartPending:
    if (requestCount == 0) {
      StartLayerLoad(mgr, layer);
    }
    break;
  case kLP_CancelPending:
    if (requestCount == 0) {
      ClearLayer(mgr, layer);
    }
    break;
  case kLP_Loading: {
    int pending = 0;
    rstl::vector< CToken >& tokens = mPostConst->GetLayerTokens()[layerIdx];
    for (int i = 0; i < tokens.size(); ++i) {
      CToken& token = tokens[i];
      if (token.IsLoaded()) {
        token.Lock();
        if (token.GetReferenceType() == 'TXTR') {
          TToken< CTexture > texture(token);
          CTexture* resource = texture.GetT();
          resource->MakeSwappable();
          if (mPostConst->mOcclusionState == kOS_Occluded) {
            resource->LoadToARAM();
          }
        }
      } else {
        if (!token.HasLock()) {
          gpResourceFactory->GetResLoader().FindResource(SObjectTag('MREA', mAreaAssetId));
          token.Lock();
        }
        ++pending;
      }
    }

    if (pending <= 80) {
      rstl::vector< CRELFileToken >& rels = mPostConst->GetLayerRelTokens()[layerIdx];
      for (int i = 0; i < rels.size(); ++i) {
        rels[i].Load();
        if (!rels[i].IsLoaded()) {
          ++pending;
        }
      }
      pending += requestCount;
      pending += mPostConst->mDecompressionRequests.size();
      if (pending == 0) {
        mLayerPhases[layerIdx] = kLP_Ready;
      }
    }
    break;
  }
  default:
    break;
  }
}

void CGameArea::PreRender() {
  if (IsLoaded()) {
    if (mPostConst->mOcclusionPinged) {
      mPostConst->mOcclusionPinged = false;
    } else {
      PingOcclusionState();
    }
  }
}

void CGameArea::PingOcclusionState() {
  if (mPostConst->mOcclusionState == kOS_Occluded) {
    if (mPostConst->mOcclusionFrameCount < 2) {
      ++mPostConst->mOcclusionFrameCount;
      return;
    }
    mPostConst->mOcclusionFrameCount = 3;
    if (!mPostConst->mFinishedOccluding) {
      const bool unloaded = UnloadAllloadedTextures();
      const bool transferred = TransferTokensToARAM();
      if (unloaded && transferred) {
        mPostConst->mFinishedOccluding = true;
      }
    }
  }
  mPostConst->x178_2_ = true;
}

void CGameArea::OtherAreaOcclusionChanged() {
  if (mPostConst->mOcclusionFrameCount == 3 &&
      mPostConst->mOcclusionState == kOS_Occluded) {
    const bool unloaded = UnloadAllloadedTextures();
    const bool transferred = TransferTokensToARAM();
    mPostConst->mFinishedOccluding = unloaded && transferred;
    return;
  }
  if (mPostConst->mOcclusionState == kOS_Visible) {
    ReloadAllUnloadedTextures();
  }
}

IGameArea::Dock::Dock(CInputStream& in, const CTransform4f& xf)
: mReferenceCount(0), mIsReferenced(false) {
  const int count = in.ReadInt32();
  mDockReferences.reserve(count);
  for (int i = 0; i < count; ++i) {
    const TAreaId area(in.ReadInt32());
    const short dock = in.ReadInt32();
    mDockReferences.push_back_unsafe(SDockReference(area, dock));
  }
  const int vertexCount = in.ReadInt32();
  for (int i = 0; i < vertexCount; ++i) {
    mPlaneVertices.push_back(xf * CVector3f(in));
  }
}

bool IGameArea::Dock::IsReferenced() const { return mIsReferenced; }

int IGameArea::Dock::GetReferenceCount() const { return mReferenceCount; }

void IGameArea::Dock::SetReferenceCount(int count) {
  mReferenceCount = count;
  mIsReferenced = true;
}

TAreaId IGameArea::Dock::GetConnectedAreaId(int other) const {
  return mDockReferences.empty() ? TAreaId(-1) : mDockReferences[other].mArea;
}

int IGameArea::Dock::GetOtherDockNumber(int other) const {
  return mDockReferences.empty() ? -1 : mDockReferences[other].mDock;
}

void IGameArea::Dock::SetShouldLoadOther(int other, bool should) {
  if (other < mDockReferences.size()) {
    mDockReferences[other].mLoadOther = should;
  }
}

bool IGameArea::Dock::GetShouldLoadOther(int other) const {
  if (other < mDockReferences.size()) {
    return mDockReferences[other].mLoadOther;
  }
  return false;
}

void IGameArea::Dock::SetLoadOtherBlocked(int other, bool blocked) {
  if (other < mDockReferences.size()) {
    mDockReferences[other].mLoadOtherBlocked = blocked;
  }
}

bool IGameArea::Dock::GetLoadOtherBlocked(int other) const {
  if (other < mDockReferences.size()) {
    return mDockReferences[other].mLoadOtherBlocked;
  }
  return false;
}

uchar CGameArea::CAreaObjectList::IsQualified(const CEntity& entity) {
  return mAreaId == entity.GetCurrentAreaId();
}

CGameArea::CAreaFog::CAreaFog()
: mFogMode(kRFM_None)
, mRangeCur(0.f, 1024.f)
, mRangeTarget(mRangeCur)
, mRangeDelta(0.f, 0.f)
, mColorCur(0.5f, 0.5f, 0.5f)
, mColorTarget(mColorCur)
, mColorDelta(0.f) {}

void CGameArea::CAreaFog::DisableFog() { mFogMode = kRFM_None; }

bool CGameArea::CAreaFog::IsFogDisabled() const { return mFogMode == kRFM_None; }

void CGameArea::CAreaFog::SetFogExplicit(ERglFogMode mode, const CColor& color,
                                         const CVector2f& range) {
  mFogMode = mode;
  mColorCur = mColorTarget = CVector3f(color.GetRed(), color.GetGreen(), color.GetBlue());
  mRangeCur = mRangeTarget = range;
}

void CGameArea::CAreaFog::FadeFog(ERglFogMode mode, const CColor& color, const CVector2f& range,
                                  float colorSpeed, const CVector2f& rangeSpeed) {
  if (mFogMode == kRFM_None) {
    mFogMode = mode;
    mColorCur = mColorTarget = CVector3f(color.GetRed(), color.GetGreen(), color.GetBlue());
    mRangeCur = CVector2f(range.GetY(), range.GetY());
    mRangeTarget = range;
  } else {
    mFogMode = mode;
    mColorTarget = CVector3f(color.GetRed(), color.GetGreen(), color.GetBlue());
    mRangeTarget = range;
  }
  mColorDelta = colorSpeed;
  mRangeDelta = rangeSpeed;
}

void CGameArea::CAreaFog::RollFogOut(float rangeSpeed, float colorSpeed, const CColor& color) {
  mRangeDelta = CVector2f(rangeSpeed, 2.f * rangeSpeed);
  mRangeTarget = CVector2f(4096.f, 4096.f);
  mColorDelta = colorSpeed;
  mColorTarget = CVector3f(color.GetRed(), color.GetGreen(), color.GetBlue());
}

CColor CGameArea::CAreaFog::GetColor() const {
  return CColor(mColorCur.GetX(), mColorCur.GetY(), mColorCur.GetZ(), 1.f);
}

void CGameArea::CAreaFog::Update(float dt) {
  if (mFogMode == kRFM_None) {
    return;
  }

  if (!(mColorDelta > 0.f) && mRangeDelta == CVector2f(0.f, 0.f)) {
    return;
  }

  float current[5];
  float target[5];
  float result[5];
  float step[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
  step[2] = step[1] = step[0] = mColorDelta * dt;
  step[3] = dt * mRangeDelta.GetX();
  step[4] = dt * mRangeDelta.GetY();
  current[0] = mColorCur.GetX();
  target[0] = mColorTarget.GetX();
  current[1] = mColorCur.GetY();
  target[1] = mColorTarget.GetY();
  current[2] = mColorCur.GetZ();
  target[2] = mColorTarget.GetZ();
  current[3] = mRangeCur.GetX();
  current[4] = mRangeCur.GetY();
  target[3] = mRangeTarget.GetX();
  target[4] = mRangeTarget.GetY();

  int finished = 0;
  for (int i = 0; i < 5; ++i) {
    const float cur = current[i];
    const float tar = target[i];
    const float delta = tar - cur;
    const float amount = step[i];
    if (CMath::AbsF(delta) <= amount) {
      result[i] = tar;
      ++finished;
    } else {
      result[i] = cur + CMath::FastFSel(delta, amount, -amount);
    }
  }

  if (finished == 5) {
    mColorDelta = 0.f;
    mRangeDelta = CVector2f(0.f, 0.f);
    if (result[3] == result[4]) {
      mFogMode = kRFM_None;
    }
  }
  if (result[3] > result[4]) {
    result[3] = result[4];
  }
  mColorCur = CVector3f(result[0], result[1], result[2]);
  mRangeCur = CVector2f(result[3], result[4]);
}

void CGameArea::CAreaFog::SetCurrent() const {
  gpRender->SetWorldFog(mFogMode, mRangeCur.GetX(), mRangeCur.GetY(), GetColor());
}

void CGameArea::UpdateFog(float dt) {
  if (mPostConst->mAreaFog.get()) {
    mPostConst->mAreaFog->Update(dt);
  }
}

bool CGameArea::DoesAreaNeedSkyNow() const {
  if (!mPostConst.get()) {
    return false;
  }
  if (mPostConst->mAreaAttributes) {
    return mPostConst->mAreaAttributes->GetNeedsSky();
  }
  return false;
}

int CGameArea::DoesAreaNeedEnvFx() const {
  if (!mPostConst.get()) {
    return 0;
  }
  if (!mPostConst->mAreaAttributes) {
    return 0;
  }
  if (mPostConst->mOcclusionState != kOS_Visible) {
    return 0;
  }
  return mPostConst->mAreaAttributes->GetEnvFxType();
}

bool CGameArea::TryTakingOutOfARAM() {
  if (mPostConst->mOcclusionState == kOS_Occluded) {
    mPostConst->mOcclusionPinged = true;
  }
  return TransferARAMTokensOver(kAT_Async) && ReloadAllUnloadedTextures();
}

const CTransform4f& CGameArea::IGetTM() const { return mTransform; }

CAssetId CGameArea::IGetStringTableAssetId() const { return mNameSTRG; }

uint CGameArea::IGetNumAttachedAreas() const { return mAttachedAreaIndices.size(); }

TAreaId CGameArea::IGetAttachedAreaId(int index) const {
  return TAreaId(mAttachedAreaIndices[index]);
}

bool CGameArea::IIsActive() const { return mActive; }

CAssetId CGameArea::IGetAreaAssetId() const { return mAreaAssetId; }

int CGameArea::IGetAreaSaveId() const { return mAreaSaveId; }

CDummyGameArea::CDummyGameArea(CInputStream& in, int index, int mlvlVersion)
: mSelfIdx(index), mNameSTRG(kInvalidAssetId), mTransform(CTransform4f::Identity()) {
  mNameSTRG = in.ReadInt32();
  mTransform = CTransform4f(in);
  const CAABox bounds(in);
  mAreaAssetId = in.ReadInt32();
  if (mlvlVersion > 15) {
    mAreaSaveId = in.ReadInt32();
  }
  mAttachedAreaIndices = rstl::vector< ushort >(in);
  {
    const rstl::vector< rstl::pair< CAssetId, uint > > dependencies1(in);
    const rstl::vector< rstl::pair< CAssetId, uint > > dependencies2(in);
  }
  if (mlvlVersion > 13) {
    const rstl::vector< uint > layerOffsets(in);
  }
  const int dockCount = in.ReadInt32();
  mDocks.reserve(dockCount);
  for (int i = 0; i < dockCount; ++i) {
    mDocks.push_back_unsafe(Dock(in, mTransform));
  }
  if (mlvlVersion > 18) {
    mRelModules = rstl::vector< rstl::string >(in);
    if (mlvlVersion > 20) {
      mRelOffsets = rstl::vector< int >(in);
    }
  }
  if (mlvlVersion >= 20) {
    const rstl::string name(in);
    if (mNameSTRG == kInvalidAssetId) {
      mInternalAreaName = name;
    }
  }
}

const CTransform4f& CDummyGameArea::IGetTM() const { return mTransform; }

CAssetId CDummyGameArea::IGetStringTableAssetId() const { return mNameSTRG; }

uint CDummyGameArea::IGetNumAttachedAreas() const { return mAttachedAreaIndices.size(); }

TAreaId CDummyGameArea::IGetAttachedAreaId(int index) const {
  return TAreaId(mAttachedAreaIndices[index]);
}

bool CDummyGameArea::IIsActive() const { return true; }

CAssetId CDummyGameArea::IGetAreaAssetId() const { return mAreaAssetId; }

int CDummyGameArea::IGetAreaSaveId() const { return mAreaSaveId; }

bool CGameArea::IsFinishedOccluding() const {
  if (mPostConst->mOcclusionState == kOS_Occluded) {
    return mPostConst->mFinishedOccluding;
  }
  return true;
}

rstl::pair< const uchar*, int > CGameArea::GetLayerScriptBuffer(const TLayerId layer) const {
  if (mPhase > kP_WaitForData) {
    return rstl::pair< const uchar*, int >(
        reinterpret_cast< const uchar* >(
            mPostConst->mLayerScriptBuffers[layer.Value()].get()),
        GetLayerScriptSize(layer));
  }
  return rstl::pair< const uchar*, int >(nullptr, 0);
}

rstl::pair< const uchar*, int > CGameArea::GetGeneratedScriptBuffer() const {
  if (mPhase > kP_WaitForData) {
    return rstl::pair< const uchar*, int >(
        reinterpret_cast< const uchar* >(mPostConst->mGeneratedScriptBuffer.get()),
        mPostConst->mGeneratedScriptSize);
  }
  return rstl::pair< const uchar*, int >(nullptr, 0);
}

int CGameArea::GetLayerScriptSize(const TLayerId layer) const {
  return mPhase > kP_WaitForData ? mPostConst->mLayerScriptSizes[layer.Value()] : 0;
}

void CGameArea::SetLoadPauseState(bool paused) {
  bool ready = true;
  for (int i = 0; i < mLayerPhases.size(); ++i) {
    if (mLayerPhases[i] == kLP_Loading) {
      ready = false;
      break;
    }
  }
  if (!ready) {
    mLoadPaused = paused;
    if (paused) {
      for (int layer = 0; layer < mPostConst->GetLayerTokens().size(); ++layer) {
        rstl::vector< CToken >& tokens = mPostConst->GetLayerTokens()[layer];
        for (int i = 0; i < tokens.size(); ++i) {
          if (!tokens[i].IsLoaded()) {
            tokens[i].Unlock();
          }
        }
      }
    }
  }
}

void CGameArea::SetAreaAttributes(CScriptAreaProperties* attributes) {
  mPostConst->mAreaAttributes = attributes;
}

void CGameArea::SetXRaySpeedAndTarget(float speed, float target) {
  mPostConst->mXraySpeed = speed;
  mPostConst->mXrayTarget = target;
}

void CGameArea::SetWeaponWorldLighting(float speed, float target) {
  mPostConst->mWeaponWorldLightingSpeed = speed;
  mPostConst->mWeaponWorldLightingTarget = target;
}

void CGameArea::UpdateWeaponWorldLighting(float dt) {
  float lighting = mPostConst->mWorldLightingLevel;
  if (0.f != mPostConst->mXraySpeed) {
    float delta = dt * mPostConst->mXraySpeed;
    if (CMath::AbsF(mPostConst->mXrayTarget - lighting) < delta) {
      lighting = mPostConst->mXrayTarget;
      mPostConst->mWeaponWorldLightingSpeed = 0.f;
    } else if (mPostConst->mXrayTarget < lighting) {
      lighting -= delta;
    } else {
      lighting += delta;
    }
  }

  if (0.f != mPostConst->mWeaponWorldLightingSpeed) {
    float weaponLighting = mPostConst->mWorldLightingLevel;
    float delta = dt * mPostConst->mWeaponWorldLightingSpeed;
    if (CMath::AbsF(mPostConst->mWeaponWorldLightingTarget - lighting) < delta) {
      weaponLighting = mPostConst->mWeaponWorldLightingTarget;
      mPostConst->mWeaponWorldLightingSpeed = 0.f;
    } else if (mPostConst->mWeaponWorldLightingTarget < weaponLighting) {
      weaponLighting -= delta;
    } else {
      weaponLighting += delta;
    }
    if (mPostConst->mXraySpeed != 0.f) {
      lighting = rstl::min_val(weaponLighting, lighting);
    } else {
      lighting = weaponLighting;
    }
  }

  float epsilon = 0.00001f;
  if (!(CMath::AbsF(mPostConst->mWorldLightingLevel - lighting) < epsilon)) {
    mPostConst->mWorldLightingLevel = lighting;
    CObjectList& objects = *mPostConst->mAreaObjectList;
    for (int i = objects.GetFirstObjectIndex(); i != -1; i = objects.GetNextObjectIndex(i)) {
      if (CActor* actor = TCastToPtr< CActor >(objects[i])) {
        actor->SetWorldLightingDirty(true);
      }
    }
  }
}

uint CGameArea::Get1stPVSLightFeature(uint index) const {
  const CPVSAreaSet* pvs = mPostConst->mPvs.get();
  if (!pvs || pvs->GetNumLights() == 0) {
    return uint(-1);
  }
  if (static_cast< int >(index) >= pvs->GetNumLights() - pvs->GetNum2ndLights()) {
    return uint(-1);
  }
  const int& count = pvs->GetNumFeatures();
  return count + pvs->GetNum2ndLights() + index;
}

uint CGameArea::Get2ndPVSLightFeature(uint index) const {
  const CPVSAreaSet* pvs = mPostConst->mPvs.get();
  if (!mPostConst->mPvsHasLights || !pvs) {
    return uint(-1);
  }
  const int& count = pvs->GetNumFeatures();
  return count + index;
}

void CGameArea::InitializeDocks(CStateManager& mgr) {
  bool hasUnloadedDock = false;
  for (rstl::list< TUniqueId >::iterator it = mPostConst->mDockIds.begin();
       it != mPostConst->mDockIds.end();) {
    const CScriptDock* dock = TCastToConstPtr< CScriptDock >(mgr.GetObjectById(*it));
    rstl::list< TUniqueId >::iterator current = it;
    ++it;
    if (dock && !mDocks[dock->GetDockId()].GetDockRefs().empty()) {
      if (dock->IsVirtual()) {
        mPostConst->mDockIds.erase(current);
      } else if (!dock->GetLoadConnected()) {
        hasUnloadedDock = true;
      }
    } else {
      mPostConst->mDockIds.erase(current);
    }
  }

  bool loadDynamically = true;
  if (hasUnloadedDock) {
    loadDynamically = false;
  }
  for (rstl::list< TUniqueId >::iterator it = mPostConst->mDockIds.begin();
       it != mPostConst->mDockIds.end(); ++it) {
    if (CScriptDock* dock = TCastToPtr< CScriptDock >(mgr.ObjectById(*it))) {
      if (loadDynamically) {
        dock->SetLoadConnected(false);
      }
      dock->InitializeConnectedArea(mgr);
    }
  }
  if (!loadDynamically) {
    mPostConst->mDockIds.clear();
  }
}

void CGameArea::UpdateDocks(CStateManager& mgr) {
  if (mPostConst->mDockIds.empty() || mPostConst->mDocksDisabled) {
    return;
  }

  float nearestDistance = FLT_MAX;
  float secondDistance = FLT_MAX;
  CScriptDock* nearest = nullptr;
  CScriptDock* second = nullptr;
  const CPlayer& player = *mgr.GetPlayer(0);
  const TAreaId previousArea = mgr.GetPreviousAreaId();
  for (rstl::list< TUniqueId >::iterator it = mPostConst->mDockIds.begin();
       it != mPostConst->mDockIds.end(); ++it) {
    if (CScriptDock* dock = TCastToPtr< CScriptDock >(mgr.ObjectById(*it))) {
      float distance = (player.GetTranslation() - dock->GetTranslation()).MagSquared();
      const Dock& areaDock = mDocks[dock->GetDockId()];
      if (areaDock.GetConnectedAreaId(areaDock.GetReferenceCount()) == previousArea) {
        distance *= 1.5f;
      }
      if (distance < nearestDistance) {
        secondDistance = nearestDistance;
        second = nearest;
        nearestDistance = distance;
        nearest = dock;
      } else if (distance < secondDistance) {
        second = dock;
        secondDistance = distance;
      }
    }
  }

  const Dock& nearestDock = mDocks[nearest->GetDockId()];
  if (nearestDock.GetShouldLoadOther(nearestDock.GetReferenceCount())) {
    return;
  }
  if (second) {
    const Dock& secondDock = mDocks[second->GetDockId()];
    if (secondDock.GetShouldLoadOther(nearestDock.GetReferenceCount())) {
      float nearestLength = CMath::SqrtF(nearestDistance);
      float secondLength = CMath::SqrtF(secondDistance);
      float difference = CMath::AbsF(nearestLength - secondLength);
      if (difference < 2.f ||
          (rstl::max_val(nearestLength, secondLength) > 20.f && difference < 5.f)) {
        return;
      }
    }
  }

  for (rstl::list< TUniqueId >::iterator it = mPostConst->mDockIds.begin();
       it != mPostConst->mDockIds.end(); ++it) {
    if (const CScriptDock* dock = TCastToConstPtr< CScriptDock >(mgr.GetObjectById(*it))) {
      if (dock != nearest) {
        mgr.SendScriptMsg(CScriptMsg(kInvalidUniqueId, dock->GetUniqueId(), kSM_SetToZero));
      }
    }
  }
  mgr.SendScriptMsg(CScriptMsg(kInvalidUniqueId, nearest->GetUniqueId(), kSM_SetToMax));
}

void CGameArea::fn_80054F74() {}

int CGameArea::GetTokenCount() const {
  int count = 0;
  if (mPostConst.get()) {
    for (int i = 0; i < mPostConst->GetLayerTokens().size(); ++i) {
      count += mPostConst->GetLayerTokens()[i].size();
    }
  }
  return count;
}

CGameArea::ELayerPhase CGameArea::GetLayerPhase(const TLayerId layer) const {
  if (layer.Value() < mLayerPhases.size() && layer.Value() >= 0) {
    return mLayerPhases[layer.Value()];
  }
  return kLP_Inactive;
}

rstl::vector< CRELFileToken >* CGameArea::GetLayerRelTokens(const TLayerId layer) const {
  if (mPostConst.get() && layer.Value() >= 0 &&
      layer.Value() < mPostConst->GetLayerRelTokens().size()) {
    return &mPostConst->GetLayerRelTokens()[layer.Value()];
  }
  return nullptr;
}

bool CGameArea::IsValidLayerNumber(CStateManager& mgr, const TLayerId layer) const {
  const int layerCount = mgr.mCurrentWorldLayerState->GetLayerCount(mSelfIdx);
  if (layer.Value() < layerCount && layer.Value() >= 0) {
    return true;
  }
  return false;
}

void CGameArea::LoadLayerDynamic(CStateManager& mgr, const TLayerId layer) {
  CWorldLayerState& layers = *mgr.mCurrentWorldLayerState;
  if (!IsValidLayerNumber(mgr, layer)) {
    return;
  }
  if (!layers.IsLayerActive(mSelfIdx, layer)) {
    return;
  }

  const int layerIdx = layer.Value();
  const ELayerPhase phase = mLayerPhases[layerIdx];
  switch (phase) {
  case kLP_Inactive:
    StartLayerLoad(mgr, layer);
    break;
  case kLP_CancelPending:
    mLayerPhases[layerIdx] = kLP_RestartPending;
    break;
  default:
    break;
  }
}

void CGameArea::StartLayerLoad(CStateManager& mgr, const TLayerId layer) {
  if (!IsValidLayerNumber(mgr, layer)) {
    return;
  }

  const int layerIdx = layer.Value();
  const int first = mLayerDependencyOffsets[layerIdx];
  const int last = layerIdx + 1 < mLayerDependencyOffsets.size()
                       ? mLayerDependencyOffsets[layerIdx + 1]
                       : mDependencies2.size();
  const int count = last - first;
  rstl::vector< CToken >& tokens = mPostConst->GetLayerTokens()[layerIdx];
  if (tokens.capacity() == 0) {
    tokens.reserve(count);
  }
  AddLayerTokens(layerIdx, mPostConst->GetLayerTokens()[layerIdx]);

  if (mPostConst->mMreaVersion < 24) {
    const SObjectTag tag('MREA', mAreaAssetId);
    const int size = GetLayerScriptSize(layer);
    rstl::auto_ptr< char > buffer(
        static_cast< char* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen)));
    mPostConst->GetLayerLoadTransactions().push_back(
        rstl::pair< int, rstl::auto_ptr< CDvdRequest > >(
            layer.Value(),
            gpResourceFactory->GetResLoader().LoadResourcePartAsync(
                tag, mPostConst->mLayerFileOffsets[layerIdx], size, buffer.get())));
    mPostConst->GetSectionBuffers()[layerIdx + mPostConst->mFirstScriptSection] =
        rstl::pair< rstl::auto_ptr< char >, int >(buffer, size);
    mPostConst->mLayerScriptBuffers[layerIdx] = rstl::auto_ptr< char >(buffer.get());
    mPostConst->mLayerScriptBuffers[layerIdx].release();
  } else {
    rstl::auto_ptr< char > buffer;
    rstl::auto_ptr< CDvdRequest > request;
    const int size = GetLayerScriptSize(layer);
    ReadCompressedLayer(mPostConst->mLayerFileOffsets[layerIdx], request, buffer);
    mPostConst->GetLayerLoadTransactions().push_back(
        rstl::pair< int, rstl::auto_ptr< CDvdRequest > >(layer.Value(), request));
    mPostConst->GetSectionBuffers()[layerIdx + mPostConst->mFirstScriptSection] =
        rstl::pair< rstl::auto_ptr< char >, int >(buffer, size);
    mPostConst->mLayerScriptBuffers[layerIdx] = rstl::auto_ptr< char >(buffer.get());
    mPostConst->mLayerScriptBuffers[layerIdx].release();
  }

  mgr.World()->CancelLayerRelUnload(mSelfIdx, layer);
  LoadLayerRelModules(mgr, layer);
}

void CGameArea::RemoveLayerObjects(CStateManager& mgr, const TLayerId layer) {
  CScriptObjectLoaderHelper& loader = mgr.ScriptObjectLoaderHelper();
  loader.RemoveLayerObjects(mSelfIdx, layer, mgr);
  mgr.World()->QueueLayerRelUnload(CWorld::SLayerRelUnload(mSelfIdx, layer, 2));
}

void CGameArea::ClearLayer(CStateManager& mgr, const TLayerId layer) {
  if (!IsValidLayerNumber(mgr, layer)) {
    return;
  }

  const int layerIdx = layer.Value();
  RemoveLayerObjects(mgr, layer);
  mPostConst->GetLayerTokens()[layerIdx] = rstl::vector< CToken >();
  mPostConst->GetSectionBuffers()[layerIdx + mPostConst->mFirstScriptSection].first =
      rstl::auto_ptr< char >();
  mPostConst->mLayerScriptBuffers[layerIdx] = rstl::auto_ptr< char >();
  mLayerPhases[layerIdx] = kLP_Inactive;
}

void CGameArea::UnloadLayerDynamic(CStateManager& mgr, const TLayerId layer) {
  if (!IsValidLayerNumber(mgr, layer)) {
    return;
  }

  const int layerIdx = layer.Value();
  const ELayerPhase phase = mLayerPhases[layerIdx];
  bool cleared = false;
  switch (phase) {
  case kLP_RestartPending:
  case kLP_Loading:
    mLayerPhases[layerIdx] = kLP_CancelPending;
    break;
  case kLP_Ready:
  case kLP_Active:
    ClearLayer(mgr, layer);
    cleared = true;
    break;
  default:
    break;
  }
  if (!cleared) {
    RemoveLayerObjects(mgr, layer);
  }
}

void CGameArea::ActivateLayerDynamic(CStateManager& mgr, const TLayerId layer) {
  if (!IsValidLayerNumber(mgr, layer)) {
    return;
  }

  CScriptObjectLoaderHelper& loader = mgr.ScriptObjectLoaderHelper();
  const CWorldLayerState& layers = *mgr.mCurrentWorldLayerState;
  if (!layers.IsLayerActive(mSelfIdx, layer)) {
    return;
  }

  const int layerIdx = layer.Value();
  layers.GetLayerCount(mSelfIdx);
  rstl::vector< TEditorId >& ids = mPostConst->mLayerEditorIds[layerIdx];
  ids = rstl::vector< TEditorId >();
  const rstl::pair< const uchar*, int > buffer = GetLayerScriptBuffer(layer);
  CMemoryInStream in(buffer.first, buffer.second);
  mPostConst->mScriptObjectsInitialized = false;
  loader.LoadScriptObjects(GetId(), in, ids, mgr);
  loader.InitScriptObjects(ids, mgr);
  mPostConst->mScriptObjectsInitialized = true;

  mPostConst->GetSectionBuffers()[layerIdx + mPostConst->mFirstScriptSection].first =
      rstl::auto_ptr< char >();
  mPostConst->mLayerScriptBuffers[layerIdx] = rstl::auto_ptr< char >();
  mLayerPhases[layerIdx] = kLP_Active;
}

void CGameArea::LoadLayerRelModules(CStateManager& mgr, const TLayerId layer) {
  const int layerIdx = layer.Value();
  const int first = mRelOffsets[layerIdx * 2];
  const int last = mRelOffsets[layerIdx * 2 + 1];
  const int count = last - first;
  rstl::vector< CRELFileToken >& tokens = mPostConst->GetLayerRelTokens()[layerIdx];
  if (count > 0 && count != tokens.size()) {
    mLayerPhases[layerIdx] = kLP_Loading;
    tokens.clear();
    tokens.reserve(count);
    for (int i = first; i < last; ++i) {
      tokens.push_back_unsafe(CRELFileToken(mRelModules[i], 1));
    }
  }
}

void CGameArea::SortTextureDependencies() {
  rstl::vector< int > indices;
  rstl::vector< uint > offsets;
  const rstl::vector< rstl::pair< CAssetId, uint > > dependencies(mDependencies2);
  const SObjectTag areaTag('MREA', mAreaAssetId);
  indices.reserve(mDependencies2.size());
  offsets.reserve(mDependencies2.size());

  for (int layer = 0; layer < mLayerDependencyOffsets.size(); ++layer) {
    const int first = mLayerDependencyOffsets[layer];
    const int last = layer + 1 < mLayerDependencyOffsets.size() ? mLayerDependencyOffsets[layer + 1]
                                                                : mDependencies2.size();
    for (int i = first; i < last; ++i) {
      const rstl::pair< CAssetId, uint >& dependency = mDependencies2[i];
      if (dependency.second == 'TXTR') {
        int end = i + 1;
        for (; end < last; ++end) {
          if (mDependencies2[end].second != 'TXTR') {
            break;
          }
        }
        if (end - i > 1) {
          for (int j = i; j < end; ++j) {
            gpResourceFactory->GetResLoader().FindResource(areaTag);
            const SObjectTag tag(mDependencies2[j].second, mDependencies2[j].first);
            indices.push_back_unsafe(j - i);
            offsets.push_back_unsafe(gpResourceFactory->GetResLoader().GetResourceOffset(tag));
          }
          rstl::sort(indices.begin(), indices.end(), CTextureDependencySorter(offsets));
          for (int j = i; j < end; ++j) {
            mDependencies2[j] = dependencies[i + indices[j - i]];
          }
          offsets.clear();
          indices.clear();
          i = end - 1;
        }
      }
    }
  }
}

void CGameArea::DisableDocks(CStateManager& mgr) {
  if (!mPostConst->mDockIds.empty()) {
    mPostConst->mDocksDisabled = true;
    for (rstl::list< TUniqueId >::const_iterator it = mPostConst->mDockIds.begin();
         it != mPostConst->mDockIds.end(); ++it) {
      mgr.SendScriptMsg(*it, kInvalidUniqueId, kSM_SetToZero, kInvalidUniqueId);
    }
  }
}

void CGameArea::EnableDocks() {
  if (!mPostConst->mDockIds.empty()) {
    mPostConst->mDocksDisabled = false;
  }
}

void CGameArea::ClearDecompressionRequest(CDvdRequest* request) {
  for (rstl::list< SDecompressionRequest >::iterator it =
           mPostConst->mDecompressionRequests.begin();
       it != mPostConst->mDecompressionRequests.end(); ++it) {
    if (it->mRequest == request) {
      it->mRequest = nullptr;
    }
  }
}

void CGameArea::ReadCompressedLayer(const int offset, rstl::auto_ptr< CDvdRequest >& request,
                                    rstl::auto_ptr< char >& buffer) {
  const uint* header =
      reinterpret_cast< const uint* >(mPostConst->GetSectionBuffers().front().first.get());
  if (mPostConst->mMreaVersion < 24) {
    return;
  }

  const SMreaCompressedBlock* blocks = reinterpret_cast< const SMreaCompressedBlock* >(
      mPostConst->GetSectionBuffers()[2].first.get());
  uint blockOffset = 0;
  for (int i = 0; i < 3; ++i) {
    blockOffset += mPostConst->GetSectionBuffers()[i].second;
  }
  const int blockCount = header[28];
  for (int i = 0; i < blockCount; ++i) {
    const SMreaCompressedBlock& block = blocks[i];
    const int readSize =
        ALIGN_UP(block.mCompressedSize ? block.mCompressedSize : block.mBufferSize, 32);
    if (offset == blockOffset) {
      const int bufferSize = block.mBufferSize;
      buffer = rstl::auto_ptr< char >(
          static_cast< char* >(CMemory::Alloc(bufferSize, IAllocator::kHI_RoundUpLen)));
      const SObjectTag tag('MREA', mAreaAssetId);
      request = gpResourceFactory->GetResLoader().LoadResourcePartAsync(
          tag, offset, readSize, buffer.get() + block.mBufferSize - readSize);
      if (block.mCompressedSize) {
        mPostConst->mDecompressionRequests.push_back(
            SDecompressionRequest(request.get(), reinterpret_cast< uchar* >(buffer.get()),
                                  reinterpret_cast< const uchar* >(
                                      buffer.get() + block.mBufferSize - block.mCompressedSize),
                                  block.mCompressedSize, block.mDecompressedSize));
      }
      break;
    }
    blockOffset += readSize;
  }
}

rstl::string CDummyGameArea::IGetInternalAreaName() const { return mInternalAreaName; }
