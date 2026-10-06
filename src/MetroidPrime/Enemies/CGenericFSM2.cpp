#include "MetroidPrime/Enemies/CGenericFSM2.hpp"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/CGenericFSM2State.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

template class CGenericFSM2State< CPatterned >;

// Guessed name.
CFactoryFnReturn FAiStateMachine2Factory(const SObjectTag& tag, CInputStream& in,
                                         const CVParamTransfer& params) {
  return CFactoryFnReturn(rs_new CGenericFSM2(in));
}

CGenericFSM2::CGenericFSM2(CInputStream& in) {
  in.ReadInt32();
  in.ReadInt32();
  const int numStates = in.ReadInt32();
  const int numTriggers = in.ReadInt32();
  const int numCodes = in.ReadInt32();
  const int numSubMachines = in.ReadInt32();
  mStates.resize(numStates, CState2State());
  mTriggers.resize(numTriggers, CState2Trigger());
  mCodes.resize(numCodes, CState2Code());
  mSubMachines.resize(numSubMachines, CState2SubMachine());

  for (int i = 0; i < numStates; ++i) {
    rstl::string name(in);
    rstl::vector< SStateMachine2SerializedTransition > serialized(in);
    rstl::vector< SStateMachine2Transition > transitions;
    transitions.reserve(serialized.size());
    for (int j = 0; j < serialized.size(); ++j) {
      transitions.push_back_unsafe(
          SStateMachine2Transition(serialized[j].mName, ResolveNode(serialized[j].mTarget)));
    }
    mStates[i].Setup(name, transitions);
  }

  for (int i = 0; i < numTriggers; ++i) {
    rstl::string name(in);
    const CTriggerData argument(in.ReadFloat());
    rstl::vector< SStateMachine2SerializedTransition > serialized(in);
    rstl::vector< SStateMachine2Transition > transitions;
    transitions.reserve(serialized.size());
    for (int j = 0; j < serialized.size(); ++j) {
      transitions.push_back_unsafe(
          SStateMachine2Transition(serialized[j].mName, ResolveNode(serialized[j].mTarget)));
    }
    mTriggers[i].Setup(name, argument, in.ReadBool(), transitions);
  }

  for (int i = 0; i < numCodes; ++i) {
    rstl::string name(in);
    rstl::vector< SStateMachine2SerializedTransition > serialized(in);
    rstl::vector< SStateMachine2Transition > transitions;
    transitions.reserve(serialized.size());
    for (int j = 0; j < serialized.size(); ++j) {
      transitions.push_back_unsafe(
          SStateMachine2Transition(serialized[j].mName, ResolveNode(serialized[j].mTarget)));
    }
    mCodes[i].Setup(name, transitions);
  }

  for (int i = 0; i < numSubMachines; ++i) {
    rstl::string name(in);
    rstl::vector< SStateMachine2SerializedTransition > serialized(in);
    rstl::vector< SStateMachine2Transition > transitions;
    transitions.reserve(serialized.size());
    for (int j = 0; j < serialized.size(); ++j) {
      transitions.push_back_unsafe(
          SStateMachine2Transition(serialized[j].mName, ResolveNode(serialized[j].mTarget)));
    }
    mSubMachines[i].Setup(name, transitions, in.ReadInt32());
  }
}

CGenericFSM2::~CGenericFSM2() {}

const CState2* CGenericFSM2::ResolveNode(uint target) const {
  const uint index = target & 0xffffff;
  switch (target >> 24) {
  case 0:
    return &mTriggers[index];
  case 1:
    return &mStates[index];
  case 2:
    return &mCodes[index];
  case 3:
    return &mSubMachines[index];
  default:
    return nullptr;
  }
}

SStateMachine2SerializedTransition::SStateMachine2SerializedTransition(CInputStream& in)
: mName(in.Get< rstl::string >()), mTarget(in.ReadInt32()) {}

CState2State::CState2State() : mName(rstl::string_l("")) {}

CState2Code::CState2Code() : mName(rstl::string_l("")) {}

CState2Trigger::CState2Trigger() : mName(rstl::string_l("")), mArgument(0.f), mNegate(false) {}

CState2SubMachine::CState2SubMachine() : mName(rstl::string_l("")) {}

void CState2State::Setup(const rstl::string& name,
                         const rstl::vector< SStateMachine2Transition >& transitions) {
  mName = name;
  mTransitions = transitions;
}

void CState2Code::Setup(const rstl::string& name,
                        const rstl::vector< SStateMachine2Transition >& transitions) {
  mName = name;
  mTransitions = transitions;
}

void CState2Trigger::Setup(const rstl::string& name, const CTriggerData& argument, bool negate,
                           const rstl::vector< SStateMachine2Transition >& transitions) {
  mName = name;
  mArgument = argument;
  mNegate = negate;
  mTransitions = transitions;
}

void CState2SubMachine::Setup(const rstl::string& name,
                              const rstl::vector< SStateMachine2Transition >& transitions,
                              CAssetId assetId) {
  mName = name;
  mTransitions = transitions;
  mMachine = gpSimplePool->GetObj(SObjectTag('FSM2', assetId));
  mMachine->Lock();
}

const CGenericFSM2* CState2SubMachine::GetMachine() const {
  if (mMachine->IsLoaded()) {
    CToken token = *mMachine;
    return static_cast< const CGenericFSM2* >(token.GetObj()->GetContents());
  }
  return nullptr;
}
