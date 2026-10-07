#include "MetroidPrime/ScriptObjects/CScriptFrontEndDataNetwork.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "rstl/algorithm.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CSaveGameScreen.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrFrontEndDataNetwork.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptColorModulate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTextPane.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "REL/REL_Setup.h"

// The ScriptGui REL entity (native type 11) that forwards a controller index.
struct SGuiControllerSource {
  uchar x0_pad[0x38];
  int mController;
};
extern "C" SGuiControllerSource* fn_8009A6E4(CEntity* entity);

// Guessed name. One billboard queued by RenderNode, sorted back to front.
struct SRenderItem {
  enum EType {
    kT_Center = 1,
    kT_Child = 2,
    kT_Selected = 3,
  };

  SRenderItem(const SDataNetworkNode* node, CVector3f pos, EType type)
  : mNode(node), mPos(pos), mDepth(0.f), mType(type) {}

  const SDataNetworkNode* mNode;
  CVector3f mPos;
  float mDepth;
  EType mType;
};

struct SRenderItemDepthSort {
  bool operator()(const SRenderItem& a, const SRenderItem& b) const {
    return a.mDepth > b.mDepth;
  }
};

static float sMaxSpin = 1080.f;
static float sSpinAccel = 120.f;

SDataNetworkNode::SDataNetworkNode(TUniqueId id, int index, int parent, const bool isProxy,
                                   const bool parentIsProxy)
: mId(id)
, mIndex(index)
, mParent(parent)
, mSelectedChild(-1)
, mOffset(CVector3f::Zero())
, mPos(CVector3f::Zero())
, x38(CVector3f::Zero())
, mVelocity(CVector3f::Zero())
, mRenderPos(CVector3f::Zero())
, x5c(1.f)
, x60(0.f)
, x64(0.f)
, mIsProxy(isProxy)
, mParentIsProxy(parentIsProxy) {
  mChildren.reserve(8);
}

CScriptFrontEndDataNetwork* SDataNetworkNode::GetNetwork(CStateManager& mgr) {
  return TCastToPtr< CScriptFrontEndDataNetwork >(mgr.ObjectById(mId));
}

const CScriptFrontEndDataNetwork* SDataNetworkNode::GetConstNetwork(const CStateManager& mgr) const {
  return TCastToConstPtr< CScriptFrontEndDataNetwork >(mgr.GetObjectById(mId));
}

void SDataNetworkNode::SetX64(float v) { x64 = v; }
void SDataNetworkNode::SetX60(float v) { x60 = v; }
void SDataNetworkNode::SetX5C(float v) { x5c = v; }
void SDataNetworkNode::SetSelectedChild(int v) { mSelectedChild = v; }
int SDataNetworkNode::GetSelectedChild() const { return mSelectedChild; }
void SDataNetworkNode::SetParent(int v) { mParent = v; }
void SDataNetworkNode::SetVelocity(const CVector3f& v) { mVelocity = v; }
const CVector3f& SDataNetworkNode::GetVelocity() const { return mVelocity; }
void SDataNetworkNode::SetX38(const CVector3f& v) { x38 = v; }
const CVector3f& SDataNetworkNode::GetX38() const { return x38; }
void SDataNetworkNode::SetRenderPos(const CVector3f& v) { mRenderPos = v; }
const CVector3f& SDataNetworkNode::GetRenderPos() const { return mRenderPos; }
void SDataNetworkNode::SetPos(const CVector3f& v) { mPos = v; }
const CVector3f& SDataNetworkNode::GetPos() const { return mPos; }
void SDataNetworkNode::SetOffset(const CVector3f& v) { mOffset = v; }
const CVector3f& SDataNetworkNode::GetOffset() const { return mOffset; }
void SDataNetworkNode::AddChild(int idx) { mChildren.push_back_unsafe(idx); }

CScriptFrontEndDataNetwork::CScriptFrontEndDataNetwork(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CMayaSpline& shrinkSpline, const CMayaSpline& moveSpline,
    const CMayaSpline& expandSpline, const CMayaSpline& moveInSpline, const bool isRoot,
    const bool b2, const bool b3, const bool isProxy, const bool canBeSelected,
    const bool isLocked, const bool b7, const bool b8, CAssetId hotDotTexture,
    CAssetId hotDotHaloTexture, CAssetId hotDotAButtonTexture, const CColor& selectedColor,
    const CColor& unselectedMinColor, const CColor& unselectedMaxColor,
    const CColor& disabledColor, TSfxId rotationSound, int rotationSoundVolume,
    float shrinkTime, float moveTime, float expandTime, float moveInTime,
    float connectionRadius)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
, mRootId(isRoot ? uid : kInvalidUniqueId)
, mPlatformId(kInvalidUniqueId)
, mPrevIndex(0)
, mCurIndex(0)
, mSpin(CVector2f::Zero())
, mSpinAccel(CVector2f::Zero())
, mOrientation(CQuaternion::NoRotation())
, mTransitionState(kTS_Idle)
, mTransitionForward(1)
, mTransitionT(0.f)
, mTransitionDuration(1.f)
, mShrinkSpline(shrinkSpline)
, mShrinkTime(shrinkTime)
, mMoveSpline(moveSpline)
, mMoveTime(moveTime)
, mExpandSpline(expandSpline)
, mExpandTime(expandTime)
, mMoveInSpline(moveInSpline)
, mMoveInTime(moveInTime)
, mIsRoot(isRoot)
, x2cd(b2)
, x2ce(b3)
, mIsProxy(isProxy)
, mCanBeSelected(canBeSelected)
, mIsLocked(isLocked)
, x2d2(b7)
, x2d3(b8)
, mConnectionRadius(connectionRadius)
, mHotDotTexture(hotDotTexture)
, mHotDotHaloTexture(hotDotHaloTexture)
, mHotDotAButtonTexture(hotDotAButtonTexture)
, mSelectedColor(selectedColor)
, mUnselectedMinColor(unselectedMinColor)
, mUnselectedMaxColor(unselectedMaxColor)
, mDisabledColor(disabledColor)
, mController(0)
, mActiveController(0)
, mRotationSound(rotationSound)
, mRotationSoundVolume(rotationSoundVolume) {
  if (mIsRoot) {
    mControllers.reserve(4);
  }
}

