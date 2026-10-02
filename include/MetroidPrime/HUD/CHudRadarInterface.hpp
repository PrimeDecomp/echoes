#ifndef _CHUDRADARINTERFACE
#define _CHUDRADARINTERFACE
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/TToken.hpp"
class CGuiFrame;
class CGuiWidget;
class CGuiCamera;
class CTexture;
class CStateManager;
class CPlayer;
class CHudRadarInterface {
public:
  CHudRadarInterface(CGuiFrame& frame, const CStateManager& mgr, int playerIndex,
                     const CColor& color);
  ~CHudRadarInterface() {}
  void Update(float dt, const CStateManager& mgr);
  void SetColor(const CColor& color);
  void Draw(const CStateManager& mgr, float alpha) const;

private:
  struct SRadarPaintDrawParms {
    SRadarPaintDrawParms(const CVector3f& playerPos, const CTransform4f& preTranslate,
                         const CTransform4f& postTranslate, float scopeRadius, float scopeScalar,
                         float alpha, float xyRadius, float zRadius, float zCloseRadius)
    : mPlayerPos(playerPos)
    , mPreTranslate(preTranslate)
    , mPostTranslate(postTranslate)
    , mScopeRadius(scopeRadius)
    , mScopeScalar(scopeScalar)
    , mAlpha(alpha)
    , mXyRadius(xyRadius)
    , mZRadius(zRadius)
    , mZCloseRadius(zCloseRadius) {}

    CVector3f mPlayerPos;
    CTransform4f mPreTranslate;
    CTransform4f mPostTranslate;
    float mScopeRadius;
    float mScopeScalar;
    float mAlpha;
    float mXyRadius;
    float mZRadius;
    float mZCloseRadius;
  };

  void DrawRadarPaint(const CVector3f& position, float radius, float alpha,
                      const SRadarPaintDrawParms& parms, const CColor& color) const;
  void DoDrawRadarPaint(float radius) const;
  // Guessed name
  void DrawEchoPulse(const CPlayer& player, const SRadarPaintDrawParms& parms) const;

  TCachedToken< CTexture > mRadarPaint;
  TCachedToken< CTexture > mBigRing;
  CTransform4f mRadarTransform;
  bool mVisibleGame : 1;
  bool mVisibleDebug : 1;
  CGuiWidget* mRadarModel;
  CGuiWidget* mRadarRoot;
  CGuiCamera* mCamera;
  int mPlayerIndex;
};
CHECK_SIZEOF(CHudRadarInterface, 0x5c)
#endif // _CHUDRADARINTERFACE
