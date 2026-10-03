#include "MetroidPrime/Enemies/CStateMachine2.hpp"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

// Guessed name.
CFactoryFnReturn FAiStateMachine2Factory(const SObjectTag& tag, CInputStream& in,
                                         const CVParamTransfer& params) {}

CStateMachine2::CStateMachine2(CInputStream& in) {}

CStateMachine2::~CStateMachine2() {}

const CState2* CStateMachine2::ResolveNode(uint target) const {}

SStateMachine2SerializedTransition::SStateMachine2SerializedTransition(CInputStream& in) {}

CState2State::CState2State() {}

CState2State::CState2State(const CState2State& other)
: CState2(other), mTransitions(other.mTransitions) {}

CState2State::~CState2State() {}

CState2::EType CState2State::GetType() const {}

void CState2State::Setup(const rstl::string& name,
                         const rstl::vector< SStateMachine2Transition >& transitions) {}

CState2Code::CState2Code() {}

CState2Code::CState2Code(const CState2Code& other)
: CState2(other), mTransitions(other.mTransitions) {}

CState2Code::~CState2Code() {}

CState2::EType CState2Code::GetType() const {}

void CState2Code::Setup(const rstl::string& name,
                        const rstl::vector< SStateMachine2Transition >& transitions) {}

CState2Trigger::CState2Trigger() : mArgument(0.f), mNegate(false) {}

CState2Trigger::CState2Trigger(const CState2Trigger& other)
: CState2(other)
, mArgument(other.mArgument)
, mTransitions(other.mTransitions)
, mNegate(other.mNegate) {}

CState2Trigger::~CState2Trigger() {}

CState2::EType CState2Trigger::GetType() const {}

void CState2Trigger::Setup(const rstl::string& name, const float& argument, bool negate,
                           const rstl::vector< SStateMachine2Transition >& transitions) {}

CState2SubMachine::CState2SubMachine() {}

CState2SubMachine::CState2SubMachine(const CState2SubMachine& other)
: CState2(other), mTransitions(other.mTransitions), mMachine(other.mMachine) {}

CState2SubMachine::~CState2SubMachine() {}

CState2::EType CState2SubMachine::GetType() const {}

void CState2SubMachine::Setup(const rstl::string& name,
                              const rstl::vector< SStateMachine2Transition >& transitions,
                              CAssetId assetId) {}

const CStateMachine2* CState2SubMachine::GetMachine() const {}