void CScriptFrontEndDataNetwork::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Lock:
    SetLocked(true, mgr);
    break;
  case kSM_Unlock:
    SetLocked(false, mgr);
    break;
  case kSM_Open:
    if (CScriptFrontEndDataNetwork* root =
            TCastToPtr< CScriptFrontEndDataNetwork >(mgr.ObjectById(mRootId))) {
      root->OpenNode(GetUniqueId(), mgr);
    }
    break;
  case kSM_Close:
    if (mIsRoot) {
      CloseNode(mgr);
    } else if (CScriptFrontEndDataNetwork* root =
                   TCastToPtr< CScriptFrontEndDataNetwork >(mgr.ObjectById(mRootId))) {
      root->CloseNode(mgr);
    }
    break;
  case kSM_AreaLoaded: {
    {
      rstl::vector< TUniqueId > ids(FindConnectedObjects(mgr, kSS_Connect, kSM_Attach));
      for (rstl::vector< TUniqueId >::iterator it = ids.begin(); it != ids.end(); ++it) {
        if (TCastToConstPtr< CScriptPlatform >(mgr.GetObjectById(*it))) {
          mPlatformId = *it;
          break;
        }
      }
    }
    if (mIsRoot) {
      BuildNetwork(mgr);
      return;
    }
    break;
  }
  case kSM_Increment:
    if (!GetActive()) {
      mgr.SendScriptMsg(this, GetUniqueId(), kSM_Activate);
      CScriptColorModulate::FadeInHelper(mgr, GetUniqueId(), 0.75f);
    }
    break;
  case kSM_Decrement:
    CScriptColorModulate::FadeOutHelper(mgr, GetUniqueId(), 0.75f);
    break;
  case kSM_Follow:
    if (mIsRoot) {
      if (SGuiControllerSource* source =
              fn_8009A6E4(const_cast< CEntity* >(mgr.GetObjectById(msg.GetSenderId())))) {
        AddController(source->mController);
      }
    }
    break;
  case kSM_Escape:
    ClearControllers(mgr);
    break;
  case kSM_Reset:
    ResetTransition(mgr);
    break;
  case kSM_InternalMessage02:
    if (CScriptFrontEndDataNetwork* root =
            TCastToPtr< CScriptFrontEndDataNetwork >(mgr.ObjectById(mRootId))) {
      root->FaceNode(GetUniqueId(), mgr, false);
    }
    break;
  case kSM_InternalMessage03:
    if (CScriptFrontEndDataNetwork* root =
            TCastToPtr< CScriptFrontEndDataNetwork >(mgr.ObjectById(mRootId))) {
      root->FaceNode(GetUniqueId(), mgr, true);
    }
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(mgr, msg);
  if (!GetActive()) {
    return;
  }
}

void CScriptFrontEndDataNetwork::Think(float dt, CStateManager& mgr) {
  CActor::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }
  if (!mIsRoot) {
    return;
  }

  x1a8 = rstl::max_val(x1a8 - 3.f * dt, 0.f);
  UpdateTransition(mgr, dt);
  mSpin += mSpinAccel * dt;
  mSpin *= 0.97f;
  if (mSpin.MagSquared() > 1166400.f) {
    mSpin = mSpin.AsNormalized() * sMaxSpin;
  }
  const CVector2f spin = mSpin * dt;
  mOrientation = mOrientation * CQuaternion::ZRotation(CRelAngle::FromDegrees(spin.GetX())) *
                 CQuaternion::XRotation(CRelAngle::FromDegrees(spin.GetY()));

  if (mTransitionState == 0 && !mIsLocked) {
    SDataNetworkNode& node = mNodes[mCurIndex];
    const CTransform4f xf(mOrientation.BuildTransform4f());
    const CVector3f target = (-1.f * node.GetConstNetwork(mgr)->GetConnectionRadius()) * xf.GetForward();
    float best = 10000.f;
    if (!node.GetConstNetwork(mgr)->x2d3) {
      const int prevSelected = node.GetSelectedChild();
      const CVector3f& pos = node.GetPos();
      for (int i = 0; i < node.mChildren.size(); ++i) {
        SDataNetworkNode& child = mNodes[node.mChildren[i]];
        const CScriptFrontEndDataNetwork* net = child.GetConstNetwork(mgr);
        if (net->GetActive() && net->mCanBeSelected) {
          const CVector3f delta = (child.GetPos() - pos) - target;
          const float score = (prevSelected == i ? 1.f : 1.5f) * delta.MagSquared();
          if (score < best) {
            node.SetSelectedChild(i);
            best = score;
          }
        }
      }
      if (prevSelected != node.GetSelectedChild()) {
        x1a8 = 1.f;
        if (node.GetSelectedChild() != -1) {
          SendScriptMsgs(kSS_Modify, mgr);
        }
        if (prevSelected != -1) {
          CScriptFrontEndDataNetwork* prevNet = mNodes[node.mChildren[prevSelected]].GetNetwork(mgr);
          prevNet->SendScriptMsgs(kSS_Zero, mgr);
        }
        CScriptFrontEndDataNetwork* selNet =
            mNodes[node.mChildren[node.GetSelectedChild()]].GetNetwork(mgr);
        selNet->SendScriptMsgs(kSS_MaxReached, mgr);
      }
    }
    node.GetNetwork(mgr)->SendScriptMsgs(kSS_Inside, mgr);
  }

  const CSaveGameScreen* saveScreen = mgr.mSaveGameScreen.get();
  if (saveScreen == nullptr || saveScreen->GetUIType() == CSaveGameScreen::kUIT_SaveReady) {
    if (mControllers.size() == 0) {
      const CFinalInput& input = mgr.mFinalInputs[mController];
      HandleRotation(input, mgr);
      HandleButtons(input, mgr);
      HandleStick(input, mgr);
    } else {
      const CFinalInput* input = &mgr.mFinalInputs[mControllers[mActiveController]];
      uchar handled = HandleRotation(*input, mgr);
      handled = handled | HandleButtons(*input, mgr);
      handled = handled | HandleStick(*input, mgr);
      if (!handled) {
        for (int i = 0; i < mControllers.size(); ++i) {
          if (i != mActiveController) {
            input = &mgr.mFinalInputs[mControllers[i]];
            handled = HandleRotation(*input, mgr);
            handled = handled | HandleButtons(*input, mgr);
            handled = handled | HandleStick(*input, mgr);
            if (handled) {
              mActiveController = i;
              break;
            }
          }
        }
      }
    }
  }
  const uchar volume = mRotationSoundVolume;
  CSfxManager::SfxVolume(mRotationSfx, volume);
}

void CScriptFrontEndDataNetwork::AddToRenderer(const CStateManager& mgr) const {
  EnsureRendered(mgr);
}

bool CScriptFrontEndDataNetwork::CanRenderUnsorted(const CStateManager&) const { return false; }

void CScriptFrontEndDataNetwork::SetRootId(TUniqueId id) { mRootId = id; }

TUniqueId CScriptFrontEndDataNetwork::GetPlatformId() const { return mPlatformId; }

