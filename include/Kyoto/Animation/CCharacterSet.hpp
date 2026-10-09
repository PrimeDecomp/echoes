#ifndef _CCHARACTERSET
#define _CCHARACTERSET

#include "Kyoto/Animation/CCECharacterInfo.hpp"

#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

class CCharacterSet {
public:
  explicit CCharacterSet(CInputStream& in);

  const rstl::vector< rstl::pair< int, CCECharacterInfo > >& GetCharacterList() const {
    return mCharacters;
  }

private:
  ushort mTableCount;
  rstl::vector< rstl::pair< int, CCECharacterInfo > > mCharacters;
};
CHECK_SIZEOF(CCharacterSet, 0x14)

#endif // _CCHARACTERSET
