#include "MetroidPrime/Player/CScanDisplay.hpp"

#include "GuiSys/CAuiBitmapMeter.hpp"
#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CStaticGeometryMap.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPointOfInterest.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "rstl/math.hpp"

bool CScanDisplay::CScanTargetPredicate::IsValid(const CStateManager& mgr, TUniqueId id) const {
  if (id.value != mObject.value) {
    return false;
  }
  if (const CScriptPointOfInterest* point =
          TCastToConstPtr< CScriptPointOfInterest >(mgr.GetObjectById(id))) {
    return point->GetActive();
  }
  return false;
}

SScanHierarchyNode::SScanHierarchyNode(CInputStream& in)
: mStringTable(in.ReadInt32())
, mName(in)
, mScan(in.ReadInt32())
, mParent(in.ReadInt32())
, mTotalScans(0)
, mCompletedScans(0) {}

SScanHistoryWidgets::SScanHistoryWidgets(CGuiWidget* root, CGuiTextPane* history,
                                         CGuiTextPane* number, CAuiBitmapMeter* percent,
                                         CGuiWidget* flash, CGuiWidget* doubleWidget)
: mRoot(root)
, mHistory(history)
, mNumber(number)
, mPercent(percent)
, mFlash(flash)
, mDouble(doubleWidget) {}

void CScanDisplay::SetScanMessageTypeEffect(CGuiTextPane* pane, bool type) {
  if (type) {
    pane->TextSupport().SetTypeWriteEffectOptions(true, 0.1f, 60.f);
  } else {
    pane->TextSupport().SetTypeWriteEffectOptions(false, 0.f, 0.f);
  }
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
  if (mHintsSuppressed) {
    gpGameState->HintOptions().SetScanDisplayActive(false);
    mHintsSuppressed = false;
  }
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

  mTextGroup->SetVisibility(showText, kTM_Children);
  mTextGroup->SetColor(CColor::White().WithAlphaOf(0.f));
  gpGameState->HintOptions().SetScanDisplayActive(true);
  mHintsSuppressed = true;

  if (mScannableInfo->GetStringTableId() != kInvalidAssetId) {
    mScanString = TCachedToken< CStringTable >(
        gpSimplePool->GetObj(SObjectTag('STRG', mScannableInfo->GetStringTableId())));
    mScanString->Lock();
  }
  if (!mgr.IsMultiplayer()) {
    mHistoryRoot = historyRoot;
    mHistoryRight = historyRight;
    mHistory = history;
    mHistoryWidgets = widgets;
    if (history.empty()) {
      mHistoryRoot->SetVisibility(false, kTM_Children);
    } else {
      mHistoryRoot->SetVisibility(true, kTM_Children);
      mHistoryRoot->SetColor(CColor::White().WithAlphaOf(0.f));
      mHistoryRight->SetColor(gpTweakGuiColors->GetScanHudHierarchyInactiveFrameColor());
      const int count = mHistory.size();
      for (int i = 0; i < mHistoryWidgets.size(); ++i) {
        SScanHistoryWidgets& widget = mHistoryWidgets[i];
        widget.mRoot->SetVisibility(i < count, kTM_Children);
        if (i < count) {
          if (i < count - 1) {
            widget.mHistory->TextSupport().SetFontColor(
                gpTweakGuiColors->GetScanHudHierarchyTextColor());
            widget.mDouble->SetColor(gpTweakGuiColors->GetScanHudHierarchyTextFrameColor());
          } else if (i == count - 1) {
            widget.mHistory->TextSupport().SetFontColor(
                gpTweakGuiColors->GetScanHudHierarchyFinalTextColor());
            widget.mDouble->SetColor(gpTweakGuiColors->GetScanHudHierarchyFinalTextFrameColor());
          }
          const SScanHierarchyNode& node = mHistory[i];
          const bool complete = node.mCompletedScans == node.mTotalScans || node.mTotalScans == 0;
          widget.mFlash->SetColor(
              complete ? gpTweakGuiColors->GetScanHudHierarchyCompleteFlashIconColor()
                       : gpTweakGuiColors->GetScanHudHierarchyFlashIconColor());
        }
      }
    }
    mCategoryName = rstl::wstring_l(L"");
    mHistoryStrings.clear();
    mHistoryStrings.reserve(history.size());
    for (int i = 0; i < history.size(); ++i) {
      mHistoryStrings.push_back_unsafe(TCachedToken< CStringTable >(
          gpSimplePool->GetObj(SObjectTag('STRG', history[i].mStringTable))));
    }
    for (int i = 0; i < mHistoryStrings.size(); ++i) {
      mHistoryStrings[i].Lock();
    }
    if (mScannableInfo) {
      if (mScannableInfo->GetScanTextureId() != kInvalidAssetId) {
        mScanTexture = TCachedToken< CTexture >(
            gpSimplePool->GetObj(SObjectTag('TXTR', mScannableInfo->GetScanTextureId())));
      }
      if (mScannableInfo->UsesScanModel() &&
          mScannableInfo->GetStaticModelId(0) != kInvalidAssetId) {
        mScanModelToken = TCachedToken< CModel >(
            gpSimplePool->GetObj(SObjectTag('CMDL', mScannableInfo->GetStaticModelId(0))));
        mScanModelToken->Lock();
      }
    }
  }
}

