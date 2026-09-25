#ifndef _CCAMERAFILTERPASS
#define _CCAMERAFILTERPASS

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/auto_ptr.hpp"

class CTexture;
class CCameraFilterPass {
public:
  enum EFilterType {
    kFT_Passthru,
    kFT_Multiply,
    kFT_Invert,
    kFT_Add,
    kFT_Subtract,
    kFT_Blend,
    kFT_Widescreen,
    kFT_SceneAdd,
    kFT_NoColor,
    kFT_InvDstMultiply
  };
  enum EFilterShape {
    kFS_Fullscreen,
    kFS_FullscreenHalvesLeftRight,
    kFS_FullscreenHalvesTopBottom,
    kFS_FullscreenQuarters,
    kFS_CinemaBars,
    kFS_ScanLinesEven,
    kFS_ScanLinesOdd,
    kFS_RandomStatic,
    kFS_CookieCutterDepthRandomStatic
  };
  CCameraFilterPass();
  void SetFilter(EFilterType type, EFilterShape shape, float time, const CColor& color,
                 CAssetId texture);
  void DisableFilter(float time);
  void Update(float dt);
  void Draw() const;

private:
  EFilterType mCurrentType;
  EFilterType mNextType;
  EFilterShape mShape;
  float mDuration;
  float mRemainingTime;
  CColor mPreviousColor;
  CColor mCurrentColor;
  CColor mNextColor;
  CAssetId mNextTexture;
  rstl::auto_ptr< TLockedToken< CTexture > > mTexture;
};

CHECK_SIZEOF(CCameraFilterPass, 0x2c)

#endif // _CCAMERAFILTERPASS
