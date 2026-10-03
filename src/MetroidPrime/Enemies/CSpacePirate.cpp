#include "MetroidPrime/Enemies/CSpacePirate.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"

SSpacePirate_FuncPtrs* gLoader_SpacePirate; // Guessed global name.

void CSpacePirate::DetachActorFromPirate() {}

bool CSpacePirate::AttachActorToPirate(TUniqueId id) {}

// Guessed loader name.
CEntity* LoadSpacePirate(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {}

void SetSSpacePirate_FuncPtrs(SSpacePirate_FuncPtrs* callbacks) {}

CEntity* CSpacePirate::TypesMatch(int typeId) const {}
