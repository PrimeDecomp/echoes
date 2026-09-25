#ifndef _TSTATEMACHINESTATE
#define _TSTATEMACHINESTATE

#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CState;
class CStateMachine;
class CStateManager;

// Guessed name. Echoes dispatches owner-specific member functions, unlike Prime's AI state.
// The resource reader and dispatcher are not reconstructed here.
template < class T >
struct TStateMachineFunctionTypes {
  typedef void (T::*StateFunc)(CStateManager&, int, float);
  typedef bool (T::*TriggerFunc)(CStateManager&, const float&);
};

// Guessed name. AFSM and FSM2 dispatchers share this virtual interface.
template < class T >
class TStateMachineStateBase {
public:
  typedef typename TStateMachineFunctionTypes< T >::StateFunc StateFunc;
  typedef typename TStateMachineFunctionTypes< T >::TriggerFunc TriggerFunc;
  struct SStateFunction {
    const char* mName;
    StateFunc mFunction;
  };
  struct STriggerFunction {
    const char* mName;
    TriggerFunc mFunction;
  };

  virtual ~TStateMachineStateBase();
  virtual int GetType() const = 0;
  virtual void Reset(CStateManager& mgr, T& owner) = 0;
  virtual void SetStateFunctions(const SStateFunction* functions, int count) = 0;
  virtual void SetTriggerFunctions(const STriggerFunction* functions, int count) = 0;
  virtual void fn_80194bf0() = 0;
  virtual void SetState(CStateManager& mgr, T& owner, const rstl::string& name) = 0;
  virtual void Update(CStateManager& mgr, T& owner, float dt) = 0;
  virtual bool HasState() const = 0;
  virtual const char* GetName() const = 0;
  virtual float GetTime() const = 0;
  virtual float GetDelay() const = 0;
  virtual void SetDelay(float delay) = 0;
};

template < class T >
class TStateMachineState : public TStateMachineStateBase< T > {
public:
  typedef typename TStateMachineStateBase< T >::StateFunc StateFunc;
  typedef typename TStateMachineStateBase< T >::TriggerFunc TriggerFunc;
  typedef typename TStateMachineStateBase< T >::SStateFunction SStateFunction;
  typedef typename TStateMachineStateBase< T >::STriggerFunction STriggerFunction;

  TStateMachineState();

  // TStateMachineStateBase
  virtual ~TStateMachineState();
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

  void Setup(const CStateMachine& machine);
  float GetRandom() const { return mRandom; }
  float GetFixedRandom() const { return mFixedRandom; }
  bool GetCodeTrigger() const { return mCodeTrigger; }

private:
  rstl::vector< StateFunc > mStateFunctions;
  rstl::vector< TriggerFunc > mTriggerFunctions;
  const CStateMachine* mMachine;
  const CState* mState;
  float mTime;
  float mRandom;
  float mDelay;
  float mFixedRandom;
  bool mCodeTrigger : 1;
};

#endif // _TSTATEMACHINESTATE
