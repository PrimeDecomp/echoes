#include "Kyoto/Animation/CCharacterSet.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CCharacterSet::CCharacterSet(CInputStream& in) : mTableCount(in.Get< ushort >()) {
  int count = in.Get< int >();
  mCharacters.reserve(count);
  for (int i = 0; i < count; ++i) {
    mCharacters.push_back_unsafe(in);
  }
}

extern "C" void fn_80293EA4(int obj) {
  rstl::destroy_impl< rstl::pair< int, CCECharacterInfo > >((rstl::pair< int, CCECharacterInfo >*)obj);
}
