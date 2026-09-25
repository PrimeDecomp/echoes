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
class CHudRadarInterface {
public:
  CHudRadarInterface(CGuiFrame& frame, const CStateManager& mgr, int playerIndex,
                     const CColor& color);
  ~CHudRadarInterface();

private:
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
