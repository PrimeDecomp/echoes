#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

const TEditorId kInvalidEditorId = TEditorId(-1);
const TUniqueId kInvalidUniqueId = TUniqueId(-1, -1);
const TAreaId kInvalidAreaId = TAreaId(-1);
const TEditorId kUnkId = TEditorId(-1);

// Guessed names; GC callers load these shared constants from TGameTypes' .sdata2.
const uint kInvalidPlayerIndex = uint(-1);
const uint kUnkPlayerIndexZero = 0;
const float kDefaultGravityAccel = 9.81f * 2.5f;

TEditorId::TEditorId(CInputStream& in) : value(in.Get< uint >()) {}
