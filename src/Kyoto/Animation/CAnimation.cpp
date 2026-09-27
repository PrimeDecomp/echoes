#include "Kyoto/Animation/CAnimation.hpp"

#include "Kyoto/Animation/CMetaAnimFactory.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CAnimation::CAnimation(CInputStream& in) : mName(in), mAnim(CMetaAnimFactory::CreateMetaAnim(in)) {}
