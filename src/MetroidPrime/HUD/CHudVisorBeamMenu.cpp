#include "MetroidPrime/HUD/CHudVisorBeamMenu.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "rstl/math.hpp"

#include <float.h>

static const char* const skBaseWidgetNames[] = {"basewidget_visormenuicons",
                                                "basewidget_beammenuicons"};
static const char* const skMenuTitleWidgetNames[] = {"textpane_visormenu", "textpane_beammenu"};
static const char* const skBaseTitleWidgetNames[] = {"basewidget_visormenutitle",
                                                     "basewidget_beammenutitle"};
static const char* const skBackgroundWidgetNames[] = {"model_visormenubg", "model_beammenubg"};
static const char* const skVisorWidgetBaseName = "model_visor";
static const char* const skBeamWidgetBaseName = "model_beam";
static const char* const skLozSuffix = "loz";
static const char* const skIconSuffix = "icon";
static const char* const skGhostSuffix = "ghost";
static const char skVisorWidgetIndices[] = "2310";
static const char skBeamWidgetIndices[] = "2103";
static const ushort skSelectionSounds[] = {0x193, 0x194};
static const char* const skMenuStringNames[2][4] = {
    {"CombatVisor", "EchoVisor", "ScanVisor", "DarkVisor"},
    {"PowerBeam", "DarkBeam", "LightBeam", "AnnihilatorBeam"}};
static const int skVisorDirections[] = {0, 2, 3, 1};

CHudVisorBeamMenu::CHudVisorBeamMenu(CGuiFrame& frame, const TLockedToken< CStringTable >& strings,
                                     EVisorBeamMenu type,
                                     const rstl::reserved_vector< bool, 4 >& enables, int selection,
                                     const bool multiplayer)
: mFrame(frame)
, mStrings(strings)
, mType(type)
, mSelectedItem(selection)
, mPendingSelection(0)
, mInterpolation(1.f)
, mVisibleDebug(true)
, mVisibleGame(true)
, mDirty(true)
, mItems(4, SMenuItem())
, mAnimationPhase(kAP_Steady)
, x84(FLT_EPSILON)
, x88(FLT_EPSILON)
, mTextFade(0.f) {
  mAnimationDuration = gpTweakGui->GetBeamVisorMenuAnimTime();
  mSwapBeamControls = gpGameState->GameOptions().GetSwapBeamControls();
  mMultiplayer = multiplayer;
  const CStringTable& table = **strings;

  mTitle = static_cast< CGuiTextPane* >(frame.FindWidget(skMenuTitleWidgetNames[GetSwappedType()]));
  mTitleRoot = frame.FindWidget(skBaseTitleWidgetNames[GetSwappedType()]);
  mMenuRoot = frame.FindWidget(skBaseWidgetNames[GetSwappedType()]);
  const char* ghostName = CBasics::Stringize(
      "%s%s", mType == kVBM_Visor ? skVisorWidgetBaseName : skBeamWidgetBaseName, skGhostSuffix);
  mGhost = frame.FindWidget(ghostName);
  for (int i = 0; i < 4; ++i) {
    const char* const base = mType == kVBM_Visor ? skVisorWidgetBaseName : skBeamWidgetBaseName;
    const char* const indices = mType == kVBM_Visor ? skVisorWidgetIndices : skBeamWidgetIndices;
    const char* lozengeName = CBasics::Stringize("%s%s%c", base, skLozSuffix, indices[i]);
    mItems[i].mLozenge = frame.FindWidget(lozengeName);
    const char* iconName = CBasics::Stringize("%s%s%c", base, skIconSuffix, indices[i]);
    mItems[i].mIcon = frame.FindWidget(iconName);
    mItems[i].mEnabled = enables[i];
    mItems[i].mOpacity = enables[i] ? 1.f : 0.f;
  }

  if (mType == kVBM_Visor) {
    mTitle->TextSupport().SetFontColor(gpTweakGuiColors->GetVisorMenuTitleForegroundColor());
    mTitle->TextSupport().SetOutlineColor(gpTweakGuiColors->GetVisorMenuTitleOutlineColor());
  } else {
    mTitle->TextSupport().SetFontColor(gpTweakGuiColors->GetBeamMenuTitleForegroundColor());
    mTitle->TextSupport().SetOutlineColor(gpTweakGuiColors->GetBeamMenuTitleOutlineColor());
  }
  if (CGuiWidget* background = frame.FindWidget(skBackgroundWidgetNames[GetSwappedType()])) {
    background->SetColor(gpTweakGuiColors->GetVisorMenuTitleForegroundColor());
  }
  if (mMultiplayer && mType != kVBM_Visor) {
    mTitleRoot->SetColor(CColor::White());
  } else {
    mTitleRoot->SetColor(CColor::White().WithAlphaOf(0.f));
  }
  mTitle->TextSupport().SetText(
      rstl::wstring_l(table.GetString(skMenuStringNames[mType][mSelectedItem])));
  mTitle->SetDepthTest(false);
  for (int i = 0; i < 4; ++i) {
    CGuiWidget* lozenge = mItems[i].mLozenge;
    if (lozenge != nullptr) {
      lozenge->SetColor(gpTweakGuiColors->GetVisorBeamMenuLozColor());
      UpdateMenuWidgetTransform(i, *mItems[i].mLozenge, 1.f);
    }
  }
  Update(0.f, true);
}

