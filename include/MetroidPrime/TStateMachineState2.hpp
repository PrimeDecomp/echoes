#ifndef _TSTATEMACHINESTATE2
#define _TSTATEMACHINESTATE2

#include "MetroidPrime/TStateMachineState.hpp"
#include "rstl/rc_ptr.hpp"

class CStateMachine2;
class CState2;

// Guessed name. The FSM2 dispatcher owns nested dispatchers and three callback tables.
template < class T >
class TStateMachineState2 : public TStateMachineStateBase< T > {
public:
  typedef typename TStateMachineStateBase< T >::StateFunc StateFunc;
  typedef typename TStateMachineStateBase< T >::TriggerFunc TriggerFunc;
  typedef typename TStateMachineStateBase< T >::SStateFunction SStateFunction;
  typedef typename TStateMachineStateBase< T >::STriggerFunction STriggerFunction;

  TStateMachineState2();

  // TStateMachineStateBase
  virtual ~TStateMachineState2();
  virtual int GetType() const;
  virtual void Reset(CStateManager& mgr, T& owner);
  virtual void SetStateFunctions(const SStateFunction* functions, int count);
  virtual void SetTriggerFunctions(const STriggerFunction* functions, int count);
  virtual void fn_80194bf0();
  virtual void SetState(CStateManager& mgr, T& owner, const rstl::string& name);
  virtual void Update(CStateManager& mgr, T& owner, float dt);
  virtual bool HasState() const;
  virtual const char* GetName() const;
  virtual float GetTime() const;
  virtual float GetDelay() const;
  virtual void SetDelay(float delay);

  void Setup(const CStateMachine2& machine);

private:
  struct SCodeFunction; // Third callback signature remains unresolved.
  struct SSubMachine {
    const CState2* mState;
    rstl::rc_ptr< TStateMachineState2< T > > mMachine;
  };

  const CStateMachine2* mMachine;
  rstl::vector< StateFunc > mStateFunctions;
  rstl::vector< TriggerFunc > mTriggerFunctions;
  rstl::vector< SCodeFunction > mCodeFunctions;
  rstl::vector< SSubMachine > mSubMachines;
  const CState2* mState;
  float mTime;
  float mDelay;
};

#endif
