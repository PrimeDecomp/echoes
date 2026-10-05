#include "MetroidPrime/CScanTree.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "rstl/algorithm.hpp"
#include "Kyoto/Math/CMayaSpline.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScanTreeInventory.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeCategory.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeInventory.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeMenu.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeNode.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeScan.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeSlider.hpp"

// Guessed name.
static const CPlayerState::EItemType kInventorySlotToItemType[] = {
    CPlayerState::kIT_PowerBeam,      CPlayerState::kIT_DarkBeam,
    CPlayerState::kIT_LightBeam,      CPlayerState::kIT_AnnihilatorBeam,
    CPlayerState::kIT_SuperMissile,   CPlayerState::kIT_Darkburst,
    CPlayerState::kIT_Sunburst,       CPlayerState::kIT_SonicBoom,
    CPlayerState::kIT_CombatVisor,    CPlayerState::kIT_ScanVisor,
    CPlayerState::kIT_DarkVisor,      CPlayerState::kIT_EchoVisor,
    CPlayerState::kIT_VariaSuit,      CPlayerState::kIT_DarkSuit,
    CPlayerState::kIT_LightSuit,      CPlayerState::kIT_MorphBall,
    CPlayerState::kIT_BoostBall,      CPlayerState::kIT_SpiderBall,
    CPlayerState::kIT_MorphBallBombs, CPlayerState::kIT_DarkBomb,
    CPlayerState::kIT_LightBomb,      CPlayerState::kIT_AnnihilatorBomb,
    CPlayerState::kIT_ChargeBeam,     CPlayerState::kIT_GrappleBeam,
    CPlayerState::kIT_SpaceJumpBoots, CPlayerState::kIT_GravityBoost,
    CPlayerState::kIT_SeekerLauncher, CPlayerState::kIT_ScrewAttack,
    CPlayerState::kIT_Powerbomb,      CPlayerState::kIT_Missile,
    CPlayerState::kIT_DarkAmmo,       CPlayerState::kIT_LightAmmo,
    CPlayerState::kIT_EnergyTanks,    CPlayerState::kIT_TempleKey1,
    CPlayerState::kIT_TempleKey2,     CPlayerState::kIT_TempleKey3,
    CPlayerState::kIT_TempleKey4,     CPlayerState::kIT_TempleKey5,
    CPlayerState::kIT_TempleKey6,     CPlayerState::kIT_TempleKey7,
    CPlayerState::kIT_TempleKey8,     CPlayerState::kIT_TempleKey9,
    CPlayerState::kIT_AgonKey1,       CPlayerState::kIT_AgonKey2,
    CPlayerState::kIT_AgonKey3,       CPlayerState::kIT_TorvusKey1,
    CPlayerState::kIT_TorvusKey2,     CPlayerState::kIT_TorvusKey3,
    CPlayerState::kIT_HiveKey1,       CPlayerState::kIT_HiveKey2,
    CPlayerState::kIT_HiveKey3,       CPlayerState::kIT_EnergyTransferModule,
    CPlayerState::kIT_ChargeCombo};

// Guessed name.
static CVector3f kLogbookPosition(0.f, -1.f, 0.f);
static const char* const kLogbookCategoryName = "Logbook";
static const char* const kSamusGearCategoryName = "Samus Gear";

// Matches the comparator symbol shared with CSlideShow.
struct SlideShowScanIdLess {
  bool operator()(const CPlayerState::SPersistentState::SScanState& scan, CAssetId id) const {
    return scan.mAssetId < id;
  }
  bool operator()(CAssetId id, const CPlayerState::SPersistentState::SScanState& scan) const {
    return id < scan.mAssetId;
  }
};

CScanTreeInventory::~CScanTreeInventory() {}

CScanTreeNode* LoadScanTreeNode(uint type, int* id, const rstl::vector< int >& children,
                                CInputStream& input) {}

void ReadScanTree(CInputStream& input, CScanTree& tree) {}

CScanTreeNode::CScanTreeNode(int id, const SLdrTransform& transform, CAssetId nameStringTable,
                             const rstl::string& nameStringName)
