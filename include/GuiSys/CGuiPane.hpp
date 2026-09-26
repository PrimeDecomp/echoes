#ifndef _CGUIPANE
#define _CGUIPANE

#include "GuiSys/CGuiWidget.hpp"
#include "Kyoto/Math/CVector2f.hpp"

class COutputStream;

class CGuiPane : public CGuiWidget {
public:
  CGuiPane(const CGuiWidgetParms& parms, float width, float height, const CVector3f& scaleCenter);

  // CGuiObject
  ~CGuiPane() override;

  // CGuiWidget
  FourCC GetWidgetTypeID() const override;
  int GetWidgetTypeIndex() const override;
  void Draw(const CGuiWidgetDrawParms& parms) const override;

  virtual void ScaleDimensions(const CVector3f& scale);
  virtual void SetDimensions(const CVector2f& dim, bool initVBO);
  virtual CVector2f GetDimensions() const;
  virtual void InitializeBuffers();
  virtual void WriteData(COutputStream& out, bool flag) const;

  const float* GetVtxBuf() const { return mPanePoints; }
  const CVector3f& GetScaleCenter() const { return mScaleCenter; }
  float GetWidth() const { return mWidth; }
  float GetHeight() const { return mHeight; }

private:
  float mWidth;
  float mHeight;
  float* mPanePoints;
  CVector3f mScaleCenter;
};
CHECK_SIZEOF(CGuiPane, 0xd4)

#endif // _CGUIPANE
