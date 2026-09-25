#include "Kyoto/Animation/CPOINode.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/CCrc32.hpp"
#include "rstl/string.hpp"

static uint ReadNameHash(ushort version, CInputStream& in) {
  if (version >= 3) {
    // Skip the stored name; the hash follows it.
    while (in.ReadBool()) {
    }
    return in.ReadInt32();
  }
  rstl::string name(in);
  return CPOINode::GetHashForString(name.data());
}

CPOINode::CPOINode(uint nameHash, ushort type, const CCharAnimTime& time, int index, bool unique,
                   float weight, int charIdx, int flags)
: mVersion(2)
, mNameHash(nameHash)
, mType(type)
, mTime(time)
, mIndex(index)
, mUnique(unique)
, mWeight(weight)
, mCharIdx(charIdx)
, mFlags(flags) {}

CPOINode::CPOINode(CInputStream& in)
: mVersion(in.Get< ushort >())
, mNameHash(ReadNameHash(mVersion, in))
, mType(in.Get< ushort >())
, mTime(in)
, mIndex(in.Get< int >())
, mUnique(in.Get< bool >())
, mWeight(in.Get< float >())
, mCharIdx(in.Get< int >())
, mFlags(in.Get< int >()) {}

uint CPOINode::GetHashForString(const char* str) { return CCRC32::CalculateString(str, -1); }

int CPOINode::compare(const void* a, const void* b) {
  const CPOINode& aNode = *static_cast< const CPOINode* >(a);
  const CPOINode& bNode = *static_cast< const CPOINode* >(b);
  return aNode > bNode ? 1 : aNode < bNode ? -1 : 0;
}
