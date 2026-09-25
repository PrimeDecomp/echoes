#ifndef _CGUIFRAME
#define _CGUIFRAME
#include "GuiSys/CGuiWidgetIdDB.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
class CGuiWidget;
class CGuiCamera;
class CGuiLight;
class CGuiWidgetDrawParms;
class CGuiFrameAnimationDatabase; // Guessed name
class CSimplePool;
class CInputStream;
class CGuiFrame {
public:
  CGuiFrame(CInputStream& in, CSimplePool* pool);
  ~CGuiFrame();
  CGuiWidget* FindWidget(const char* name) const;
  bool GetIsFinishedLoading() const;
  void Update(float dt);
  void Draw(const CGuiWidgetDrawParms& parms) const;
  CGuiCamera* GetFrameCamera() const { return mCamera; }

private:
  uint mVersion;
  rstl::vector< CToken > mAssets;
  CGuiWidget* mRootWidget;
  CGuiCamera* mCamera;
  CGuiWidgetIdDB mWidgetIds;
  rstl::vector< CGuiWidget* > mWidgets;
  rstl::vector< CGuiWidget* > mDrawWidgets;
  rstl::vector< CGuiWidget* > mPreDrawWidgets;
  rstl::vector< CGuiWidget* > mUpdateWidgets;
  rstl::vector< CGuiWidget* > mInputWidgets;
  rstl::vector< CGuiLight* > mLights;
  rstl::reserved_vector< CLight, 8 > mActiveLights;
  CColor mAmbientColor;
  rstl::auto_ptr< CGuiFrameAnimationDatabase > mAnimations;
  mutable bool mLoaded : 1;
};
CHECK_SIZEOF(CGuiFrame, 0x334)
#endif // _CGUIFRAME
