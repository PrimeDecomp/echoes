#include "MetroidPrime/Enemies/CSandworm.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"

SSandworm_FuncPtrs* gLoader_Sandworm; // Guessed name.

void SetSSandworm_FuncPtrs(SSandworm_FuncPtrs* callbacks) { gLoader_Sandworm = callbacks; }

CEntity* LoadSandworm(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Sandworm->mLoadSandworm(mgr, input, info);
}

CVector3f GetSandwormRadarPointPosition(const CSandworm* sandworm, int index) {
  return (sandworm->*gLoader_Sandworm->mGetRadarPointPosition)(index);
}

int GetSandwormRadarPointCount(const CSandworm* sandworm) {
  return (sandworm->*gLoader_Sandworm->mGetRadarPointCount)();
}
