#include "MetroidPrime/ScriptLoaderRel.hpp"

SIngPuddle_FuncPtrs* gLoader_IngPuddle; // Guessed global name.

void SetSIngPuddle_FuncPtrs(SIngPuddle_FuncPtrs* callbacks) { gLoader_IngPuddle = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadIngPuddle(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_IngPuddle->mLoader(mgr, input, info);
}