void CScriptFrontEndDataNetwork::BuildNetwork(CStateManager& mgr) {
  mNodes.reserve(64);
  mNodes.push_back_unsafe(SDataNetworkNode(GetUniqueId(), 0, -1, false, false));
  for (int i = 0; i < mNodes.size(); ++i) {
    rstl::vector< TUniqueId > ids(
        mgr.GetObjectById(mNodes[i].GetId())->FindConnectedObjects(mgr, kSS_Connect, kSM_Attach));
    for (rstl::vector< TUniqueId >::iterator it = ids.begin(); it != ids.end(); ++it) {
      if (CScriptFrontEndDataNetwork* net =
              TCastToPtr< CScriptFrontEndDataNetwork >(mgr.ObjectById(*it))) {
        net->SetRootId(GetUniqueId());
        const int child = AddNode(mgr, *it, i);
        mNodes[i].AddChild(child);
      }
    }
  }
  LayoutChildren(0, mgr);
}

int CScriptFrontEndDataNetwork::AddNode(CStateManager& mgr, TUniqueId id, int parent) {
  const int count = mNodes.size();
  for (int i = 0; i < count; ++i) {
    if (id == mNodes[i].mId) {
      return i;
    }
  }
  const CScriptFrontEndDataNetwork* net =
      TCastToConstPtr< CScriptFrontEndDataNetwork >(mgr.GetObjectById(id));
  const bool parentIsProxy = mNodes[parent].GetConstNetwork(mgr)->mIsProxy;
  const bool isProxy = net->mIsProxy;
  mNodes.push_back_unsafe(SDataNetworkNode(id, count, parent, isProxy, parentIsProxy));
  return count;
}

void CScriptFrontEndDataNetwork::LayoutChildren(int idx, CStateManager& mgr) {
  const SDataNetworkNode& root = mNodes[0];
  SDataNetworkNode& node = mNodes[idx];
  const CScriptFrontEndDataNetwork* net = node.GetConstNetwork(mgr);
  for (rstl::vector< int >::iterator it = node.mChildren.begin(); it != node.mChildren.end();
       ++it) {
    const int childIdx = *it;
    SDataNetworkNode& child = mNodes[childIdx];
    const CVector3f offset = child.GetConstNetwork(mgr)->GetTranslation() - net->GetTranslation();
    child.SetOffset(offset);
    CVector3f dir = offset;
    if (dir.CanBeNormalized()) {
      dir = dir.AsNormalized() * net->GetConnectionRadius();
    }
    child.SetPos((node.GetPos() + dir) - root.GetPos());
    child.SetRenderPos(child.GetPos());
    LayoutChildren(childIdx, mgr);
  }
}

void CScriptFrontEndDataNetwork::ResetTransition(CStateManager& mgr) {
  if (mCurIndex != 0) {
    mNodes[mCurIndex].SetSelectedChild(-1);
  }
  mPrevIndex = 0;
  mCurIndex = 0;
  mNodes[0].SetSelectedChild(-1);
  mSpin = CVector2f::Zero();
  mSpinAccel = CVector2f::Zero();
  mTransitionState = kTS_Expand;
  mTransitionT = 1.f;
  mTransitionDuration = mExpandTime;
}

