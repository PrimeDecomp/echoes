#ifndef _CSCRIPTTEXTPANE
#define _CSCRIPTTEXTPANE

#include "MetroidPrime/CActor.hpp"

#include "Kyoto/Text/CGuiTextSupport.hpp"
#include "Kyoto/Text/CStringTable.hpp"

class CScriptTextPane : public CActor {
public:
  CScriptTextPane(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                  const CTransform4f& xf, const CVector3f& pivotOffset, float width, float height,
                  int extentX, int extentY, const CColor& fontColor, const CColor& outlineColor,
                  const CColor& geometryColor, CAssetId font, CAssetId defaultString,
                  const rstl::string& defaultStringName, int blendMode, float fadeInTime,
                  float fadeOutTime, const CGuiTextProperties& textProperties,
                  const rstl::string& guiLabel, bool depthCompare, bool depthUpdate,
                  bool depthBackwards);

  // CEntity
  ~CScriptTextPane() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;

  void SetRenderScale(float scale);

private:
  // Guessed member names.
  float mFadeOpacity;
  CVector3f mPivotOffset;
  float mWidth;
  float mHeight;
  CGuiTextSupport mTextSupport;
  int mBlendMode;
  rstl::optional_object< TLockedToken< CStringTable > > mDefaultString;
  int mStringIndex;
  rstl::string mGuiLabel;
  float mFadeInTime;
  float mFadeOutTime;
  float mRenderScale;
  float mTargetRenderScale;
  bool mDepthCompare : 1;
  bool mDepthUpdate : 1;
  bool mDepthBackwards : 1;
};
CHECK_SIZEOF(CScriptTextPane, 0xeb8)

#endif
