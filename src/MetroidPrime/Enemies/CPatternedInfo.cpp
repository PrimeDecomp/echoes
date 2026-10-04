#include "MetroidPrime/Enemies/CPatternedInfo.hpp"

CPatternedInfo::~CPatternedInfo() {}

CPatternedInfo::CPatternedInfo(const CHealthInfo& health,
                             const CDamageVulnerability& vulnerability,
                             CAssetId stateMachine, CAssetId stateMachine2)
: mHealthInfo(health)
, mDamageVulnerability(vulnerability)
, mAnimationParameters(kInvalidAssetId, -1, 0)
, mStateMachineId(stateMachine)
, mStateMachine2Id(stateMachine2)
, mEchoParameters(SEchoParameters::None()) {}