void CScriptFrontEndDataNetwork::UpdateTransition(CStateManager& mgr, float dt) {
  if (mTransitionState != 0) {
    mTransitionT = rstl::max_val(mTransitionT - dt / mTransitionDuration, 0.f);
    if (mTransitionT == 0.f) {
      SDataNetworkNode& node = mNodes[mCurIndex];
      CScriptFrontEndDataNetwork* net = node.GetNetwork(mgr);
      switch (mTransitionState) {
      case 1: {
        float duration;
        if (mTransitionForward == 1) {
          duration = mNodes[mPrevIndex].GetConstNetwork(mgr)
                         ->mMoveInTime;
        } else {
          const SDataNetworkNode& prev = mNodes[mPrevIndex];
          const SDataNetworkNode& cur = mNodes[mCurIndex];
          if (mCurIndex == prev.mParent ||
              (prev.mParentIsProxy && mCurIndex == mNodes[prev.mParent].mParent)) {
            duration = cur.GetConstNetwork(mgr)->mMoveTime;
          } else {
            duration = 0.f;
          }
        }
        if (duration > 0.f) {
          mTransitionT = 1.f;
          mTransitionDuration = duration;
          mTransitionState = kTS_Move;
          EScriptObjectState state = kSS_Up;
          if (mTransitionForward == 1) {
            state = kSS_Down;
          }
          net->SendScriptMsgs(state, mgr);
          break;
        }
      }
      case 2: {
        mTransitionT = 1.f;
        mTransitionDuration = net->mExpandTime;
        mTransitionState = kTS_Expand;
        EScriptObjectState state = kSS_Retreat;
        if (mTransitionForward == 1) {
          state = kSS_Approach;
        }
        net->SendScriptMsgs(state, mgr);
        if (mTransitionForward == 1) {
          CScriptFrontEndDataNetwork* parentNet = mNodes[node.mParent].GetNetwork(mgr);
          if (parentNet->mIsProxy) {
            parentNet->SendScriptMsgs(state, mgr);
          }
        }
        if (mTransitionForward != 1 || node.mChildren.size() != 0) {
          break;
        }
      }
      case 3:
        mTransitionState = kTS_Idle;
        net->SendScriptMsgs(kSS_Arrived, mgr);
        break;
      }
    }
  }

  if (mCurIndex == -1) {
    return;
  }

  for (rstl::vector< SDataNetworkNode >::iterator it = mNodes.begin(); it != mNodes.end(); ++it) {
    it->SetX60(0.f);
    it->SetX64(0.f);
  }

  SDataNetworkNode& cur = mNodes[mCurIndex];
  SDataNetworkNode& prev = mNodes[mPrevIndex];
  const bool forward = mTransitionForward == 1;
  switch (mTransitionState) {
  case 0:
    cur.SetX64(1.f);
    prev.SetX64(1.f);
    for (int i = 0; i < cur.mChildren.size(); ++i) {
      SDataNetworkNode& child = mNodes[cur.mChildren[i]];
      child.SetX60(1.f);
      child.SetX64(1.f);
    }
    break;
  case 1:
    for (int i = 0; i < prev.mChildren.size(); ++i) {
      SDataNetworkNode& child = mNodes[prev.mChildren[i]];
      child.SetX60(mTransitionT);
      child.SetX64(mTransitionT);
    }
    prev.SetX60(forward ? 0.f : 1.f - mTransitionT);
    prev.SetX64(1.f);
    if (forward) {
      if (cur.mParentIsProxy) {
        SDataNetworkNode& parent = mNodes[cur.mParent];
        parent.SetX60(1.f);
        parent.SetX64(1.f);
      } else {
        cur.SetX60(1.f);
        cur.SetX64(1.f);
      }
    } else {
      cur.SetX60(0.f);
      cur.SetX64(0.f);
    }
    break;
  case 2:
    if (forward) {
      prev.SetX64(1.f);
      if (cur.mParentIsProxy) {
        SDataNetworkNode& parent = mNodes[cur.mParent];
        parent.SetX60(1.f);
        parent.SetX64(1.f);
      } else {
        prev.SetX60(1.f);
        cur.SetX60(1.f);
        cur.SetX64(1.f);
      }
    } else {
      cur.SetX64(1.f);
      if (prev.mParentIsProxy) {
        SDataNetworkNode& parent = mNodes[prev.mParent];
        parent.SetX60(1.f - mTransitionT);
        parent.SetX64(1.f - mTransitionT);
      } else {
        cur.SetX60(1.f - mTransitionT);
        cur.SetX64(1.f - mTransitionT);
      }
      prev.SetX60(1.f);
      prev.SetX64(1.f - mTransitionT);
    }
    break;
  case 3:
    for (int i = 0; i < cur.mChildren.size(); ++i) {
      SDataNetworkNode& child = mNodes[cur.mChildren[i]];
      child.SetX60(1.f - mTransitionT);
      child.SetX64(1.f - mTransitionT);
    }
    if (forward) {
      if (cur.mParentIsProxy) {
        SDataNetworkNode& parent = mNodes[cur.mParent];
        parent.SetX60(mTransitionT);
        parent.SetX64(1.f);
        cur.SetX64(1.f);
      } else {
        cur.SetX60(mTransitionT);
        cur.SetX64(1.f);
        prev.SetX64(0.f);
      }
      prev.SetX60(0.f);
    } else {
      if (prev.mParentIsProxy) {
        SDataNetworkNode& parent = mNodes[prev.mParent];
        parent.SetX60(1.f);
        parent.SetX64(1.f);
      } else {
        prev.SetX60(1.f);
        prev.SetX64(1.f);
        cur.SetX60(1.f);
      }
      cur.SetX64(1.f);
    }
    break;
  }

  SimulateChildren(mgr, mCurIndex, dt);
  int renderIdx = mCurIndex;
  if (mTransitionState == 1) {
    renderIdx = mPrevIndex;
  } else if (mTransitionState == 2 && mTransitionForward == 1) {
    renderIdx = mPrevIndex;
  }
  UpdateRenderPositions(mgr, renderIdx, dt);

  for (rstl::vector< SDataNetworkNode >::iterator it = mNodes.begin(); it != mNodes.end(); ++it) {
    const float alpha = it->x60 * GetModelFlags().GetColorRef().GetAlpha();
    const CTransform4f xf(mOrientation.BuildTransform4f());
    CVector3f pos = it->GetPos();
    float radius = mConnectionRadius;
    if (mNodes.data() != &*it) {
      SDataNetworkNode& parent = mNodes[it->mParent];
      pos -= parent.GetPos();
      radius = parent.GetConstNetwork(mgr)->GetConnectionRadius();
    }
    const CVector3f local = xf.TransposeRotate(pos - xf.GetTranslation());
    float facing = (-1.f * local).GetY() / radius;
    if (-1.f > facing) {
      facing = -1.f;
    } else if (1.f < facing) {
      facing = 1.f;
    }
    facing *= 0.5f;
    it->SetX5C(facing + 0.5f);

    if (CScriptFrontEndDataNetwork* net =
            TCastToPtr< CScriptFrontEndDataNetwork >(mgr.ObjectById(it->GetId()))) {
      const TUniqueId& platformId = net->GetPlatformId();
      if (CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(mgr.ObjectById(platformId))) {
        SDataNetworkNode& current = mNodes[mCurIndex];
        bool selected = false;
        if (it->mIndex == current.mIndex) {
          selected = true;
        } else if (current.GetSelectedChild() != -1 &&
                   current.mChildren[current.GetSelectedChild()] == it->mIndex) {
          selected = true;
        } else if (it->mIsProxy && current.mParent == it->mIndex) {
          selected = true;
        }
        bool disabled = false;
        if (it->GetConstNetwork(mgr)->mIsLocked &&
            it->GetConstNetwork(mgr)->x2d2) {
          disabled = true;
        }
        CVector3f center = current.GetRenderPos();
        switch (mTransitionState) {
        case kTS_Idle:
          break;
        case kTS_Shrink:
          center = mNodes[mPrevIndex].GetRenderPos();
          break;
        case kTS_Move:
          if (mTransitionForward == 1) {
            center = mNodes[mPrevIndex].GetRenderPos();
          }
          break;
        case kTS_Expand:
          break;
        }
        const CTransform4f nodeXf(CTransform4f::Translate(center) *
                                  mOrientation.BuildTransform4f());
        CTransform4f platformXf(GetTransform());
        const CVector3f& renderPos = it->GetRenderPos();
        const CVector3f offset = nodeXf.TransposeRotate(renderPos - nodeXf.GetTranslation());
        platformXf.SetTranslation(GetTransform().GetTranslation() + offset);
        platform->SetTransformIfNoPositionSpline(platformXf);
        CColor color = CColor::Lerp(mUnselectedMinColor, mUnselectedMaxColor, it->x5c);
        if (disabled) {
          color = mDisabledColor;
        } else if (selected) {
          color = mSelectedColor;
        }
        rstl::vector< TUniqueId > ids(platform->FindConnectedObjects(mgr, kSS_Play, kSM_Activate));
        for (rstl::vector< TUniqueId >::iterator id = ids.begin(); id != ids.end(); ++id) {
          if (TCastToPtr< CActor >(mgr.ObjectById(*id))) {
            if (CScriptTextPane* pane = TCastToPtr< CScriptTextPane >(mgr.ObjectById(*id))) {
              pane->SetModelColor(color.WithAlphaModulatedBy(alpha));
              pane->SetRenderScale(selected && !disabled ? 1.0625f : 1.f);
            }
          }
        }
      }
    }
  }
}

void CScriptFrontEndDataNetwork::SimulateChildren(CStateManager& mgr, int idx, float dt) {
  SDataNetworkNode& node = mNodes[idx];
  const CVector3f& center = node.GetPos();
  const float radius = node.GetConstNetwork(mgr)->GetConnectionRadius();
  const float maxAttractStep = 1.83f * radius;
  const float maxFlockStep = 0.83f * radius;
  for (int i = 0; i < node.mChildren.size(); ++i) {
    const int childIdx = node.mChildren[i];
    SDataNetworkNode& child = mNodes[childIdx];
    CVector3f pos = child.GetPos();
    if (node.GetSelectedChild() == -1 || childIdx != node.mChildren[node.GetSelectedChild()] ||
        !node.GetConstNetwork(mgr)->x2ce) {
      CVector3f accel = CVector3f::Zero();
      const bool attract = child.GetConstNetwork(mgr)->x2cd;
      if (attract) {
        CVector3f target = child.GetOffset();
        target = mOrientation.BuildTransform4f().Rotate(target);
        target += node.GetPos();
        accel += GetAttraction(child, target);
      } else {
        accel += GetSeparation(mgr, node, childIdx);
        accel += GetCohesion(mgr, node, childIdx);
      }
      const CVector3f velocity = child.GetVelocity() + accel;
      CVector3f step = attract ? accel : child.GetX38() + 3.f * (dt * velocity);
      if (step.CanBeNormalized()) {
        const float len = step.Magnitude();
        const float clamped = CMath::Clamp(0.1f, len, attract ? maxAttractStep : maxFlockStep);
        step = clamped * ((1.f / len) * step);
      }
      pos = pos + dt * step;
      child.SetX38(step);
      child.SetVelocity(velocity);
    } else {
      child.SetX38(CVector3f::Zero());
      child.SetVelocity(CVector3f::Zero());
    }
    const CVector3f delta = pos - center;
    if (delta.CanBeNormalized()) {
      const CVector3f dir = delta.AsNormalized();
      const CVector3f newPos = child.GetConstNetwork(mgr)->x2cd ? pos : center + radius * dir;
      child.SetPos(newPos);
    }
  }
}

