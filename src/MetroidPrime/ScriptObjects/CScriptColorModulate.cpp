#include "MetroidPrime/ScriptObjects/CScriptColorModulate.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "math.h"
#include "rstl/math.hpp"

CScriptColorModulate::CScriptColorModulate(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CColor& colorA,
    const CColor& colorB, EBlendMode blendMode, float timeA2B, float timeB2A, bool doReverse,
    bool resetTargetWhenDone, bool depthCompare, bool depthUpdate, bool depthBackwards,
    bool autoStart, bool updateTime, bool loopForever, bool externalTime,
    bool copyModelColorToColorA, const SLdrSpline& controlSpline)
: CEntity(uid, info, name, 0)
, mParent(kInvalidUniqueId)
, mFadeState(kFS_AtoB)
, mCurTime(0.f)
, mColorA(colorA)
, mColorB(colorB)
, mBlendMode(blendMode)
, mTimeA2B(timeA2B)
, mTimeB2A(timeB2A)
, mControlSpline(controlSpline)
, mDoReverse(doReverse)
, mResetTargetWhenDone(resetTargetWhenDone)
, mDepthCompare(depthCompare)
, mDepthUpdate(depthUpdate)
, mDepthBackwards(depthBackwards)
, mReversing(false)
, mEnable(false)
, mDieOnEnd(false)
, mIsFadeOutHelper(false)
, mUpdateTime(updateTime)
, mAutoStart(autoStart)
, mLoopForever(loopForever)
, mExternalTime(externalTime)
, mCopyModelColorToColorA(copyModelColorToColorA) {}

TUniqueId CScriptColorModulate::FadeInHelper(CStateManager& mgr, TUniqueId obj, float fadeTime) {
  const CEntity* entity = mgr.GetObjectById(obj);
  const TAreaId area = entity ? entity->GetCurrentAreaId() : mgr.GetNextAreaId();
  const CActor* actor = TCastToConstPtr< CActor >(entity);
  const CModelFlags flags = actor ? actor->GetModelFlags() : CModelFlags::Normal();
  const uint depthFlags = flags.GetOtherFlags();
  const TUniqueId uid = mgr.AllocateUniqueId();
  CScriptColorModulate* mod = rs_new CScriptColorModulate(
      uid, rstl::string(), CEntityInfo(area, NullConnectionList, true), CColor(1.f, 1.f, 1.f, 0.f),
      CColor::White(), kBM_Alpha, fadeTime, 0.f, false, true,
      (depthFlags & CModelFlags::kF_DepthCompare) != 0,
      (depthFlags & CModelFlags::kF_DepthUpdate) != 0,
      (depthFlags & CModelFlags::kF_DepthGreater) != 0, true, true, false, false, false,
      SLdrSpline());
  mod->mParent = obj;
  mod->mEnable = true;
  mod->mDieOnEnd = true;
  mgr.AddObject(mod);
  mod->Think(0.f, mgr);
  return uid;
}

TUniqueId CScriptColorModulate::FadeOutHelper(CStateManager& mgr, TUniqueId obj, float fadeTime) {
  const CEntity* entity = mgr.GetObjectById(obj);
  const TAreaId area = entity ? entity->GetCurrentAreaId() : mgr.GetNextAreaId();
  const CActor* actor = TCastToConstPtr< CActor >(entity);
  const CModelFlags flags = actor ? actor->GetModelFlags() : CModelFlags::Normal();
  const uint depthFlags = flags.GetOtherFlags();
  const TUniqueId uid = mgr.AllocateUniqueId();
  CScriptColorModulate* mod = rs_new CScriptColorModulate(
      uid, rstl::string(), CEntityInfo(area, NullConnectionList, true), CColor::White(),
      CColor(1.f, 1.f, 1.f, 0.f), kBM_Alpha, fadeTime, 0.f, false, true,
      (depthFlags & CModelFlags::kF_DepthCompare) != 0,
      (depthFlags & CModelFlags::kF_DepthUpdate) != 0,
      (depthFlags & CModelFlags::kF_DepthGreater) != 0, true, true, true, false, false,
      SLdrSpline());
  mod->mParent = obj;
  mod->mEnable = true;
  mod->mDieOnEnd = true;
  mod->mIsFadeOutHelper = true;
  mgr.AddObject(mod);
  mod->Think(0.f, mgr);
  return uid;
}

