#include "MetroidPrime/Enemies/CGenericFSM2.hpp"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

// Guessed name.
CFactoryFnReturn FAiStateMachine2Factory(const SObjectTag& tag, CInputStream& in,
                                         const CVParamTransfer& params) {}

CGenericFSM2::CGenericFSM2(CInputStream& in) {}

CGenericFSM2::~CGenericFSM2() {}

const CState2* CGenericFSM2::ResolveNode(uint target) const { return nullptr; }

SStateMachine2SerializedTransition::SStateMachine2SerializedTransition(CInputStream& in) {}

CState2State::CState2State() : mName(rstl::string_l("")) {}

CState2State::CState2State(const CState2State& other)
: CState2(other), mName(other.mName), mTransitions(other.mTransitions) {}

CState2State::~CState2State() {}

CState2::EType CState2State::GetType() const { return kType_Code; }

void CState2State::Setup(const rstl::string& name,
                         const rstl::vector< SStateMachine2Transition >& transitions) {}

CState2Code::CState2Code() : mName(rstl::string_l("")) {}

CState2Code::CState2Code(const CState2Code& other)
: CState2(other), mName(other.mName), mTransitions(other.mTransitions) {}

CState2Code::~CState2Code() {}

CState2::EType CState2Code::GetType() const {}

void CState2Code::Setup(const rstl::string& name,
                        const rstl::vector< SStateMachine2Transition >& transitions) {}

CState2Trigger::CState2Trigger() : mName(rstl::string_l("")), mArgument(0.f), mNegate(false) {}

CState2Trigger::CState2Trigger(const CState2Trigger& other)
: CState2(other)
, mName(other.mName)
, mArgument(other.mArgument)
, mTransitions(other.mTransitions)
, mNegate(other.mNegate) {}

CState2Trigger::~CState2Trigger() {}

CState2::EType CState2Trigger::GetType() const { return kType_Trigger; }

void CState2Trigger::Setup(const rstl::string& name, const float& argument, bool negate,
                           const rstl::vector< SStateMachine2Transition >& transitions) {}

CState2SubMachine::CState2SubMachine() : mName(rstl::string_l("")) {}

CState2SubMachine::CState2SubMachine(const CState2SubMachine& other)
: CState2(other), mName(other.mName), mTransitions(other.mTransitions), mMachine(other.mMachine) {}

CState2SubMachine::~CState2SubMachine() {}

CState2::EType CState2SubMachine::GetType() const { return kType_SubMachine; }

void CState2SubMachine::Setup(const rstl::string& name,
                              const rstl::vector< SStateMachine2Transition >& transitions,
                              CAssetId assetId) {}

const CGenericFSM2* CState2SubMachine::GetMachine() const { return nullptr; }
