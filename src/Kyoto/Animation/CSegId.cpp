#include "Kyoto/Animation/CSegId.hpp"

CSegId::CSegId(CInputStream& in) : mId(in.ReadInt32()) {}