: mDisplayPosition(transform.position)
, mPosition(transform.position)
, mVelocity(CVector3f::Zero())
, mAcceleration(CVector3f::Zero())
, mId(id)
, mParentNode(-1)
, mDescendantCount(0)
, mVisibleDescendantCount(0)
, mOpacity(1.f)
, mNameStringName(nameStringName)
, mNameStringTable(rs_new TCachedToken< CStringTable >(
      gpSimplePool->GetObj(SObjectTag('STRG', nameStringTable))))
, mVisible(true)
, mViewed(true) {}

void CScanTreeNode::LockResources() { mNameStringTable->Lock(); }

void CScanTreeNode::UnlockResources() { mNameStringTable->Unlock(); }

bool CScanTreeNode::AreResourcesLoaded() { return mNameStringTable->IsLoaded(); }

const CVector3f& CScanTreeNode::GetDisplayPosition() const { return mDisplayPosition; }

void CScanTreeNode::SetDisplayPosition(const CVector3f& position) { mDisplayPosition = position; }

const CVector3f& CScanTreeNode::GetPosition() const { return mPosition; }

void CScanTreeNode::SetPosition(const CVector3f& position) { mPosition = position; }

int CScanTreeNode::GetId() const { return mId; }

int CScanTreeNode::GetParentNode() const { return mParentNode; }

void CScanTreeNode::SetParentNode(int node) { mParentNode = node; }

rstl::wstring CScanTreeNode::GetName() const {
  if (mNameStringName.size() == 0) {
    return rstl::wstring(mNameStringTable->GetObject()->GetString(0));
  }
  return rstl::wstring(mNameStringTable->GetObject()->GetString(mNameStringName.c_str()));
}

const rstl::string& CScanTreeNode::GetNameStringName() const { return mNameStringName; }

CVector3f CScanTreeNode::GetVelocity() const { return mVelocity; }

void CScanTreeNode::SetVelocity(const CVector3f& velocity) { mVelocity = velocity; }

CVector3f CScanTreeNode::GetAcceleration() const { return mAcceleration; }

void CScanTreeNode::SetAcceleration(const CVector3f& acceleration) { mAcceleration = acceleration; }

bool CScanTreeNode::IsVisible() const { return mVisible; }

void CScanTreeNode::SetVisible(bool visible) { mVisible = visible; }

float CScanTreeNode::GetOpacity() const { return mOpacity; }

void CScanTreeNode::SetOpacity(float opacity) { mOpacity = opacity; }

int CScanTreeNode::GetDescendantCount() const { return mDescendantCount; }

void CScanTreeNode::SetDescendantCount(int count) { mDescendantCount = count; }

int CScanTreeNode::GetVisibleDescendantCount() const { return mVisibleDescendantCount; }

void CScanTreeNode::SetVisibleDescendantCount(int count) { mVisibleDescendantCount = count; }

bool CScanTreeNode::IsViewed() const { return mViewed; }

void CScanTreeNode::SetViewed(bool viewed) { mViewed = viewed; }

CScanTreeCategory::CScanTreeCategory(int id, const rstl::vector< int >& children,
                                     const SLdrTransform& transform, CAssetId nameStringTable,
                                     const rstl::string& nameStringName)
: CScanTreeNode(id, transform, nameStringTable, nameStringName)
, mChildren(children)
, mSelectedChild(-1) {}

CScanTreeNode::ENodeType CScanTreeCategory::GetNodeType() const { return kNT_Category; }

int CScanTreeCategory::GetChildCount() const { return mChildren.size(); }

int CScanTreeCategory::GetChild(int index) const { return mChildren[index]; }

int CScanTreeCategory::GetSelectedChild() const { return mSelectedChild; }

void CScanTreeCategory::SetSelectedChild(int node) { mSelectedChild = node; }

CScanTreeScan::CScanTreeScan(int id, const SLdrTransform& transform, CAssetId nameStringTable,
                             CAssetId scannableInfo, const rstl::string& nameStringName)
: CScanTreeNode(id, transform, nameStringTable, nameStringName), mScannableInfo(scannableInfo) {}

CScanTreeNode::ENodeType CScanTreeScan::GetNodeType() const { return kNT_Scan; }

CAssetId CScanTreeScan::GetScannableInfo() const { return mScannableInfo; }

