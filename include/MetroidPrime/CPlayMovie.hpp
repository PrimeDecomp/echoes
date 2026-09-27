#ifndef _CPLAYMOVIE
#define _CPLAYMOVIE

#include "types.h"

#include "MetroidPrime/CIOWin.hpp"

class CPlayMovie : public CIOWin {
public:
  explicit CPlayMovie(int which);

  ~CPlayMovie() override;
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;

private:
  char x14_[0xc0];
};
CHECK_SIZEOF(CPlayMovie, 0xd4)

#endif // _CPLAYMOVIE