CHudVisorBeamMenu::~CHudVisorBeamMenu() {}

// Guessed name
void CHudVisorBeamMenu::RefreshText() {
  mTitle->TextSupport().SetText(
      rstl::wstring_l(mStrings->GetString(skMenuStringNames[mType][mSelectedItem])));
  mTitle->TextSupport().SetTypeWriteEffectOptions(true, 0.1f, 16.f);
}

void CHudVisorBeamMenu::SetSelection(int selection, int pending, float interpolation) {
  if (mSelectedItem == selection && mPendingSelection == pending &&
      interpolation == mInterpolation) {
    return;
  }
  if (pending != selection) {
    if (mAnimationPhase != kAP_SelectFlash) {
      CSfxManager::SfxStart(skSelectionSounds[mType], 127, 64);
    }
    mAnimationPhase = kAP_SelectFlash;
  } else if (interpolation < 1.f) {
    mAnimationPhase = kAP_Animate;
    const CColor& unselected = gpTweakGuiColors->GetUnselectedVisorBeamColor();
    mItems[mSelectedItem].mLozenge->SetColor(unselected);
    mTitle->TextSupport().SetText(
        rstl::wstring_l(mStrings->GetString(skMenuStringNames[mType][mSelectedItem])));
    mTitle->TextSupport().SetTypeWriteEffectOptions(true, 0.1f, 16.f);
  } else {
    if (mAnimationPhase != kAP_Steady) {
      mTextFade = mAnimationDuration;
    }
    mAnimationPhase = kAP_Steady;
  }
  mDirty = true;
  mSelectedItem = selection;
  mPendingSelection = pending;
  mInterpolation = interpolation;
}

void CHudVisorBeamMenu::SetPlayerHas(const rstl::reserved_vector< bool, 4 >& enables) {
  for (int i = 0; i < 4; ++i) {
    mItems[i].mEnabled = enables[i];
  }
}

void CHudVisorBeamMenu::SetPlayerHas(const rstl::reserved_vector< bool, 4 >& enables,
                                     int selection) {
  for (int i = 0; i < 4; ++i) {
    mItems[i].mEnabled = enables[i];
  }
  mTitle->TextSupport().SetText(
      rstl::wstring_l(mStrings->GetString(skMenuStringNames[mType][selection])));
  mTitle->TextSupport().SetTypeWriteEffectOptions(true, 0.1f, 16.f);
}

void CHudVisorBeamMenu::UpdateHudAlpha(float alpha) {
  const float hudAlpha = gpGameState->GameOptions().GetHudAlpha();
  const CColor white = CColor::White();
  const float opacity = alpha * hudAlpha;
  const CColor color = white.WithAlphaOf(opacity);
  mMenuRoot->SetColor(color);
}

void CHudVisorBeamMenu::SetIsVisibleGame(const bool visible) {
  mVisibleGame = visible;
  const bool isVisible = GetIsVisible();
  mMenuRoot->SetVisibility(isVisible, kTM_Children);
  if (isVisible) {
    Update(0.f, true);
  }
}

