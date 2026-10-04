#include "MetroidPrime/ScriptLoaderRel.hpp"

SGeomBlobV2_FuncPtrs* gFactory_GeomBlobV2; // Guessed global name.

void SetSGeomBlobV2_FuncPtrs(SGeomBlobV2_FuncPtrs* callbacks) { gFactory_GeomBlobV2 = callbacks; }