CScanTreeInventory::CScanTreeInventory(int id, const SLdrTransform& transform,
                                       CAssetId nameStringTable, CAssetId scannableInfo,
                                       CPlayerState::EItemType inventoryItem,
                                       const rstl::string& nameStringName)
: CScanTreeScan(id, transform, nameStringTable, scannableInfo, nameStringName)
, mInventoryItem(inventoryItem) {}

CScanTreeNode::ENodeType CScanTreeInventory::GetNodeType() const { return kNT_Inventory; }

CPlayerState::EItemType CScanTreeInventory::GetInventoryItem() const { return mInventoryItem; }

CScanTreeMenu::CScanTreeMenu(int id, const SLdrTransform& transform, CAssetId nameStringTable,
                             const rstl::string& nameStringName, ESetting setting,
                             CAssetId optionStringTable, const rstl::string& option1, int value1,
                             const rstl::string& option2, int value2, const rstl::string& option3,
                             int value3, const rstl::string& option4, int value4)
: CScanTreeNode(id, transform, nameStringTable, nameStringName)
, mSetting(setting)
, mOptionValue(0)
, mSelectedOption(0)
, mOptionStringTable(rs_new TCachedToken< CStringTable >(
      gpSimplePool->GetObj(SObjectTag('STRG', optionStringTable))))
, mOptions() {
  typedef rstl::pair< rstl::string, int > Option;
  if (option1.size() != 0) {
    mOptions.push_back(Option(option1, value1));
  }
  if (option2.size() != 0) {
    mOptions.push_back(Option(option2, value2));
  }
  if (option3.size() != 0) {
    mOptions.push_back(Option(option3, value3));
  }
  if (option4.size() != 0) {
    mOptions.push_back(Option(option4, value4));
  }
  mOptionStringTable->Lock();
}

CScanTreeNode::ENodeType CScanTreeMenu::GetNodeType() const { return kNT_Menu; }

CScanTreeMenu::ESetting CScanTreeMenu::GetSetting() const { return mSetting; }

void CScanTreeMenu::LockResources() {
  CScanTreeNode::LockResources();
  mOptionStringTable->Lock();
}

void CScanTreeMenu::UnlockResources() {
  CScanTreeNode::UnlockResources();
  mOptionStringTable->Unlock();
}

bool CScanTreeMenu::AreResourcesLoaded() {
  return CScanTreeNode::AreResourcesLoaded() && mOptionStringTable->IsLoaded();
}

void CScanTreeMenu::RefreshSelectedOption() { mSelectedOption = GetCurrentOptionIndex(); }

int CScanTreeMenu::GetSelectedOption() const { return mSelectedOption; }

void CScanTreeMenu::ApplySelectedOption() { ApplyOption(mSelectedOption); }

int CScanTreeMenu::GetCurrentOptionIndex() const {}

void CScanTreeMenu::ApplyOption(int index) {
  CGameOptions& options = gpGameState->GameOptions();
  const int value = mOptions[index].second;
  switch (mSetting) {
  case kS_SurroundMode:
    options.SetSurroundMode(static_cast< CAudioSys::ESurroundModes >(value), true);
    break;
  case kS_HudLag:
    options.SetHUDLag(value != 0);
    break;
  case kS_HintSystem:
    options.SetIsHintSystemEnabled(value != 0);
    break;
  case kS_Unknown3:
    options.SetFlag3(value != 0);
    break;
  case kS_InvertYAxis:
    options.SetInvertYAxis(value != 0);
    break;
  case kS_SwapBeamControls:
    options.ToggleControls(value != 0);
    break;
  case kS_Rumble:
    options.SetIsRumbleEnabled(value != 0);
    break;
  default:
    mOptionValue = value;
    break;
  }
}

rstl::wstring CScanTreeMenu::GetOptionName(int index) const {
  if (index >= mOptions.size() || mOptions[index].first.size() == 0) {
    return rstl::wstring(mOptionStringTable->GetObject()->GetString(index));
  }
  return rstl::wstring(mOptionStringTable->GetObject()->GetString(mOptions[index].first.c_str()));
}

int CScanTreeMenu::GetOptionCount() const { return mOptions.size(); }

CScanTreeSlider::CScanTreeSlider(int id, const SLdrTransform& transform, CAssetId nameStringTable,
                                 const rstl::string& nameStringName, ESetting setting)
