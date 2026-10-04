#include "MetroidPrime/ScriptLoaderRel.hpp"

// Guessed global name; the record is borrowed from the loaded Tweaks REL.
STweaks_FuncPtrs* gLoader_Tweaks;

void SetSTweaks_FuncPtrs(STweaks_FuncPtrs* callbacks) { gLoader_Tweaks = callbacks; }

void LoadTweaks(CInputStream& input) { gLoader_Tweaks->mLoadTweaks(input); }

void CreateTweakGlobals() { gLoader_Tweaks->mCreateGlobals(); }

void FreeTweaks() { gLoader_Tweaks->mFreeTweaks(); }
