#include "MetroidPrime/CScanTree.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/Text/CStringTable.hpp"
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

void CScanTreeNode::LockResources() {}

void CScanTreeNode::UnlockResources() {}

bool CScanTreeNode::AreResourcesLoaded() {}

const CVector3f& CScanTreeNode::GetDisplayPosition() const {}

void CScanTreeNode::SetDisplayPosition(const CVector3f& position) {}

const CVector3f& CScanTreeNode::GetPosition() const {}

void CScanTreeNode::SetPosition(const CVector3f& position) {}

int CScanTreeNode::GetId() const {}

int CScanTreeNode::GetParentNode() const {}

void CScanTreeNode::SetParentNode(int node) {}

rstl::wstring CScanTreeNode::GetName() const {}

const rstl::string& CScanTreeNode::GetNameStringName() const {}

CVector3f CScanTreeNode::GetVelocity() const {}

void CScanTreeNode::SetVelocity(const CVector3f& velocity) {}

CVector3f CScanTreeNode::GetAcceleration() const {}

void CScanTreeNode::SetAcceleration(const CVector3f& acceleration) {}

bool CScanTreeNode::IsVisible() const {}

void CScanTreeNode::SetVisible(bool visible) {}

float CScanTreeNode::GetOpacity() const {}

void CScanTreeNode::SetOpacity(float opacity) {}

int CScanTreeNode::GetDescendantCount() const {}

void CScanTreeNode::SetDescendantCount(int count) {}

int CScanTreeNode::GetVisibleDescendantCount() const {}

void CScanTreeNode::SetVisibleDescendantCount(int count) {}

bool CScanTreeNode::IsViewed() const {}

void CScanTreeNode::SetViewed(bool viewed) {}

CScanTreeCategory::CScanTreeCategory(int id, const rstl::vector< int >& children,
                                     const SLdrTransform& transform, CAssetId nameStringTable,
                                     const rstl::string& nameStringName)
: CScanTreeNode(id, transform, nameStringTable, nameStringName)
, mChildren(children)
, mSelectedChild(-1) {}

CScanTreeNode::ENodeType CScanTreeCategory::GetNodeType() const {}

int CScanTreeCategory::GetChildCount() const {}

int CScanTreeCategory::GetChild(int index) const {}

int CScanTreeCategory::GetSelectedChild() const {}

void CScanTreeCategory::SetSelectedChild(int node) {}

CScanTreeScan::CScanTreeScan(int id, const SLdrTransform& transform, CAssetId nameStringTable,
                             CAssetId scannableInfo, const rstl::string& nameStringName)
: CScanTreeNode(id, transform, nameStringTable, nameStringName), mScannableInfo(scannableInfo) {}

CScanTreeNode::ENodeType CScanTreeScan::GetNodeType() const {}

CAssetId CScanTreeScan::GetScannableInfo() const {}

CScanTreeInventory::CScanTreeInventory(int id, const SLdrTransform& transform,
                                       CAssetId nameStringTable, CAssetId scannableInfo,
                                       CPlayerState::EItemType inventoryItem,
                                       const rstl::string& nameStringName)
: CScanTreeScan(id, transform, nameStringTable, scannableInfo, nameStringName)
, mInventoryItem(inventoryItem) {}

CScanTreeNode::ENodeType CScanTreeInventory::GetNodeType() const {}

CPlayerState::EItemType CScanTreeInventory::GetInventoryItem() const {}

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
, mOptions() {}

CScanTreeNode::ENodeType CScanTreeMenu::GetNodeType() const {}

CScanTreeMenu::ESetting CScanTreeMenu::GetSetting() const {}

void CScanTreeMenu::LockResources() {}

void CScanTreeMenu::UnlockResources() {}

bool CScanTreeMenu::AreResourcesLoaded() {}

void CScanTreeMenu::RefreshSelectedOption() {}

int CScanTreeMenu::GetSelectedOption() const {}

void CScanTreeMenu::ApplySelectedOption() {}

int CScanTreeMenu::GetCurrentOptionIndex() const {}

void CScanTreeMenu::ApplyOption(int index) {}

rstl::wstring CScanTreeMenu::GetOptionName(int index) const {}

int CScanTreeMenu::GetOptionCount() const {}

CScanTreeSlider::CScanTreeSlider(int id, const SLdrTransform& transform, CAssetId nameStringTable,
                                 const rstl::string& nameStringName, ESetting setting)
: CScanTreeNode(id, transform, nameStringTable, nameStringName)
, mNormalizedValue(0.f)
, mSavedNormalizedValue(0.f)
, mSetting(setting) {}

CScanTreeNode::ENodeType CScanTreeSlider::GetNodeType() const {}

int CScanTreeSlider::GetMinOptionValue() const {}

int CScanTreeSlider::GetMaxOptionValue() const {}

int CScanTreeSlider::GetOptionValue() const {}

int CScanTreeSlider::GetDefaultOptionValue() const {}

void CScanTreeSlider::SetOptionValue(int value) {}

void CScanTreeSlider::RefreshNormalizedValue() {}

void CScanTreeSlider::ApplyNormalizedValue() {}

void CScanTreeSlider::SaveValue() {}

void CScanTreeSlider::RestoreSavedValue() {}

float CScanTreeSlider::GetSavedNormalizedValue() const {}

void CScanTreeSlider::SetNormalizedValue(float value) {}

float CScanTreeSlider::GetNormalizedValue() const {}

float CScanTreeSlider::GetNormalizedDefaultValue() const {}

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

void CScanTree::LoadAsync() {}

void CScanTree::ReserveNodes(int count) {}

void CScanTree::AddNode(CScanTreeNode* node) {}

void CScanTree::SetRootNode(int node) {}

void CScanTree::UpdateDescendantCounts() {}

void CScanTree::InitializeHierarchy() {}

bool CScanTree::UpdateNodeVisibility(CStateManager& mgr, int node) {}

void CScanTree::RefreshVisibility(CStateManager& mgr) {}

rstl::pair< uint, uint > CScanTree::GetScanCounts() const {}

void CScanTree::RandomizeChildPositions(int node) {}

void CScanTree::InitializeNodePositions(int node) {}

bool CScanTree::PollLoad() {}

void CScanTree::SelectNode(int node) {}

void CScanTree::SelectNode(int node, float duration) {}

void CScanTree::SelectScan(CAssetId scannableInfo, float duration) {}

int CScanTree::GetSelectedNode() const {}

int CScanTree::GetPreviousNode() const {}

float CScanTree::GetTransition() const {}

bool CScanTree::IsLoaded() const {}

rstl::rc_ptr< CScanTreeNode > CScanTree::GetNode(int node) const {}

int CScanTree::GetRootNode() const {}

void CScanTree::ScaleChildren(int node, float otherScale, float selectedScale,
                              bool excludeOptions) {}

void CScanTree::UpdateLayout() {}

float CScanTree::GetLayoutProgress() const {}

void CScanTree::UpdateNodePhysics(float dt) {}

void CScanTree::Update(float dt) {}

void CScanTree::RefreshViewed(CStateManager& mgr) {}

void CScanTree::MarkViewed(CStateManager& mgr, int node) {}

void CScanTree::UpdateViewedCategories() {}

bool CScanTree::UpdateCategoryViewed(int node) {}

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