void CHudVisorBeamMenu::Update(float dt, const bool init) {
  const CColor itemColors[2][4] = {
      {gpTweakGuiColors->GetVisorMenuIconColor0(), gpTweakGuiColors->GetVisorMenuIconColor1(),
       gpTweakGuiColors->GetVisorMenuIconColor2(), gpTweakGuiColors->GetVisorMenuIconColor3()},
      {gpTweakGuiColors->GetBeamMenuIconColor0(), gpTweakGuiColors->GetBeamMenuIconColor1(),
       gpTweakGuiColors->GetBeamMenuIconColor2(), gpTweakGuiColors->GetBeamMenuIconColor3()}};
  const bool swapBeamControls = gpGameState->GameOptions().GetSwapBeamControls();
  const CColor* palette = itemColors[mType];
  if (swapBeamControls != mSwapBeamControls) {
    mSwapBeamControls = swapBeamControls;
    mMenuRoot = mFrame.FindWidget(skBaseWidgetNames[GetSwappedType()]);
    mTitle->TextSupport().SetText(rstl::wstring_l(L""));
    mTitle =
        static_cast< CGuiTextPane* >(mFrame.FindWidget(skMenuTitleWidgetNames[GetSwappedType()]));
    mTitleRoot = mFrame.FindWidget(skBaseTitleWidgetNames[GetSwappedType()]);
    for (int i = 0; i < 4; ++i) {
      SMenuItem& item = mItems[i];
      UpdateMenuWidgetTransform(i, *item.mIcon, item.mPosition);
      UpdateMenuWidgetTransform(i, *item.mLozenge, 1.f);
    }
    UpdateMenuWidgetTransform(mSelectedItem, *mGhost, mItems[mSelectedItem].mPosition);
  }

  const CColor& active = gpTweakGuiColors->GetSelectedVisorBeamColor();
  const CColor& inactive = gpTweakGuiColors->GetUnselectedVisorBeamColor();
  const CColor& lozenge = gpTweakGuiColors->GetVisorBeamMenuLozColor();
  rstl::reserved_vector< CColor, 4 > lozengeFades;
  rstl::reserved_vector< CColor, 4 > iconFades;
  for (int i = 0; i < 4; ++i) {
    SMenuItem& item = mItems[i];
    const bool enabled = item.mEnabled;
    if (mMultiplayer) {
      if (enabled) {
        item.mOpacity = rstl::min_val(1.f, item.mOpacity + dt / 3.f);
      } else {
        item.mOpacity = rstl::max_val(0.f, item.mOpacity - 8.f * dt);
      }
      if (enabled) {
        iconFades.push_back(CColor::Lerp(
            CColor::Red(), inactive, (1.f + CMath::FastCosR(3.f * M_2PIF * item.mOpacity)) / 2.f));
        lozengeFades.push_back(CColor::Lerp(
            CColor::Red(), inactive, (1.f + CMath::FastCosR(3.f * M_2PIF * item.mOpacity)) / 2.f));
      } else {
        iconFades.push_back(CColor::White().WithAlphaOf(item.mOpacity));
        lozengeFades.push_back(CColor::White().WithAlphaOf(item.mOpacity));
      }
    } else {
      if (enabled) {
        item.mOpacity = rstl::min_val(1.f, item.mOpacity + dt);
      } else {
        item.mOpacity = rstl::max_val(0.f, item.mOpacity - dt);
      }
      iconFades.push_back(CColor::White());
      lozengeFades.push_back(CColor::Lerp(active, inactive, item.mOpacity));
    }
  }

  switch (mAnimationPhase) {
  case kAP_None:
    break;
  case kAP_Steady:
    for (int i = 0; i < 4; ++i) {
      SMenuItem& item = mItems[i];
      const CColor iconBase = CColor::Modulate(
          palette[i], i == mSelectedItem ? gpTweakGuiColors->GetVisorBeamMenuIconSelectedColor()
                                         : gpTweakGuiColors->GetVisorBeamMenuIconUnselectedColor());
      const CColor& lozengeBase = i == mSelectedItem ? lozenge : inactive;
      const CColor icon =
          item.mOpacity == 0.f ? CColor(0) : CColor::Modulate(iconBase, iconFades[i]);
      const CColor loz =
          item.mOpacity == 0.f ? lozenge : CColor::Modulate(lozengeBase, lozengeFades[i]);
      item.mIcon->SetColor(icon);
      item.mLozenge->SetColor(loz);
      item.mPosition = i == mSelectedItem ? 0.f : 1.f;
    }
    mGhost->SetColor(active);
    break;
  case kAP_Animate: {
    const CColor& flashBase = CMath::ModF(mInterpolation, 0.1f) > 0.05f
                                  ? gpTweakGuiColors->GetVisorBeamMenuIconSelectedColor()
                                  : gpTweakGuiColors->GetVisorBeamMenuIconUnselectedColor();
    const CColor icon = CColor::Modulate(palette[mSelectedItem], flashBase);
    mItems[mSelectedItem].mIcon->SetColor(icon);
    const CColor loz = lozenge;
    mItems[mSelectedItem].mLozenge->SetColor(loz);
    const float position = 1.f - mInterpolation;
    mItems[mSelectedItem].mPosition = position;
    mGhost->SetColor(CColor::Lerp(active, inactive, position));
    break;
  }
  case kAP_SelectFlash: {
    mTitleRoot->SetColor(CColor::White().WithAlphaOf(0.f));
    const CColor selectedColor =
        CColor::Lerp(gpTweakGuiColors->GetVisorBeamMenuIconUnselectedColor(),
                     gpTweakGuiColors->GetVisorBeamMenuIconSelectedColor(), mInterpolation);
    const CColor& icon = CColor::Modulate(palette[mSelectedItem], selectedColor);
    mItems[mSelectedItem].mIcon->SetColor(icon);
    mItems[mSelectedItem].mLozenge->SetColor(
        CColor::Modulate(lozenge, CColor(mInterpolation, mInterpolation, mInterpolation, 1.f)));
    for (int i = 0; i < 4; ++i) {
      mItems[i].mPosition = i == mSelectedItem ? 1.f - mInterpolation : 1.f;
    }
    mGhost->SetColor(CColor::Lerp(active, inactive, mItems[mSelectedItem].mPosition));
    break;
  }
  default:
    break;
  }

  if (mTextFade > 0.f) {
    if (!mMultiplayer || mType == kVBM_Visor) {
      mTextFade = rstl::max_val(0.f, mTextFade - dt);
    }
    mTitleRoot->SetColor(CColor::White().WithAlphaOf(mTextFade / mAnimationDuration));
  }
  if (mDirty || init) {
    mDirty = false;
    for (int i = 0; i < 4; ++i) {
      UpdateMenuWidgetTransform(i, *mItems[i].mIcon, mItems[i].mPosition);
    }
    UpdateMenuWidgetTransform(mSelectedItem, *mGhost, mItems[mSelectedItem].mPosition);
  }
  if (!GetIsVisible()) {
    return;
  }
  if (mTitleRoot->GetModifiedColor().GetAlphau8() != 0) {
    mTitleRoot->SetVisibility(true, kTM_Children);
  } else {
    mTitleRoot->SetVisibility(false, kTM_Children);
  }
  for (int i = 0; i < 4; ++i) {
    if (mItems[i].mIcon->GetModifiedColor().GetAlphau8() != 0) {
      mItems[i].mIcon->SetIsVisible(true);
    } else {
      mItems[i].mIcon->SetIsVisible(false);
    }
  }
}

