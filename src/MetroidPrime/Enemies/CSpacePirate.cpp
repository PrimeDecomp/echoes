#include "MetroidPrime/Enemies/CSpacePirate.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"

SSpacePirate_FuncPtrs* gLoader_SpacePirate; // Guessed global name.

CEntity* CSpacePirate::TypesMatch(int typeId) const {}

void SetSSpacePirate_FuncPtrs(SSpacePirate_FuncPtrs* callbacks) {}

// Guessed loader name.
CEntity* LoadSpacePirate(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {}

bool CSpacePirate::AttachActorToPirate(TUniqueId id) {}

void CSpacePirate::DetachActorFromPirate() {}
