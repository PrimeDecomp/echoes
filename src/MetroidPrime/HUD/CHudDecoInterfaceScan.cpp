#include "MetroidPrime/HUD/CHudDecoInterfaceScan.hpp"

#include "GuiSys/CAuiBitmapMeter.hpp"
#include "GuiSys/CAuiEnergyBarT01.hpp"
#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiFrameLoader.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Player/CScanDisplay.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"

// Structure-first scaffold; scan-display integration and widget behavior remain incomplete.

// Matches the comparator symbol shared with CSlideShow.
struct SlideShowScanIdLess {
  bool operator()(const CPlayerState::SPersistentState::SScanState& scan, CAssetId id) const {
    return scan.mAssetId < id;
  }
  bool operator()(CAssetId id, const CPlayerState::SPersistentState::SScanState& scan) const {
    return id < scan.mAssetId;
  }
};

// Scan-bar coordinate callbacks for one-, two- and four-player layouts. Source names are unknown.
rstl::pair< CVector3f, CVector3f > GetScanBarCoordsOnePlayer(float t) {
  const float x = 4.1f * t - 2.05f;
  return rstl::pair< CVector3f, CVector3f >(CVector3f(x, 0.f, -0.2f), CVector3f(x, 0.f, 0.2f));
}

rstl::pair< CVector3f, CVector3f > GetScanBarCoordsTwoPlayer(float t) {
  const float x = 2.75f * t - 1.375f;
  return rstl::pair< CVector3f, CVector3f >(CVector3f(x, 0.f, -1.1f), CVector3f(x, 0.f, -0.7f));
}

rstl::pair< CVector3f, CVector3f > GetScanBarCoordsFourPlayer(float t) {
  const float x = 5.2f * t - 2.6f;
  return rstl::pair< CVector3f, CVector3f >(CVector3f(x, 0.f, -1.2f), CVector3f(x, 0.f, -0.6f));
}

static const CAuiEnergyBarT01::FCoordFunc skScanBarCoordFuncs[] = {
    GetScanBarCoordsOnePlayer, GetScanBarCoordsTwoPlayer, GetScanBarCoordsFourPlayer};

static const char* const skFlatFrameNames[] = {"FRME_ScanHudFlat", "FRME_ScanHudFlat2",
                                               "FRME_ScanHudFlat4"};

CHudDecoInterfaceScan::CHudDecoInterfaceScan(const CStateManager& mgr, CGuiFrame& frame,
                                             const TLockedToken< CStringTable >& strings,
                                             int playerIndex)
: mPlayerIndex(playerIndex)
, mFrameLoader(rs_new CGuiFrameLoader(
      gpResourceFactory->GetResourceIdByName(skFlatFrameNames[mgr.GetViewportLayoutIndex()])->id,
      *gpResourceFactory, *gpSimplePool))
, mFlatFrame(nullptr)
, mLoadedFlatFrame(nullptr)
, mStrings(strings)
, mScanDisplay(rs_new CScanDisplay(&frame))
, mLatestOrbitTarget(kInvalidUniqueId)
, mLatestScanningObject(kInvalidUniqueId)
, mLatestScanState(0)
, mScanningTime(0.f)
, mCurrentScan(kInvalidAssetId)
, x30(0.f)
, x34(1.f)
, x38(CVector3f::Zero())
, x44(CVector3f::Zero())
, x50(0.f)
, x54(gpTweakGui->GetScanSidesPositionStart())
, mScanningTextAlpha(0.f)
, mScanBarAlpha(0.f)
, mCamera(frame.GetFrameCamera())
, mHierarchyBufferLength(0) {}

CHudDecoInterfaceScan::~CHudDecoInterfaceScan() {}

