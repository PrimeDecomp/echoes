#include "Kyoto/Animation/CBoolPOINode.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CBoolPOINode::CBoolPOINode(CInputStream& in) : CPOINode(in), mVal(in.ReadBool()) {}
