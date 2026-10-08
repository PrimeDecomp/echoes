#include "MetroidPrime/ScriptObjects/CScriptRubiksPuzzle.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRubiksPuzzle.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"

#include "rstl/algorithm.hpp"

namespace {
// Guessed name: orders the pieces row by row as seen from the puzzle's centre.
class CPieceOrder {
public:
  CPieceOrder(const CStateManager& mgr, const CTransform4f& xf) : mMgr(&mgr), mXf(xf) {}

  bool operator()(const rstl::pair< TUniqueId, int >& a,
                  const rstl::pair< TUniqueId, int >& b) const {
    const CActor* actorA = TCastToConstPtr< CActor >(mMgr->GetObjectById(a.first));
    const CActor* actorB = TCastToConstPtr< CActor >(mMgr->GetObjectById(b.first));
    if (actorA == nullptr || actorB == nullptr) {
      return false;
    }
    const CVector3f localA = mXf.TransposeRotate(actorA->GetTranslation() - mXf.GetTranslation());
    const CVector3f localB = mXf.TransposeRotate(actorB->GetTranslation() - mXf.GetTranslation());
    return localA.GetX() + -20.f * localA.GetZ() < localB.GetX() + -20.f * localB.GetZ();
  }

private:
  const CStateManager* mMgr;
  CTransform4f mXf;
};
} // namespace

// Guessed name: the pieces turned by each of the four buttons.
static int skBlockPieces[4][4] = {
    {0, 1, 3, 4},
    {1, 2, 4, 5},
    {3, 4, 6, 7},
    {4, 5, 7, 8},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Start", reinterpret_cast< CPatterned::StateMachine::StateFunc >(&CScriptRubiksPuzzle::Start)},
    {"Waiting",
     reinterpret_cast< CPatterned::StateMachine::StateFunc >(&CScriptRubiksPuzzle::Waiting)},
    {"Rotating",
     reinterpret_cast< CPatterned::StateMachine::StateFunc >(&CScriptRubiksPuzzle::Rotating)},
};

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"ButtonPressed", reinterpret_cast< CPatterned::StateMachine::TriggerFunc >(
                          &CScriptRubiksPuzzle::ButtonPressed)},
    {"RotationOver",
     reinterpret_cast< CPatterned::StateMachine::TriggerFunc >(&CScriptRubiksPuzzle::RotationOver)},
};

CScriptRubiksPuzzle::CPressRing::~CPressRing() {
  clear();
  rstl::rmemory_allocator::deallocate(mItems);
}

void CScriptRubiksPuzzle::CPressRing::clear() { erase(mFirst, mEnd); }

void CScriptRubiksPuzzle::CPressRing::erase(int first, int last) {
  int dest = first;
  for (int i = last; i != mEnd; ++i) {
    mItems[dest] = mItems[i];
    ++dest;
  }
  mEnd = dest;
}

void CScriptRubiksPuzzle::CPressRing::reserve(int capacity) {
  if (mCapacity < capacity) {
    SButtonPress* items;
    rstl::rmemory_allocator::allocate(items, capacity);
    for (int i = mFirst; i != mEnd; ++i) {
      items[i] = mItems[i];
    }
    rstl::rmemory_allocator::deallocate(mItems);
    mItems = items;
    mCapacity = capacity;
  }
}

void CScriptRubiksPuzzle::CPressRing::grow(int count) { reserve(mEnd + count); }

void CScriptRubiksPuzzle::CPressRing::push_back(const SButtonPress& press) {
  if (mCapacity - mEnd < 1) {
    grow(mCapacity > 0 ? mCapacity : 4);
  }
  mItems[mEnd] = press;
  ++mEnd;
}

CScriptRubiksPuzzle::CPressQueue::~CPressQueue() {}

void CScriptRubiksPuzzle::CPressQueue::push_back(const SButtonPress& press) {
  mRing.push_back(press);
}

CScriptRubiksPuzzle::CScriptRubiksPuzzle(TUniqueId uid, const CEntityInfo& info,
                                         const rstl::string& name, const SLdrRubiksPuzzleData& data)
