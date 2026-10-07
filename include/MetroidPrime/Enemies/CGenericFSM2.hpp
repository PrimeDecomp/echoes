#ifndef _CGENERICFSM2
#define _CGENERICFSM2

#include "Kyoto/CToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CInputStream;
class CGenericFSM2;

// Guessed name.
class CState2 {
public:
  // Guessed names.
  enum EType { kType_Trigger, kType_State, kType_Code, kType_SubMachine };
  virtual EType GetType() const = 0;

protected:
  CState2() {}
};
CHECK_SIZEOF(CState2, 0x4)

// Guessed name.
struct SStateMachine2Transition {
  rstl::string mName;     // Guessed name.
  const CState2* mTarget; // Guessed name.
};
CHECK_SIZEOF(SStateMachine2Transition, 0x14)

// Guessed name.
struct SStateMachine2SerializedTransition {
  explicit SStateMachine2SerializedTransition(CInputStream& in);
  rstl::string mName; // Guessed name.
  uint mTarget;       // Guessed name.
};
CHECK_SIZEOF(SStateMachine2SerializedTransition, 0x14)

// Guessed name.
class CState2State : public CState2 {
public:
  CState2State();
  CState2State(const CState2State& other);
  ~CState2State();

  // CState2
  virtual EType GetType() const;

  void Setup(const rstl::string& name,
             const rstl::vector< SStateMachine2Transition >& transitions); // Guessed name.

private:
  rstl::string mName;                                    // Guessed name.
  rstl::vector< SStateMachine2Transition > mTransitions; // Guessed name.
};
CHECK_SIZEOF(CState2State, 0x24)

// Guessed name.
class CState2Code : public CState2 {
public:
  CState2Code();
  CState2Code(const CState2Code& other);
  ~CState2Code();

  // CState2
  virtual EType GetType() const;

  void Setup(const rstl::string& name,
             const rstl::vector< SStateMachine2Transition >& transitions); // Guessed name.

private:
  rstl::string mName;                                    // Guessed name.
  rstl::vector< SStateMachine2Transition > mTransitions; // Guessed name.
};
CHECK_SIZEOF(CState2Code, 0x24)

// Guessed name.
class CState2Trigger : public CState2 {
public:
  CState2Trigger();
  CState2Trigger(const CState2Trigger& other);
  ~CState2Trigger();

  // CState2
  virtual EType GetType() const;

  void Setup(const rstl::string& name, const float& argument, bool negate,
             const rstl::vector< SStateMachine2Transition >& transitions); // Guessed name.

private:
  rstl::string mName;                                    // Guessed name.
  float mArgument;                                       // Guessed name.
  rstl::vector< SStateMachine2Transition > mTransitions; // Guessed name.
  bool mNegate : 1;                                      // Guessed name.
};
CHECK_SIZEOF(CState2Trigger, 0x2c)

// Guessed name.
class CState2SubMachine : public CState2 {
public:
  CState2SubMachine();
  CState2SubMachine(const CState2SubMachine& other);
  ~CState2SubMachine();

  // CState2
  virtual EType GetType() const;

  void Setup(const rstl::string& name, const rstl::vector< SStateMachine2Transition >& transitions,
             CAssetId assetId);           // Guessed name.
  const CGenericFSM2* GetMachine() const; // Guessed name.

private:
  rstl::string mName;                                    // Guessed name.
  rstl::vector< SStateMachine2Transition > mTransitions; // Guessed name.
  rstl::optional_object< CToken > mMachine;              // Guessed name.
};
CHECK_SIZEOF(CState2SubMachine, 0x30)

// MP3 Proto name.
class CGenericFSM2 {
public:
  explicit CGenericFSM2(CInputStream& in);
  ~CGenericFSM2();

private:
  const CState2* ResolveNode(uint target) const;  // Guessed name.
  rstl::vector< CState2State > mStates;           // Guessed name.
  rstl::vector< CState2Code > mCodes;             // Guessed name.
  rstl::vector< CState2Trigger > mTriggers;       // Guessed name.
  rstl::vector< CState2SubMachine > mSubMachines; // Guessed name.
};
CHECK_SIZEOF(CGenericFSM2, 0x40)

#endif