void CHudDecoInterfaceScan::InitializeFlatFrame(const CStateManager& mgr) {
  CGuiCamera* flatCamera = mLoadedFlatFrame->GetFrameCamera();
  CGuiCamera::UCameraParms parms = flatCamera->GetParms();
  parms.mPerspective.mFov = mCamera->GetParms().mPerspective.mFov;
  flatCamera->SetParms(parms);
  flatCamera->SetO2WTransform(CTransform4f::Translate(mCamera->GetIdleXform().GetTranslation()));

  mScanGauge = mLoadedFlatFrame->FindWidget("basewidget_scanguage");
  mScanGauge->SetVisibility(false, kTM_Children);
  mScanningText = static_cast< CGuiTextPane* >(mLoadedFlatFrame->FindWidget("textpane_scanning"));
  mScanBar = static_cast< CAuiEnergyBarT01* >(mLoadedFlatFrame->FindWidget("energybart01_scanbar"));
  mMessage = static_cast< CGuiTextPane* >(mLoadedFlatFrame->FindWidget("textpane_message"));
  mScrollMessage =
      static_cast< CGuiTextPane* >(mLoadedFlatFrame->FindWidget("textpane_scrollmessage"));
  mTextGroup = mLoadedFlatFrame->FindWidget("basewidget_textgroup");
  mXMark = mLoadedFlatFrame->FindWidget("model_xmark");
  mAButton = mLoadedFlatFrame->FindWidget("model_abutton");
  mDash = mLoadedFlatFrame->FindWidget("model_dash");
  mStartButton = mLoadedFlatFrame->FindWidget("model_strtbutton");
  mPressStart = static_cast< CGuiTextPane* >(mLoadedFlatFrame->FindWidget("textpane_pstart"));

  mTextGroup->SetVisibility(false, kTM_Children);
  mScanningText->SetIsVisible(false);
  mScanningText->TextSupport().SetFontColor(gpTweakGuiColors->GetHUDMemoTextForegroundColor());
  mScanningText->TextSupport().SetOutlineColor(gpTweakGuiColors->GetHUDMemoTextOutlineColor());
  mMessage->TextSupport().SetFontColor(gpTweakGuiColors->GetHUDMemoTextForegroundColor());
  mMessage->TextSupport().SetOutlineColor(gpTweakGuiColors->GetHUDMemoTextOutlineColor());
  mScrollMessage->TextSupport().SetFontColor(gpTweakGuiColors->GetHUDMemoTextForegroundColor());
  mScrollMessage->TextSupport().SetOutlineColor(gpTweakGuiColors->GetHUDMemoTextOutlineColor());

  mScanBar->SetCoordFunc(skScanBarCoordFuncs[mgr.GetViewportLayoutIndex()]);
  mScanBar->SetTesselation(1.f);
  mScanBar->SetMaxEnergy(1.f);
  mScanBar->SetFilledColor(CColor(uchar(0x67), uchar(0xae), uchar(0xe1), uchar(0xff)));
  mScanBar->SetShadowColor(CColor(0u));
  mScanBar->SetEmptyColor(CColor(0u));
  mScanBar->SetFilledDrainSpeed(999.f);
  mScanBar->SetShadowDrainSpeed(999.f);
  mScanBar->SetShadowDrainDelay(0.f);
  mScanBar->SetIsAlwaysResetTimer(false);

  mXMark->SetVisibility(false, kTM_Children);
  mAButton->SetVisibility(false, kTM_Children);
  mDash->SetVisibility(false, kTM_Children);
  if (mStartButton != nullptr) {
    mStartButton->SetVisibility(false, kTM_Children);
  }
  if (mPressStart != nullptr) {
    mPressStart->SetVisibility(false, kTM_Children);
    mPressStart->TextSupport().SetText(rstl::wstring(mStrings->GetString("GoToLogbook")));
  }

  mHistoryWidgets.reserve(6);
  for (int i = 0; i < 6; ++i) {
    CGuiWidget* root =
        mLoadedFlatFrame->FindWidget(CBasics::Stringize("basewidget_crazy%d", i + 1));
    CGuiTextPane* history = static_cast< CGuiTextPane* >(
        mLoadedFlatFrame->FindWidget(CBasics::Stringize("textpane_history%d", i + 1)));
    CGuiTextPane* number = static_cast< CGuiTextPane* >(
        mLoadedFlatFrame->FindWidget(CBasics::Stringize("textpane_number%d", i + 1)));
    CAuiBitmapMeter* percent = static_cast< CAuiBitmapMeter* >(
        mLoadedFlatFrame->FindWidget(CBasics::Stringize("barmeter_percent%d", i + 1)));
    CGuiWidget* flash = mLoadedFlatFrame->FindWidget(CBasics::Stringize("model_flash%d", i + 1));
    CGuiWidget* doubleWidget =
        mLoadedFlatFrame->FindWidget(CBasics::Stringize("model_double%d", i + 1));
    CGuiWidget* percentFrame =
        mLoadedFlatFrame->FindWidget(CBasics::Stringize("model_percent%d", i + 1));
    if (number != nullptr) {
      number->TextSupport().SetFontColor(gpTweakGuiColors->GetScanHudHierarchyPercentTextColor());
    }
    if (percent != nullptr) {
      percent->SetShadowColor(CColor(0.f, 0.f, 0.f, 0.f));
      percent->SetColor(gpTweakGuiColors->GetScanHudHierarchyBarMeterColor());
    }
    if (percentFrame != nullptr) {
      percentFrame->SetColor(gpTweakGuiColors->GetScanHudHierarchyFrameColor());
    }
    mHistoryWidgets.push_back_unsafe(
        SScanHistoryWidgets(root, history, number, percent, flash, doubleWidget));
  }

  mHistoryLeft = mLoadedFlatFrame->FindWidget("basewidget_crazyleft");
  mHistoryRight = mLoadedFlatFrame->FindWidget("basewidget_crazyright");
  mHistoryRoot = mLoadedFlatFrame->FindWidget("basewidget_crazy");
  if (mHistoryRoot != nullptr) {
    mHistoryRoot->SetVisibility(false, kTM_Children);
  }
  CGuiWidget* bracket = mLoadedFlatFrame->FindWidget("model_message_bracket");
  if (bracket != nullptr) {
    bracket->SetColor(gpTweakGuiColors->GetScanWindowFrameActiveColor());
  }
  if (mHistoryLeft != nullptr) {
    mHistoryLeft->SetColor(gpTweakGuiColors->GetScanHudHierarchyFrameColor());
  }
  if (mHistoryRight != nullptr) {
    mHistoryRight->SetColor(gpTweakGuiColors->GetScanHudHierarchyFrameColor());
  }
}

