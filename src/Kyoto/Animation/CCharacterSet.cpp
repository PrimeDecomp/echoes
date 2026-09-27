#include "Kyoto/Animation/CCharacterSet.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CCharacterSet::CCharacterSet(CInputStream& in) : mTableCount(in.Get< ushort >()) {
  const int count = in.Get< int >();
  mCharacters.reserve(count);
  for (int i = 0; i < count; ++i) {
    mCharacters.push_back(in.Get< rstl::pair< int, CCharacterInfo > >());
  }
}