: CScanTreeNode(id, transform, nameStringTable, nameStringName)
, mNormalizedValue(0.f)
, mSavedNormalizedValue(0.f)
, mSetting(setting) {}

CScanTreeNode::ENodeType CScanTreeSlider::GetNodeType() const { return kNT_Slider; }

int CScanTreeSlider::GetMinOptionValue() const {
  switch (mSetting) {
  case kS_ScreenBrightness:
    return 0;
  case kS_ScreenPositionX:
    return -30;
  case kS_ScreenPositionY:
    return -19;
  case kS_ScreenStretch:
    return -10;
  case kS_SfxVolume:
    return 0;
  case kS_MusicVolume:
    return 0;
  case kS_HudAlpha:
    return 0;
  case kS_HelmetAlpha:
    return 0;
  default:
    return 0;
  }
}

int CScanTreeSlider::GetMaxOptionValue() const {
  switch (mSetting) {
  case kS_ScreenBrightness:
    return 8;
  case kS_ScreenPositionX:
    return 30;
  case kS_ScreenPositionY:
    return 19;
  case kS_ScreenStretch:
    return 10;
  case kS_SfxVolume:
    return 105;
  case kS_MusicVolume:
    return 105;
  case kS_HudAlpha:
    return 255;
  case kS_HelmetAlpha:
    return 255;
  default:
    return 0;
  }
}

int CScanTreeSlider::GetOptionValue() const {}

int CScanTreeSlider::GetDefaultOptionValue() const {
  switch (mSetting) {
  case kS_ScreenBrightness:
    return 4;
  case kS_ScreenPositionX:
    return 0;
  case kS_ScreenPositionY:
    return 0;
  case kS_ScreenStretch:
    return 0;
  case kS_SfxVolume:
    return 105;
  case kS_MusicVolume:
    return 79;
  case kS_HudAlpha:
    return 255;
  case kS_HelmetAlpha:
    return 255;
  default:
    return 0;
  }
}

void CScanTreeSlider::SetOptionValue(int value) {
  CGameOptions& options = gpGameState->GameOptions();
  switch (mSetting) {
  case kS_ScreenBrightness:
    options.SetScreenBrightness(value, true);
    break;
  case kS_ScreenPositionX:
    options.SetScreenPositionX(value, true);
    break;
  case kS_ScreenPositionY:
    options.SetScreenPositionY(value, true);
    break;
  case kS_ScreenStretch:
    options.SetScreenStretch(value, true);
    break;
  case kS_SfxVolume:
    options.SetSfxVolume(value, true);
    break;
  case kS_MusicVolume:
    options.SetMusicVolume(value, true);
    break;
  case kS_HudAlpha:
    options.SetHudAlpha(value);
    break;
  case kS_HelmetAlpha:
    options.SetHelmetAlpha(value);
    break;
  }
}

void CScanTreeSlider::RefreshNormalizedValue() {
  const int offset = GetOptionValue() - GetMinOptionValue();
  const int range = GetMaxOptionValue() - GetMinOptionValue();
  mNormalizedValue = float(offset) / float(range);
}

void CScanTreeSlider::ApplyNormalizedValue() {
  SetOptionValue(int(mNormalizedValue * float(GetMaxOptionValue() - GetMinOptionValue()) +
                     float(GetMinOptionValue())));
}

void CScanTreeSlider::SaveValue() {
  const int offset = GetOptionValue() - GetMinOptionValue();
  const int range = GetMaxOptionValue() - GetMinOptionValue();
  mSavedNormalizedValue = float(offset) / float(range);
}

void CScanTreeSlider::RestoreSavedValue() {
  SetOptionValue(int(mSavedNormalizedValue * float(GetMaxOptionValue() - GetMinOptionValue()) +
                     float(GetMinOptionValue())));
  RefreshNormalizedValue();
}

float CScanTreeSlider::GetSavedNormalizedValue() const { return mSavedNormalizedValue; }

void CScanTreeSlider::SetNormalizedValue(float value) { mNormalizedValue = value; }

float CScanTreeSlider::GetNormalizedValue() const { return mNormalizedValue; }

