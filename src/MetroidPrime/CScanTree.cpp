#include "MetroidPrime/CScanTree.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScanTreeCategory.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScanTreeInventory.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScanTreeMenu.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScanTreeScan.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScanTreeSlider.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeCategory.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeInventory.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeMenu.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeNode.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeScan.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeSlider.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "rstl/algorithm.hpp"

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
                                CInputStream& input) {
  switch (type) {
  case 'SCND': {
    int nodeId = *id;
    return LoadScanTreeCategory(&nodeId, children, input);
  }
  case 'SCSN': {
    int nodeId = *id;
    return LoadScanTreeScan(&nodeId, input);
  }
  case 'SCIN': {
    int nodeId = *id;
    return LoadScanTreeInventory(&nodeId, input);
  }
  case 'SCMN': {
    int nodeId = *id;
    return LoadScanTreeMenu(&nodeId, input);
  }
  case 'SCSL': {
    int nodeId = *id;
    return LoadScanTreeSlider(&nodeId, input);
  }
  default:
    return nullptr;
  }
}

void ReadScanTree(CInputStream& input, CScanTree& tree) {
  if (input.Get< uint >() != 'TREE') {
    return;
  }
  tree.SetRootNode(input.Get< int >());
  if (input.Get< uchar >() != 1) {
    return;
  }
  int nodeCount = input.Get< int >();
  tree.ReserveNodes(nodeCount);
  while (nodeCount-- != 0) {
    const uint type = input.Get< uint >();
    int remaining = input.Get< ushort >() - 6;
    int id = input.Get< int >();
    rstl::vector< int > children;
    int childCount = input.Get< ushort >();
    children.reserve(childCount);
    for (int i = 0; i < childCount; i++) {
      input.Get< uint >();
      input.Get< uint >();
      children.push_back_unsafe(static_cast< ushort >(input.Get< uint >()));
      remaining -= 12;
    }
    const uint start = input.GetReadPosition();
    input.Get< uint >();
    input.Get< ushort >();
    int loadId = id;
    CScanTreeNode* node = LoadScanTreeNode(type, &loadId, children, input);
    if (node != nullptr) {
      tree.AddNode(node);
    }
    remaining -= input.GetReadPosition() - start;
    for (int i = 0; i < remaining; ++i) {
      input.Get< uchar >();
    }
  }
}

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
, mViewed(true) {
  mNameStringTable->Lock();
}

void CScanTreeNode::LockResources() { mNameStringTable->Lock(); }

void CScanTreeNode::UnlockResources() { mNameStringTable->Unlock(); }

bool CScanTreeNode::AreResourcesLoaded() {
  return mNameStringTable->IsLoaded() && mNameStringTable->TryCache();
}

const CVector3f& CScanTreeNode::GetDisplayPosition() const { return mDisplayPosition; }

void CScanTreeNode::SetDisplayPosition(const CVector3f& position) { mDisplayPosition = position; }

const CVector3f& CScanTreeNode::GetPosition() const { return mPosition; }

void CScanTreeNode::SetPosition(const CVector3f& position) { mPosition = position; }

int CScanTreeNode::GetId() const { return mId; }

int CScanTreeNode::GetParentNode() const { return mParentNode; }

void CScanTreeNode::SetParentNode(int node) { mParentNode = node; }

