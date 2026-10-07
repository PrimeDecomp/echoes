#include "GuiSys/CGuiWidget.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include <stdio.h>

static const char* const skDrawFlagNames[] = {
    "kGUIModelDrawFlags_None",
    "kGUIModelDrawFlags_RGBModulate",
    "kGUIModelDrawFlags_AlphaBlend",
    "kGUIModelDrawFlags_AdditiveAlpha",
    "kGUIModelDrawFlags_TwoPassAddAndBlendAlpha",
    "kGuiModelDrawFlags_DrawToAlphaBuffer",
    "kGuiModelDrawFlags_2xModulateSolid",
};

CGuiWidget::CGuiWidgetParms::CGuiWidgetParms(CGuiFrame* frame, short selfId, short parentId,
                                             const CColor& color, EGuiModelDrawFlags drawFlags,
                                             bool cullFaces, bool defaultVisible,
                                             bool defaultActive, bool depthTest, bool depthWrite,
                                             bool depthGreater)
: mFrame(frame)
, mSelfId(selfId)
, mParentId(parentId)
, mColor(color)
, mDrawFlags(drawFlags)
, mCullFaces(cullFaces)
, mDefaultVisible(defaultVisible)
, mDefaultActive(defaultActive)
, mDepthTest(depthTest)
, mDepthWrite(depthWrite)
, mDepthGreater(depthGreater) {}

CGuiWidget* CGuiWidget::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* pool,
                               uint version) {
  const CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  CGuiWidget* widget = rs_new CGuiWidget(parms);
  widget->ParseBaseInfo(frame, in, parms, version);
  return widget;
}

CGuiWidget* CGuiWidget::CreateGroup(CGuiFrame* frame, CInputStream& in, CSimplePool* pool,
                                    uint version) {
  const CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  in.ReadInt16();
  in.ReadBool();

  CGuiWidget* widget = rs_new CGuiWidget(parms);
  widget->ParseBaseInfo(frame, in, parms, version);
  return widget;
}

CGuiWidget::CGuiWidgetParms CGuiWidget::ReadWidgetHeader(CGuiFrame* frame, CInputStream& in) {
  rstl::string name(in);
  short selfId = frame->WidgetIdDB().AddWidget(name);
  rstl::string parent(in);
  short parentId = frame->WidgetIdDB().AddWidget(parent);

  in.ReadBool(); // Legacy animation-controller setting is no longer used.
  bool visible = in.ReadBool();
  bool active = in.ReadBool();
  bool cullFaces = in.ReadBool();
  CColor color(in);
  EGuiModelDrawFlags drawFlags = static_cast< EGuiModelDrawFlags >(in.ReadInt32());
  return CGuiWidgetParms(frame, selfId, parentId, color, drawFlags, cullFaces, visible, active,
                         true, false, false);
}

CGuiWidget::CGuiWidget(const CGuiWidgetParms& parms)
: mSelfId(parms.mSelfId)
, mParentId(parms.mParentId)
, mTransform(CTransform4f::Identity())
, mColor(parms.mColor)
, mColor2(mColor)
, mDrawFlags(parms.mDrawFlags)
, mFrame(parms.mFrame)
, mWorkerId(-1)
, mIsVisible(parms.mDefaultVisible)
, mIsActive(parms.mDefaultActive)
, mIsSelectable(true)
, mEventLock(false)
, mCullFaces(parms.mCullFaces)
, mDepthGreater(parms.mDepthGreater)
, mDepthTest(parms.mDepthTest)
, mDepthWrite(parms.mDepthWrite)
, xbb_24_(true) {
  RecalcWidgetColor(kTM_Single);
}

CGuiWidget::~CGuiWidget() {}

void CGuiWidget::ParseBaseInfo(CGuiFrame* frame, CInputStream& in, const CGuiWidgetParms& parms,
                               uint version) {
  CGuiWidget* parent = frame->FindWidget(parms.mParentId);
  bool isWorker = in.ReadBool();
  if (isWorker) {
    mWorkerId = in.ReadInt16();
  }

  CVector3f translation(in);
  CMatrix3f orientation(in);
  SetIdleXform(CTransform4f(orientation, translation));
  if (version < 2) {
    CVector3f unused(in);
    ReadUnusedThing(in);
    in.ReadInt16();
  }

  if (parent != nullptr) {
    if (isWorker && !parent->AddWorkerWidget(this)) {
      printf("Warning: Discarding useless worker id.  Parent is not a compound widget.");
      mWorkerId = -1;
    }
    parent->AddChildWidget(this, false, true);
  }
}