void CScriptFrontEndDataNetwork::UpdateRenderPositions(CStateManager& mgr, int idx, float dt) {
  SDataNetworkNode& node = mNodes[idx];
  const bool forward = mTransitionForward == 1;
  int curIdx = mCurIndex;
  if (mCurIndex > 0 && mNodes[mCurIndex].mParentIsProxy) {
    curIdx = mNodes[mCurIndex].mParent;
  }
  int prevIdx = mPrevIndex;
  if (mPrevIndex > 0 && mNodes[mPrevIndex].mParentIsProxy) {
    prevIdx = mNodes[mPrevIndex].mParent;
  }
  const CVector3f& pos = node.GetPos();
  node.SetRenderPos(pos);
  if (mTransitionState == 3 && idx > 0) {
    mNodes[node.mParent].SetRenderPos(pos);
  }
  for (int i = 0; i < node.mChildren.size(); ++i) {
    const int childIdx = node.mChildren[i];
    SDataNetworkNode& child = mNodes[childIdx];
    CScriptFrontEndDataNetwork* net = node.GetNetwork(mgr);
    const float invT = 1.f - mTransitionT;
    const float radius = net->GetConnectionRadius();
    float scale = 1.f;
    switch (mTransitionState) {
    case 1:
      if (childIdx != curIdx) {
        const CMayaSpline& spline = net->mShrinkSpline;
        scale = spline.GetKnots().size() == 0 ? mTransitionT : spline.EvaluateAt(invT);
      }
      break;
    case 2:
      if (forward) {
        if (childIdx == curIdx) {
          const CMayaSpline& spline = net->mMoveInSpline;
          scale =
              spline.GetKnots().size() == 0 ? mTransitionT : spline.EvaluateAt(mTransitionT);
        }
      } else if (childIdx == prevIdx) {
        const CMayaSpline& spline = net->mMoveSpline;
        scale = spline.GetKnots().size() == 0 ? invT : spline.EvaluateAt(invT);
      }
      break;
    case 3:
      if (childIdx != prevIdx && childIdx != curIdx) {
        const CMayaSpline& spline = net->mExpandSpline;
        scale = spline.GetKnots().size() == 0 ? 1.f - mTransitionT : spline.EvaluateAt(invT);
      }
      break;
    }
    const CVector3f delta = child.GetPos() - pos;
    if (delta.CanBeNormalized()) {
      const CVector3f dir = delta.AsNormalized();
      if (child.GetConstNetwork(mgr)->x2cd) {
        const CVector3f offset = scale * delta;
        child.SetRenderPos(pos + offset);
      } else {
        const CVector3f offset = scale * (radius * dir);
        child.SetRenderPos(pos + offset);
      }
    }
  }
}

void CScriptFrontEndDataNetwork::SetLocked(bool locked, CStateManager& mgr) {
  mIsLocked = locked;
  if (mIsLocked) {
    SendScriptMsgs(kSS_Locked, mgr);
    mSpin = CVector2f::Zero();
    mSpinAccel = CVector2f::Zero();
  } else {
    SendScriptMsgs(kSS_Unlocked, mgr);
  }
}

void CScriptFrontEndDataNetwork::OpenNode(TUniqueId id, CStateManager& mgr) {
  int idx = 0;
  for (int i = 0; i < mNodes.size(); ++i, ++idx) {
    SDataNetworkNode* node = &mNodes[i];
    if (node->mId == id) {
      SDataNetworkNode* target = node;
      if (node->GetConstNetwork(mgr)->mIsProxy) {
        node->GetNetwork(mgr)->SendScriptMsgs(kSS_Entered, mgr);
        idx = node->mChildren[0];
        target = &mNodes[idx];
        target->SetParent(node->mIndex);
        target->SetPos(node->GetPos());
        target->SetRenderPos(node->GetRenderPos());
        LayoutChildren(idx, mgr);
      }
      CScriptFrontEndDataNetwork* targetNet = target->GetNetwork(mgr);
      targetNet->SendScriptMsgs(kSS_Entered, mgr);
      SetSelection(mgr, idx, !GetActive());
      return;
    }
  }
}

void CScriptFrontEndDataNetwork::CloseNode(CStateManager& mgr) {
  SDataNetworkNode& node = mNodes[mCurIndex];
  CScriptFrontEndDataNetwork* net = node.GetNetwork(mgr);
  net->SendScriptMsgs(kSS_PressB, mgr);
  const int parent = node.mParent;
  int selection = parent;
  if (parent != -1) {
    node.SetSelectedChild(-1);
    SDataNetworkNode* parentNode = &mNodes[parent];
    if (parentNode->GetConstNetwork(mgr)->mIsProxy) {
      parentNode->GetNetwork(mgr)->SendScriptMsgs(kSS_PressB, mgr);
      selection = parentNode->mParent;
      parentNode = &mNodes[parentNode->mParent];
    }
    CScriptFrontEndDataNetwork* parentNet = parentNode->GetNetwork(mgr);
    parentNet->SendScriptMsgs(kSS_Entered, mgr);
    SetSelection(mgr, selection, false);
  }
}

void CScriptFrontEndDataNetwork::FaceNode(TUniqueId id, const CStateManager& mgr,
                                          bool onlyWhenInactive) {
  if (TCastToConstPtr< CScriptFrontEndDataNetwork >(mgr.GetObjectById(id))) {
    for (int i = 1; i < mNodes.size(); ++i) {
      SDataNetworkNode& node = mNodes[i];
      if (node.mId == id) {
        SDataNetworkNode& parent = mNodes[node.mParent];
        if (onlyWhenInactive && GetActive()) {
          return;
        }
        const CQuaternion target = CQuaternion::FromMatrix(
            CTransform4f::LookAt(node.GetPos(), parent.GetPos(), CVector3f::Up()));
        const float angle = mOrientation.AngleFrom(target).AsDegrees();
        if (angle > 20.f) {
          mOrientation = CQuaternion::Slerp(mOrientation, target, (angle - 20.f) / angle);
        }
        return;
      }
    }
  }
}