float CScanTreeSlider::GetNormalizedDefaultValue() const {
  const int offset = GetDefaultOptionValue() - GetMinOptionValue();
  const int range = GetMaxOptionValue() - GetMinOptionValue();
  return float(offset) / float(range);
}

CScanTree::CScanTree()
: mSelectedNode(-1)
, mPreviousNode(-1)
, mTransition(0.f)
, mTransitionDuration(1.f)
, mInitialLayoutTransition(0.f)
, mBuffer(nullptr)
, mBufferLength(0)
, mLoadRequest(nullptr)
, mNodes()
, mRootNode(-1)
, mRandom(0) {}

void CScanTree::LoadAsync() {
  const SObjectTag* tag = gpResourceFactory->GetResourceIdByName("DUMB_ScanTree");
  mBufferLength = gpResourceFactory->GetResLoader().ResourceSize(*tag);
  mBuffer = rstl::auto_ptr< uchar >(
      static_cast< uchar* >(CMemory::Alloc(mBufferLength, IAllocator::kHI_RoundUpLen)));
  mLoadRequest = rstl::auto_ptr< CDvdRequest >(gpResourceFactory->GetResLoader().LoadResourceAsync(
      *tag, reinterpret_cast< char* >(mBuffer.get())));
}

void CScanTree::ReserveNodes(int count) { mNodes.reserve(count); }

void CScanTree::AddNode(CScanTreeNode* node) {
  mNodes.push_back_unsafe(rstl::rc_ptr< CScanTreeNode >(node));
}

void CScanTree::SetRootNode(int node) { mRootNode = node; }

void CScanTree::UpdateDescendantCounts() {
  for (rstl::vector< rstl::rc_ptr< CScanTreeNode > >::const_iterator it = mNodes.begin();
       it != mNodes.end(); ++it) {
    const rstl::rc_ptr< CScanTreeNode > node = *it;
    if (node->GetNodeType() != CScanTreeNode::kNT_Category) {
      node->SetDescendantCount(1);
      node->SetVisibleDescendantCount(1);
      const bool visible = node->IsVisible();
      int parent = node->GetParentNode();
      while (parent != -1) {
        const rstl::rc_ptr< CScanTreeCategory > category(mNodes[parent]);
        category->SetDescendantCount(category->GetDescendantCount() + 1);
        if (visible) {
          category->SetVisibleDescendantCount(category->GetVisibleDescendantCount() + 1);
        }
        parent = category->GetParentNode();
      }
    }
  }
}

void CScanTree::InitializeHierarchy() {
  for (rstl::vector< rstl::rc_ptr< CScanTreeNode > >::const_iterator it = mNodes.begin();
       it != mNodes.end(); ++it) {
    if ((*it)->GetNodeType() == CScanTreeNode::kNT_Category) {
      const rstl::rc_ptr< CScanTreeCategory > category(*it);
      const int childCount = category->GetChildCount();
      for (int i = 0; i < childCount; ++i) {
        const int child = category->GetChild(i);
        mNodes[child]->SetParentNode(category->GetId());
        if (category->GetSelectedChild() == -1 && mNodes[child]->IsVisible()) {
          category->SetSelectedChild(child);
        }
      }
      if (category->GetSelectedChild() == -1) {
        category->SetVisible(false);
      }
    }
  }
  mSelectedNode = mRootNode;
  InitializeNodePositions(mSelectedNode);
  mInitialLayoutTransition = 1.f;
}

bool CScanTree::UpdateNodeVisibility(CStateManager& mgr, int node) {}

void CScanTree::RefreshVisibility(CStateManager& mgr) {}

rstl::pair< uint, uint > CScanTree::GetScanCounts() const {
  const rstl::rc_ptr< CScanTreeNode > root = mNodes[mRootNode];
  if (root->GetNodeType() == CScanTreeNode::kNT_Category) {
    const rstl::rc_ptr< CScanTreeCategory > category(root);
    const int childCount = category->GetChildCount();
    for (int i = 0; i < childCount; ++i) {
      const rstl::rc_ptr< CScanTreeNode > child = mNodes[category->GetChild(i)];
      if (child->GetNameStringName() == kLogbookCategoryName) {
        const uint total = child->GetDescendantCount();
        return rstl::pair< uint, uint >(child->GetVisibleDescendantCount(), total);
      }
    }
  }
  return rstl::pair< uint, uint >(0, 1);
}

