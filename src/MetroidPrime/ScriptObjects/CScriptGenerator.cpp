#include "MetroidPrime/ScriptObjects/CScriptGenerator.hpp"

CScriptGenerator::CScriptGenerator(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                                   int spawnCount, bool noReuseFollowers, const CVector3f& offset,
                                   bool noInheritTransform, bool unknown, float minScale,
                                   float maxScale)
: CEntity(uid, info, name, 0)
, mSpawnCount(spawnCount)
, mNoReuseFollowers(noReuseFollowers)
, mNoInheritTransform(noInheritTransform)
, x28_26_(unknown)
, mOffset(offset)
, mMinScale(minScale)
, mMaxScale(maxScale) {}

CScriptGenerator::~CScriptGenerator() {}

void CScriptGenerator::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  // TODO: follower selection, generation, transform/scale and connection propagation.
  CEntity::AcceptScriptMsg(mgr, msg);
}
