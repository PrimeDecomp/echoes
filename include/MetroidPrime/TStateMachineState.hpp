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
class TStateMachineState {
public:
  typedef void (T::*StateFunc)(CStateManager&, int, float);
  typedef bool (T::*TriggerFunc)(CStateManager&, const float&);
  struct SStateFunction {
    const char* mName;
    StateFunc mFunction;
  };
  struct STriggerFunction {
    const char* mName;
    TriggerFunc mFunction;
  };

  TStateMachineState();
  virtual ~TStateMachineState();
  virtual bool fn_801956b4() const;
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

private:
  rstl::vector< StateFunc > mStateFunctions;
  rstl::vector< TriggerFunc > mTriggerFunctions;
  const CStateMachine* mMachine;
  const CState* mState;
  float mTime;
  float mRandom;
  float mDelay;
  float x38_; // Uninitialized by the observed constructor; role unresolved.
  bool mCodeTrigger;
};

#endif // _TSTATEMACHINESTATE
