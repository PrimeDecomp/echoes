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
    kFS_DialogBox,
    kFS_CinematicPlaceholderLabel,
    kFS_CookieCutterDepthRandomStatic
  };
  CCameraFilterPass();
  void SetFilter(EFilterType type, EFilterShape shape, float time, const CColor& color,
                 CAssetId texture);
  void DisableFilter(float time);
  void Update(float dt);
  void Draw() const;
  float GetT(bool invert) const;
  // Guessed names; visible screen proportions during cinema-bar transitions.
  float GetWidthScale() const;
  float GetHeightScale() const;

  static void DrawWideScreen(const CColor& color, const CTexture* texture, float lod);
  static void DrawFilterShape(EFilterShape shape, const CColor& color, const CTexture* texture,
                              float lod);
  static void DrawFullScreenColoredQuad(const CColor& color);
  static void DrawFullScreenTexturedQuad(const CColor& color, const CTexture* texture, float lod);
  static void DrawFullScreenTexturedQuadQuarters(const CColor& color, const CTexture* texture,
                                                 float lod);
  static void DrawScanLines(const CColor& color, bool even);
  static void DrawRandomStatic(const CColor& color, float alpha, bool cookieCutterDepth);
  // Guessed method names; shape names are present in the original debug strings.
  static void DrawDialogBox(const CColor& color, const CTexture* texture, float alpha);
  static void DrawCinematicPlaceholderLabel();
  static void DrawFilter(EFilterType type, EFilterShape shape, const CColor& color,
                         const CTexture* texture, float lod);

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
