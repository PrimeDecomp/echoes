#include "MetroidPrime/ScriptLoaderRel.hpp"

// Guessed global name; the record is borrowed from the loaded Tweaks REL.
STweaks_FuncPtrs* gLoader_Tweaks;

void SetSTweaks_FuncPtrs(STweaks_FuncPtrs* callbacks) { gLoader_Tweaks = callbacks; }

void RelProxy_LoadTweaks(CInputStream& input) { gLoader_Tweaks->mLoadTweaks(input); }

void RelProxy_CreateTweakGlobals() { gLoader_Tweaks->mCreateGlobals(); }

void RelProxy_FreeTweaks() { gLoader_Tweaks->mFreeTweaks(); }
