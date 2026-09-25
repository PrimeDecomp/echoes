#include "Kyoto/Animation/CSegId.hpp"

CSegId::CSegId(CInputStream& in) : x0_id(in.ReadInt32()) {}