: CEntity(uid, info, name, 0)
, mProperties(data)
, mStateMachine(gpSimplePool->GetObj(SObjectTag('FSM2', data.stateMachine)))
, mStateMachineState(rs_new CGenericFSM2State< CPatterned >)
, mCenterId(kInvalidUniqueId)
, mButtonId(kInvalidUniqueId)
, mBlock(4)
, mRotationAngle(0.f)
, x104_(0)
, mRotating(false)
, mSolved(false)
, mTopRowLocked(false)
, mBottomRowLocked(false) {
  mStateMachine->Lock();
}

void CScriptRubiksPuzzle::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CEntity::AcceptScriptMsg(mgr, msg);
  switch (msg.GetMessage()) {
  case kSM_Delete:
    if (mStateMachineState->HasState()) {
      mStateMachineState->Reset(mgr, reinterpret_cast< CPatterned& >(*this));
    }
    break;
  case kSM_InternalMessage00:
    if (!mTopRowLocked && mPressQueue.size() < kMaxQueuedPresses) {
      mPressQueue.push_back(SButtonPress(msg.GetSenderId(), 0));
    }
    break;
  case kSM_InternalMessage01:
    if (!mTopRowLocked && mPressQueue.size() < kMaxQueuedPresses) {
      mPressQueue.push_back(SButtonPress(msg.GetSenderId(), 1));
    }
    break;
  case kSM_InternalMessage02:
    if (!mBottomRowLocked && mPressQueue.size() < kMaxQueuedPresses) {
      mPressQueue.push_back(SButtonPress(msg.GetSenderId(), 2));
    }
    break;
  case kSM_InternalMessage03:
    if (!mBottomRowLocked && mPressQueue.size() < kMaxQueuedPresses) {
      mPressQueue.push_back(SButtonPress(msg.GetSenderId(), 3));
    }
    break;
  default:
    break;
  }
}

void CScriptRubiksPuzzle::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  if (!mStateMachineState->HasState()) {
    TToken< CGenericFSM2 > machine(*mStateMachine);
    mStateMachineState->Setup(**machine);
    mStateMachineState->SetStateFunctions(skStates, 3);
    mStateMachineState->SetTriggerFunctions(skTriggers, 2);
    mStateMachineState->SetState(mgr, reinterpret_cast< CPatterned& >(*this),
                                 rstl::string_l("Start"));
    SendToConnected(mgr, kSS_InternalState00, kSM_Increment);
    SendToConnected(mgr, kSS_InternalState01, kSM_Increment);
    SendToConnected(mgr, kSS_InternalState02, kSM_Increment);
    SendToConnected(mgr, kSS_InternalState16, kSM_Increment);
    SendToConnected(mgr, kSS_InternalState17, kSM_Increment);
    SendToConnected(mgr, kSS_InternalState03, kSM_Increment);
    SendToConnected(mgr, kSS_InternalState04, kSM_Increment);
    SendToConnected(mgr, kSS_InternalState05, kSM_Increment);
  }
  mStateMachineState->Update(mgr, reinterpret_cast< CPatterned& >(*this), dt);
}

void CScriptRubiksPuzzle::Start(CStateManager& mgr, int msg, float dt) {
  if (msg != 0) {
    return;
  }
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (rstl::vector< SConnection >::const_iterator it = connections.begin();
       it != connections.end(); ++it) {
    const EScriptObjectState state = it->state;
    if (state != kSS_InternalState00 && state != kSS_InternalState01 &&
        state != kSS_InternalState02) {
      continue;
    }
    CStateManager::TIdListResult ids = mgr.GetIdListForScript(it->objId);
    CStateManager::TIdList::const_iterator current = ids.first;
    while (current != ids.second) {
      const TUniqueId id = current->second;
      switch (state) {
      case kSS_InternalState00:
        mPieces.push_back(SPiece(id, 0));
        break;
      case kSS_InternalState01:
        mPieces.push_back(SPiece(id, 1));
        break;
      case kSS_InternalState02:
        mPieces.push_back(SPiece(id, 2));
        break;
      default:
        break;
      }
      ++current;
    }
  }
  mCenterId = FindConnectedObject(mgr, kSS_InternalState15, kSM_None);
  SortPieces(mgr);
  RecordPiecePositions(mgr);
}