void CHudVisorBeamMenu::UpdateMenuWidgetTransform(int index, CGuiWidget& widget, float factor) {
  const float translationScale = gpTweakGui->GetVisorBeamMenuItemTranslate();
  const float magnitude = CMath::AbsF(factor);
  const float translate = magnitude * translationScale;
  const float scale = magnitude * gpTweakGui->GetVisorBeamMenuItemInactiveScale() +
                      (1.f - magnitude) * gpTweakGui->GetVisorBeamMenuItemActiveScale();
  int direction;
  if (mType == kVBM_Visor) {
    direction = skVisorDirections[index];
  } else {
    direction = index;
    if (index == 2) {
      direction = 3;
    } else if (index == 3) {
      direction = 2;
    }
  }

  switch (direction) {
  case 0:
    widget.SetO2WTransform(mMenuRoot->GetWorldTransform() *
                           CTransform4f::Translate(0.f, 0.f, translate) *
                           CTransform4f::Scale(scale));
    break;
  case 1:
    widget.SetO2WTransform(mMenuRoot->GetWorldTransform() *
                           CTransform4f::Translate(translate, 0.f, 0.f) *
                           CTransform4f::Scale(scale));
    break;
  case 2:
    widget.SetO2WTransform(mMenuRoot->GetWorldTransform() *
                           CTransform4f::Translate(0.f, 0.f, -translate) *
                           CTransform4f::Scale(scale));
    break;
  case 3:
    widget.SetO2WTransform(mMenuRoot->GetWorldTransform() *
                           CTransform4f::Translate(-translate, 0.f, 0.f) *
                           CTransform4f::Scale(scale));
    break;
  default:
    break;
  }
}
