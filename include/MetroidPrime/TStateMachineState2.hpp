#ifndef _TSTATEMACHINESTATE2
#define _TSTATEMACHINESTATE2

#include "MetroidPrime/Enemies/CStateMachine2.hpp"
#include "MetroidPrime/TStateMachineState.hpp"
#include "rstl/rc_ptr.hpp"

// Guessed name.
template < class T >
class TStateMachineState2 : public TStateMachineStateBase< T > {
public:
  typedef typename TStateMachineStateBase< T >::StateFunc StateFunc;
  typedef typename TStateMachineStateBase< T >::TriggerFunc TriggerFunc;
  typedef typename TStateMachineStateBase< T >::CodeFunc CodeFunc;
  typedef typename TStateMachineStateBase< T >::SStateFunction SStateFunction;
  typedef typename TStateMachineStateBase< T >::STriggerFunction STriggerFunction;
  typedef typename TStateMachineStateBase< T >::SCodeFunction SCodeFunction;

  TStateMachineState2();

  // TStateMachineStateBase
  virtual ~TStateMachineState2();
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

  void Setup(const CStateMachine2& machine);
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
    const CState2* mState;
    rstl::rc_ptr< TStateMachineState2< T > > mMachine;
  };

  // Guessed names.
  void CallCode(const CState2Code& state, CStateManager& mgr, T& owner, float dt);
  bool CallTrigger(const CState2Trigger& state, CStateManager& mgr, T& owner);
  void CallSubMachine(const CState2SubMachine& state, const rstl::string& name, CStateManager& mgr,
                      T& owner, EStateMsg msg, float dt);
  void CallState(const CState2State& state, CStateManager& mgr, T& owner, EStateMsg msg, float dt);
  const CState2SubMachine* GetActiveSubMachine() const;
  const CState2State* GetActiveState() const;
  bool EvaluateTransitions(CStateManager& mgr, T& owner, float dt,
                           const rstl::vector< SStateMachine2Transition >& transitions);
  void ExitActive(CStateManager& mgr, T& owner);

  const CStateMachine2* mMachine;
  rstl::vector< StateFunc > mStateFunctions;
  rstl::vector< CodeFunc > mCodeFunctions;
  rstl::vector< TriggerFunc > mTriggerFunctions;
  rstl::vector< SSubMachine > mSubMachines;
  const CState2* mState;
  float mTime;
  float mDelay;
};

template < class T >
TStateMachineState2< T >::TStateMachineState2()
: mMachine(nullptr), mState(nullptr), mTime(0.f), mDelay(0.f) {}

template < class T >
TStateMachineState2< T >::~TStateMachineState2() {}

template < class T >
int TStateMachineState2< T >::GetType() const {}

template < class T >
void TStateMachineState2< T >::Reset(CStateManager& mgr, T& owner) {}

template < class T >
void TStateMachineState2< T >::SetStateFunctions(const SStateFunction* functions, int count) {}

template < class T >
void TStateMachineState2< T >::SetTriggerFunctions(const STriggerFunction* functions, int count) {}

template < class T >
void TStateMachineState2< T >::SetCodeFunctions(const SCodeFunction* functions, int count) {}

template < class T >
void TStateMachineState2< T >::SetState(CStateManager& mgr, T& owner, const rstl::string& name) {}

template < class T >
void TStateMachineState2< T >::Update(CStateManager& mgr, T& owner, float dt) {}

template < class T >
bool TStateMachineState2< T >::HasState() const {}

template < class T >
const char* TStateMachineState2< T >::GetName() const {}

template < class T >
float TStateMachineState2< T >::GetTime() const {}

template < class T >
float TStateMachineState2< T >::GetDelay() const {}

template < class T >
void TStateMachineState2< T >::SetDelay(float delay) {}

template < class T >
void TStateMachineState2< T >::Setup(const CStateMachine2& machine) {}

template < class T >
int TStateMachineState2< T >::GetStateIndex(const rstl::string& name) const {}

template < class T >
int TStateMachineState2< T >::GetTriggerIndex(const rstl::string& name) const {}

template < class T >
int TStateMachineState2< T >::GetCodeIndex(const rstl::string& name) const {}

template < class T >
int TStateMachineState2< T >::GetSubMachineIndex(const CState2* state) const {}

template < class T >
void TStateMachineState2< T >::SetStateFunction(const rstl::string& name, StateFunc function) {}

template < class T >
void TStateMachineState2< T >::SetTriggerFunction(const rstl::string& name, TriggerFunc function) {}

template < class T >
void TStateMachineState2< T >::SetCodeFunction(const rstl::string& name, CodeFunc function) {}

template < class T >
void TStateMachineState2< T >::CallCode(const CState2Code& state, CStateManager& mgr, T& owner,
                                        float dt) {}

template < class T >
bool TStateMachineState2< T >::CallTrigger(const CState2Trigger& state, CStateManager& mgr,
                                           T& owner) {}

template < class T >
void TStateMachineState2< T >::CallSubMachine(const CState2SubMachine& state,
                                              const rstl::string& name, CStateManager& mgr,
                                              T& owner, EStateMsg msg, float dt) {}

template < class T >
void TStateMachineState2< T >::CallState(const CState2State& state, CStateManager& mgr, T& owner,
                                         EStateMsg msg, float dt) {}

template < class T >
const CState2SubMachine* TStateMachineState2< T >::GetActiveSubMachine() const {}

template < class T >
const CState2State* TStateMachineState2< T >::GetActiveState() const {}

template < class T >
bool TStateMachineState2< T >::EvaluateTransitions(
    CStateManager& mgr, T& owner, float dt,
    const rstl::vector< SStateMachine2Transition >& transitions) {}

template < class T >
void TStateMachineState2< T >::ExitActive(CStateManager& mgr, T& owner) {}

#endif