void CScriptRubiksPuzzle::Waiting(CStateManager& mgr, int msg, float dt) {
  switch (msg) {
  case 0:
    SortPieces(mgr);
    RestorePiecePositions(mgr);
    UpdateIndicators(mgr);
    break;
  }
}

void CScriptRubiksPuzzle::Rotating(CStateManager& mgr, int msg, float dt) {
  switch (msg) {
  case 0: {
    mRotationAngle = 0.f;
    mRotating = true;
    mButtonId = mPressQueue.front().first;
    mBlock = mPressQueue.front().second;
    mPressQueue.pop_front();
    if ((mBlock & 2) == 0 ? mTopRowLocked : mBottomRowLocked) {
      mRotating = false;
    } else {
      UpdateIndicators(mgr);
    }
    break;
  }
  case 1: {
    if (!mRotating) {
      break;
    }
    float step = dt * mProperties.rotationSpeed;
    const float remaining = 90.f - mRotationAngle;
    if (step >= remaining) {
      step = remaining;
    }
    if (0.f >= step) {
      step = 0.f;
    }
    mRotationAngle += step;
    if ((mBlock & 1) != 0) {
      step = -step;
    }
    const CTransform4f rotation = CTransform4f::RotateY(CRelAngle::FromDegrees(step));
    const CActor* button = TCastToConstPtr< CActor >(mgr.GetObjectById(mButtonId));
    const CActor* center = TCastToConstPtr< CActor >(mgr.GetObjectById(mCenterId));
    if (button != nullptr && center != nullptr) {
      for (int i = 0; i < 4; ++i) {
        CActor* piece =
            TCastToPtr< CActor >(mgr.ObjectById(mPieces[skBlockPieces[mBlock][i]].first));
        if (piece != nullptr) {
          const CVector3f local = center->GetTransform().TransposeRotate(piece->GetTranslation() -
                                                                         button->GetTranslation());
          const CVector3f turned = rotation * local;
          piece->SetTranslation(center->GetTransform().Rotate(turned) + button->GetTranslation());
        }
      }
    }
    break;
  }
  case 2:
    mRotating = false;
    break;
  }
}

bool CScriptRubiksPuzzle::ButtonPressed(CStateManager& mgr, const float& arg) {
  return mPressQueue.size() > 0;
}

bool CScriptRubiksPuzzle::RotationOver(CStateManager& mgr, const float& arg) {
  return !mRotating || mRotationAngle >= 90.f;
}

void CScriptRubiksPuzzle::SortPieces(CStateManager& mgr) {
  if (mPieces.size() > 1) {
    const CActor* center = TCastToConstPtr< CActor >(mgr.GetObjectById(mCenterId));
    if (center != nullptr) {
      rstl::sort(mPieces.begin(), mPieces.end(), CPieceOrder(mgr, center->GetTransform()));
    }
  }
}

void CScriptRubiksPuzzle::RecordPiecePositions(CStateManager& mgr) {
  for (int i = 0; i < mPieces.size(); ++i) {
    const CActor* piece = TCastToConstPtr< CActor >(mgr.GetObjectById(mPieces[i].first));
    mPiecePositions.push_back(piece->GetTranslation());
  }
}

void CScriptRubiksPuzzle::RestorePiecePositions(CStateManager& mgr) {
  for (int i = 0; i < mPieces.size(); ++i) {
    CActor* piece = TCastToPtr< CActor >(mgr.ObjectById(mPieces[i].first));
    piece->SetTranslation(mPiecePositions[i]);
  }
}

void CScriptRubiksPuzzle::SendToConnected(CStateManager& mgr, EScriptObjectState state,
                                          const EScriptObjectMessage& msg) {
  const rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, state, kSM_None);
  for (rstl::vector< TUniqueId >::const_iterator it = ids.begin(); it != ids.end(); ++it) {
    const TUniqueId id = *it;
    if (id == kInvalidUniqueId) {
      continue;
    }
    CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id));
    if (actor != nullptr) {
      actor->AcceptScriptMsg(mgr, CScriptMsg(GetUniqueId(), actor->GetUniqueId(), msg));
    }
  }
}