void CScriptColorModulate::SetTargetFlags(CStateManager& mgr, const CModelFlags& flags) {
  for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
       it != GetConnectionList().end(); ++it) {
    if (it->state != kSS_Play || it->msg != kSM_Activate) {
      continue;
    }
    const CStateManager::TIdListResult ids = mgr.GetIdListForScript(it->objId);
    for (CStateManager::TIdList::const_iterator id = ids.first; id != ids.second; ++id) {
      if (CActor* actor = TCastToPtr< CActor >(mgr.GetObjectByIdFromListAll(id->second))) {
        actor->SetModelFlags(flags);
      }
    }
  }
  if (mParent != kInvalidUniqueId) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.GetObjectByIdFromListAll(mParent))) {
      actor->SetModelFlags(flags);
    }
  }
}

void CScriptColorModulate::End(CStateManager& mgr) {
  bool done = false;
  if (mControlSpline.GetKnots().empty()) {
    if (mDoReverse && !mReversing) {
      mReversing = true;
      mFadeState = mFadeState == kFS_AtoB ? kFS_BtoA : kFS_AtoB;
    } else {
      done = true;
    }
  } else {
    if (mLoopForever) {
      mCurTime -= mControlSpline.GetMaxTime();
      return;
    }
    done = true;
  }
  mCurTime = 0.f;
  if (!done) {
    return;
  }
  mEnable = false;
  mReversing = false;
  if (mResetTargetWhenDone) {
    CModelFlags flags = CModelFlags::Normal().DepthCompareUpdate(mDepthCompare, mDepthUpdate);
    if (mDepthBackwards) {
      flags = CModelFlags(flags, flags.GetOtherFlags() | CModelFlags::kF_DepthGreater |
                                     CModelFlags::kF_Unknown200);
    }
    SetTargetFlags(mgr, flags);
  }
  if (mIsFadeOutHelper) {
    mgr.SendScriptMsg(
        CScriptMsg(GetUniqueId(), kInvalidUniqueId, mParent, kSM_Deactivate, kSS_InvalidState));
  }
  SendScriptMsgs(kSS_MaxReached, mgr, kInvalidUniqueId, kSM_None);
  if (mDieOnEnd) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

CModelFlags CScriptColorModulate::CalculateFlags(const CColor& col) const {
  if (mDepthBackwards) {
    switch (mBlendMode) {
    case kBM_Alpha:
      return CModelFlags::AlphaBlended(col)
          .DepthCompareUpdate(mDepthCompare, mDepthUpdate)
          .DepthBackwards();
    case kBM_Additive:
      return CModelFlags::Additive(col)
          .DepthCompareUpdate(mDepthCompare, mDepthUpdate)
          .DepthBackwards();
    case kBM_Additive2:
      return CModelFlags(CModelFlags::kT_Additive2, col)
          .DepthCompareUpdate(mDepthCompare, mDepthUpdate)
          .DepthBackwards();
    case kBM_Opaque:
      return CModelFlags(CModelFlags::kT_One, col)
          .DepthCompareUpdate(mDepthCompare, mDepthUpdate)
          .DepthBackwards();
    case kBM_OpaqueAdd:
      return CModelFlags(CModelFlags::kT_Two, col)
          .DepthCompareUpdate(mDepthCompare, mDepthUpdate)
          .DepthBackwards();
    }
  }
  switch (mBlendMode) {
  case kBM_Alpha:
    if (col == CColor::White()) {
      const bool update = mDepthUpdate;
      const bool compare = mDepthCompare;
      return CModelFlags::Normal().DepthCompareUpdate(compare, update);
    }
    return CModelFlags::AlphaBlended(col).DepthCompareUpdate(mDepthCompare, mDepthUpdate);
  case kBM_Additive:
    return CModelFlags::Additive(col).DepthCompareUpdate(mDepthCompare, mDepthUpdate);
  case kBM_Additive2:
    return CModelFlags(CModelFlags::kT_Additive2, col)
        .DepthCompareUpdate(mDepthCompare, mDepthUpdate);
  case kBM_Opaque:
    if (col == CColor::White()) {
      const bool update = mDepthUpdate;
      const bool compare = mDepthCompare;
      return CModelFlags::Normal().DepthCompareUpdate(compare, update);
    }
    return CModelFlags(CModelFlags::kT_One, col).DepthCompareUpdate(mDepthCompare, mDepthUpdate);
  case kBM_OpaqueAdd:
    return CModelFlags(CModelFlags::kT_Two, col).DepthCompareUpdate(mDepthCompare, mDepthUpdate);
  }
  return CModelFlags::Normal();
}

void CScriptColorModulate::Think(float dt, CStateManager& mgr) {
  if (!GetActive() || !mEnable) {
    return;
  }
  if (mUpdateTime && !mExternalTime) {
    mCurTime += dt;
  }
  if (!mControlSpline.GetKnots().empty()) {
    const CColor color = CColor::Lerp(mColorA, mColorB, mControlSpline.EvaluateAt(mCurTime));
    SetTargetFlags(mgr, CalculateFlags(color));
    if (mCurTime >= mControlSpline.GetMaxTime()) {
      End(mgr);
    }
    return;
  }
  switch (mFadeState) {
  case kFS_AtoB: {
    const float t = close_enough(mTimeA2B, 0.f) ? 1.f : rstl::min_val(1.f, mCurTime / mTimeA2B);
    SetTargetFlags(mgr, CalculateFlags(CColor::Lerp(mColorA, mColorB, t)));
    if (mCurTime > mTimeA2B) {
      End(mgr);
    }
    break;
  }
  case kFS_BtoA: {
    const float t = close_enough(mTimeB2A, 0.f) ? 1.f : rstl::min_val(1.f, mCurTime / mTimeB2A);
    SetTargetFlags(mgr, CalculateFlags(CColor::Lerp(mColorB, mColorA, t)));
    if (mCurTime > mTimeB2A) {
      End(mgr);
    }
    break;
  }
  }
}

void CScriptColorModulate::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CEntity::AcceptScriptMsg(mgr, msg);
  if (!GetActive()) {
    return;
  }
  switch (message) {
  case kSM_Increment:
  case kSM_Decrement: {
    CopyTargetColor(mgr);
    if (mReversing) {
      mFadeState = mFadeState == kFS_AtoB ? kFS_BtoA : kFS_AtoB;
      mReversing = false;
    } else {
      const bool forward = message == kSM_Increment;
      if (mEnable) {
        if (mFadeState == kFS_AtoB) {
          mCurTime = 0.f;
        } else if (forward) {
          mCurTime = mTimeA2B - mTimeA2B * (mCurTime / mTimeB2A);
        } else {
          mCurTime = mTimeB2A - mTimeB2A * (mCurTime / mTimeA2B);
        }
      } else {
        SetTargetFlags(mgr, CalculateFlags(forward ? mColorA : mColorB));
      }
      mEnable = true;
      mFadeState = forward ? kFS_AtoB : kFS_BtoA;
    }
    if ((mFadeState == kFS_AtoB && mTimeA2B == 0.f) ||
        (mFadeState == kFS_BtoA && mTimeB2A == 0.f)) {
      Think(0.f, mgr);
    }
    break;
  }
  case kSM_Start:
    CopyTargetColor(mgr);
    mEnable = true;
    break;
  case kSM_Stop:
    mEnable = false;
    break;
  case kSM_Reset:
    mCurTime = 0.f;
    break;
  case kSM_XALD:
    mEnable = mAutoStart;
    if (mExternalTime) {
      mEnable = true;
    }
    mCurTime = 0.f;
    break;
  }
}

// Guessed name
void CScriptColorModulate::SetExternalTime(float time) {
  if (mExternalTime) {
    if (mControlSpline.GetKnots().empty()) {
      const float duration = mFadeState == kFS_BtoA ? mTimeB2A : mTimeA2B;
      mCurTime = fmod(time, duration);
    } else {
      mCurTime = time;
    }
  }
}

// Guessed name
void CScriptColorModulate::CopyTargetColor(CStateManager& mgr) {
  if (mCopyModelColorToColorA) {
    const TUniqueId target = FindConnectedObject(mgr, kSS_Play, kSM_Activate);
    if (target != kInvalidUniqueId) {
      if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(target))) {
        mColorA = actor->GetModelFlags().GetColor();
      }
    }
  }
}

CScriptColorModulate::~CScriptColorModulate() {}