void CGuiWidget::ReadUnusedThing(CInputStream& in) { in.ReadInt32(); }

void CGuiWidget::Draw(const CGuiWidgetDrawParms& parms) const {}

void CGuiWidget::ProcessUserInput(const CFinalInput& input) {}

void CGuiWidget::Update(float dt) {}

void CGuiWidget::DispatchInitialize() {
  Initialize();
  if (ChildObject() != nullptr) {
    static_cast< CGuiWidget* >(ChildObject())->DispatchInitialize();
  }
  if (NextSibling() != nullptr) {
    static_cast< CGuiWidget* >(NextSibling())->DispatchInitialize();
  }
}

CGuiWidget* CGuiWidget::FindWidget(short id) {
  if (mSelfId == id) {
    return this;
  }
  if (ChildObject() != nullptr) {
    CGuiWidget* found = static_cast< CGuiWidget* >(ChildObject())->FindWidget(id);
    if (found != nullptr) {
      return found;
    }
  }
  if (NextSibling() != nullptr) {
    CGuiWidget* found = static_cast< CGuiWidget* >(NextSibling())->FindWidget(id);
    if (found != nullptr) {
      return found;
    }
  }
  return nullptr;
}

void CGuiWidget::SetColor(const CColor& color) {
  if (!(mColor == color)) {
    mColor = color;
    RecalcWidgetColor(kTM_Children);
  }
}

void CGuiWidget::RecalcWidgetColor(ETraversalMode mode) {
  CGuiWidget* parent = static_cast< CGuiWidget* >(Parent());
  if (parent != nullptr) {
    mColor2 = CColor::Modulate(mColor, parent->GetModifiedColor());
  } else {
    mColor2 = mColor;
  }

  switch (mode) {
  case kTM_Single:
    break;
  case kTM_ChildrenAndSiblings:
    if (NextSibling() != nullptr) {
      static_cast< CGuiWidget* >(NextSibling())->RecalcWidgetColor(kTM_ChildrenAndSiblings);
    }
  case kTM_Children:
    if (ChildObject() != nullptr) {
      static_cast< CGuiWidget* >(ChildObject())->RecalcWidgetColor(kTM_ChildrenAndSiblings);
    }
    break;
  }
}

void CGuiWidget::SetVisibility(const bool visible, ETraversalMode mode) {
  switch (mode) {
  case kTM_Single:
    break;
  case kTM_Children:
    if (ChildObject() != nullptr) {
      static_cast< CGuiWidget* >(ChildObject())->SetVisibility(visible, kTM_ChildrenAndSiblings);
    }
    break;
  case kTM_ChildrenAndSiblings:
    if (ChildObject() != nullptr) {
      static_cast< CGuiWidget* >(ChildObject())->SetVisibility(visible, kTM_ChildrenAndSiblings);
    }
    if (NextSibling() != nullptr) {
      static_cast< CGuiWidget* >(NextSibling())->SetVisibility(visible, kTM_ChildrenAndSiblings);
    }
    break;
  }
  SetIsVisible(visible);
}

void CGuiWidget::AddChildWidget(CGuiWidget* widget, bool makeWorldLocal, bool atEnd) {
  AddChildObject(widget, makeWorldLocal, atEnd);
}

CVector3f CGuiWidget::GetIdlePosition() const { return mTransform.GetTranslation(); }

void CGuiWidget::ReapplyXform() {
  RotateReset();
  SetLocalPosition(CVector3f::Zero());
  MultiplyO2P(mTransform);
}

void CGuiWidget::SetIsVisible(bool visible) {
  mIsVisible = visible;
  OnVisible();
}

void CGuiWidget::SetIsActive(const bool active) {
  if (mIsActive != active) {
    mIsActive = active;
    OnActivate();
  }
}

void CGuiWidget::OnVisible() {}

void CGuiWidget::OnActivate() {}

void CGuiWidget::SetIdleXform(const CTransform4f& xf, bool reapply) {
  mTransform = xf;
  if (reapply) {
    ReapplyXform();
  }
}

CGuiWidget* CGuiWidget::GetWorkerWidget(int workerId) {
  const CGuiWidget* widget = static_cast< const CGuiWidget* >(GetChildObject());
  for (; widget != nullptr; widget = static_cast< const CGuiWidget* >(widget->GetNextSibling())) {
    if (workerId == widget->GetWorkerId()) {
      break;
    }
  }
  return const_cast< CGuiWidget* >(widget);
}