void CScanDisplay::StopScan() {
  switch (mState) {
  case kSS_Inactive:
    return;
  case kSS_Downloading:
  case kSS_DownloadComplete:
  case kSS_ViewingScan:
    mState = kSS_Done;
    break;
  case kSS_Done:
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

void CScanDisplay::Update(float dt, float scanningTime, const CStateManager& mgr) {
  if (mState == kSS_Inactive) {
    mDataDotTexture.Unlock();
    mScanTexture.clear();
    mScanModelToken.clear();
    mScanModel = rstl::auto_ptr< CModelData >();
    return;
  }

  mDataDotTexture.Lock();
  mDataDotTexture.IsLoaded();
  if (mScanTexture) {
    mScanTexture->Lock();
    mScanTexture->IsLoaded();
  }
  if (mScanModelToken) {
    mScanModelToken->Lock();
    mScanModelToken->IsLoaded();
  }

  if (!mHistoryStrings.empty()) {
    rstl::vector< TCachedToken< CStringTable > >::iterator it = mHistoryStrings.begin();
    for (; it != mHistoryStrings.end(); ++it) {
      if (!it->IsLoaded()) {
        break;
      }
    }
    if (it == mHistoryStrings.end()) {
      const rstl::wstring spacing(gpStringTable->GetString("LogbookLineSpacing"));
      for (int i = 0; i < mHistoryStrings.size(); ++i) {
        const CStringTable* table = mHistoryStrings[i].GetObject();
        if (table) {
          mHistoryWidgets[i].mHistory->TextSupport().SetText(
              spacing + rstl::wstring(table->GetString(mHistory[i].mName.data())));
        }
      }
      const CStringTable* table = mHistoryStrings[0].GetObject();
      if (table) {
        mCategoryName = rstl::wstring(table->GetString(mHistory[0].mName.data()));
      }
      mHistoryStrings.clear();
    }
  }

  bool active = false;
  if (mState == kSS_Done) {
    mBodyAlpha = rstl::max_val(0.f, mBodyAlpha - 2.f * dt);
    mModelTransition = rstl::max_val(mModelTransition - 2.f * dt, 0.f);
    mModelYaw = 0.f;
    if (mBodyAlpha > 0.f) {
      active = true;
    } else {
      mScanString.clear();
      mHistoryStrings.clear();
      if (mHistoryRoot) {
        mHistoryRoot->SetVisibility(false, kTM_Children);
      }
      mModelObject = kInvalidUniqueId;
    }
  } else {
    active = true;
    mBodyAlpha = rstl::min_val(1.f, mBodyAlpha + 2.f * dt);
    if (mHistoryRoot) {
      mHistoryRoot->SetColor(CColor::White().WithAlphaOf(mBodyAlpha));
    }
    if (mState == kSS_DownloadComplete) {
      mModelTransition = rstl::min_val(mModelTransition + dt, 1.f);
      if (mScrollMessage->TextSupport().GetIsTextSupportFinishedLoading()) {
        mXAlpha = rstl::max_val(0.f, mXAlpha - dt);
      }
      if (mXAlpha < 0.5f) {
        UpdateAPulse(dt);
      }
      if (CMath::AbsF(mModelTransition - 1.f) < 0.00001f) {
        mModelYaw += 30.f * dt;
      } else {
        mModelYaw = 0.f;
      }
    } else if (mState == kSS_ViewingScan) {
      mModelTransition = rstl::min_val(mModelTransition + dt, 1.f);
      UpdateAPulse(dt);
      if (mXAlpha == 1.f) {
        mMessage->TextSupport().SetText(mScanString->GetObject()->GetString(0));
        SetScanMessageTypeEffect(mMessage, !mScanComplete);
      }
      if (CMath::AbsF(mModelTransition - 1.f) < 0.00001f) {
        mModelYaw += 30.f * dt;
      } else {
        mModelYaw = 0.f;
      }
    } else if (mState == kSS_Downloading) {
      mModelYaw = 0.f;
      if (mHistoryRoot) {
        if (mHistory.size() > 3) {
          const float fraction = GetDownloadFraction(3, scanningTime);
          mHistoryRight->SetColor(
              CColor::Lerp(gpTweakGuiColors->GetScanHudHierarchyInactiveFrameColor(),
                           gpTweakGuiColors->GetScanHudHierarchyFrameColor(), fraction));
        }
        for (int i = 0; i < mHistoryWidgets.size(); ++i) {
          if (i < mHistory.size()) {
            const float fraction = GetDownloadFraction(i, scanningTime);
            const SScanHierarchyNode& node = mHistory[i];
            SScanHistoryWidgets& widget = mHistoryWidgets[i];
            int percent;
            if (node.mTotalScans == 0) {
              percent = rstl::min_val(int(105.f * fraction), 100);
            } else if (mScanComplete) {
              percent = node.mCompletedScans * 100 / node.mTotalScans;
            } else {
              const int before = node.mCompletedScans * 100 / node.mTotalScans;
              const int after = (node.mCompletedScans + 1) * 100 / node.mTotalScans;
              percent = (1.f - fraction) * before + fraction * after;
            }
            const bool complete =
                node.mCompletedScans >= node.mTotalScans - 1 || node.mTotalScans == 0;
            const CColor& color =
                complete ? gpTweakGuiColors->GetScanHudHierarchyCompleteFlashIconColor()
                         : gpTweakGuiColors->GetScanHudHierarchyFlashIconColor();
            if (fraction < 0.5f) {
              widget.mFlash->SetColor(CColor::White().WithAlphaOf(0.f));
            } else {
              widget.mFlash->SetColor(
                  CColor::Lerp(gpTweakGuiColors->GetScanHudHierarchyFlashFlashIconColor(), color,
                               2.f * (fraction - 0.5f)));
            }
            widget.mPercent->SetTargetFraction(percent / 100.f);
            widget.mPercent->SetCurrentFraction(percent / 100.f);
            widget.mNumber->TextSupport().SetText(
                rstl::string(CBasics::Stringize("%d%%", percent)));
            widget.mRoot->SetColor(CColor::White().WithAlphaOf(fraction));
          }
        }
      }
      if (scanningTime >= mScannableInfo->GetTotalDownloadTime() && mScanString &&
          mScanString->IsLoaded()) {
        if (mScanComplete || mHistory.empty()) {
          mState = kSS_ViewingScan;
          mXAlpha = 1.f;
          mAPulse = 1.f;
          CSfxManager::SfxStart(0x41b, 127, 64);
        } else {
          mState = kSS_DownloadComplete;
          mXAlpha = 1.f;
          mAPulse = 1.f;
          rstl::wstring message(gpStringTable->GetString("DownloadedLogBookMsgLeftPart"));
          message.append(mCategoryName);
          message.append(gpStringTable->GetString("DownloadedLogBookMsgRightPart"), -1);
          mMessage->TextSupport().SetText(message);
          SetScanMessageTypeEffect(mMessage, true);
          CSfxManager::SfxStart(0xdb0, 127, 64);
        }
        if (mScanString->GetObject()->GetStringCount() > 2) {
          mScrollMessage->TextSupport().SetText(mScanString->GetObject()->GetString(1), true);
          SetScanMessageTypeEffect(mScrollMessage, !mScanComplete);
        }
        mScrollMessage->SetIsVisible(false);
        RequestScanDisplay();
        const CActor* boss = TCastToConstPtr< CActor >(mgr.GetObjectById(mgr.GetBossId()));
        mCanOpenLogbook = (!boss || !boss->GetHealthInfo()) && !CSamusHud::IsHudMemoVisible(0);
      }
    }
  }

  if (active) {
    const CColor color = CColor::White().WithAlphaOf(mBodyAlpha);
    mTextGroup->SetColor(color);
    if (mHistoryRoot) {
      mHistoryRoot->SetColor(color);
    }
  } else {
    mState = kSS_Inactive;
    mObject = kInvalidUniqueId;
    mScannableInfo = rstl::optional_object_null();
    mMessage->TextSupport().SetText(rstl::wstring_l(L""));
    mScrollMessage->TextSupport().SetText(rstl::wstring_l(L""));
    mTextGroup->SetVisibility(false, kTM_Children);
    mXMark->SetVisibility(false, kTM_Children);
    mAButton->SetVisibility(false, kTM_Children);
    mDash->SetVisibility(false, kTM_Children);
    if (mStartButton) {
      mStartButton->SetVisibility(false, kTM_Children);
    }
    if (mPressStart) {
      mPressStart->SetVisibility(false, kTM_Children);
    }
    if (mHintsSuppressed) {
      gpGameState->HintOptions().SetScanDisplayActive(false);
      mHintsSuppressed = false;
    }
    mMessage = nullptr;
    mScrollMessage = nullptr;
    mTextGroup = nullptr;
    mXMark = nullptr;
    mAButton = nullptr;
    mDash = nullptr;
    mStartButton = nullptr;
    mPressStart = nullptr;
    mWorldModels = rstl::vector< rstl::pair< TAreaId, int > >();
    mModelObject = kInvalidUniqueId;
    mScanString = rstl::optional_object_null();
    mPageCounter = 0;
    mScanComplete = false;
  }
}

void CScanDisplay::ProcessInput(const CFinalInput& input) {
  const bool inactive = GetScanState() == kSS_Inactive || GetScanState() == kSS_Done;
  if (inactive) {
    return;
  }
  if (mState == kSS_DownloadComplete && mXAlpha == 0.f) {
    if (input.PA()) {
      CGuiTextSupport& support = mMessage->TextSupport();
      if (support.GetCurTime() < support.GetTotalAnimationTime()) {
        support.SetCurTime(support.GetTotalAnimationTime());
      } else {
        mState = kSS_ViewingScan;
        mXAlpha = 1.f;
        CSfxManager::SfxStart(0x10c9, 127, 64);
      }
    }
  } else if (mState == kSS_ViewingScan) {
    const int oldCounter = mPageCounter;
    const int totalPages = mScrollMessage->TextSupport().GetTotalPageCount();
    if (input.PA() && totalPages != -1) {
      CGuiTextSupport& support =
          oldCounter == 0 ? mMessage->TextSupport() : mScrollMessage->TextSupport();
      if (support.GetCurTime() < support.GetTotalAnimationTime()) {
        support.SetCurTime(support.GetTotalAnimationTime());
      } else {
        mPageCounter = rstl::min_val(mPageCounter + 1, int(totalPages));
      }
    }
    if (mPageCounter != oldCounter) {
      CSfxManager::SfxStart(0x10c9, 127, 64);
      if (mPageCounter == 0) {
        mMessage->SetIsVisible(true);
        mScrollMessage->SetIsVisible(false);
      } else {
        if (oldCounter == 0) {
          mMessage->SetIsVisible(false);
          mScrollMessage->SetIsVisible(true);
        }
        mScrollMessage->TextSupport().SetPage(mPageCounter - 1);
        SetScanMessageTypeEffect(mScrollMessage, !mScanComplete);
      }
    }
  }

  float xAlpha = 0.f;
  float aAlpha = 0.f;
  float dashAlpha = 0.f;
  float startAlpha = 0.f;
  if (mState == kSS_DownloadComplete) {
    xAlpha = rstl::min_val(1.f, 2.f * mXAlpha);
    aAlpha = (1.f - xAlpha) * CMath::AbsF(mAPulse);
  } else if (mState == kSS_ViewingScan) {
    if (mPageCounter < mScrollMessage->TextSupport().GetTotalPageCount()) {
      aAlpha = CMath::AbsF(mAPulse);
    } else {
      dashAlpha = 1.f;
      startAlpha = CMath::AbsF(mAPulse);
    }
  }
  mXMark->SetVisibility(xAlpha > 0.f, kTM_Children);
  mAButton->SetVisibility(aAlpha > 0.f, kTM_Children);
  mDash->SetVisibility(dashAlpha > 0.f && mHistory.empty(), kTM_Children);
  mXMark->SetColor(CColor::White().WithAlphaOf(xAlpha));
  mAButton->SetColor(CColor::White().WithAlphaOf(aAlpha));
  mDash->SetColor(CColor(uchar(137), uchar(214), uchar(255), uchar(255)).WithAlphaOf(dashAlpha));

  if (mStartButton) {
    mStartButton->SetVisibility(dashAlpha > 0.f && !mHistory.empty(), kTM_Children);
    mStartButton->SetColor(CColor(uchar(255), uchar(255), uchar(255), uchar(255)).WithAlphaOf(startAlpha));
  }
  if (mPressStart) {
    if (mCanOpenLogbook) {
      mPressStart->SetVisibility(dashAlpha > 0.f && !mHistory.empty(), kTM_Children);
      const CColor color = gpTweakGuiColors->GetHUDMemoTextForegroundColor();
      const CColor zero(0.f, 0.f, 0.f, 0.f);
      mPressStart->TextSupport().SetFontColor(CColor::Lerp(zero, color, startAlpha));
    } else {
      mPressStart->SetVisibility(false, kTM_Children);
    }
  }
}

void CScanDisplay::Draw(const CStateManager& mgr) const {
  if (mPreparePending ||
      (mWorldModels.empty() && mModelObject == kInvalidUniqueId && !mScanModel.get())) {
    return;
  }
  CGraphics::SetCullMode(kCM_None);
  const CGuiCamera* camera = mSelHud->GetFrameCamera();
  CGraphics::DisableAllLights();
  const float translation =
      gpTweakGui->GetScanObjectTranslateTransitionSpline().EvaluateAt(mModelTransition);
  const float rotation =
      gpTweakGui->GetScanObjectRotationTransitionSpline().EvaluateAt(mModelTransition);
  const float scale = gpTweakGui->GetScanObjectScaleTransitionSpline().EvaluateAt(mModelTransition);
  const CVector3f position = CVector3f::Lerp(mStartPosition, mEndPosition, translation);
  const float fov = mStartFov * (1.f - mModelTransition) + mEndFov * mModelTransition;
  const CQuaternion orientation = CQuaternion::Slerp(mStartRotation, mEndRotation, rotation);
  const CVector3f modelScale = CVector3f::Lerp(mStartScale, mEndScale, scale);
  const CRelAngle yaw = CRelAngle::FromDegrees(mModelYaw);
  const CVector3f cameraOffset = (1.f - mModelTransition) * camera->GetLocalPosition();
  const CTransform4f xf = CTransform4f::Translate(cameraOffset) * CTransform4f::RotateZ(yaw) *
                          CTransform4f::Scale(modelScale) * orientation.BuildTransform4f() *
                          CTransform4f::Translate(position) * mModelRotation.BuildTransform4f();
  CGraphics::SetPerspective(fov, camera->GetParms().mPerspective.mAspect,
                            camera->GetParms().mPerspective.mNear,
                            camera->GetParms().mPerspective.mFar);
  CGraphics::SetDepthRange(1.f / 512.f, 1.f / 256.f);
  const CColor ambient(0.2f, 0.2f, 0.2f, 0.6f * mModelTransition);
  const CModelFlags flags = CModelFlags::Additive(CColor(0.6f, 0.4f, 0.3f, 0.7f * mModelTransition))
                                .DepthCompareUpdate(false, false);
  CGraphics::SetModelMatrix(xf);
  for (rstl::vector< rstl::pair< TAreaId, int > >::const_iterator it = mWorldModels.begin();
       it != mWorldModels.end(); ++it) {
    gpRender->DrawAreaModel(it->first.Value(), it->second, flags);
  }
  if (mScannableInfo && mScannableInfo->UsesScanModel()) {
    if (mScanModel.get() && !mScanModel->IsNull() && mScanModel->IsLoaded(0)) {
      CGraphics::SetModelMatrix(CTransform4f::Identity());
      mScanModel->Render(CModelData::kWM_Normal, xf, nullptr, flags);
    }
  } else if (mModelObject != kInvalidUniqueId) {
    const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mModelObject));
    const CPatterned* patterned = TCastToConstPtr< CPatterned >(actor);
    if (patterned && patterned->IsScanVisorSelfRender()) {
      CGraphics::SetModelMatrix(CTransform4f::Identity());
      if (mScanTexture) {
        if (mScanTexture->GetObject()) {
          const CModelFlags textureFlags =
              CModelFlags::Additive(CColor(0.6f, 0.4f, 0.3f, 0.7f * mModelTransition))
                  .DepthCompareUpdate(false, false)
                  .DontLoadTextures();
          mScanTexture->GetObject()->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
          patterned->ScanVisorRender(mgr, xf, textureFlags);
        }
      } else {
        patterned->ScanVisorRender(mgr, xf, flags);
      }
    } else if (actor) {
      const CModelData& model = *actor->GetModelData();
      if (!model.IsNull()) {
        CGraphics::SetModelMatrix(CTransform4f::Identity());
        if (mScanTexture) {
          if (mScanTexture->GetObject()) {
            const CModelFlags textureFlags =
                CModelFlags::Additive(CColor(0.6f, 0.4f, 0.3f, 0.7f * mModelTransition))
                    .DepthCompareUpdate(false, false)
                    .DontLoadTextures();
            mScanTexture->GetObject()->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
            model.Render(CModelData::kWM_Normal, xf, nullptr, textureFlags);
          }
        } else {
          model.Render(CModelData::kWM_Normal, xf, nullptr, flags);
        }
      }
    }
  }
  CGraphics::SetCullMode(kCM_Front);
}

