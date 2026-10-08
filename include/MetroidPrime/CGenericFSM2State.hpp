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
  virtual ~CGenericFSM2State() {}
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

#endif // _CGENERICFSM2STATE
