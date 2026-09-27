#include "MetroidPrime/Factories/CStateMachineFactory.hpp"

#include "MetroidPrime/Enemies/CStateMachine.hpp"

CFactoryFnReturn FAiFiniteStateMachineFactory(const SObjectTag& tag, CInputStream& in,
                                              const CVParamTransfer& params) {
  return rs_new CStateMachine(in);
}
