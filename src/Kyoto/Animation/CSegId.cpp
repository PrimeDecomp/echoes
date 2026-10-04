#include "Kyoto/Animation/CSegId.hpp"

#include "rstl/string.hpp"

// Guessed names.
rstl::string skSkeletonRootName = rstl::string_l("Skeleton_Root");
rstl::string skRootName = rstl::string_l("root");

CSegId::CSegId(CInputStream& in) : mId(in.ReadInt32()) {}
