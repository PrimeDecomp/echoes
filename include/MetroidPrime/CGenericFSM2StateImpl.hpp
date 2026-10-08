#ifndef _CGENERICFSM2STATEIMPL
#define _CGENERICFSM2STATEIMPL

// Member definitions of CGenericFSM2State. Only the translation units that instantiate the
// template include this; everything else uses the instantiation linked from the main program.
#include "MetroidPrime/CGenericFSM2State.hpp"

template < class T >
CGenericFSM2State< T >::CGenericFSM2State()
: mMachine(nullptr), mState(nullptr), mTime(0.f), mDelay(0.f) {}

template < class T >
int CGenericFSM2State< T >::GetType() const {
  return 1;
}

template < class T >
void CGenericFSM2State< T >::Reset(CStateManager& mgr, T& owner) {
  const CState2State* state = GetActiveState();
  if (state != nullptr) {
    ExecuteState(*state, mgr, owner, kStateMsg_Deactivate, 0.f);
  }
  mState = nullptr;
  mMachine = nullptr;
  mStateFunctions.clear();
  mTriggerFunctions.clear();
  mCodeFunctions.clear();
}

template < class T >
void CGenericFSM2State< T >::SetStateFunctions(const SStateFunction* functions, int count) {
  for (int i = 0; i < mSubMachines.size(); ++i) {
    mSubMachines[i].mMachine->SetStateFunctions(functions, count);
  }
  if (functions != nullptr) {
    for (int i = 0; i < count; ++i) {
      SetStateFunction(rstl::string_l(functions[i].mName), functions[i].mFunction);
    }
  }
}

template < class T >
void CGenericFSM2State< T >::SetTriggerFunctions(const STriggerFunction* functions, int count) {
  for (int i = 0; i < mSubMachines.size(); ++i) {
    mSubMachines[i].mMachine->SetTriggerFunctions(functions, count);
  }
  if (functions != nullptr) {
    for (int i = 0; i < count; ++i) {
      SetTriggerFunction(rstl::string_l(functions[i].mName), functions[i].mFunction);
    }
  }
}

template < class T >
void CGenericFSM2State< T >::SetCodeFunctions(const SCodeFunction* functions, int count) {
  for (int i = 0; i < mSubMachines.size(); ++i) {
    mSubMachines[i].mMachine->SetCodeFunctions(functions, count);
  }
  if (functions != nullptr) {
    for (int i = 0; i < count; ++i) {
      SetCodeFunction(rstl::string_l(functions[i].mName), functions[i].mFunction);
    }
  }
}

template < class T >
void CGenericFSM2State< T >::SetState(CStateManager& mgr, T& owner, const rstl::string& name) {
  const int index = GetStateIndex(name);
  if (mMachine != nullptr && index >= 0 && index < mMachine->GetStates().size()) {
    const CState2State* state = &mMachine->GetStates()[index];
    if (mState != state) {
      ExitActive(mgr, owner);
      ExecuteState(*state, mgr, owner, kStateMsg_Activate, 0.f);
    }
  }
}

template < class T >
void CGenericFSM2State< T >::Update(CStateManager& mgr, T& owner, float dt) {
  if (mState != nullptr) {
    mTime += dt;
    const CState2SubMachine* subMachine = GetActiveSubMachine();
    if (subMachine != nullptr) {
      ExecuteSubflow(*subMachine, rstl::string_l(""), mgr, owner, kStateMsg_Update, dt);
      EvaluateTransitions(mgr, owner, dt, subMachine->GetTransitions());
    } else {
      const CState2State* state = GetActiveState();
      if (state != nullptr) {
        ExecuteState(*state, mgr, owner, kStateMsg_Update, dt);
        EvaluateTransitions(mgr, owner, dt, state->GetTransitions());
      }
    }
  }
}

template < class T >
bool CGenericFSM2State< T >::HasState() const {
  return mState != nullptr;
}

template < class T >
const char* CGenericFSM2State< T >::GetName() const {
  const CState2SubMachine* subMachine = GetActiveSubMachine();
  if (subMachine != nullptr) {
    return mSubMachines[GetSubMachineIndex(subMachine)].mMachine->GetName();
  }
  const CState2State* state = GetActiveState();
  return state != nullptr ? state->GetName().data() : nullptr;
}