void CHudDecoInterfaceScan::Update(float dt, const CStateManager& mgr) {
  const CPlayer* player = mgr.GetPlayer(mPlayerIndex);
  if (player->GetPlayerScanState() != CPlayer::kSS_NotScanning) {
    mScanningTime = player->GetScanningTime();
  }

  if (mLoadedFlatFrame == nullptr) {
    if (mFrameLoader.get() == nullptr) {
      if (mFlatFrame->GetIsFinishedLoading()) {
        mLoadedFlatFrame = mFlatFrame.get();
        InitializeFlatFrame(mgr);
      }
    } else {
      CGuiFrame* frame = mFrameLoader->CreateFrame();
      if (frame != nullptr) {
        mFlatFrame = rstl::auto_ptr< CGuiFrame >(frame);
        mFrameLoader = rstl::auto_ptr< CGuiFrameLoader >();
      }
    }
  } else {
    mLoadedFlatFrame->Update(dt);
    UpdateScanDisplay(mgr, dt);
  }

  if (player->GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan) {
    if (mHierarchy.size() == 0) {
      if (mHierarchyRequest.get() == nullptr) {
        StartHierarchyLoad();
      }
      if (CheckHierarchyLoadComplete()) {
        UpdateHierarchyProgress(mgr);
      }
    }
  } else if (mHierarchy.size() != 0) {
    ClearHierarchy();
  }
}

void CHudDecoInterfaceScan::Draw(const CStateManager& mgr) const {
  if (mLoadedFlatFrame != nullptr) {
    mLoadedFlatFrame->Draw(CGuiWidgetDrawParms::Default());
  }
  mScanDisplay->Draw(mgr);
}

void CHudDecoInterfaceScan::ProcessControllerInput(const CFinalInput& input) {
  mScanDisplay->ProcessInput(input);
}

