#ifndef _CGENERICFSM2STATE
#define _CGENERICFSM2STATE

#include "MetroidPrime/Enemies/CGenericFSM2.hpp"
#include "MetroidPrime/TStateMachineState.hpp"
#include "rstl/rc_ptr.hpp"

// MP3 Proto name; guessed template form.
template < class T >
class CGenericFSM2State : public TStateMachineStateBase< T > {
public:
  typedef typename TStateMachineStateBase< T >::StateFunc StateFunc;
  typedef typename TStateMachineStateBase< T >::TriggerFunc TriggerFunc;
  typedef typename TStateMachineStateBase< T >::CodeFunc CodeFunc;
  typedef typename TStateMachineStateBase< T >::SStateFunction SStateFunction;
  typedef typename TStateMachineStateBase< T >::STriggerFunction STriggerFunction;
  typedef typename TStateMachineStateBase< T >::SCodeFunction SCodeFunction;

  CGenericFSM2State();

  // TStateMachineStateBase
  virtual ~CGenericFSM2State();
  virtual int GetType() const;
  virtual void Reset(CStateManager& mgr, T& owner);
  virtual void SetStateFunctions(const SStateFunction* functions, int count);
  virtual void SetTriggerFunctions(const STriggerFunction* functions, int count);
  virtual void SetCodeFunctions(const SCodeFunction* functions, int count);
  virtual void SetState(CStateManager& mgr, T& owner, const rstl::string& name);
  virtual void Update(CStateManager& mgr, T& owner, float dt);
  virtual bool HasState() const;
  virtual const char* GetName() const;
  virtual float GetTime() const;
  virtual float GetDelay() const;
  virtual void SetDelay(float delay);

  void Setup(const CGenericFSM2& machine);
  // Guessed names.
  int GetStateIndex(const rstl::string& name) const;
  int GetTriggerIndex(const rstl::string& name) const;
  int GetCodeIndex(const rstl::string& name) const;
  int GetSubMachineIndex(const CState2* state) const;
  void SetStateFunction(const rstl::string& name, StateFunc function);
  void SetTriggerFunction(const rstl::string& name, TriggerFunc function);
  void SetCodeFunction(const rstl::string& name, CodeFunc function);

private:
  struct SSubMachine { // Guessed name.
    SSubMachine(const CState2* state, const rstl::rc_ptr< CGenericFSM2State< T > >& machine)
    : mState(state), mMachine(machine) {}
    const CState2* mState;
    rstl::rc_ptr< CGenericFSM2State< T > > mMachine;
  };

  // Guessed names.
  void CallCode(const CState2Code& state, CStateManager& mgr, T& owner, float dt);
  bool CallTrigger(const CState2Trigger& state, CStateManager& mgr, T& owner, float dt);
  // MP3 Proto names.
  void ExecuteSubflow(const CState2SubMachine& state, const rstl::string& name, CStateManager& mgr,
                      T& owner, EStateMsg msg, float dt);
  void ExecuteState(const CState2State& state, CStateManager& mgr, T& owner, EStateMsg msg,
                    float dt);
  // Guessed names.
  const CState2SubMachine* GetActiveSubMachine() const;
  const CState2State* GetActiveState() const;
  bool EvaluateTransitions(CStateManager& mgr, T& owner, float dt,
                           const rstl::vector< SStateMachine2Transition >& transitions);
  void ExitActive(CStateManager& mgr, T& owner);

  // Guessed member names.
  const CGenericFSM2* mMachine;
  rstl::vector< StateFunc > mStateFunctions;
  rstl::vector< CodeFunc > mCodeFunctions;
  rstl::vector< TriggerFunc > mTriggerFunctions;
  rstl::vector< SSubMachine > mSubMachines;
  const CState2* mState;
  float mTime;
  float mDelay;
};

template < class T >
CGenericFSM2State< T >::CGenericFSM2State()
: mMachine(nullptr), mState(nullptr), mTime(0.f), mDelay(0.f) {}

template < class T >
CGenericFSM2State< T >::~CGenericFSM2State() {}

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

#endif