rstl::wstring CScanTreeNode::GetName() const {
  if (mNameStringName.length() == 0) {
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
  if (option1.length() != 0) {
    mOptions.push_back(Option(option1, value1));
  }
  if (option2.length() != 0) {
    mOptions.push_back(Option(option2, value2));
  }
  if (option3.length() != 0) {
    mOptions.push_back(Option(option3, value3));
  }
  if (option4.length() != 0) {
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
  return CScanTreeNode::AreResourcesLoaded() && mOptionStringTable->IsLoaded() &&
         mOptionStringTable->TryCache();
}

void CScanTreeMenu::RefreshSelectedOption() { mSelectedOption = GetCurrentOptionIndex(); }

int CScanTreeMenu::GetSelectedOption() const { return mSelectedOption; }

void CScanTreeMenu::ApplySelectedOption() { ApplyOption(mSelectedOption); }

int CScanTreeMenu::GetCurrentOptionIndex() const {
  const CGameOptions& options = gpGameState->GameOptions();
  int value = 0;
  switch (mSetting) {
  case kS_SurroundMode:
    value = options.soundMode;
    break;
  case kS_HudLag:
    value = options.hudLag;
    break;
  case kS_HintSystem:
    value = options.hintSystem;
    break;
  case kS_Unknown3:
    value = options.hudEnglish;
    break;
  case kS_InvertYAxis:
    value = options.invertY;
    break;
  case kS_SwapBeamControls:
    value = options.swapBeamsControls;
    break;
  case kS_Rumble:
    value = options.rumble;
    break;
  case kS_Unknown8:
  case kS_Unknown9:
  case kS_Unknown10:
  case kS_Unknown11:
    value = mOptionValue;
    break;
  }
  for (int i = 0; i < mOptions.size(); ++i) {
    if (mOptions[i].second == value) {
      return i;
    }
  }
  return 0;
}

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
    options.SetIsHudEnglish(value != 0);
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
  case kS_Unknown8:
  case kS_Unknown9:
  case kS_Unknown10:
  case kS_Unknown11:
    mOptionValue = value;
    break;
  }
}

