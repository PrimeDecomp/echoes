#ifndef _CGUIMODEL
#define _CGUIMODEL

#include "GuiSys/CGuiWidget.hpp"

class CModelFlags;

class CGuiModel : public CGuiWidget {
public:
  CGuiModel(const CGuiWidgetParms& parms, CAssetId modelId, uint lightMask, int modelIndex);

  // CGuiObject
  ~CGuiModel() override;

  // CGuiWidget
  FourCC GetWidgetTypeID() const override;
  int GetWidgetUsageFlags() const override;
  void Draw(const CGuiWidgetDrawParms& parms) const override;

  void DrawModel(const CModelFlags& flags) const; // Guessed name
  static void BeginDraw();                        // Guessed name
  static void EndDraw();                          // Guessed name

private:
  CAssetId mModelId;
  int mModelIndex;
  uint mLightMask;
};
CHECK_SIZEOF(CGuiModel, 0xc8)

#endif // _CGUIMODEL
