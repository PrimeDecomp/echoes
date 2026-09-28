#ifndef _CGUIFRAME
#define _CGUIFRAME
#include "GuiSys/CGuiWidgetIdDB.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
class CGuiWidget;
class CGuiHeadWidget;
class CGuiCamera;
class CGuiLight;
class CGuiWidgetDrawParms;
class CGuiFrameModelDatabase; // Guessed name
class CSimplePool;
class CInputStream;
class CFinalInput;
class CGuiFrame {
public:
  CGuiFrame(CInputStream& in, CSimplePool* pool);
  ~CGuiFrame();
  CGuiWidget* FindWidget(const char* name) const;
  CGuiWidget* FindWidget(short id) const;
  CGuiWidget* FindWidget(const rstl::string& name) const;
  bool GetIsFinishedLoading() const;
  void Update(float dt);
  void ProcessUserInput(const CFinalInput& input);
  void Draw(const CGuiWidgetDrawParms& parms) const;
  CGuiCamera* GetFrameCamera() const { return mCamera; }
  CGuiWidgetIdDB& WidgetIdDB() { return mWidgetIds; }
  void SetFrameCamera(CGuiCamera* camera);
  void SetHeadWidget(CGuiHeadWidget* widget);
  void Initialize();
  void SortDrawOrder();
  void AddLight(CGuiLight* light);
  void RemoveLight(CGuiLight* light);
  void EnableLights(uint mask) const;
  void DisableLights() const;
  void ApplyLights() const; // Guessed name

private:
  uint mVersion;
  rstl::vector< CToken > mAssets;
  CGuiHeadWidget* mRootWidget;
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
  rstl::auto_ptr< CGuiFrameModelDatabase > mModelDatabase;
  mutable bool mLoaded : 1;
};
CHECK_SIZEOF(CGuiFrame, 0x334)
#endif // _CGUIFRAME