rstl::wstring CScanTreeMenu::GetOptionName(int index) const {
  if (index >= mOptions.size() || mOptions[index].first.length() == 0) {
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

int CScanTreeSlider::GetOptionValue() const {
  const CGameOptions& options = gpGameState->GameOptions();
  switch (mSetting) {
  case kS_ScreenBrightness:
    return options.screenBrightness;
  case kS_ScreenPositionX:
    return options.screenXOffset;
  case kS_ScreenPositionY:
    return options.screenYOffset;
  case kS_ScreenStretch:
    return options.screenStretch;
  case kS_SfxVolume:
    return options.sfxVol;
  case kS_MusicVolume:
    return options.musicVol;
  case kS_HudAlpha:
    return options.GetHudAlphaRaw();
  case kS_HelmetAlpha:
    return options.GetHelmetAlphaRaw();
  default:
    return 0;
  }
}

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
  mNormalizedValue = float(offset) / float(GetMaxOptionValue() - GetMinOptionValue());
}

void CScanTreeSlider::ApplyNormalizedValue() {
  SetOptionValue(int(mNormalizedValue * float(GetMaxOptionValue() - GetMinOptionValue()) +
                     float(GetMinOptionValue())));
}

void CScanTreeSlider::SaveValue() {
  const int offset = GetOptionValue() - GetMinOptionValue();
  mSavedNormalizedValue = float(offset) / float(GetMaxOptionValue() - GetMinOptionValue());
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
  return float(offset) / float(GetMaxOptionValue() - GetMinOptionValue());
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
      for (int i = 0; i < childCount; i++) {
        int child = category->GetChild(i);
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

bool CScanTree::UpdateNodeVisibility(CStateManager& mgr, int node) {
  bool visible = false;
  const rstl::rc_ptr< CScanTreeNode > treeNode = mNodes[node];
  if (treeNode->GetNodeType() == CScanTreeNode::kNT_Category) {
    const rstl::rc_ptr< CScanTreeCategory > category(treeNode);
    const int childCount = category->GetChildCount();
    for (int i = 0; i < childCount; i++) {
      int child = category->GetChild(i);
      if (UpdateNodeVisibility(mgr, child)) {
        visible = true;
      }
    }
  } else if (treeNode->GetNodeType() == CScanTreeNode::kNT_Scan) {
    const rstl::rc_ptr< CScanTreeScan > scan(treeNode);
    const CAssetId scannableInfo = scan->GetScannableInfo();
    const rstl::vector< CPlayerState::SPersistentState::SScanState >& scanStates =
        mgr.PlayerState(0)->ScanStates();
    rstl::vector< CPlayerState::SPersistentState::SScanState >::const_iterator state =
        rstl::binary_find(scanStates.begin(), scanStates.end(), scannableInfo,
                          SlideShowScanIdLess());
    visible = state != scanStates.end() && state->mProgress == 0xff;
  } else if (treeNode->GetNodeType() == CScanTreeNode::kNT_Inventory) {
    const rstl::rc_ptr< CScanTreeInventory > inventory(treeNode);
    CPlayerState::EItemType item = inventory->GetInventoryItem();
    visible = mgr.PlayerState(0)->GetItemCapacity(item) > 0;
  }
  const bool result = visible;
  treeNode->SetVisible(result);
  return result;
}

void CScanTree::RefreshVisibility(CStateManager& mgr) {
  const rstl::rc_ptr< CScanTreeNode > root = mNodes[mRootNode];
  if (root->GetNodeType() == CScanTreeNode::kNT_Category) {
    const rstl::rc_ptr< CScanTreeCategory > category(root);
    const int childCount = category->GetChildCount();
    for (int i = 0; i < childCount; i++) {
      int index = category->GetChild(i);
      const rstl::rc_ptr< CScanTreeNode > child = mNodes[index];
      if (child->GetNameStringName() == kLogbookCategoryName ||
          child->GetNameStringName() == kSamusGearCategoryName) {
        UpdateNodeVisibility(mgr, index);
      }
      if (child->GetNameStringName() == kLogbookCategoryName && child->IsVisible()) {
        mNodes[index]->SetPosition(kLogbookPosition);
        const rstl::rc_ptr< CScanTreeNode > parent = mNodes[child->GetParentNode()];
        if (root->GetNodeType() == CScanTreeNode::kNT_Category) {
          const rstl::rc_ptr< CScanTreeCategory > rootCategory(root);
          rootCategory->SetSelectedChild(index);
        }
      }
    }
  }
  UpdateDescendantCounts();
}

rstl::pair< uint, uint > CScanTree::GetScanCounts() const {
  const rstl::rc_ptr< CScanTreeNode > root = mNodes[mRootNode];
  if (root->GetNodeType() == CScanTreeNode::kNT_Category) {
    const rstl::rc_ptr< CScanTreeCategory > category(root);
    const int childCount = category->GetChildCount();
    for (int i = 0; i < childCount; i++) {
      const rstl::rc_ptr< CScanTreeNode > child = mNodes[category->GetChild(i)];
      if (child->GetNameStringName() == kLogbookCategoryName) {
        const uint total = child->GetDescendantCount();
        return rstl::pair< uint, uint >(child->GetVisibleDescendantCount(), total);
      }
    }
  }
  return rstl::pair< uint, uint >(0, 1);
}

void CScanTree::RandomizeChildPositions(int node) {
  const float branchLength = gpTweakGui->GetLogBookBranchLength();
  const rstl::rc_ptr< CScanTreeNode > treeNode = mNodes[node];
  if (treeNode->GetNodeType() == CScanTreeNode::kNT_Category) {
    const rstl::rc_ptr< CScanTreeCategory > category(treeNode);
    const int childCount = category->GetChildCount();
    for (int i = 0; i < childCount; i++) {
      const int child = category->GetChild(i);
      const float angle = mRandom.Range(0.f, 2.f * M_PIF);
      const float height = mRandom.Range(-branchLength, branchLength);
      const float ratio = height / branchLength;
      const float ratioSquared = ratio * ratio;
      const float planarRatio = CMath::SqrtF(1.f - ratioSquared);
      const float x = branchLength * planarRatio * CMath::FastCosR(angle);
      const float y = branchLength * planarRatio * CMath::FastSinR(angle);
      const CVector3f direction(x, y, height);
      const CVector3f position = direction.AsNormalized() * 2.f + category->GetPosition();
      mNodes[child]->SetPosition(position);
      mNodes[child]->SetDisplayPosition(position);
    }
  }
}

void CScanTree::InitializeNodePositions(int node) {
  const float branchLength = gpTweakGui->GetLogBookBranchLength();
  const rstl::rc_ptr< CScanTreeNode > treeNode = mNodes[node];
  if (treeNode->GetNodeType() == CScanTreeNode::kNT_Category) {
    const rstl::rc_ptr< CScanTreeCategory > category(treeNode);
    const int childCount = category->GetChildCount();
    for (int i = 0; i < childCount; i++) {
      const int child = category->GetChild(i);
      const float angle = mRandom.Range(0.f, 2.f * M_PIF);
      const float height = mRandom.Range(-branchLength, branchLength);
      const float ratio = height / branchLength;
      const float ratioSquared = ratio * ratio;
      const float planarRatio = CMath::SqrtF(1.f - ratioSquared);
      const float x = branchLength * planarRatio * CMath::FastCosR(angle);
      const float y = branchLength * planarRatio * CMath::FastSinR(angle);
      const CVector3f position = CVector3f(x, y, height) + category->GetPosition();
      mNodes[child]->SetPosition(position);
      mNodes[child]->SetDisplayPosition(position);
    }
    for (int i = 0; i < childCount; i++) {
      int child = category->GetChild(i);
      InitializeNodePositions(child);
    }
  }
}

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

void CScanTree::SelectNode(int node) { SelectNode(node, gpTweakGui->GetLogBookTransitionTime()); }

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
                              bool excludeOptions) {
  const rstl::rc_ptr< CScanTreeNode > treeNode = GetNode(node);
  if (treeNode->GetNodeType() == CScanTreeNode::kNT_Category) {
    const rstl::rc_ptr< CScanTreeCategory > category(treeNode);
    const CVector3f& displayPosition = category->GetDisplayPosition();
    mNodes[category->GetId()]->SetPosition(displayPosition);
    for (int i = 0; i < category->GetChildCount(); ++i) {
      const int child = category->GetChild(i);
      const rstl::rc_ptr< CScanTreeNode > childNode = mNodes[child];
      if (excludeOptions && (childNode->GetNodeType() == CScanTreeNode::kNT_Menu ||
                             childNode->GetNodeType() == CScanTreeNode::kNT_Slider)) {
        continue;
      }
      const rstl::rc_ptr< CScanTreeNode > target = mNodes[child];
      const CVector3f offset = childNode->GetPosition() - displayPosition;
      if (offset.CanBeNormalized()) {
        const CVector3f direction = offset.AsNormalized();
        if (child == category->GetSelectedChild()) {
          const float length = gpTweakGui->GetLogBookBranchLength();
          target->SetDisplayPosition(
              CVector3f(displayPosition.GetX() + selectedScale * (length * direction.GetX()),
                        displayPosition.GetY() + selectedScale * (length * direction.GetY()),
                        displayPosition.GetZ() + selectedScale * (length * direction.GetZ())));
          target->SetOpacity(CMath::Clamp(0.f, selectedScale, 1.f));
        } else {
          const float length = gpTweakGui->GetLogBookBranchLength();
          target->SetDisplayPosition(
              CVector3f(displayPosition.GetX() + otherScale * (length * direction.GetX()),
                        displayPosition.GetY() + otherScale * (length * direction.GetY()),
                        displayPosition.GetZ() + otherScale * (length * direction.GetZ())));
          target->SetOpacity(CMath::Clamp(0.f, otherScale, 1.f));
        }
      }
    }
  }
}

void CScanTree::UpdateLayout(float) {
  if (!CMath::IsEpsilon(mInitialLayoutTransition, 0.f, FLT_EPSILON)) {
    const float progress = 1.f - mInitialLayoutTransition;
    ScaleChildren(mSelectedNode, progress, progress, false);
  }
  if (!CMath::IsEpsilon(mTransition, 0.f, FLT_EPSILON)) {
    const rstl::rc_ptr< CScanTreeNode > selected = GetNode(mSelectedNode);
    const rstl::rc_ptr< CScanTreeNode > previous = GetNode(mPreviousNode);
    if (selected->GetNodeType() != CScanTreeNode::kNT_Menu &&
        selected->GetNodeType() != CScanTreeNode::kNT_Slider &&
        previous->GetNodeType() != CScanTreeNode::kNT_Menu &&
        previous->GetNodeType() != CScanTreeNode::kNT_Slider) {
      const float collapse =
          0.0001f + gpTweakGui->GetLogBookNodeCollapseMotion().EvaluateAt(1.f - mTransition);
      const float selectedCollapse =
          0.0001f +
          gpTweakGui->GetLogBookSelectedNodeCollapseMotion().EvaluateAt(1.f - mTransition);
      const float expand =
          0.001f + gpTweakGui->GetLogBookNodeExpandMotion().EvaluateAt(1.f - mTransition);
      ScaleChildren(mSelectedNode, expand, expand, false);
      ScaleChildren(mPreviousNode, collapse, selectedCollapse, false);
    }
  }
}

float CScanTree::GetLayoutProgress() const {
  if (!CMath::IsEpsilon(mInitialLayoutTransition, 0.f, FLT_EPSILON)) {
    return 1.f - mInitialLayoutTransition;
  }
  const float collapse = gpTweakGui->GetLogBookNodeCollapseMotion().EvaluateAt(1.f - mTransition);
  const float expand = gpTweakGui->GetLogBookNodeExpandMotion().EvaluateAt(1.f - mTransition);
  return collapse < expand ? expand : collapse;
}

void CScanTree::UpdateNodePhysics(float dt) {
  const rstl::rc_ptr< CScanTreeNode > treeNode = GetNode(mSelectedNode);
  if (treeNode->GetNodeType() == CScanTreeNode::kNT_Category) {
    const rstl::rc_ptr< CScanTreeCategory > category(treeNode);
    const CVector3f& categoryPosition = category->GetPosition();
    for (int i = 0; i < category->GetChildCount(); ++i) {
      const int child = category->GetChild(i);
      const rstl::rc_ptr< CScanTreeNode > childNode = mNodes[child];
      CVector3f position = childNode->GetPosition();
      const rstl::rc_ptr< CScanTreeNode > target = mNodes[child];
      if (child != category->GetSelectedChild()) {
        CVector3f force = CVector3f::Zero();
        force += CalculateSeparationForce(rstl::rc_ptr< CScanTreeCategory >(category), child);
        force += CalculateNeighborForce(rstl::rc_ptr< CScanTreeCategory >(category), child);
        const CVector3f acceleration = childNode->GetAcceleration() + force;
        CVector3f velocity = childNode->GetVelocity() + 3.f * (dt * acceleration);
        if (velocity.CanBeNormalized()) {
          const float magnitude = velocity.Magnitude();
          const float speed = CMath::Clamp(0.1f, magnitude, 1.f);
          velocity = speed * (velocity * (1.f / magnitude));
        }
        position += dt * velocity;
        target->SetVelocity(velocity);
        target->SetAcceleration(acceleration);
      } else {
        target->SetVelocity(CVector3f::Zero());
        target->SetAcceleration(CVector3f::Zero());
      }
      const CVector3f offset = position - categoryPosition;
      if (offset.CanBeNormalized()) {
        const CVector3f direction = offset.AsNormalized();
        const CVector3f newPosition =
            categoryPosition + gpTweakGui->GetLogBookBranchLength() * direction;
        target->SetPosition(newPosition);
        target->SetDisplayPosition(newPosition);
      }
    }
  }
}

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
      const CAssetId scannableInfo = scan->GetScannableInfo();
      rstl::vector< CPlayerState::SPersistentState::SScanState >::const_iterator state =
          rstl::binary_find(scanStates.begin(), scanStates.end(), scannableInfo,
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
        rstl::rc_ptr< CPlayerState > playerState = gpGameState->PlayerState(0);
        playerState->SetScanFlag(scannableInfo, true);
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
    for (int i = 0; i < childCount; i++) {
      int index = category->GetChild(i);
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
    for (int i = 0; i < childCount; i++) {
      int child = category->GetChild(i);
      if (!UpdateCategoryViewed(child)) {
        allViewed = false;
      }
    }
  } else {
    return treeNode->IsViewed();
  }
  const bool result = allViewed;
  treeNode->SetViewed(result);
  return result;
}

CVector3f CScanTree::CalculatePairForce(float radius, float strength, const CVector3f& position,
                                        const CVector3f& otherPosition) const {
  const CVector3f delta = position - otherPosition;
  const float radiusSquared = radius * radius;
  if (delta.MagSquared() < radiusSquared) {
    const float falloff = 1.f - delta.MagSquared() / radiusSquared;
    if (delta.CanBeNormalized()) {
      return delta.AsNormalized() * falloff * strength;
    }
  }
  return CVector3f::Zero();
}

CVector3f CScanTree::CalculateSeparationForce(const rstl::rc_ptr< CScanTreeCategory >& category,
                                              int node) const {
  // Holds the nearest sibling position until it is replaced by the resulting force.
  CVector3f force = CVector3f::Zero();
  float nearestDistanceSquared = 999999.f;
  const rstl::rc_ptr< CScanTreeNode > self = mNodes[node];
  const CVector3f position = self->GetPosition();
  for (int i = 0; i < category->GetChildCount(); ++i) {
    const int child = category->GetChild(i);
    if (child == node) {
      continue;
    }
    const rstl::rc_ptr< CScanTreeNode > other = mNodes[child];
    if (!other->IsVisible()) {
      continue;
    }
    const CVector3f delta = other->GetPosition() - position;
    const float distanceSquared = CVector3f::Dot(delta, delta);
    if (distanceSquared < nearestDistanceSquared) {
      nearestDistanceSquared = distanceSquared;
      force = other->GetPosition();
    }
  }
  force = CalculatePairForce(6.1f, 0.8f, position, force);
  if (category->GetParentNode() != -1) {
    const rstl::rc_ptr< CScanTreeNode > parent = mNodes[category->GetParentNode()];
    force += CalculatePairForce(6.1f, 0.8f, position, parent->GetPosition());
  }
  return force;
}

CVector3f CScanTree::CalculateNeighborForce(const rstl::rc_ptr< CScanTreeCategory >& category,
                                            int node) const {
  CVector3f center = CVector3f::Zero();
  const rstl::rc_ptr< CScanTreeNode > self = mNodes[node];
  const CVector3f position = self->GetPosition();
  int neighborCount = 0;
  for (int i = 0; i < category->GetChildCount(); ++i) {
    const int child = category->GetChild(i);
    if (child == node) {
      continue;
    }
    const rstl::rc_ptr< CScanTreeNode > other = mNodes[child];
    if (!other->IsVisible()) {
      continue;
    }
    const CVector3f delta = position - other->GetPosition();
    if (CVector3f::Dot(delta, delta) < 16.f) {
      center += other->GetPosition();
      ++neighborCount;
    }
  }
  if (neighborCount > 0) {
    center *= 1.f / float(neighborCount);
    const CVector3f toCenter = center - position;
    if (toCenter.CanBeNormalized()) {
      const float distanceSquared = toCenter.MagSquared();
      const float scale = distanceSquared < 16.f ? distanceSquared / 16.f : 1.f;
      return toCenter.AsNormalized() * scale * 0.2f;
    }
  }
  return CVector3f::Zero();
}

CScanTreeCategory* LoadScanTreeCategory(int* id, const rstl::vector< int >& children,
                                        CInputStream& input) {
  SLdrScanTreeCategory sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrScanTreeCategory.inc"

  return rs_new CScanTreeCategory(*id & 0xffff, children, sldrThis.editorProperties.transform,
                                  sldrThis.nodeName, sldrThis.stringName);
}

CScanTreeScan* LoadScanTreeScan(int* id, CInputStream& input) {
  SLdrScanTreeScan sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrScanTreeScan.inc"

  return rs_new CScanTreeScan(*id & 0xffff, sldrThis.editorProperties.transform, sldrThis.nodeName,
                              sldrThis.scannableInfo.scannableInfo0, sldrThis.stringName);
}

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

CScanTreeMenu* LoadScanTreeMenu(int* id, CInputStream& input) {
  SLdrScanTreeMenu sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrScanTreeMenu.inc"

  return rs_new CScanTreeMenu(
      *id & 0xffff, sldrThis.editorProperties.transform, sldrThis.nodeName, sldrThis.stringName,
      static_cast< CScanTreeMenu::ESetting >(sldrThis.unknown_0x0261a4e0), sldrThis.menuStringTable,
      sldrThis.stringTableOption1, sldrThis.menuValue1, sldrThis.stringTableOption2,
      sldrThis.menuValue2, sldrThis.stringTableOption3, sldrThis.menuValue3,
      sldrThis.stringTableOption4, sldrThis.menuValue4);
}

CScanTreeSlider* LoadScanTreeSlider(int* id, CInputStream& input) {
  SLdrScanTreeSlider sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrScanTreeSlider.inc"

  return rs_new CScanTreeSlider(
      *id & 0xffff, sldrThis.editorProperties.transform, sldrThis.nodeName, sldrThis.stringName,
      static_cast< CScanTreeSlider::ESetting >(sldrThis.unknown_0x0261a4e0));
}
