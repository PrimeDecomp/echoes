#include "MetroidPrime/Player/CScanDisplay.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"

class CScriptPointOfInterest;

// Structure-first scaffold. GUI, hint suppression and model presentation remain incomplete.

bool CScanDisplay::CScanTargetPredicate::IsValid(const CStateManager& mgr, TUniqueId id) const {
  if (id != mObject) {
    return false;
  }
  const CEntity* entity = mgr.GetObjectById(id);
  return TCastToConstPtr< CScriptPointOfInterest >(entity) != nullptr && entity->GetActive();
}

void CScanDisplay::SetScanMessageTypeEffect(CGuiTextPane*, bool) {
  // TODO: use CGuiTextSupport's typewriter options once the shared text-pane interface exists.
}

CScanDisplay::CScanDisplay(const CGuiFrame* selHud)
: mDataDotTexture(gpSimplePool->GetObj("TXTR_DataDot"))
, mState(kSS_Inactive)
, mObject(kInvalidUniqueId)
, mSelHud(selHud)
, mTextGroup(nullptr)
, mMessage(nullptr)
, mScrollMessage(nullptr)
, mXMark(nullptr)
, mAButton(nullptr)
, mDash(nullptr)
, mHistoryRoot(nullptr)
, mHistoryRight(nullptr)
, mStartButton(nullptr)
, mPressStart(nullptr)
, mModelObject(kInvalidUniqueId)
, mModelBounds(CAABox::Identity())
, mStartPosition(CVector3f::Zero())
, mEndPosition(CVector3f::Zero())
, mStartRotation(CQuaternion::NoRotation())
, mEndRotation(CQuaternion::NoRotation())
, mModelRotation(CQuaternion::NoRotation())
, mStartScale(CVector3f::Zero())
, mEndScale(CVector3f::Zero())
, mModelTransition(0.f)
, mXAlpha(0.f)
, mBodyAlpha(0.f)
, mPageCounter(0)
, mAPulse(1.f)
, mModelYaw(0.f)
, mAPulseCount(0)
, mScanComplete(false)
, mHintsSuppressed(false)
, mPreparePending(false)
, mCanOpenLogbook(false) {}

CScanDisplay::~CScanDisplay() {
  // TODO: release hint suppression through CHintOptions when its interface is recovered.
}

void CScanDisplay::StartScan(TUniqueId uid, const CScannableObjectInfo& info, CGuiTextPane* message,
                             CGuiTextPane* scrollMessage, CGuiWidget* textGroup, CGuiWidget* xMark,
                             CGuiWidget* aButton, CGuiWidget* dash, CGuiWidget* startButton,
                             CGuiTextPane* pressStart, CGuiWidget* historyRoot,
                             CGuiWidget* historyRight,
                             const rstl::vector< SScanHierarchyNode >& history,
                             const rstl::vector< SScanHistoryWidgets >& widgets, float scanTime,
                             bool showText, const CStateManager& mgr) {
  mScanComplete = scanTime >= info.GetTotalDownloadTime();
  mObject = uid;
  mScannableInfo = info;
  mState = kSS_Downloading;
  mPageCounter = 0;
  mXAlpha = 0.f;
  mAPulseCount = 0;
  mCanOpenLogbook = false;
  mMessage = message;
  mScrollMessage = scrollMessage;
  mTextGroup = textGroup;
  mXMark = xMark;
  mAButton = aButton;
  mDash = dash;
  mStartButton = startButton;
  mPressStart = pressStart;

  if (info.GetStringTableId() != kInvalidAssetId) {
    mScanString = TCachedToken< CStringTable >(
        gpSimplePool->GetObj(SObjectTag('STRG', info.GetStringTableId())));
    mScanString->Lock();
  }
  if (!mgr.fn_80036F10()) {
    mHistoryRoot = historyRoot;
    mHistoryRight = historyRight;
    mHistory = history;
    mHistoryWidgets = widgets;
    mCategoryName = rstl::wstring_l(L"");
    mHistoryStrings.clear();
    mHistoryStrings.reserve(history.size());
    for (int i = 0; i < history.size(); ++i) {
      mHistoryStrings.push_back(TCachedToken< CStringTable >(
          gpSimplePool->GetObj(SObjectTag('STRG', history[i].mStringTable))));
    }
    for (int i = 0; i < mHistoryStrings.size(); ++i) {
      mHistoryStrings[i].Lock();
    }
    if (info.GetScanTextureId() != kInvalidAssetId) {
      mScanTexture = TCachedToken< CTexture >(
          gpSimplePool->GetObj(SObjectTag('TXTR', info.GetScanTextureId())));
    }
    if (info.UsesScanModel() && info.GetStaticModelId(0) != kInvalidAssetId) {
      mScanModelToken = TCachedToken< CModel >(
          gpSimplePool->GetObj(SObjectTag('CMDL', info.GetStaticModelId(0))));
      mScanModelToken->Lock();
    }
    // TODO: configure history widgets.
  }
  // TODO: apply showText, initialize GUI colors and acquire hint suppression.
}

void CScanDisplay::StopScan() {
  switch (mState) {
  case kSS_Downloading:
  case kSS_DownloadComplete:
  case kSS_ViewingScan:
    mState = kSS_Done;
    break;
  default:
    break;
  }
}

void CScanDisplay::UpdateAPulse(float dt) {
  mAPulse += dt * (mAPulseCount < 3 ? 4.f : 2.f);
  if (mAPulse > 1.f) {
    mAPulse -= 2.f;
    if (mAPulseCount < 1) {
      CSfxManager::SfxStart(0x5aa, 127, 63);
    }
    ++mAPulseCount;
  }
}

void CScanDisplay::Update(float, float, const CStateManager&) {
  if (mState == kSS_Inactive) {
    mDataDotTexture.Unlock();
    mScanTexture.clear();
    mScanModelToken.clear();
    mScanModel = rstl::auto_ptr< CModelData >();
    return;
  }

  // TODO: update cached assets, scan/history text, fades and model transitions; release completed
  // scans.
}

void CScanDisplay::ProcessInput(const CFinalInput&) {
  // TODO: advance typewriter text/pages and refresh A/Start/dash prompts through shared GUI types.
}

void CScanDisplay::Draw(const CStateManager&) const {
  // TODO: interpolate presentation transforms and draw world, actor or resource scan geometry.
}

float CScanDisplay::GetTotalDownloadTime() const {
  return mScannableInfo ? mScannableInfo->GetTotalDownloadTime() : 0.f;
}

void CScanDisplay::RequestScanDisplay() { mPreparePending = true; }

CScanDisplay::CScanTargetPredicate::~CScanTargetPredicate() {}

void CScanDisplay::PrepareScanDisplay(const CStateManager&, int) {
  // TODO: resolve scan geometry while soft-paused, accumulate its bounds and prepare camera
  // endpoints.
}

float CScanDisplay::GetDownloadStartTime(int historyIndex) const {
  const float duration = GetTotalDownloadTime();
  return mHistory.empty() ? 0.f : historyIndex * duration / mHistory.size();
}

float CScanDisplay::GetDownloadFraction(int historyIndex, float time) const {
  return CMath::Clamp(0.f, (time - GetDownloadStartTime(historyIndex)) * mHistory.size(), 1.f);
}