float CScanDisplay::GetTotalDownloadTime() const {
  return mScannableInfo ? mScannableInfo->GetTotalDownloadTime() : 0.f;
}

void CScanDisplay::RequestScanDisplay() { mPreparePending = true; }

CScanDisplay::CScanTargetPredicate::~CScanTargetPredicate() {}

void CScanDisplay::PrepareScanDisplay(const CStateManager& mgr, int playerIndex) {
  if (!mPreparePending || mgr.GetGameState() != CStateManager::kGS_SoftPaused) {
    return;
  }
  mModelObject = kInvalidUniqueId;
  mWorldModels.clear();
  CVector3f objectPosition = CVector3f::Zero();
  mModelRotation = CQuaternion::NoRotation();
  mModelBounds = CAABox::MakeMaxInvertedBox();
  if (mScannableInfo && mScannableInfo->UsesScanModel()) {
    mScanModel = mScannableInfo->CreateStaticModel(0);
    if (!mScanModel.get() || mScanModel->IsNull() || !mScanModel->IsLoaded(0)) {
      return;
    }
    CAABox bounds = CAABox::MakeMaxInvertedBox();
    if (mScanModel->GetAnimationData()) {
      const CModelData::EWhichModel which =
          CModelData::GetRenderingModel(mgr, *mgr.GetPlayerState(playerIndex));
      bounds = mScanModel->PickAnimatedModel(which).GetModel()->GetAABB();
    } else {
      bounds = mScanModel->GetBounds(CTransform4f::Identity());
    }
    mModelBounds.Include(bounds);
    mScanModel->Touch(CModelData::kWM_Normal, 0);
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mObject))) {
      objectPosition = actor->GetTranslation();
    }
  } else {
    const CPlayer* player = mgr.GetPlayer(playerIndex);
    const CEntity* entity = mgr.GetObjectById(mObject);
    const TAreaId areaId = entity ? entity->GetCurrentAreaId() : player->GetCurrentAreaId();
    const CStaticGeometryMapData& geometry =
        mgr.GetWorld()->GetAreaAlways(areaId).GetPostConstructed()->mStaticGeometryMap->GetData();
    const TEditorId editorId = mgr.GetEditorIdForUniqueId(mObject);
    const rstl::vector< CStaticGeometryMapData::TMapping >& mappings = geometry.GetMappings();
    int count = 0;
    for (rstl::vector< CStaticGeometryMapData::TMapping >::const_iterator it = mappings.begin();
         it != mappings.end(); ++it) {
      if (it->second == editorId) {
        ++count;
      }
    }
    mWorldModels.reserve(count);
    for (rstl::vector< CStaticGeometryMapData::TMapping >::const_iterator it = mappings.begin();
         it != mappings.end(); ++it) {
      if (it->second == editorId) {
        mWorldModels.push_back_unsafe(rstl::pair< TAreaId, int >(areaId, it->first));
        mModelBounds.Include(gpRender->GetAreaModelBounds(areaId.Value(), it->first));
      }
    }
    if (count == 0) {
      const CActor* actor = nullptr;
      const CPatterned* patterned = nullptr;
      if (TCastToConstPtr< CScriptPointOfInterest >(mgr.GetObjectById(mObject))) {
        const CObjectList& actors = mgr.GetObjectListById(kOL_Actor);
        for (int i = actors.GetFirstObjectIndex(); i != -1; i = actors.GetNextObjectIndex(i)) {
          actor = TCastToConstPtr< CActor >(actors[i]);
          if (actor && actor->GetActive() && actor->HasModelData()) {
            const CScanTargetPredicate predicate(mObject);
            if (actor->CheckConnectedObject_if(mgr, kSS_ScanSource, kSM_None, predicate) ==
                mObject) {
              break;
            }
          }
          actor = nullptr;
        }
      } else {
        actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mObject));
        patterned = TCastToConstPtr< CPatterned >(actor);
      }
      if (patterned && patterned->IsScanVisorSelfRender()) {
        mModelBounds = patterned->GetScanVisorRenderBounds(mgr);
        mModelObject = patterned->GetUniqueId();
        objectPosition = patterned->GetTranslation();
        mModelRotation = CQuaternion::FromMatrix(patterned->GetTransform());
      } else if (actor && actor->HasModelData()) {
        const CModelData& model = *actor->GetModelData();
        if (!model.IsNull()) {
          mModelObject = actor->GetUniqueId();
          CAABox bounds = CAABox::MakeMaxInvertedBox();
          if (model.GetAnimationData()) {
            const CAABox modelBounds = model.GetAnimationData()->CalcBoundingBoxFromModelVerts();
            for (float angle = 0.f; angle < 60.f; angle += 15.f) {
              const CTransform4f rotation = CTransform4f::RotateZ(CRelAngle::FromDegrees(angle));
              const CTransform4f pivot = CTransform4f::Translate(modelBounds.GetCenterPoint()) *
                                         rotation *
                                         CTransform4f::Translate(-modelBounds.GetCenterPoint());
              bounds = modelBounds.GetTransformedAABox(
                  CTransform4f::Translate(-actor->GetTranslation()) * actor->GetTransform() *
                  CTransform4f::Scale(model.GetScale()) * pivot);
            }
          } else {
            const CAABox modelBounds = model.GetBounds();
            for (float angle = 0.f; angle < 60.f; angle += 15.f) {
              const CTransform4f rotation = CTransform4f::RotateZ(CRelAngle::FromDegrees(angle));
              const CTransform4f pivot = CTransform4f::Translate(modelBounds.GetCenterPoint()) *
                                         rotation *
                                         CTransform4f::Translate(-modelBounds.GetCenterPoint());
              bounds.Include(model.GetBounds(actor->GetTransform().GetRotation() * pivot));
            }
          }
          mModelBounds.Include(bounds);
          objectPosition = actor->GetTranslation();
          mModelRotation = CQuaternion::FromMatrix(actor->GetTransform());
        }
      }
    }
  }
  mPreparePending = false;
  mStartScale = CVector3f(1.f, 1.f, 1.f);
  const float largestDimension = rstl::max_val(
      rstl::max_val(mModelBounds.GetWidth(), mModelBounds.GetDepth()), mModelBounds.GetHeight());
  const float scale = gpTweakGui->GetScanObjectModelScale() / largestDimension;
  mEndScale = CVector3f(scale, scale, scale);
  const CCameraManager& cameras = *mgr.GetCameraManager(playerIndex);
  const CTransform4f camera = cameras.GetCurrentCameraTransform(mgr, true);
  mStartAspect = cameras.GetCurrentCamera(mgr, true)->GetAspectRatio();
  mStartFov = cameras.GetCurrentCamera(mgr, true)->GetFov();
  mEndFov = mStartFov;
  mStartRotation = CQuaternion::FromMatrix(camera).BuildInverted();
  mEndRotation = CQuaternion::NoRotation();
  mStartPosition = -camera.GetTranslation() + objectPosition;
  mEndPosition = -mModelBounds.GetCenterPoint();
  mModelTransition = 0.f;
}

float CScanDisplay::GetDownloadStartTime(int historyIndex) const {
  const float duration = GetTotalDownloadTime();
  return mHistory.empty() ? 0.f : historyIndex * duration / mHistory.size();
}

float CScanDisplay::GetDownloadFraction(int historyIndex, float time) const {
  return CMath::Clamp(0.f, (time - GetDownloadStartTime(historyIndex)) * mHistory.size(), 1.f);
}
