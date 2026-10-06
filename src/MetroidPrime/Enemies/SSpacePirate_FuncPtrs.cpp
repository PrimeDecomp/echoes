#include "MetroidPrime/Enemies/CSpacePirate.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"

SSpacePirate_FuncPtrs* gLoader_SpacePirate; // Guessed global name.

void SetSSpacePirate_FuncPtrs(SSpacePirate_FuncPtrs* callbacks) { gLoader_SpacePirate = callbacks; }

// Guessed loader name.
CEntity* LoadSpacePirate(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_SpacePirate->mLoadSpacePirate(mgr, input, info);
}

bool CSpacePirate::AttachActorToPirate(TUniqueId id) {
  return (this->*gLoader_SpacePirate->mAttachActor)(id);
}

void CSpacePirate::DetachActorFromPirate() { (this->*gLoader_SpacePirate->mDetachActor)(); }
