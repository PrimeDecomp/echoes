#include "MetroidPrime/CGameArea.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CARAMManager.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/CMemoryStreamOut.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAreaProperties.hpp"

// The complex loading paths below remain scaffolds. This TU is NonMatching.

rstl::string CGameArea::IGetInternalAreaName() const { return mInternalAreaName; }

IGameArea::~IGameArea() {}

int CGameArea::VerifyHeader() const {
  if (!mPostConstructed->mMreaSectionBuffers.empty()) {
    const uint* header =
        reinterpret_cast< const uint* >(mPostConstructed->mMreaSectionBuffers.front().first.get());
    if (header[0] == 0xdeadbeef && header[1] >= 23 && header[1] <= 25) {
      return header[1];
    }
  }
  return 0;
}

int CGameArea::GetSectionIndex(int section) const {
  if (VerifyHeader() < 11) {
    return -1;
  }

  const uint* header =
      reinterpret_cast< const uint* >(mPostConstructed->mMreaSectionBuffers.front().first.get());
  if (section >= 0 && section < 8) {
    return header[17 + section] + 2;
  }
  if (section == 8 || section == 9) {
    return header[26 + section - 8] + 2;
  }
  return -1;
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
, mPostConstructed(nullptr)
, mLoadPaused(false)
, mValidationPaused(false)
, mActive(true)
, x108_3_(false) {
  mBounds = mBounds.GetTransformedAABox(mTransform);
  mLayerDependencyOffsets = rstl::vector< uint >(in);

  const int dockCount = in.ReadInt32();
  mDocks.reserve(dockCount);
  for (int i = 0; i < dockCount; ++i) {
    mDocks.push_back(Dock(in, mTransform));
  }

  if (mlvlVersion > 18) {
    mRelModules = rstl::vector< rstl::string >(in);
    if (mlvlVersion > 20) {
      mRelOffsets = rstl::vector< uint >(in);
    }
  }
  if (mlvlVersion > 19) {
    const rstl::string name(in);
    if (mNameSTRG == kInvalidAssetId) {
      mInternalAreaName = name;
    }
  }

  SortTextureDependencies();
  mSerializedDependencySize = CalculateDependencyListByteCount();
  mDependenciesInAram = CARAMManager::Alloc(mSerializedDependencySize);
  {
    rstl::auto_ptr< uchar > buffer(static_cast< uchar* >(
        CMemory::Alloc(mSerializedDependencySize, IAllocator::kHI_RoundUpLen)));
    {
      CMemoryStreamOut out(buffer.get(), mSerializedDependencySize, CMemoryStreamOut::kOS_NotOwned,
                           64);
      mDependencies2.PutTo(out);
    }

    const uint handle = CARAMManager::DMAToARAM(
        buffer.get(), mDependenciesInAram, mSerializedDependencySize, CARAMManager::kDMAPrio_One);
    mDependencies2 = rstl::vector< rstl::pair< CAssetId, uint > >();
    CARAMManager::WaitForDMACompletion(handle);
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
  if (mPostConstructed.get()) {
    mPostConstructed->mLayerTokens.clear();
    mPostConstructed->mLayerRelTokens.clear();
    mPostConstructed->mSortedRelTokens.clear();
  }
}

void CGameArea::AddLayerTokens(int layer, rstl::vector< CToken >& tokens) {
  // TODO: Populate this layer's dependency tokens, including the missing-AGSC fallback.
}

bool CGameArea::UpdateDependencyLoading(CStateManager& mgr) {
  // TODO: Poll active layers, throttle outstanding resources, and load sorted REL tokens.
  return false;
}

void CGameArea::VerifyTokenList(CStateManager& mgr) {
  // TODO: Initialize layer phases, active-layer masks, resource tokens, and REL tokens.
}

void CGameArea::SortRelTokens() {
  // TODO: Sort pending REL tokens by file size.
}

void CGameArea::FillInStaticGeometry() {
  // TODO: Construct model instances, surface records, and ambient-light lookup arrays.
}

void CGameArea::PostConstructArea() {
  // TODO: Decode MREA sections and construct collision, geometry, lights, PVS, paths, portals, and
  // object lists.
}

void CGameArea::FinishDependencyLoading(CStateManager& mgr) {
  // TODO: Wait for outstanding requests, decompression, REL modules, and resource tokens.
}

void CGameArea::PrepareScriptObjects(CStateManager& mgr) {
  // TODO: Allocate per-layer editor IDs and the script-loading context.
}

bool CGameArea::LoadScriptObjects(CStateManager& mgr) {
  // TODO: Incrementally instantiate active script layers within the frame budget.
  return false;
}

void CGameArea::FinishScriptObjects(CStateManager& mgr) {
  // TODO: Initialize script objects and generated objects, docks, and actor PVS IDs.
}

bool CGameArea::StartStreamIn(CStateManager& mgr) {
  if (IsLoaded()) {
    return true;
  }
  if (mPostConstructed.get() && mPostConstructed->mStreamingDelay) {
    --mPostConstructed->mStreamingDelay;
    return false;
  }
  if (mLoadPaused) {
    return false;
  }
  return StartStreamingMainArea(mgr);
}

void CGameArea::Validate(CStateManager& mgr) {
  // TODO: Drive StartStreamIn while servicing the resource factory.
}

void CGameArea::CullDeadAreaRequests() {
  while (!mPostConstructed->mLoadTransactions.empty() &&
         mPostConstructed->mLoadTransactions.front()->IsComplete()) {
    ClearDecompressionRequest(mPostConstructed->mLoadTransactions.front().get());
    mPostConstructed->mLoadTransactions.pop_front();
  }
}

bool CGameArea::Invalidate(CStateManager* mgr) {
  // TODO: Cancel pending requests and unload area state and script objects.
  return true;
}

char* CGameArea::AllocNewAreaData(int offset, int size) {
  // TODO: Allocate a section buffer and queue its asynchronous MREA read.
  return nullptr;
}

uint CGameArea::CalculateDependencyListByteCount() const {
  return ALIGN_UP(4 + mDependencies2.size() * 8, 32);
}

int CGameArea::GetNumPartSizes() const {
  const uint* header =
      reinterpret_cast< const uint* >(mPostConstructed->mMreaSectionBuffers.front().first.get());
  return header[16];
}

int CGameArea::GetNumCompressedBlocks() const {
  const uint* header =
      reinterpret_cast< const uint* >(mPostConstructed->mMreaSectionBuffers.front().first.get());
  return header[1] < 24 ? 0 : header[28];
}

bool CGameArea::ReloadAllUnloadedTextures() {
  bool finished = true;

  for (int layer = 0; layer < mPostConstructed->mLayerTokens.size(); ++layer) {
    rstl::vector< CToken >& tokens = mPostConstructed->mLayerTokens[layer];
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

  for (int layer = 0; layer < mPostConstructed->mLayerTokens.size(); ++layer) {
    rstl::vector< CToken >& tokens = mPostConstructed->mLayerTokens[layer];
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
  // TODO: Restore the dependency, section-read, decompression, and script-loading state machine.
  return IsLoaded();
}

void CGameArea::DecompressAreaData() {
  // TODO: Process completed requests and decompress at most the target frame budget.
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
  if (mPostConstructed->mModelsInMram) {
    return true;
  }

  bool finished = true;
  int part = mPostConstructed->mFirstAramSection;
  for (int i = 0; i < mPostConstructed->mAramTokens.size(); ++i) {
    rstl::pair< CARAMToken, int >& entry = mPostConstructed->mAramTokens[i];
    if (entry.first.GetStatus() != CARAMToken::kS_One) {
      mPostConstructed->mAramBytes -= entry.first.GetSize();
    }
    if (mode == kAT_Async && !entry.first.LoadToMRAM()) {
      finished = false;
    } else if (finished) {
      char* buffer = static_cast< char* >(entry.first.GetMRAMSafe());
      int offset = 0;
      for (int j = 0; j < entry.second; ++j) {
        rstl::auto_ptr< char > section(buffer + offset);
        section.release();
        offset += mPostConstructed->mMreaSectionBuffers[part].second;
        mPostConstructed->mMreaSectionBuffers[part++].first = section;
      }
    }
  }
  mPostConstructed->mModelsInMram = finished;
  return finished;
}

bool CGameArea::TransferTokensToARAM() {
  bool finished = true;
  int part = mPostConstructed->mFirstAramSection;
  rstl::auto_ptr< char > empty;
  for (int i = 0; i < mPostConstructed->mAramTokens.size(); ++i) {
    rstl::pair< CARAMToken, int >& entry = mPostConstructed->mAramTokens[i];
    for (int j = 0; j < entry.second; ++j) {
      mPostConstructed->mMreaSectionBuffers[part++].first = empty;
    }
    const CARAMToken::EStatus oldStatus = entry.first.GetStatus();
    entry.first.LoadToARAM();
    if (oldStatus == CARAMToken::kS_One && entry.first.GetStatus() != CARAMToken::kS_One) {
      mPostConstructed->mAramBytes += entry.first.GetSize();
    }
    if (entry.first.GetStatus() >= CARAMToken::kS_Two &&
        entry.first.GetStatus() <= CARAMToken::kS_Five) {
      finished = false;
    }
  }
  mPostConstructed->mModelsInMram = false;
  mPostConstructed->mModelsConstructed = false;
  return finished;
}

void CGameArea::AddStaticGeometry() {
  if (mPostConstructed->mOcclusionState != kOS_Visible) {
    mPostConstructed->mOcclusionFrameCount = 0;
    mPostConstructed->mOcclusionState = kOS_Visible;
    TransferARAMTokensOver(kAT_Blocking);
    if (!mPostConstructed->mModelsConstructed) {
      FillInStaticGeometry();
    }
    CPostConstructed& post = *mPostConstructed;
    gpRender->AddStaticGeometry(
        &post.mModelInstances,
        post.mRenderOctTree.valid() ? post.mRenderOctTree.get_ptr() : nullptr, &post.mSurfaces,
        &post.mAmbientLightIds, &post.mAmbientLightIndices, mSelfIdx.Value());
  }
}

void CGameArea::RemoveStaticGeometry() {
  if (IsLoaded() && mPostConstructed.get() && mPostConstructed->mOcclusionState != kOS_Occluded) {
    mPostConstructed->mOcclusionFrameCount = 0;
    mPostConstructed->mOcclusionState = kOS_Occluded;
    gpRender->RemoveStaticGeometry(&mPostConstructed->mModelInstances);
  }
}

void CGameArea::SetOcclusionState(EOcclusionState state) {
  if (IsLoaded() && state != mPostConstructed->mOcclusionState) {
    if (state == kOS_Occluded) {
      mPostConstructed->x178_2_ = true;
      mPostConstructed->mFinishedOccluding = false;
      RemoveStaticGeometry();
    } else {
      ReloadAllUnloadedTextures();
      AddStaticGeometry();
    }
  }
}

void CGameArea::AliveUpdate(float dt) {
  if (mPostConstructed->mOcclusionState == kOS_Occluded) {
    mPostConstructed->mOccludedTime += dt;
  } else {
    mPostConstructed->mOccludedTime = 0.f;
  }
  UpdateFog(dt);
  UpdateWeaponWorldLighting(dt);
  fn_80054F74();
}

void CGameArea::UpdateDynamicLayers(CStateManager& mgr) {
  typedef rstl::list< rstl::pair< int, rstl::auto_ptr< CDvdRequest > > > TRequests;
  TRequests& requests = mPostConstructed->mLayerLoadTransactions;
  for (TRequests::iterator it = requests.begin(); it != requests.end();) {
    if (it->second->IsComplete()) {
      ClearDecompressionRequest(it->second.get());
      it = requests.erase(it);
    } else {
      ++it;
    }
  }
  DecompressAreaData();
  if (mPostConstructed->mDecompressionRequests.empty()) {
    for (int layer = 0; layer < mPostConstructed->mLayerTokens.size(); ++layer) {
      UpdateLayerLoading(mgr, TLayerId(layer));
    }
  }
}

bool CGameArea::HasPendingLayerLoads() const {
  if (!mPostConstructed->mDecompressionRequests.empty()) {
    return true;
  }
  for (int i = 0; i < mPostConstructed->mLayerTokens.size(); ++i) {
    const ELayerPhase phase = mLayerPhases[i];
    if (phase != kLP_Inactive && phase != kLP_Ready && phase != kLP_Active) {
      return true;
    }
  }
  return false;
}

int CGameArea::GetLayerRequestCount(const TLayerId& layer) const {
  int count = 0;
  typedef rstl::list< rstl::pair< int, rstl::auto_ptr< CDvdRequest > > > TRequests;
  const TRequests& requests = mPostConstructed->mLayerLoadTransactions;
  for (TRequests::const_iterator it = requests.begin(); it != requests.end(); ++it) {
    if (it->first == layer.Value()) {
      ++count;
    }
  }
  return count;
}

void CGameArea::UpdateLayerLoading(CStateManager& mgr, const TLayerId& layer) {
  // TODO: Advance cancellation/restart/loading phases, polling resources and REL tokens.
}

void CGameArea::PreRender() {
  if (IsLoaded()) {
    if (mPostConstructed->mOcclusionPinged) {
      mPostConstructed->mOcclusionPinged = false;
    } else {
      PingOcclusionState();
    }
  }
}

void CGameArea::PingOcclusionState() {
  if (mPostConstructed->mOcclusionState == kOS_Occluded) {
    if (mPostConstructed->mOcclusionFrameCount < 2) {
      ++mPostConstructed->mOcclusionFrameCount;
      return;
    }
    mPostConstructed->mOcclusionFrameCount = 3;
    if (!mPostConstructed->mFinishedOccluding) {
      const bool unloaded = UnloadAllloadedTextures();
      const bool transferred = TransferTokensToARAM();
      if (unloaded && transferred) {
        mPostConstructed->mFinishedOccluding = true;
      }
    }
  }
  mPostConstructed->x178_2_ = true;
}

void CGameArea::OtherAreaOcclusionChanged() {
  if (mPostConstructed->mOcclusionFrameCount == 3 &&
      mPostConstructed->mOcclusionState == kOS_Occluded) {
    const bool unloaded = UnloadAllloadedTextures();
    const bool transferred = TransferTokensToARAM();
    mPostConstructed->mFinishedOccluding = unloaded && transferred;
    return;
  }
  if (mPostConstructed->mOcclusionState == kOS_Visible) {
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
    mDockReferences.push_back(SDockReference(area, dock));
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

void IGameArea::Dock::fn_80056EE0(int other, bool value) {
  if (other < mDockReferences.size()) {
    mDockReferences[other].x6_1_ = value;
  }
}

bool IGameArea::Dock::fn_80056EA0(int other) const {
  if (other < mDockReferences.size()) {
    return mDockReferences[other].x6_1_;
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
  if (mFogMode == kRFM_None || (mColorDelta <= 0.f && mRangeDelta == CVector2f(0.f, 0.f))) {
    return;
  }

  const float current[5] = {mColorCur.GetX(), mColorCur.GetY(), mColorCur.GetZ(), mRangeCur.GetX(),
                            mRangeCur.GetY()};
  const float target[5] = {mColorTarget.GetX(), mColorTarget.GetY(), mColorTarget.GetZ(),
                           mRangeTarget.GetX(), mRangeTarget.GetY()};
  const float step[5] = {mColorDelta * dt, mColorDelta * dt, mColorDelta * dt,
                         dt * mRangeDelta.GetX(), dt * mRangeDelta.GetY()};
  float result[5];
  int finished = 0;

  for (int i = 0; i < 5; ++i) {
    const float delta = target[i] - current[i];
    if (step[i] < CMath::AbsF(delta)) {
      result[i] = current[i] + CMath::FastFSel(delta, step[i], -step[i]);
    } else {
      result[i] = target[i];
      ++finished;
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
  if (mPostConstructed->mAreaFog.get()) {
    mPostConstructed->mAreaFog->Update(dt);
  }
}

bool CGameArea::DoesAreaNeedSkyNow() const {
  if (mPostConstructed.get() && mPostConstructed->mAreaAttributes) {
    return mPostConstructed->mAreaAttributes->GetNeedsSky();
  }
  return false;
}

int CGameArea::DoesAreaNeedEnvFx() const {
  if (!mPostConstructed.get() || !mPostConstructed->mAreaAttributes ||
      mPostConstructed->mOcclusionState != kOS_Visible) {
    return 0;
  }
  return mPostConstructed->mAreaAttributes->GetEnvFxType();
}

bool CGameArea::TryTakingOutOfARAM() {
  if (mPostConstructed->mOcclusionState == kOS_Occluded) {
    mPostConstructed->mOcclusionPinged = true;
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
    mDocks.push_back(Dock(in, mTransform));
  }
  if (mlvlVersion > 18) {
    mRelModules = rstl::vector< rstl::string >(in);
    if (mlvlVersion > 20) {
      mRelOffsets = rstl::vector< uint >(in);
    }
  }
  if (mlvlVersion > 19) {
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
  return mPostConstructed->mOcclusionState != kOS_Occluded || mPostConstructed->mFinishedOccluding;
}

rstl::pair< const uchar*, int > CGameArea::GetLayerScriptBuffer(const TLayerId& layer) const {
  if (mPhase < kP_WaitForValidation) {
    return rstl::pair< const uchar*, int >(nullptr, 0);
  }
  return rstl::pair< const uchar*, int >(
      reinterpret_cast< const uchar* >(mPostConstructed->mLayerScriptBuffers[layer.Value()].get()),
      GetLayerScriptSize(layer));
}

rstl::pair< const uchar*, int > CGameArea::GetGeneratedScriptBuffer() const {
  if (mPhase > kP_WaitForData) {
    return rstl::pair< const uchar*, int >(
        reinterpret_cast< const uchar* >(mPostConstructed->mGeneratedScriptBuffer.get()),
        mPostConstructed->mGeneratedScriptSize);
  }
  return rstl::pair< const uchar*, int >(nullptr, 0);
}

int CGameArea::GetLayerScriptSize(const TLayerId& layer) const {
  return mPhase > kP_WaitForData ? mPostConstructed->mLayerScriptSizes[layer.Value()] : 0;
}

void CGameArea::SetLoadPauseState(bool paused) {
  bool loading = false;
  for (int i = 0; i < mLayerPhases.size(); ++i) {
    if (mLayerPhases[i] == kLP_Loading) {
      loading = true;
      break;
    }
  }
  if (loading) {
    mLoadPaused = paused;
    if (paused) {
      for (int layer = 0; layer < mPostConstructed->mLayerTokens.size(); ++layer) {
        rstl::vector< CToken >& tokens = mPostConstructed->mLayerTokens[layer];
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
  mPostConstructed->mAreaAttributes = attributes;
}

void CGameArea::SetXRaySpeedAndTarget(float speed, float target) {
  mPostConstructed->mXraySpeed = speed;
  mPostConstructed->mXrayTarget = target;
}

void CGameArea::SetWeaponWorldLighting(float speed, float target) {
  mPostConstructed->mWeaponWorldLightingSpeed = speed;
  mPostConstructed->mWeaponWorldLightingTarget = target;
}

void CGameArea::UpdateWeaponWorldLighting(float dt) {
  // TODO: Blend x-ray and weapon lighting, then mark area actors' lighting dirty.
}

uint CGameArea::Get1stPVSLightFeature(uint index) const {
  // TODO: Resolve first-set light feature indices through CPVSAreaSet.
  return uint(-1);
}

uint CGameArea::Get2ndPVSLightFeature(uint index) const {
  // TODO: Resolve second-set light feature indices through CPVSAreaSet.
  return uint(-1);
}

void CGameArea::InitializeDocks(CStateManager& mgr) {
  // TODO: Filter dock IDs and initialize connected-area loading.
}

void CGameArea::UpdateDocks(CStateManager& mgr) {
  // TODO: Select nearby docks and send Echoes script messages.
}

void CGameArea::fn_80054F74() {}

int CGameArea::GetTokenCount() const {
  int count = 0;
  if (mPostConstructed.get()) {
    for (int i = 0; i < mPostConstructed->mLayerTokens.size(); ++i) {
      count += mPostConstructed->mLayerTokens[i].size();
    }
  }
  return count;
}

CGameArea::ELayerPhase CGameArea::GetLayerPhase(const TLayerId& layer) const {
  if (layer.Value() >= 0 && layer.Value() < mLayerPhases.size()) {
    return mLayerPhases[layer.Value()];
  }
  return kLP_Inactive;
}

const rstl::vector< CRELFileToken >* CGameArea::GetLayerRelTokens(const TLayerId& layer) const {
  if (mPostConstructed.get() && layer.Value() >= 0 &&
      layer.Value() < mPostConstructed->mLayerRelTokens.size()) {
    return &mPostConstructed->mLayerRelTokens[layer.Value()];
  }
  return nullptr;
}

bool CGameArea::IsValidLayerNumber(CStateManager& mgr, const TLayerId& layer) const {
  // TODO: Check the world-layer manager's area layer count.
  return false;
}

void CGameArea::LoadLayerDynamic(CStateManager& mgr, const TLayerId& layer) {
  // TODO: Check world-layer activation before starting or resuming a load.
}

void CGameArea::StartLayerLoad(CStateManager& mgr, const TLayerId& layer) {
  // TODO: Queue the layer's script read, resource tokens, and REL modules.
}

void CGameArea::RemoveLayerObjects(CStateManager& mgr, const TLayerId& layer) {
  // TODO: Remove script objects and queue the world's layer-unload notification.
}

void CGameArea::ClearLayer(CStateManager& mgr, const TLayerId& layer) {
  // TODO: Release layer objects, resources, and script buffers.
}

void CGameArea::UnloadLayerDynamic(CStateManager& mgr, const TLayerId& layer) {
  // TODO: Cancel loading or clear an activated layer.
}

void CGameArea::ActivateLayerDynamic(CStateManager& mgr, const TLayerId& layer) {
  // TODO: Instantiate and initialize this layer, then release its serialized script buffer.
}

void CGameArea::LoadLayerRelModules(CStateManager& mgr, const TLayerId& layer) {
  // TODO: Build the layer's REL tokens from its pair of module offsets.
}

void CGameArea::SortTextureDependencies() {
  // TODO: Sort contiguous TXTR runs within each layer by resource offset.
}

void CGameArea::DisableDocks(CStateManager& mgr) {
  // TODO: Mark docks disabled and free their script objects.
}

void CGameArea::EnableDocks() {
  if (!mPostConstructed->mDockIds.empty()) {
    mPostConstructed->mDocksDisabled = false;
  }
}

void CGameArea::ClearDecompressionRequest(CDvdRequest* request) {
  for (rstl::list< SDecompressionRequest >::iterator it =
           mPostConstructed->mDecompressionRequests.begin();
       it != mPostConstructed->mDecompressionRequests.end(); ++it) {
    if (it->mRequest == request) {
      it->mRequest = nullptr;
    }
  }
}

void CGameArea::ReadCompressedLayer(int offset, rstl::auto_ptr< CDvdRequest >& request,
                                    rstl::auto_ptr< uchar >& buffer) {
  // TODO: Locate the compressed block and queue the read/decompression request.
}

rstl::string CDummyGameArea::IGetInternalAreaName() const { return mInternalAreaName; }