void CScriptFrontEndDataNetwork::AddController(int controller) {
  for (int i = 0; i < mControllers.size(); ++i) {
    if (controller == mControllers[i]) {
      return;
    }
  }
  mControllers.push_back_unsafe(controller);
}

void CScriptFrontEndDataNetwork::ClearControllers(CStateManager& mgr) {
  mControllers.clear();
  mActiveController = 0;
}

void CScriptFrontEndDataNetwork::Render(const CStateManager& mgr) const {
  if (!mIsRoot) {
    return;
  }
  CVector3f pos = mNodes[mCurIndex].GetRenderPos();
  const CTransform4f rot(mOrientation.BuildTransform4f());
  CGraphics::SetCullMode(kCM_None);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetBlendMode_Replace();
  gpRender->PrimColor(CColor::White());
  const int prevIdx = mPrevIndex;
  const SDataNetworkNode& prev = mNodes[prevIdx];
  switch (mTransitionState) {
  case 0:
    RenderNode(mgr, CTransform4f::Translate(pos) * rot, mCurIndex, 1.f - mTransitionT);
    break;
  case 1:
    pos = prev.GetRenderPos();
    RenderNode(mgr, CTransform4f::Translate(pos) * rot, mPrevIndex, mTransitionT);
    RenderNode(mgr, CTransform4f::Translate(pos) * rot, mCurIndex, 1.f - mTransitionT);
    break;
  case 2:
    if (mTransitionForward == 1) {
      pos = prev.GetRenderPos();
    }
    RenderNode(mgr, CTransform4f::Translate(pos) * rot, mPrevIndex, mTransitionT);
    RenderNode(mgr, CTransform4f::Translate(pos) * rot, mCurIndex, 1.f - mTransitionT);
    break;
  case 3:
    RenderNode(mgr, CTransform4f::Translate(pos) * rot, prevIdx, mTransitionT);
    RenderNode(mgr, CTransform4f::Translate(pos) * rot, mCurIndex, 1.f - mTransitionT);
    break;
  }
  CGraphics::SetCullMode(kCM_Front);
}

void CScriptFrontEndDataNetwork::RenderNode(const CStateManager& mgr, const CTransform4f& xf,
                                            int idx, float alpha) const {
  rstl::vector< SRenderItem > items;
  items.reserve(2);
  const CColor nodeColor = GetModelFlags().GetColor();
  const CColor selectedColor = GetModelFlags().GetColor();
  const SDataNetworkNode& node = mNodes[idx];
  const CVector3f center = node.GetRenderPos();
  gpRender->SetAmbientColor(CColor::White());
  CGraphics::DisableAllLights();
  gpRender->SetBlendMode_AdditiveAlpha();
  gpRender->SetModelMatrix(CTransform4f::Identity());
  items.push_back_unsafe(SRenderItem(&node, center, SRenderItem::kT_Center));

  TLockedToken< CTexture > hotDot = gpSimplePool->GetObj(SObjectTag('TXTR', mHotDotTexture));
  TLockedToken< CTexture > hotDotHalo =
      gpSimplePool->GetObj(SObjectTag('TXTR', mHotDotHaloTexture));
  TLockedToken< CTexture > hotDotAButton =
      gpSimplePool->GetObj(SObjectTag('TXTR', mHotDotAButtonTexture));

  items.reserve(node.mChildren.size() + 2);
  for (int i = 0; i < node.mChildren.size(); ++i) {
    const int childIdx = node.mChildren[i];
    const SDataNetworkNode& child = mNodes[childIdx];
    if (child.GetConstNetwork(mgr)->GetActive() && 0.f != child.x64 && 0.f != node.x64) {
      bool selected = false;
      if (node.GetSelectedChild() != -1 &&
          childIdx == node.mChildren[node.GetSelectedChild()]) {
        selected = true;
      }
      const CVector3f childPos = child.GetRenderPos();
      items.push_back_unsafe(SRenderItem(
          &child, childPos, selected ? SRenderItem::kT_Selected : SRenderItem::kT_Child));
      const CColor lineColor = CColor::Modulate(gpTweakGui->GetLogBookNodeColor(), nodeColor);
      DrawConnection(xf, center, childPos,
                     lineColor.WithAlphaModulatedBy((0.75f * node.x5c + 0.25f) * node.x64),
                     lineColor.WithAlphaModulatedBy((0.75f * child.x5c + 0.25f) * child.x64),
                     1.f);
    }
  }

  for (rstl::vector< SRenderItem >::iterator it = items.begin(); it != items.end(); ++it) {
    it->mDepth = xf.TransposeRotate(it->mPos).GetY();
  }
  rstl::sort(items.begin(), items.end(), SRenderItemDepthSort());

  for (rstl::vector< SRenderItem >::iterator it = items.begin(); it != items.end(); ++it) {
    const SDataNetworkNode* itemNode = it->mNode;
    const float itemAlpha = (0.5f * itemNode->x5c + 0.5f) * itemNode->x60;
    const CScriptFrontEndDataNetwork* net = itemNode->GetConstNetwork(mgr);
    switch (it->mType) {
    case SRenderItem::kT_Center:
      break;
    case SRenderItem::kT_Child: {
      const CColor color =
          itemNode->GetConstNetwork(mgr)->mCanBeSelected
              ? CColor::Modulate(gpTweakGui->GetLogBookNodeColor(), nodeColor)
                    .WithAlphaModulatedBy(itemAlpha)
              : CColor::Modulate(gpTweakGui->GetLogBookSelectedNodeColor(), selectedColor)
                    .WithAlphaModulatedBy(itemAlpha);
      hotDot->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
      DrawBillboard(xf, it->mPos, gpTweakGui->GetLogBookNodeScale(), color, true);
      break;
    }
    case SRenderItem::kT_Selected: {
      hotDot->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
      const CColor dotColor =
          CColor::Modulate(gpTweakGui->GetLogBookSelectedNodeColor(), selectedColor)
              .WithAlphaModulatedBy(itemAlpha);
      DrawBillboard(xf, it->mPos, gpTweakGui->GetLogBookSelectedNodeScale(), dotColor, true);
      if (!net->mIsLocked || !net->x2d2) {
        hotDotHalo->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
        const CColor haloColor =
            CColor::Modulate(gpTweakGui->GetLogBookSelectedNodeColor(), selectedColor)
                .WithAlphaModulatedBy(itemAlpha);
        DrawBillboard(xf, it->mPos, 1.2f * gpTweakGui->GetLogBookSelectedNodeScale(), haloColor,
                      true);
        hotDotAButton->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
        const CColor buttonColor =
            CColor::Modulate(gpTweakGui->GetLogBookSelectedNodeColor(), selectedColor)
                .WithAlphaModulatedBy(itemAlpha);
        DrawBillboard(xf, it->mPos, gpTweakGui->GetLogBookSelectedNodeScale(), buttonColor, false);
      }
      if (x1a8 > 0.f) {
        hotDotHalo->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
        const CColor flashColor = CColor::Lerp(CColor(0.f, 0.f, 0.f, 0.f), CColor::White(), x1a8)
                                      .WithAlphaModulatedBy(itemAlpha);
        DrawBillboard(xf, it->mPos, 1.2f * gpTweakGui->GetLogBookSelectedNodeScale(), flashColor,
                      true);
      }
      break;
    }
    }
  }
}

