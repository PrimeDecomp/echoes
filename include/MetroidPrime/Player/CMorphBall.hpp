#ifndef _CMORPHBALL
#define _CMORPHBALL

#include "Kyoto/Math/CTransform4f.hpp"

class CMorphBall {
public:
  CTransform4f Get_800cb764() const;
  void SwitchToTire();

  char m_pad[0xc78];
  int xc78;
  int xc7c;
  int xc80_maybe_sa_state;
  char m_pad2[0xAC];
  CTransform4f xd28;
};

#endif // _CMORPHBALL