template < class T >
float CGenericFSM2State< T >::GetTime() const {
  const CState2SubMachine* subMachine = GetActiveSubMachine();
  if (subMachine != nullptr) {
    return mSubMachines[GetSubMachineIndex(subMachine)].mMachine->GetTime();
  }
  return mTime;
}

template < class T >
float CGenericFSM2State< T >::GetDelay() const {
  return mDelay;
}

template < class T >
void CGenericFSM2State< T >::SetDelay(float delay) {
  mDelay = delay;
}

template < class T >
void CGenericFSM2State< T >::Setup(const CGenericFSM2& machine) {
  if (mMachine == nullptr) {
    mMachine = &machine;
    mStateFunctions.resize(machine.GetStates().size(), StateFunc());
    mTriggerFunctions.resize(machine.GetTriggers().size(), TriggerFunc());
    mCodeFunctions.resize(machine.GetCodes().size(), CodeFunc());
    mSubMachines.reserve(machine.GetSubMachines().size());
    for (int i = 0; i < machine.GetSubMachines().size(); ++i) {
      const CState2SubMachine& subMachine = machine.GetSubMachines()[i];
      rstl::rc_ptr< CGenericFSM2State< T > > subState(rs_new CGenericFSM2State< T >);
      subState->Setup(*subMachine.GetMachine());
      mSubMachines.push_back_unsafe(SSubMachine(&subMachine, subState));
    }
  }
}

template < class T >
int CGenericFSM2State< T >::GetStateIndex(const rstl::string& name) const {
  if (mMachine != nullptr) {
    const rstl::vector< CState2State >& states = mMachine->GetStates();
    for (int i = 0; i < states.size(); ++i) {
      if (states[i].GetName() == name) {
        return i;
      }
    }
  }
  return -1;
}

template < class T >
int CGenericFSM2State< T >::GetTriggerIndex(const rstl::string& name) const {
  if (mMachine != nullptr) {
    const rstl::vector< CState2Trigger >& triggers = mMachine->GetTriggers();
    for (int i = 0; i < triggers.size(); ++i) {
      if (triggers[i].GetName() == name) {
        return i;
      }
    }
  }
  return -1;
}

template < class T >
int CGenericFSM2State< T >::GetCodeIndex(const rstl::string& name) const {
  if (mMachine != nullptr) {
    const rstl::vector< CState2Code >& codes = mMachine->GetCodes();
    for (int i = 0; i < codes.size(); ++i) {
      if (codes[i].GetName() == name) {
        return i;
      }
    }
  }
  return -1;
}

template < class T >
int CGenericFSM2State< T >::GetSubMachineIndex(const CState2* state) const {
  for (int i = 0; i < mSubMachines.size(); ++i) {
    if (mSubMachines[i].mState == state) {
      return i;
    }
  }
  return -1;
}

template < class T >
void CGenericFSM2State< T >::SetStateFunction(const rstl::string& name, StateFunc function) {
  if (mMachine != nullptr) {
    const rstl::vector< CState2State >& states = mMachine->GetStates();
    for (int i = 0; i < states.size(); ++i) {
      if (states[i].GetName() == name) {
        mStateFunctions[i] = function;
      }
    }
  }
}

template < class T >
void CGenericFSM2State< T >::SetTriggerFunction(const rstl::string& name, TriggerFunc function) {
  if (mMachine != nullptr) {
    const rstl::vector< CState2Trigger >& triggers = mMachine->GetTriggers();
    for (int i = 0; i < triggers.size(); ++i) {
      if (triggers[i].GetName() == name) {
        mTriggerFunctions[i] = function;
      }
    }
  }
}

template < class T >
void CGenericFSM2State< T >::SetCodeFunction(const rstl::string& name, CodeFunc function) {
  if (mMachine != nullptr) {
    const rstl::vector< CState2Code >& codes = mMachine->GetCodes();
    for (int i = 0; i < codes.size(); ++i) {
      if (codes[i].GetName() == name) {
        mCodeFunctions[i] = function;
      }
    }
  }
}

template < class T >
void CGenericFSM2State< T >::CallCode(const CState2Code& state, CStateManager& mgr, T& owner,
                                      float dt) {
  const int index = GetCodeIndex(state.GetName());
  if (mMachine != nullptr && index >= 0 && index < mMachine->GetCodes().size()) {
    CodeFunc function = mCodeFunctions[index];
    if (function != nullptr) {
      (owner.*function)(mgr, dt);
    }
  }
}