void CScriptFrontEndDataNetwork::DrawConnection(const CTransform4f& xf, const CVector3f& a,
                                                const CVector3f& b, const CColor& colorA,
                                                const CColor& colorB, float t) const {
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
  const CVector3f startLocal = xf.TransposeMultiply(a);
  const CVector3f start = startLocal + GetTransform().GetTranslation();
  const CVector3f mid = CVector3f::Lerp(a, b, t);
  const CVector3f endLocal = xf.TransposeMultiply(mid);
  const CVector3f end = endLocal + GetTransform().GetTranslation();
  for (int width = 2; width != 0; --width) {
    CGraphics::SetLineWidth(width + 1, kTO_Zero);
    CGraphics::StreamBegin(kP_Lines);
    CGraphics::StreamColor(colorA.WithAlphaModulatedBy(0.33333334f));
    CGraphics::StreamVertex(start);
    CGraphics::StreamColor(colorB.WithAlphaModulatedBy(0.33333334f));
    CGraphics::StreamVertex(end);
    CGraphics::StreamEnd();
  }
}

void CScriptFrontEndDataNetwork::DrawBillboard(const CTransform4f& xf, const CVector3f& pos,
                                               float size, const CColor& color,
                                               bool additive) const {
  if (mHotDotTexture == kInvalidAssetId) {
    return;
  }
  const CVector3f local = xf.TransposeMultiply(pos);
  const CVector3f center = local + GetTransform().GetTranslation();
  if (additive) {
    gpRender->SetBlendMode_AdditiveAlpha();
  } else {
    gpRender->SetBlendMode_AlphaBlended();
  }
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  const float half = 0.5f * size;
  CGraphics::StreamBegin(kP_Quads);
  CGraphics::StreamColor(color);
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(center + CVector3f(-half, 0.f, -half));
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(center + CVector3f(half, 0.f, -half));
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(center + CVector3f(half, 0.f, half));
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(center + CVector3f(-half, 0.f, half));
  CGraphics::StreamEnd();
}

uchar CScriptFrontEndDataNetwork::HandleRotation(const CFinalInput& input, CStateManager& mgr) {
  const float scale = 100.f * input.DeltaTime();
  bool handled = false;
  float x = scale * (-input.GetAnalogLeftX() * gpTweakGui->GetLogBookRotationSpeed());
  float y = scale * (input.GetAnalogLeftY() * gpTweakGui->GetLogBookRotationSpeed());
  CScriptFrontEndDataNetwork* net = mNodes[mCurIndex].GetNetwork(mgr);
  if (net->x2d3) {
    y = x = 0.f;
  }
  if (mIsLocked || mTransitionState != 0) {
    y = x = 0.f;
  }
  if (CMath::AbsF(x) < 0.01f) {
    x = 0.f;
  }
  if (CMath::AbsF(y) < 0.01f) {
    y = 0.f;
  }
  if (!close_enough(x, 0.f) || !close_enough(y, 0.f)) {
    handled = true;
    if (!mRotationSfx) {
      mRotationSfx = CSfxManager::SfxStart(mRotationSound, mRotationSoundVolume, 0x3f,
                                           mgr.GetNextAreaId().Value(), true, true,
                                           short(CSfxManager::kMedPriority));
    }
  } else if (mRotationSfx) {
    CSfxManager::SfxStop(mRotationSfx);
    mRotationSfx = CSfxHandle();
  }
  if (!net->x2ce) {
    x = y = 0.f;
  }
  mSpinAccel = CVector2f(x, y) * sSpinAccel;
  return handled;
}

uchar CScriptFrontEndDataNetwork::HandleButtons(const CFinalInput& input, CStateManager& mgr) {
  bool handled = false;
  SDataNetworkNode* node = &mNodes[mCurIndex];
  CScriptFrontEndDataNetwork* net = node->GetNetwork(mgr);
  if (mTransitionT == 0.f && !mIsLocked) {
    if (input.PA()) {
      handled = true;
      CScriptFrontEndDataNetwork* current = node->GetNetwork(mgr);
      if (node->mChildren.size() != 0) {
        const int selected = node->GetSelectedChild();
        if (selected != -1) {
          node = &mNodes[node->mChildren[selected]];
          if (node->GetConstNetwork(mgr)->mIsLocked) {
            node->GetNetwork(mgr)->SendScriptMsgs(kSS_ResistedDamage, mgr);
          } else {
            current->SendScriptMsgs(kSS_PressA, mgr);
            OpenNode(node->GetId(), mgr);
          }
        }
      }
    } else if (input.PB()) {
      handled = true;
      if (net->mIsLocked) {
        net->SendScriptMsgs(kSS_ReflectedDamage, mgr);
      } else {
        CloseNode(mgr);
      }
    } else if (input.PX()) {
      handled = true;
      if (!net->mIsLocked) {
        net->SendScriptMsgs(kSS_PressX, mgr);
      }
    } else if (input.PY()) {
      handled = true;
      if (!net->mIsLocked) {
        net->SendScriptMsgs(kSS_PressY, mgr);
      }
    } else if (input.PZ()) {
      handled = true;
      if (!net->mIsLocked) {
        net->SendScriptMsgs(kSS_PressZ, mgr);
      }
    } else if (input.PStart()) {
      handled = true;
      if (!net->mIsLocked) {
        net->SendScriptMsgs(kSS_PressStart, mgr);
      }
    }
  }
  return handled;
}

uchar CScriptFrontEndDataNetwork::HandleStick(const CFinalInput& input, CStateManager& mgr) {
  bool handled = false;
  SDataNetworkNode& node = mNodes[mCurIndex];
  CScriptFrontEndDataNetwork* net = node.GetNetwork(mgr);
  int prevSelected = node.GetSelectedChild();
  if (!mIsLocked && net->x2d3) {
    const float x = input.GetAnalogLeftX();
    const float y = input.GetAnalogLeftY();
    if (CMath::AbsF(x) < 0.3f && CMath::AbsF(y) < 0.3f) {
      if (prevSelected == -1) {
        node.SetSelectedChild(0);
        prevSelected = 0;
      }
    } else {
      CVector3f dir(x, 0.f, y);
      handled = true;
      dir = dir.AsNormalized() * net->GetConnectionRadius();
      dir = mOrientation.Transform(dir);
      const CVector3f& pos = node.GetPos();
      float best = 3.4028235e38f;
      int bestIdx = 0;
      for (int i = 0; i < node.mChildren.size(); ++i) {
        SDataNetworkNode& child = mNodes[node.mChildren[i]];
        if (!child.GetConstNetwork(mgr)->mIsLocked) {
          const CVector3f rel = child.GetPos() - pos;
          const float distSq = (rel - dir).MagSquared();
          if (distSq < best) {
            bestIdx = i;
            best = distSq;
          }
        }
      }
      node.SetSelectedChild(bestIdx);
    }
  }
  if (prevSelected != node.GetSelectedChild() && node.GetSelectedChild() != -1) {
    SendScriptMsgs(kSS_Modify, mgr);
  }
  return handled;
}