void CScanTree::RandomizeChildPositions(int node) {}

void CScanTree::InitializeNodePositions(int node) {}

bool CScanTree::PollLoad() {
  if (mLoadRequest.get() != nullptr) {
    if (!mLoadRequest->IsComplete()) {
      return false;
    }
    CMemoryInStream in(mBuffer.get(), mBufferLength);
    ReadScanTree(in, *this);
    mBuffer = rstl::auto_ptr< uchar >(nullptr);
    mLoadRequest = rstl::auto_ptr< CDvdRequest >(nullptr);
    InitializeHierarchy();
  }
  return true;
}

void CScanTree::SelectNode(int node) {
  SelectNode(node, gpTweakGui->GetLogBookTransitionTime());
}

void CScanTree::SelectNode(int node, float duration) {
  mPreviousNode = mSelectedNode;
  mSelectedNode = node;
  if (!CMath::IsEpsilon(duration, 0.f, FLT_EPSILON)) {
    mTransition = 1.f;
    mTransitionDuration = duration;
  } else {
    mTransition = 0.f;
    mTransitionDuration = 1.f;
  }
}

void CScanTree::SelectScan(CAssetId scannableInfo, float duration) {
  int index = 0;
  for (rstl::vector< rstl::rc_ptr< CScanTreeNode > >::const_iterator it = mNodes.begin();
       it != mNodes.end(); ++it, ++index) {
    if ((*it)->GetNodeType() == CScanTreeNode::kNT_Scan ||
        (*it)->GetNodeType() == CScanTreeNode::kNT_Inventory) {
      const rstl::rc_ptr< CScanTreeScan > scan(*it);
      if (scannableInfo == scan->GetScannableInfo()) {
        SelectNode(index, duration);
        return;
      }
    }
  }
}

int CScanTree::GetSelectedNode() const { return mSelectedNode; }

int CScanTree::GetPreviousNode() const { return mPreviousNode; }

float CScanTree::GetTransition() const { return mTransition; }

bool CScanTree::IsLoaded() const { return mLoadRequest.get() == nullptr; }

rstl::rc_ptr< CScanTreeNode > CScanTree::GetNode(int node) const { return mNodes[node]; }

int CScanTree::GetRootNode() const { return mRootNode; }

void CScanTree::ScaleChildren(int node, float otherScale, float selectedScale,
                              bool excludeOptions) {}

void CScanTree::UpdateLayout(float) {}

float CScanTree::GetLayoutProgress() const {
  if (!CMath::IsEpsilon(mInitialLayoutTransition, 0.f, FLT_EPSILON)) {
    return 1.f - mInitialLayoutTransition;
  }
  const float collapse = gpTweakGui->GetLogBookNodeCollapseMotion().EvaluateAt(1.f - mTransition);
  const float expand = gpTweakGui->GetLogBookNodeExpandMotion().EvaluateAt(1.f - mTransition);
  return collapse < expand ? expand : collapse;
}

void CScanTree::UpdateNodePhysics(float dt) {}

void CScanTree::Update(float dt) {
  const float transition = mTransition - dt / mTransitionDuration;
  mTransition = transition < 0.f ? 0.f : transition;
  const float initialLayout = mInitialLayoutTransition - dt / 0.3f;
  mInitialLayoutTransition = initialLayout < 0.f ? 0.f : initialLayout;
  if (mSelectedNode != -1) {
    UpdateNodePhysics(dt);
    UpdateLayout(dt);
  }
}

void CScanTree::RefreshViewed(CStateManager& mgr) {
  const rstl::vector< CPlayerState::SPersistentState::SScanState >& scanStates =
      mgr.PlayerState(0)->ScanStates();
  for (rstl::vector< rstl::rc_ptr< CScanTreeNode > >::const_iterator it = mNodes.begin();
       it != mNodes.end(); ++it) {
    if ((*it)->GetNodeType() == CScanTreeNode::kNT_Scan ||
        (*it)->GetNodeType() == CScanTreeNode::kNT_Inventory) {
      const rstl::rc_ptr< CScanTreeScan > scan(*it);
      rstl::vector< CPlayerState::SPersistentState::SScanState >::const_iterator state =
          rstl::binary_find(scanStates.begin(), scanStates.end(), scan->GetScannableInfo(),
                            SlideShowScanIdLess());
      scan->SetViewed(state != scanStates.end() && state->mViewedInLogbook);
    }
  }
  UpdateViewedCategories();
}