void CHudDecoInterfaceScan::UpdateScanDisplay(const CStateManager& mgr, float dt) {
  CPlayer* player = mgr.Player(mPlayerIndex);
  const CPlayer::EPlayerScanState scanState = player->GetPlayerScanState();
  const TUniqueId orbitTarget = player->GetOrbitTargetId();
  const TUniqueId scanningObject = player->GetScanningObject();
  const float scanningTime = player->GetScanningTime();

  if (scanState != mLatestScanState) {
    if (player->IsNewScanScanning() && scanState == CPlayer::kSS_Scanning) {
      mScanningText->TextSupport().SetText(
          rstl::wstring_l(mStrings->GetString("DownloadingMessage")));
      mScanningText->TextSupport().SetTypeWriteEffectOptions(false, 0.f, 40.f);
      mScanningTextAlpha = 1.f;
    }
    mLatestScanState = scanState;
  }

  if (scanningObject != mLatestScanningObject) {
    mLatestScanningObject = scanningObject;
  }

  if (orbitTarget != mLatestOrbitTarget) {
    mLatestOrbitTarget = orbitTarget;
    if (orbitTarget != kInvalidUniqueId && !player->ObjectInScanningRange(orbitTarget, mgr)) {
      mScanningText->TextSupport().SetText(rstl::wstring_l(L""));
      mScanningText->TextSupport().SetText(
          rstl::wstring_l(mStrings->GetString("ScanOutOfRangeMessage")));
      mScanningText->TextSupport().SetTypeWriteEffectOptions(true, 0.f, 40.f);
      mScanningTextAlpha = 8.f;
    }
  }

  const CScannableObjectInfo* info = GetCurrScanInfo(mgr);
  CAssetId scanId = kInvalidAssetId;
  if (info != nullptr) {
    scanId = info->GetScannableObjectId();
  }
  if (mScanDisplay->GetScanningObject() != mLatestScanningObject || info == nullptr ||
      scanId != mCurrentScan) {
    mScanDisplay->StopScan();
    mCurrentScan = kInvalidAssetId;
    if (mScanDisplay->GetScanState() == CScanDisplay::kSS_Inactive && info != nullptr) {
      mScanningTextAlpha = rstl::min_val(scanningTime >= info->GetTotalDownloadTime() ? 0.f : 1.f,
                                         mScanningTextAlpha);
      UpdateHierarchyProgress(mgr);
      rstl::vector< SScanHierarchyNode > history;
      BuildScanHistory(info->GetScannableObjectId(), history);
      mCurrentScan = scanId;
      mScanDisplay->StartScan(mLatestScanningObject, *info, mMessage, mScrollMessage, mTextGroup,
                              mXMark, mAButton, mDash, mStartButton, mPressStart, mHistoryRoot,
                              mHistoryRight, history, mHistoryWidgets, scanningTime,
                              !mgr.IsMultiplayer(), mgr);
    }
  }

  mScanDisplay->Update(dt, scanningTime, mgr);

  if (mLatestScanningObject != kInvalidUniqueId && GetCurrScanInfo(mgr) != nullptr) {
    const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mLatestScanningObject));
    if (actor != nullptr && actor->GetScannableObjectInfo() != nullptr) {
      mScanBar->SetCurrEnergy(mScanningTime /
                                  actor->GetScannableObjectInfo()->GetTotalDownloadTime(),
                              CAuiEnergyBarT01::kSM_Normal);
    }
  } else if (mScanBar != nullptr) {
    mScanBar->SetCurrEnergy(0.f, CAuiEnergyBarT01::kSM_Normal);
  }

  if (mLatestScanState != CPlayer::kSS_Scanning) {
    mScanningTextAlpha = rstl::max_val(0.f, mScanningTextAlpha - 4.f * dt);
  }
  float textAlpha = mScanningTextAlpha;
  if (textAlpha > 0.f) {
    textAlpha = rstl::min_val(textAlpha, 1.f);
    mScanningText->SetColor(CColor::White().WithAlphaOf(textAlpha));
    mScanningText->SetIsVisible(true);
  } else {
    mScanningText->SetIsVisible(false);
  }

  if (GetCurrScanInfo(mgr) != nullptr) {
    mScanBarAlpha = rstl::min_val(mScanBarAlpha + 2.f * dt, 1.f);
  } else {
    mScanBarAlpha = rstl::max_val(0.f, mScanBarAlpha - 2.f * dt);
  }
  float barAlpha = mScanBarAlpha;
  if (barAlpha > 0.f) {
    barAlpha = rstl::min_val(barAlpha, 1.f);
    mScanGauge->SetColor(CColor::White().WithAlphaOf(barAlpha));
    mScanGauge->SetVisibility(true, kTM_Children);
  } else {
    mScanGauge->SetVisibility(false, kTM_Children);
  }
}

const CScannableObjectInfo* CHudDecoInterfaceScan::GetCurrScanInfo(const CStateManager& mgr) const {
  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mLatestScanningObject));
  return actor != nullptr ? actor->GetScannableObjectInfo() : nullptr;
}

float CHudDecoInterfaceScan::GetMessageTextAlpha() const {
  const float scanningAlpha = rstl::min_val(1.f, mScanningTextAlpha);
  return 1.f - rstl::max_val(scanningAlpha, mScanDisplay->GetBodyAlpha());
}

