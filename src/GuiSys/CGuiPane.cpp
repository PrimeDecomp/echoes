#include "GuiSys/CGuiPane.hpp"

#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

CGuiWidget* CGuiPane::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* pool, uint version) {
  const CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  float width = in.ReadFloat();
  float height = in.ReadFloat();
  float x = in.ReadFloat();
  float y = in.ReadFloat();
  float z = in.ReadFloat();

  CGuiPane* pane = rs_new CGuiPane(parms, width, height, CVector3f(x, y, z));
  pane->ParseBaseInfo(frame, in, parms, version);
  return pane;
}

void CGuiPane::WriteData(COutputStream& out, bool flag) const {
  out.WriteReal32(mWidth);
  out.WriteReal32(mHeight);
  out.WriteReal32(mScaleCenter.GetX());
  out.WriteReal32(mScaleCenter.GetY());
  out.WriteReal32(mScaleCenter.GetZ());
}

CGuiPane::CGuiPane(const CGuiWidgetParms& parms, float width, float height,
                   const CVector3f& scaleCenter)
: CGuiWidget(parms)
, mWidth(width)
, mHeight(height)
, mPanePoints(rs_new float[12])
, mScaleCenter(scaleCenter) {
  InitializeBuffers();
}

CGuiPane::~CGuiPane() {
  if (mPanePoints != nullptr) {
    delete[] mPanePoints;
  }
}

void CGuiPane::Draw(const CGuiWidgetDrawParms& parms) const {
  CGraphics::SetModelMatrix(GetWorldTransform() * CTransform4f::Translate(mScaleCenter));
  if (GetIsVisible()) {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    const CColor color = GetModifiedColor().WithAlphaModulatedBy(parms.GetAlpha());
    CGraphics::DrawPrimitive(kP_TriangleStrip, mPanePoints, CVector3f(0.f, -1.f, 0.f), color, 4);
  }
}

void CGuiPane::ScaleDimensions(const CVector3f& scale) {
  InitializeBuffers();
  const CVector3f& center = GetScaleCenter();
  for (int i = 0; i < 12; ++i) {
    int axis = i % 3;
    mPanePoints[i] = (mPanePoints[i] - center[axis]) * scale[axis];
    mPanePoints[i] += center[axis];
  }
}

void CGuiPane::InitializeBuffers() {
  mPanePoints[0] = -mWidth / 2.f;
  mPanePoints[1] = 0.f;
  mPanePoints[2] = mHeight / 2.f;

  mPanePoints[3] = -mWidth / 2.f;
  mPanePoints[4] = 0.f;
  mPanePoints[5] = -mHeight / 2.f;

  mPanePoints[6] = mWidth / 2.f;
  mPanePoints[7] = 0.f;
  mPanePoints[8] = mHeight / 2.f;

  mPanePoints[9] = mWidth / 2.f;
  mPanePoints[10] = 0.f;
  mPanePoints[11] = -mHeight / 2.f;
}

void CGuiPane::SetDimensions(const CVector2f& dim, bool initVBO) {
  mWidth = dim.GetX();
  mHeight = dim.GetY();
  if (initVBO) {
    InitializeBuffers();
  }
}

CVector2f CGuiPane::GetDimensions() const { return CVector2f(mWidth, mHeight); }

CGuiWidget::EWidgetUsageFlags CGuiPane::GetWidgetUsageFlags() const { return kWUF_Draw; }

FourCC CGuiPane::GetWidgetTypeID() const { return 'PANE'; }
