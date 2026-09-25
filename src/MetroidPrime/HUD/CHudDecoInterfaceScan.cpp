#include "MetroidPrime/HUD/CHudDecoInterfaceScan.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiFrameLoader.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "rstl/pair.hpp"

// Structure-first scaffold; scan-display integration and widget behavior remain incomplete.

// Scan-bar coordinate callbacks for one-, two- and four-player layouts. Source names are unknown.
rstl::pair< CVector3f, CVector3f > fn_8021dbcc(float t) {
  const float x = 4.1f * t - 2.05f;
  return rstl::pair< CVector3f, CVector3f >(CVector3f(x, 0.f, -0.2f), CVector3f(x, 0.f, 0.2f));
}

rstl::pair< CVector3f, CVector3f > fn_8021db98(float t) {
  const float x = 2.75f * t - 1.375f;
  return rstl::pair< CVector3f, CVector3f >(CVector3f(x, 0.f, -1.1f), CVector3f(x, 0.f, -0.7f));
}

rstl::pair< CVector3f, CVector3f > fn_8021db64(float t) {
  const float x = 5.2f * t - 2.6f;
  return rstl::pair< CVector3f, CVector3f >(CVector3f(x, 0.f, -1.2f), CVector3f(x, 0.f, -0.6f));
}

CHudDecoInterfaceScan::CHudDecoInterfaceScan(const CStateManager& mgr, CGuiFrame& frame,
                                             const TLockedToken< CStringTable >& strings,
                                             int playerIndex)
: mPlayerIndex(playerIndex)
, mLoadedFlatFrame(nullptr)
, mStrings(strings)
, mScanDisplay(nullptr)
, mLatestHudPoi(kInvalidUniqueId)
, mLatestScanningObject(kInvalidUniqueId)
, mLatestScanState(0)
, mScanningTime(0.f)
, mCurrentScan(kInvalidAssetId)
, x30(0.f)
, x34(1.f)
, x38(CVector3f::Zero())
, x44(CVector3f::Zero())
, x50(0.f)
, x54(0.f)
, mScanningTextAlpha(0.f)
, mScanBarAlpha(0.f)
, mCamera(frame.GetFrameCamera())
, mScanningText(nullptr)
, mScanGauge(nullptr)
, mScanBar(nullptr)
, mTextGroup(nullptr)
, mMessage(nullptr)
, mScrollMessage(nullptr)
, mXMark(nullptr)
, mAButton(nullptr)
, mDash(nullptr)
, mStartButton(nullptr)
, mPressStart(nullptr)
, mHistoryRoot(nullptr)
, mHistoryLeft(nullptr)
, mHistoryRight(nullptr)
, mHierarchyBufferLength(0) {
  // TODO: create the layout-specific flat-frame loader and CScanDisplay; read the initial tweak.
}

CHudDecoInterfaceScan::~CHudDecoInterfaceScan() {
  // TODO: destroy the owned CScanDisplay once its complete shared declaration is available.
}

void CHudDecoInterfaceScan::InitializeFlatFrame(const CStateManager&) {
  // TODO: bind scan/history widgets, copy the HUD camera and initialize colors and meter settings.
}

void CHudDecoInterfaceScan::Update(float, const CStateManager&) {
  // TODO: finish flat-frame loading, update scanning and manage the optional hierarchy resource.
}

void CHudDecoInterfaceScan::Draw(const CStateManager&) const {
  // TODO: draw the flat frame, then the scan display through shared GUI interfaces.
}

void CHudDecoInterfaceScan::ProcessControllerInput(const CFinalInput&) {
  // TODO: forward to CScanDisplay's input handler.
}

void CHudDecoInterfaceScan::UpdateScanDisplay(const CStateManager&, float) {
  // TODO: reconcile the player's scan target/state, history, scan progress and widget fades.
}

const CScannableObjectInfo* CHudDecoInterfaceScan::GetCurrScanInfo(const CStateManager& mgr) const {
  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mLatestScanningObject));
  return actor != nullptr ? actor->GetScannableObjectInfo() : nullptr;
}

float CHudDecoInterfaceScan::GetMessageTextAlpha() const {
  // TODO: combine the scanning-text fade with CScanDisplay's body alpha.
  return 0.f;
}

void CHudDecoInterfaceScan::StartHierarchyLoad() {
  // TODO: allocate the DUMB_ScanHierarchy buffer and start its asynchronous resource request.
}

bool CHudDecoInterfaceScan::CheckHierarchyLoadComplete() {
  if (mHierarchyRequest.null()) {
    return true;
  }
  if (!mHierarchyRequest->IsComplete()) {
    return false;
  }
  CInputStream in(mHierarchyBuffer.get(), mHierarchyBufferLength);
  ReadHierarchy(in);
  mHierarchyBuffer = rstl::auto_ptr< uchar >();
  mHierarchyRequest = rstl::auto_ptr< CDvdRequest >();
  return true;
}

void CHudDecoInterfaceScan::ReadHierarchy(CInputStream&) {
  // TODO: validate the HIER tag/version, then read the 0x24-byte node records.
}

void CHudDecoInterfaceScan::ClearHierarchy() { mHierarchy.clear(); }

void CHudDecoInterfaceScan::UpdateHierarchyProgress(const CStateManager&) {
  // TODO: count completed scan entries and propagate totals up each parent chain.
}

void CHudDecoInterfaceScan::BuildScanHistory(CAssetId, rstl::vector< SScanHierarchyNode >&) const {
  // TODO: collect the ancestor chain, remove its outermost categories and reverse it for the HUD.
}

void CHudDecoInterfaceScan::PrepareScanDisplay(const CStateManager&, int) {
  // TODO: forward preparation of scanned-object geometry to CScanDisplay.
}