bool CScriptRubiksPuzzle::UpdateRow(CStateManager& mgr, bool force, int row,
                                    EScriptObjectState stateA, EScriptObjectState stateB,
                                    EScriptObjectState stateC) {
  bool correct = true;
  if (!force && mPieces[row * 3].second == row) {
    SendToConnected(mgr, stateA, kSM_Activate);
  } else {
    SendToConnected(mgr, stateA, kSM_Deactivate);
    correct = false;
  }
  if (correct && mPieces[row * 3 + 1].second == row) {
    SendToConnected(mgr, stateB, kSM_Activate);
  } else {
    SendToConnected(mgr, stateB, kSM_Deactivate);
    correct = false;
  }
  if (correct && mPieces[row * 3 + 2].second == row) {
    SendToConnected(mgr, stateC, kSM_Activate);
  } else {
    SendToConnected(mgr, stateC, kSM_Deactivate);
    correct = false;
  }
  return correct;
}

void CScriptRubiksPuzzle::UpdateIndicators(CStateManager& mgr) {
  bool topRowTurning = false;
  if (mRotating && (mBlock == 0 || mBlock == 1)) {
    topRowTurning = true;
  }
  bool bottomRowTurning = false;
  if (mRotating && !topRowTurning) {
    bottomRowTurning = true;
  }
  const bool topRow = UpdateRow(mgr, topRowTurning, 0, kSS_InternalState06, kSS_InternalState07,
                                kSS_InternalState12);
  const bool middleRow =
      UpdateRow(mgr, mRotating, 1, kSS_InternalState08, kSS_InternalState09, kSS_InternalState13);
  const bool bottomRow = UpdateRow(mgr, bottomRowTurning, 2, kSS_InternalState10,
                                   kSS_InternalState11, kSS_InternalState14);
  mSolved = topRow && middleRow && bottomRow;
  if (!mRotating) {
    if (topRow) {
      SendToConnected(mgr, kSS_InternalState16, kSM_Decrement);
      mTopRowLocked = true;
    }
    if (bottomRow) {
      SendToConnected(mgr, kSS_InternalState17, kSM_Decrement);
      mBottomRowLocked = true;
    }
  }
  if (mSolved) {
    SendScriptMsgs(kSS_InternalState18, mgr, kInvalidUniqueId, kSM_None);
    SendToConnected(mgr, kSS_InternalState00, kSM_Decrement);
    SendToConnected(mgr, kSS_InternalState01, kSM_Decrement);
    SendToConnected(mgr, kSS_InternalState02, kSM_Decrement);
    SendToConnected(mgr, kSS_InternalState16, kSM_Decrement);
    SendToConnected(mgr, kSS_InternalState17, kSM_Decrement);
    SendToConnected(mgr, kSS_InternalState03, kSM_Decrement);
    SendToConnected(mgr, kSS_InternalState04, kSM_Decrement);
    SendToConnected(mgr, kSS_InternalState05, kSM_Decrement);
    SendToConnected(mgr, kSS_InternalState12, kSM_Decrement);
    SendToConnected(mgr, kSS_InternalState13, kSM_Decrement);
    SendToConnected(mgr, kSS_InternalState14, kSM_Decrement);
    SendToConnected(mgr, kSS_InternalState06, kSM_Decrement);
    SendToConnected(mgr, kSS_InternalState07, kSM_Decrement);
    SendToConnected(mgr, kSS_InternalState08, kSM_Decrement);
    SendToConnected(mgr, kSS_InternalState09, kSM_Decrement);
    SendToConnected(mgr, kSS_InternalState10, kSM_Decrement);
    SendToConnected(mgr, kSS_InternalState11, kSM_Decrement);
  }
}

CEntity* LoadRubiksPuzzle(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrRubiksPuzzle sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrRubiksPuzzle.inc"

  return rs_new CScriptRubiksPuzzle(
      mgr.AllocateUniqueId(), LdrToEntityInfo(info, sldrThis.editorProperties),
      sldrThis.editorProperties.name, sldrThis.rubiksPuzzleProperties);
}

static void SetFuncPtrs() {
  static SScriptRubiksPuzzle_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadRubiksPuzzle;
  SetSScriptRubiksPuzzle_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSScriptRubiksPuzzle_FuncPtrs(nullptr); }

CScriptRubiksPuzzle::~CScriptRubiksPuzzle() {}