void CHudDecoInterfaceScan::StartHierarchyLoad() {
  const SObjectTag* tag = gpResourceFactory->GetResourceIdByName("DUMB_ScanHierarchy");
  mHierarchyBufferLength = gpResourceFactory->GetResLoader().ResourceSize(*tag);
  if (mHierarchyBufferLength != 0) {
    mHierarchyBuffer = rstl::auto_ptr< uchar >(
        static_cast< uchar* >(CMemory::Alloc(mHierarchyBufferLength, IAllocator::kHI_RoundUpLen)));
    mHierarchyRequest =
        rstl::auto_ptr< CDvdRequest >(gpResourceFactory->GetResLoader().LoadResourceAsync(
            *tag, reinterpret_cast< char* >(mHierarchyBuffer.get())));
  }
}

bool CHudDecoInterfaceScan::CheckHierarchyLoadComplete() {
  if (!mHierarchyRequest.null()) {
    if (!mHierarchyRequest->IsComplete()) {
      return false;
    }
    CMemoryInStream in(mHierarchyBuffer.get(), mHierarchyBufferLength);
    ReadHierarchy(in);
    mHierarchyBuffer = rstl::auto_ptr< uchar >();
    mHierarchyRequest = rstl::auto_ptr< CDvdRequest >();
  }
  return true;
}

void CHudDecoInterfaceScan::ReadHierarchy(CInputStream& in) {
  if (in.ReadInt32() == 'HIER' && in.ReadUint16() == 0) {
    const uint count = in.ReadUint16();
    mHierarchy.reserve(count);
    for (uint i = 0; i < count; ++i) {
      const SScanHierarchyNode node(in);
      mHierarchy.push_back_unsafe(node);
    }
  }
}

void CHudDecoInterfaceScan::ClearHierarchy() { mHierarchy.clear(); }

void CHudDecoInterfaceScan::UpdateHierarchyProgress(const CStateManager& mgr) {
  for (rstl::vector< SScanHierarchyNode >::iterator it = mHierarchy.begin(); it != mHierarchy.end();
       ++it) {
    it->mTotalScans = 0;
    it->mCompletedScans = 0;
  }

  const rstl::vector< CPlayerState::SPersistentState::SScanState >& scanStates =
      mgr.PlayerState(mPlayerIndex)->ScanStates();
  for (rstl::vector< SScanHierarchyNode >::iterator it = mHierarchy.begin(); it != mHierarchy.end();
       ++it) {
    if (it->mScan == kInvalidAssetId) {
      continue;
    }
    rstl::vector< CPlayerState::SPersistentState::SScanState >::const_iterator state =
        rstl::binary_find(scanStates.begin(), scanStates.end(), it->mScan, SlideShowScanIdLess());
    const bool finished = state != scanStates.end() && state->mProgress == 0xff;
    for (int parent = it->mParent; parent != -1; parent = mHierarchy[parent].mParent) {
      ++mHierarchy[parent].mTotalScans;
      if (finished) {
        ++mHierarchy[parent].mCompletedScans;
      }
    }
  }
}

void CHudDecoInterfaceScan::BuildScanHistory(CAssetId scan,
                                             rstl::vector< SScanHierarchyNode >& history) const {
  rstl::reserved_vector< SScanHierarchyNode, 16 > chain;
  for (rstl::vector< SScanHierarchyNode >::const_iterator it = mHierarchy.begin();
       it != mHierarchy.end(); ++it) {
    if (it->mScan == scan) {
      chain.push_back(*it);
      for (int parent = it->mParent; parent != -1; parent = mHierarchy[parent].mParent) {
        chain.push_back(mHierarchy[parent]);
      }
      break;
    }
  }

  if (chain.size() == 0) {
    return;
  }
  chain.pop_back();
  if (chain.size() == 0) {
    return;
  }
  chain.pop_back();
  while (chain.size() > mHistoryWidgets.size()) {
    chain.pop_back();
  }

  if (chain.size() != 0) {
    history.reserve(chain.size());
    for (int i = 0; i < chain.size(); ++i) {
      history.push_back_unsafe(chain[chain.size() - i - 1]);
    }
  }
}

void CHudDecoInterfaceScan::PrepareScanDisplay(const CStateManager& mgr, int playerIndex) {
  if (!mScanDisplay.null()) {
    mScanDisplay->PrepareScanDisplay(mgr, playerIndex);
  }
}