template < class T >
bool CGenericFSM2State< T >::CallTrigger(const CState2Trigger& state, CStateManager& mgr, T& owner,
                                         float dt) {
  const int index = GetTriggerIndex(state.GetName());
  if (mMachine != nullptr && index >= 0 && index < mMachine->GetTriggers().size()) {
    TriggerFunc function = mTriggerFunctions[index];
    if (function != nullptr) {
      if (state.IsNegated()) {
        return !(owner.*function)(mgr, state.GetArgument());
      }
      return (owner.*function)(mgr, state.GetArgument());
    }
  }
  return false;
}

template < class T >
void CGenericFSM2State< T >::ExecuteSubflow(const CState2SubMachine& state,
                                            const rstl::string& name, CStateManager& mgr, T& owner,
                                            EStateMsg msg, float dt) {
  const int index = GetSubMachineIndex(&state);
  if (mMachine != nullptr && index != -1) {
    const rstl::rc_ptr< CGenericFSM2State< T > >& subMachine = mSubMachines[index].mMachine;
    switch (msg) {
    case kStateMsg_Activate:
      mState = &state;
      mTime = 0.f;
      subMachine->SetState(mgr, owner, name);
      break;
    case kStateMsg_Update:
      subMachine->Update(mgr, owner, dt);
      break;
    case kStateMsg_Deactivate:
      subMachine->ExitActive(mgr, owner);
      break;
    }
  }
}

template < class T >
void CGenericFSM2State< T >::ExecuteState(const CState2State& state, CStateManager& mgr, T& owner,
                                          EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mState = &state;
    mTime = 0.f;
  }
  const int index = GetStateIndex(state.GetName());
  if (mMachine != nullptr && index >= 0 && index < mMachine->GetStates().size()) {
    StateFunc function = mStateFunctions[index];
    if (function != nullptr) {
      (owner.*function)(mgr, msg, dt);
    }
  }
}

template < class T >
const CState2SubMachine* CGenericFSM2State< T >::GetActiveSubMachine() const {
  if (mState != nullptr && mState->GetType() == CState2::kType_SubMachine) {
    return static_cast< const CState2SubMachine* >(mState);
  }
  return nullptr;
}

template < class T >
const CState2State* CGenericFSM2State< T >::GetActiveState() const {
  if (mState != nullptr && mState->GetType() == CState2::kType_State) {
    return static_cast< const CState2State* >(mState);
  }
  return nullptr;
}

template < class T >
bool CGenericFSM2State< T >::EvaluateTransitions(
    CStateManager& mgr, T& owner, float dt,
    const rstl::vector< SStateMachine2Transition >& transitions) {
  for (int i = 0; i < transitions.size(); ++i) {
    const SStateMachine2Transition& transition = transitions[i];
    const CState2* target = transition.mTarget;
    switch (target->GetType()) {
    case CState2::kType_State:
      ExitActive(mgr, owner);
      ExecuteState(*static_cast< const CState2State* >(target), mgr, owner, kStateMsg_Activate,
                   0.f);
      return true;
    case CState2::kType_Trigger: {
      const CState2Trigger* trigger = static_cast< const CState2Trigger* >(target);
      if (CallTrigger(*trigger, mgr, owner, dt) &&
          EvaluateTransitions(mgr, owner, dt, trigger->GetTransitions())) {
        return true;
      }
      break;
    }
    case CState2::kType_Code: {
      const CState2Code* code = static_cast< const CState2Code* >(target);
      CallCode(*code, mgr, owner, dt);
      if (EvaluateTransitions(mgr, owner, dt, code->GetTransitions())) {
        return true;
      }
      break;
    }
    case CState2::kType_SubMachine:
      ExitActive(mgr, owner);
      ExecuteSubflow(*static_cast< const CState2SubMachine* >(target), transition.mName, mgr, owner,
                     kStateMsg_Activate, 0.f);
      return true;
    }
  }
  return false;
}

template < class T >
void CGenericFSM2State< T >::ExitActive(CStateManager& mgr, T& owner) {
  const CState2State* state = GetActiveState();
  if (state != nullptr) {
    ExecuteState(*state, mgr, owner, kStateMsg_Deactivate, 0.f);
  }
  const CState2SubMachine* subMachine = GetActiveSubMachine();
  if (subMachine != nullptr) {
    ExecuteSubflow(*subMachine, rstl::string_l(""), mgr, owner, kStateMsg_Deactivate, 0.f);
  }
  mState = nullptr;
}

#endif // _CGENERICFSM2STATEIMPL