void CScriptFrontEndDataNetwork::SetSelection(CStateManager& mgr, int index, bool immediate) {
  mPrevIndex = mCurIndex;
  mCurIndex = index;
  if (immediate) {
    mTransitionState = kTS_Idle;
    mTransitionT = 0.f;
    mTransitionDuration = 1.f;
  } else {
    const SDataNetworkNode& node = mNodes[mPrevIndex];
    float time = node.GetConstNetwork(mgr)->mShrinkTime;
    if (mCurIndex < mPrevIndex) {
      mTransitionForward = 0;
    } else {
      mTransitionForward = 1;
    }
    mTransitionState = kTS_Shrink;
    mTransitionDuration = time;
    if (node.mChildren.size() == 0) {
      mTransitionT = 0.f;
      UpdateTransition(mgr, 0.f);
    } else {
      mTransitionT = 1.f;
    }
  }
}

CVector3f CScriptFrontEndDataNetwork::GetFalloff(float radius, float strength, const CVector3f& a,
                                                 const CVector3f& b) const {
  const CVector3f delta = a - b;
  const float radSq = radius * radius;
  const float distSq = delta.MagSquared();
  if (distSq < radSq) {
    const float t = 1.f - distSq / radSq;
    if (delta.CanBeNormalized()) {
      return strength * (t * delta.AsNormalized());
    }
  }
  return CVector3f::Zero();
}

CVector3f CScriptFrontEndDataNetwork::GetSeparation(const CStateManager& mgr,
                                                    const SDataNetworkNode& node,
                                                    int idx) const {
  CVector3f result = CVector3f::Zero();
  float best = 999999.f;
  const CVector3f& pos = mNodes[idx].GetPos();
  for (int i = 0; i < node.mChildren.size(); ++i) {
    const int childIdx = node.mChildren[i];
    if (idx != childIdx) {
      const SDataNetworkNode& child = mNodes[childIdx];
      const CVector3f delta = child.GetPos() - pos;
      const float distSq = delta.MagSquared();
      if (distSq < best) {
        best = distSq;
        result = child.GetPos();
      }
    }
  }
  const float radius = 1.53f * node.GetConstNetwork(mgr)->GetConnectionRadius();
  result = GetFalloff(radius, 0.8f, pos, result);
  if (node.mParent != -1) {
    result += GetFalloff(radius, 0.8f, pos, mNodes[node.mParent].GetPos());
  }
  return result;
}

CVector3f CScriptFrontEndDataNetwork::GetCohesion(const CStateManager& mgr,
                                                  const SDataNetworkNode& node,
                                                  int idx) const {
  CVector3f sum = CVector3f::Zero();
  const CVector3f& pos = mNodes[idx].GetPos();
  int count = 0;
  const float maxDist = 6.667f * node.GetConstNetwork(mgr)->GetConnectionRadius();
  const float maxDistSq = maxDist * maxDist;
  for (int i = 0; i < node.mChildren.size(); ++i) {
    const int childIdx = node.mChildren[i];
    if (idx != childIdx) {
      const SDataNetworkNode& child = mNodes[childIdx];
      const CVector3f delta = pos - child.GetPos();
      if (delta.MagSquared() < maxDistSq) {
        sum += child.GetPos();
        ++count;
      }
    }
  }
  if (count > 0) {
    sum *= 1.f / count;
    const CVector3f delta = sum - pos;
    if (delta.CanBeNormalized()) {
      const float distSq = delta.MagSquared();
      float t;
      if (distSq < maxDistSq) {
        t = distSq / maxDistSq;
      } else {
        t = 1.f;
      }
      return 0.2f * (t * delta.AsNormalized());
    }
  }
  return CVector3f::Zero();
}

CVector3f CScriptFrontEndDataNetwork::GetAttraction(const SDataNetworkNode& node,
                                                    const CVector3f& pos) const {
  const CVector3f delta = pos - node.GetPos();
  if (delta.CanBeNormalized()) {
    const float distSq = delta.MagSquared();
    const float maxDist = 0.2f * GetConnectionRadius();
    const float maxDistSq = maxDist * maxDist;
    float t;
    if (distSq < maxDistSq) {
      t = distSq / maxDistSq;
    } else {
      t = 1.f;
    }
    return 10.f * (t * delta.AsNormalized());
  }
  return CVector3f::Zero();
}

CEntity* REL_LoadFrontEndDataNetwork(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrFrontEndDataNetwork sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrFrontEndDataNetwork.inc"

  const bool isRoot = sldrThis.isRoot;
  const bool unknown_0x77f59f4a = sldrThis.unknown_0x77f59f4a;
  const bool unknown_0x29c0cb7f = sldrThis.unknown_0x29c0cb7f;
  const bool isProxy = sldrThis.isProxy;
  const bool isLocked = sldrThis.isLocked;
  const CAssetId hotDotTexture = sldrThis.hotDotTexture;
  const CAssetId hotDotHaloTexture = sldrThis.hotDotHaloTexture;
  return rs_new CScriptFrontEndDataNetwork(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      sldrThis.transitionShrinkSpline, sldrThis.transitionMoveSpline,
      sldrThis.transitionExpandSpline, sldrThis.transitionMoveInSpline, isRoot,
      unknown_0x77f59f4a, unknown_0x29c0cb7f, isProxy,
      sldrThis.canBeSelected, isLocked, sldrThis.unknown_0x8b8fa0fe,
      sldrThis.unknown_0xd0f2d612, hotDotTexture, hotDotHaloTexture,
      sldrThis.hotDotAButtonTexture, sldrThis.selectedColor, sldrThis.unselectedMinColor,
      sldrThis.unselectedMaxColor, sldrThis.disabledColor, sldrThis.rotationSound,
      sldrThis.rotationSoundVolume, sldrThis.transitionShrinkTime, sldrThis.transitionMoveTime,
      sldrThis.transitionExpandTime, sldrThis.transitionMoveInTime, sldrThis.connectionRadius);
}

static void SetFuncPtrs() {
  static SFrontEndDataNetwork_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadFrontEndDataNetwork;
  SetSFrontEndDataNetwork_FuncPtrs(&funcPtrs);
}

void RELMain() { SetFuncPtrs(); }

void RELExit() { SetSFrontEndDataNetwork_FuncPtrs(nullptr); }

CScriptFrontEndDataNetwork::~CScriptFrontEndDataNetwork() {}
