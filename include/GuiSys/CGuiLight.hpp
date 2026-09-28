#ifndef _CGUILIGHT
#define _CGUILIGHT

#include "GuiSys/CGuiWidget.hpp"
#include "Kyoto/Graphics/CLight.hpp"

class CGuiLight : public CGuiWidget {
public:
  CGuiLight(const CGuiWidgetParms& parms, const CLight& light);

  // CGuiObject
  ~CGuiLight() override;

  // CGuiWidget
  FourCC GetWidgetTypeID() const override;
  EWidgetUsageFlags GetWidgetUsageFlags() const override;

  virtual void SetIsVisible(bool visible);

  CLight BuildLight() const;
  int GetLightIndex() const { return mLightId; }
  const CColor& GetAmbientContribution() const { return mAmbientColor; }

private:
  ELightType mType;
  float mSpotCutoff;
  float mDistC;
  float mDistL;
  float mDistQ;
  float mAngleC;
  float mAngleL;
  float mAngleQ;
  int mLightId;
  CColor mAmbientColor;
};
CHECK_SIZEOF(CGuiLight, 0xe4)

#endif // _CGUILIGHT
