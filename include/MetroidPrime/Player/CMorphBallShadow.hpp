#ifndef _CMORPHBALLSHADOW
#define _CMORPHBALLSHADOW
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/list.hpp"
#include "rstl/vector.hpp"
class CActor;
class CPlayer;
class CStateManager;

class CMorphBallShadow {
public:
  CMorphBallShadow(int width, int height, const TToken< CTexture >& ballFade);
  ~CMorphBallShadow();

private:
  rstl::list< CActor* > mActors;
  rstl::list< TAreaId > mAreas;
  rstl::vector< uint > mWorldModelBits;
  CTexture mTexture;
  TToken< CTexture > mBallFade;
  int mWidth;
  int mHeight;
  CAABox mShadowVolume;
  bool mHasIds;
};
CHECK_SIZEOF(CMorphBallShadow, 0xd4)
#endif // _CMORPHBALLSHADOW