void CScanTree::MarkViewed(CStateManager& mgr, int node) {
  if (node < mNodes.size()) {
    const rstl::vector< CPlayerState::SPersistentState::SScanState >& scanStates =
        mgr.PlayerState(0)->ScanStates();
    const rstl::rc_ptr< CScanTreeNode > treeNode = mNodes[node];
    if (treeNode->GetNodeType() == CScanTreeNode::kNT_Scan ||
        treeNode->GetNodeType() == CScanTreeNode::kNT_Inventory) {
      const rstl::rc_ptr< CScanTreeScan > scan(treeNode);
      scan->SetViewed(true);
      const CAssetId scannableInfo = scan->GetScannableInfo();
      rstl::vector< CPlayerState::SPersistentState::SScanState >::const_iterator state =
          rstl::binary_find(scanStates.begin(), scanStates.end(), scannableInfo,
                            SlideShowScanIdLess());
      if (state != scanStates.end()) {
        gpGameState->PlayerState(0)->SetScanFlag(scannableInfo, true);
      }
    }
    UpdateViewedCategories();
  }
}

void CScanTree::UpdateViewedCategories() {
  const rstl::rc_ptr< CScanTreeNode > root = mNodes[mRootNode];
  if (root->GetNodeType() == CScanTreeNode::kNT_Category) {
    const rstl::rc_ptr< CScanTreeCategory > category(root);
    const int childCount = category->GetChildCount();
    for (int i = 0; i < childCount; ++i) {
      const int index = category->GetChild(i);
      const rstl::rc_ptr< CScanTreeNode > child = mNodes[index];
      if (child->GetNameStringName() == kLogbookCategoryName ||
          child->GetNameStringName() == kSamusGearCategoryName) {
        UpdateCategoryViewed(index);
      }
    }
  }
}

bool CScanTree::UpdateCategoryViewed(int node) {
  const rstl::rc_ptr< CScanTreeNode > treeNode = mNodes[node];
  if (!treeNode->IsVisible()) {
    return true;
  }
  bool allViewed = true;
  if (treeNode->GetNodeType() == CScanTreeNode::kNT_Category) {
    const rstl::rc_ptr< CScanTreeCategory > category(treeNode);
    const int childCount = category->GetChildCount();
    for (int i = 0; i < childCount; ++i) {
      if (!UpdateCategoryViewed(category->GetChild(i))) {
        allViewed = false;
      }
    }
  } else {
    return treeNode->IsViewed();
  }
  treeNode->SetViewed(allViewed);
  return allViewed;
}

CVector3f CScanTree::CalculatePairForce(float radius, float strength, const CVector3f& position,
                                        const CVector3f& otherPosition) const {}

CVector3f CScanTree::CalculateSeparationForce(const rstl::rc_ptr< CScanTreeCategory >& category,
                                              int node) const {}

CVector3f CScanTree::CalculateNeighborForce(const rstl::rc_ptr< CScanTreeCategory >& category,
                                            int node) const {}

CScanTreeCategory* LoadScanTreeCategory(int* id, const rstl::vector< int >& children,
                                        CInputStream& input) {}

CScanTreeScan* LoadScanTreeScan(int* id, CInputStream& input) {}

CScanTreeInventory* LoadScanTreeInventory(int* id, CInputStream& input) {
  SLdrScanTreeInventory sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrScanTreeInventory.inc"

  return rs_new CScanTreeInventory(*id & 0xffff, sldrThis.editorProperties.transform,
                                   sldrThis.nodeName, sldrThis.scannableInfo.scannableInfo0,
                                   uint(sldrThis.inventoryItem) < 0x35
                                       ? kInventorySlotToItemType[sldrThis.inventoryItem]
                                       : CPlayerState::kIT_PowerBeam,
                                   sldrThis.stringName);
}

CScanTreeMenu* LoadScanTreeMenu(int* id, CInputStream& input) {}

CScanTreeSlider* LoadScanTreeSlider(int* id, CInputStream& input) {}
