#include "MetroidPrime/Enemies/CSandworm.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"

SSandworm_FuncPtrs* gLoader_Sandworm; // Guessed name.

void SetSSandworm_FuncPtrs(SSandworm_FuncPtrs* callbacks) {}

CEntity* LoadSandworm(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {}

CVector3f GetSandwormRadarPointPosition(const CSandworm* sandworm, int index) {}

int GetSandwormRadarPointCount(const CSandworm* sandworm) {}
